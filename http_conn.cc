#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>

#include "http_encode.h"
#include "http_conn.h"
#include "misc_util.h"

namespace imcore {

static PT_THREAD(pt_http_request(struct pt *pt));

/** libuv callback begin **/
//static void timer_cb(uv_timer_t *handle)
//{
	//TODO
	//auto conn = (HttpConn *)handle->data;
	//conn->CheckTimeoutReq();
//}

static void close_cb(uv_handle_t* handle)
{
	auto conn = (HttpConn *)handle->data;
	delete conn;
}

static void alloc_cb(uv_handle_t *handle, size_t size, uv_buf_t *buf) 
{
	*buf = uv_buf_init((char *)malloc(size), size);
}

static void read_cb(uv_stream_t *handle, ssize_t size, const uv_buf_t *buf)
{
	auto conn = (HttpConn *)handle->data;

	if(size < 0) free(buf->base);
	conn->set_status((int)size);
	if(conn->HandleError()) return;

	if(buf->base && size > 0) conn->RecvData(buf->base, size);
	free(buf->base);
}

static void write_cb(uv_write_t *req, int status)
{
	auto conn = (HttpConn *)req->handle->data;
	free(req->data);
	free(req);
	conn->set_status(status);
	if(conn->HandleError()) return;
}

static void conn_cb(uv_connect_t *req, int status)
{
	auto conn = (HttpConn *)req->data;
	conn->set_status(status);
	++(conn->copt()->step);
	PT_SCHEDULE(pt_http_request(conn->pt()));
}

static void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res) 
{
	auto conn = (HttpConn *)resolver->data;
	conn->set_status(status);
	conn->set_res(res);

	++(conn->copt()->step);
	PT_SCHEDULE(pt_http_request(conn->pt()));
}

/** libuv callback end **/

/** http parser callback begin **/

static int on_body(http_parser *p, const char *idx, size_t len)
{
	auto conn = (HttpConn *)p->data;
	if(!conn) return 0;

	conn->set_body_len(conn->body_len() + len);

	//CO_LOGD("body len: %ld", len);

	return 0;
}

static int on_message_begin(http_parser *p)
{
	//CO_LOGD("recv begin");
	auto conn = (HttpConn *)p->data;
	if(!conn) return 0;
	conn->set_body_len(0);
	return 0;
}

static int on_message_complete(http_parser *p)
{
	//CO_LOGD("recv complete");
	auto conn = (HttpConn *)p->data;
	++(conn->copt()->step);
	PT_SCHEDULE(pt_http_request(conn->pt()));

	return 0;
}
/** http parser callback end **/

/** pt begin **/
static PT_THREAD(pt_http_request(struct pt *pt))
{
	auto copt = (struct co_send_recv_t *)pt;
	auto conn = (HttpConn *)copt->data;

	PT_BEGIN(pt);

	//step 1 resolve host name
	if(conn->Resolve()) goto end;
	PT_WAIT_UNTIL(pt, 
			copt->step == 1);

	//step 2 connect to svr
	if(conn->Connect()) goto end;
	PT_WAIT_UNTIL(pt, 
			copt->step == 2);

	//step 3 send request
	if(conn->SendReq()) goto end;
	PT_WAIT_UNTIL(pt, 
			copt->step == 3);

	//step 4 parse response
	if(conn->ParseRsp()) goto end;
	PT_WAIT_UNTIL(pt, 
			copt->step == 4);

end:
	PT_END(pt);
}

/** pt end **/

HttpConn::HttpConn()
{
	//uv callback stuff
	uv_tcp_init(uv_default_loop(), &socket_);
	resolver_.data = this;
	connect_.data = this;
	socket_.data = this;

	http_parser_init(&httpparser_, HTTP_RESPONSE);
	httpparser_.data = this;

	settings_.on_body = on_body;
	settings_.on_message_begin = on_message_begin;
	settings_.on_message_complete = on_message_complete;
	
	settings_.on_status = nullptr;
	settings_.on_header_field = nullptr;
	settings_.on_header_value = nullptr;
	settings_.on_headers_complete = nullptr;
	settings_.on_url = nullptr;

	body_len_ = 0;

	co_pt_init(&copt_, this);
}

HttpConn::~HttpConn()
{
	//CO_LOGD("dtor: httpconn");
}

void HttpConn::Request(const std::string &url, const std::string &method,
		const std::string &req, std::function<void(int, const std::string &rsp)> cb)
{
	if(!cb) return;
	cb_ = cb;
	method_ = method;
	req_ = req;
	url_ = url;

	PT_SCHEDULE(pt_http_request(pt()));
}

int HttpConn::HandleError()
{
	if(status_ < 0) {
		auto err = uv_err_name(status_); 
		CO_LOGE("error: %d|desc: %s", status_, err);
		cb_(status_, err);
		uv_close((uv_handle_t *)&socket_, close_cb);
		return status_;
	}

	return 0;
}

