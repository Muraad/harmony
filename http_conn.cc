#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>

#include "http_encode.h"
#include "http_conn.h"
#include "misc_util.h"

namespace imcore {

#define PROC_ERR(conn, status) do { \
	if(!conn) return; \
	if(status < 0) { \
		auto err = uv_err_name(status); \
		CO_LOGE("error: %d|desc: %s", status, err); \
		conn->cb()(status, err); \
		uv_close((uv_handle_t *)conn->socket(), close_cb); \
		return; \
	} \
}while(0)

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
	PROC_ERR(conn, (int)size);

	if(buf->base && size > 0) conn->Recv(buf->base, size);
	free(buf->base);
}

static void write_cb(uv_write_t *req, int status)
{
	auto conn = (HttpConn *)req->handle->data;
	free(req->data);
	free(req);
	PROC_ERR(conn, status);
}

static void conn_cb(uv_connect_t *req, int status)
{
	auto conn = (HttpConn *)req->data;
	PROC_ERR(conn, status);

	uv_read_start((uv_stream_t *)conn->socket(), alloc_cb, read_cb);

	HTTPRequest httpreq;
	if(conn->method() == "GET") httpreq.set_method("GET");
	else httpreq.set_method("POST");

	if(conn->query().empty()) httpreq.set_uri(conn->path());
	else httpreq.set_uri(conn->path() + "?" + conn->query());

	httpreq.add_header("Host", conn->host());
	httpreq.add_header("Accept", "*/*");

	httpreq.set_post_body(conn->req());

	int len = httpreq.size() + 1; // + 1 for snprintf append '\0'

	uv_buf_t buf;
	buf.base = (char *)malloc(len);
	httpreq.encode(buf.base, len);
	buf.len = len;

	//CO_LOGD("send: %d", (int)buf.len);

	uv_write_t *wreq = (uv_write_t *)malloc(sizeof(uv_write_t));
	wreq->data = buf.base;

	uv_write(wreq, (uv_stream_t *)conn->socket(), &buf, 1, write_cb);
}

static void on_resolved(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res) 
{
	auto conn = (HttpConn *)resolver->data;
	PROC_ERR(conn, status);

	uint32_t iplist[32];
	static char addr[255][17];
	uint32_t addrcnt = 0;
	for(auto result_pointer = res; result_pointer != NULL && addrcnt < 32; result_pointer = result_pointer->ai_next)
	{
		uint32_t ip = 0;
		if((ip = ((struct sockaddr_in *)(result_pointer->ai_addr))->sin_addr.s_addr))
		{
			struct sockaddr_in sockaddr;
			sockaddr.sin_addr.s_addr = ip;
			uv_ip4_name(&sockaddr, addr[addrcnt], 16);
			iplist[addrcnt] = ip;
			++addrcnt;
			//CO_LOGD("get ip: %s|cnt: %d", addr, addrcnt);
		}
	}
	int i = rand() % addrcnt;
	CO_LOGD("http connect to %s", addr[i]);
	conn->SetAddr(iplist[i]);
	uv_freeaddrinfo(res);

	uv_tcp_connect(conn->connect(), conn->socket(), (sockaddr *)conn->addr(), conn_cb);
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

static PT_THREAD(pt_http_request(struct pt *pt));
static int on_message_complete(http_parser *p)
{
	//CO_LOGD("recv complete");
	auto conn = (HttpConn *)p->data;
	if(!conn) return 0;

	int headlen = conn->recvdata().size() - conn->body_len();
	if(headlen <= 0) {
		conn->cb()(-1, "");
		return 0;
	}

	conn->mutable_rsp()->assign(conn->recvdata().c_str() + headlen, conn->body_len());
	//conn->cb()(0, std::move(rsp));
	conn->set_complete(1);
	PT_SCHEDULE(pt_http_request(conn->pt()));

	uv_close((uv_handle_t *)conn->socket(), close_cb);

	//CO_LOGE("body hexdump: \n%s", bin2str(rsp.data(), rsp.size()).c_str());

	return 0;
}
/** http parser callback end **/

/** pt begin **/
static PT_THREAD(pt_http_request(struct pt *pt))
{
	auto copt = (struct co_send_recv_t *)pt;
	auto conn = (HttpConn *)copt->data;

	PT_BEGIN(pt);

	PT_WAIT_UNTIL(pt, 
			conn->complete() != 0);

	conn->cb()(0, conn->rsp());

	PT_END(pt);
}

/** pt begin **/

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

	copt_.data = this;
	PT_INIT(&copt_.pt);

	complete_ = 0;
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

	int rc = 0;
	if((rc = ParseUrl(url))) {
		cb(rc, "parse url failed");
		return;
	}

	hints_.ai_family = AF_INET;
	hints_.ai_socktype = SOCK_DGRAM;
	hints_.ai_flags = AI_CANONNAME;
	hints_.ai_protocol = 0;

	CO_LOGD("send getaddrinfo req: %s", host_.c_str());
	if((rc = uv_getaddrinfo(uv_default_loop(), &resolver_, on_resolved, host_.c_str(), NULL, &hints_))) {
		CO_LOGE("getaddrinfo failed: %s", uv_err_name(rc));
		cb(rc, "getaddrinfo failed");
		return;
	}
}

int HttpConn::ParseUrl(const std::string &url)
{
	memset(&urlparser_, 0, sizeof(urlparser_));
	int rc = http_parser_parse_url(url.c_str(), url.size(), 0, &urlparser_);
	if(rc) {
		CO_LOGE("parse url failed: %d", rc);
		return rc;
	}

	if (
			!(urlparser_.field_set & (1 << UF_SCHEMA)) ||
			!(urlparser_.field_set & (1 << UF_HOST))
	   ) {
		CO_LOGE("invalid url");
		return -1;
	}

	int i = 0;
	
	i = UF_SCHEMA;
	schema_.assign(url.c_str() + urlparser_.field_data[i].off, urlparser_.field_data[i].len);

	i = UF_HOST;
	host_.assign(url.c_str() + urlparser_.field_data[i].off, urlparser_.field_data[i].len);

	i = UF_PATH;
	path_.assign(url.c_str() + urlparser_.field_data[i].off, urlparser_.field_data[i].len);
	if(path_.empty()) path_ = "/";

	if (urlparser_.field_set & (1 << UF_QUERY)) {
		i = UF_QUERY;
		query_.assign(url.c_str() + urlparser_.field_data[i].off, urlparser_.field_data[i].len);
	}

	port_ = urlparser_.port;
	if(!port_) {
		if(schema_ == "https") port_ = 443;
		else port_ = 80;
	}

	CO_LOGI("schema: %s|host: %s|port: %u|path: %s|query: %s",
			schema_.c_str(),
			host_.c_str(),
			port_,
			path_.c_str(),
			query_.c_str());

	return 0;
}

void HttpConn::Recv(char *base, unsigned int len)
{
	if(!base || !len) return;

	recvdata_.append(base, len);

	int rc = 
		http_parser_execute(&httpparser_, &settings_, 
			base, len);
	
	//CO_LOGI("rc: %d|err: %s", rc,
			//http_errno_description((enum http_errno)httpparser_.http_errno));

	if(rc != (int)len) {
		cb()(-1, "http parse failed|recv != parsed");
		CO_LOGE("http parse failed|recv: %d|parsed: %d", len, rc);
	}
}

}