int HttpConn::ParseUrl()
{
	memset(&urlparser_, 0, sizeof(urlparser_));
	int rc = http_parser_parse_url(url_.c_str(), url_.size(), 0, &urlparser_);
	if(rc) {
		cb_(rc, "parse url failed");
		return -1;
	}

	if (
			!(urlparser_.field_set & (1 << UF_SCHEMA)) ||
			!(urlparser_.field_set & (1 << UF_HOST))
	   ) {
		cb_(rc, "invalid url");
		return -1;
	}

	int i = 0;
	
	i = UF_SCHEMA;
	schema_.assign(url_.c_str() + urlparser_.field_data[i].off, urlparser_.field_data[i].len);

	i = UF_HOST;
	host_.assign(url_.c_str() + urlparser_.field_data[i].off, urlparser_.field_data[i].len);

	i = UF_PATH;
	path_.assign(url_.c_str() + urlparser_.field_data[i].off, urlparser_.field_data[i].len);
	if(path_.empty()) path_ = "/";

	if (urlparser_.field_set & (1 << UF_QUERY)) {
		i = UF_QUERY;
		query_.assign(url_.c_str() + urlparser_.field_data[i].off, urlparser_.field_data[i].len);
	}

	port_ = urlparser_.port;
	if(!port_) {
		if(schema_ == "https") port_ = 443;
		else port_ = 80;
	}

	CO_LOGI("url parsed|schema: %s|host: %s|port: %u|path: %s|query: %s",
			schema_.c_str(),
			host_.c_str(),
			port_,
			path_.c_str(),
			query_.c_str());

	return 0;
}

int HttpConn::Resolve()
{
	int rc = 0;

	if(ParseUrl()) return -1;

	hints_.ai_family = AF_INET;
	hints_.ai_socktype = SOCK_DGRAM;
	hints_.ai_flags = AI_CANONNAME;
	hints_.ai_protocol = 0;
	CO_LOGD("send getaddrinfo req: %s", host_.c_str());
	if((rc = uv_getaddrinfo(uv_default_loop(), &resolver_, on_resolved, 
					host_.c_str(), NULL, &hints_))) {
		CO_LOGE("getaddrinfo failed: %s", uv_err_name(rc));
		cb_(rc, "getaddrinfo failed");
		return -1;
	}

	return 0;
}

int HttpConn::Connect()
{
	if(HandleError()) return -1;

	uint32_t iplist[32];
	static char saddr[255][17];
	uint32_t addrcnt = 0;
	for(auto result_pointer = res_; result_pointer != NULL && addrcnt < 32; result_pointer = result_pointer->ai_next)
	{
		uint32_t ip = 0;
		if((ip = ((struct sockaddr_in *)(result_pointer->ai_addr))->sin_addr.s_addr))
		{
			struct sockaddr_in sockaddr;
			sockaddr.sin_addr.s_addr = ip;
			uv_ip4_name(&sockaddr, saddr[addrcnt], 16);
			iplist[addrcnt] = ip;
			++addrcnt;
			//CO_LOGD("get ip: %s|cnt: %d", saddr[addrcnt], addrcnt);
		}
	}
	int i = rand() % addrcnt;
	CO_LOGD("http connect to %s", saddr[i]);

	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = iplist[i]; 
	addr_.sin_port = htons(port_);

	uv_freeaddrinfo(res_);

	uv_tcp_connect(&connect_, &socket_, (sockaddr *)&addr_, conn_cb);

	return 0;
}

int HttpConn::SendReq()
{
	if(HandleError()) return -1;

	uv_read_start((uv_stream_t *)&socket_, alloc_cb, read_cb);

	HTTPRequest httpreq;
	if(method_ == "GET") httpreq.set_method("GET");
	else httpreq.set_method("POST");

	if(query_.empty()) httpreq.set_uri(path_);
	else httpreq.set_uri(path_ + "?" + query_);

	httpreq.add_header("Host", host_);
	httpreq.add_header("Accept", "*/*");

	httpreq.set_post_body(req_);

	int len = httpreq.size() + 1; // + 1 for snprintf append '\0'

	uv_buf_t buf;
	buf.base = (char *)malloc(len);
	httpreq.encode(buf.base, len);
	buf.len = len;

	//CO_LOGD("send: %d", (int)buf.len);

	uv_write_t *wreq = (uv_write_t *)malloc(sizeof(uv_write_t));
	wreq->data = buf.base;

	uv_write(wreq, (uv_stream_t *)&socket_, &buf, 1, write_cb);

	return 0;
}

int HttpConn::ParseRsp()
{
	int headlen = recvdata_.size() - body_len_;
	if(headlen <= 0) {
		cb_(-1, "");
		return -1;
	}

	rsp_.assign(recvdata_.c_str() + headlen, body_len_);
	cb_(0, rsp_);
	uv_close((uv_handle_t *)&socket_, close_cb);

	//CO_LOGE("body hexdump: \n%s", bin2str(rsp_.data(), rsp_.size()).c_str());
	return 0;
}

void HttpConn::RecvData(char *base, unsigned int len)
{
	if(!base || !len) return;

	recvdata_.append(base, len);

	int rc = 
		http_parser_execute(&httpparser_, &settings_, 
			base, len);
	
	//CO_LOGI("rc: %d|err: %s", rc,
			//http_errno_description((enum http_errno)httpparser_.http_errno));

	if(rc != (int)len) {
		cb_(-1, "http parse failed|recv != parsed");
		CO_LOGE("http parse failed|recv: %d|parsed: %d", len, rc);
	}
}

}
