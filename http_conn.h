#ifndef __HTTP_CONN_H__
#define __HTTP_CONN_H__

#include <string>
#include <memory>
#include <functional>
#include <cstdint>

#include "co_comm.h"
#include "uv.h"
#include "http_parser.h"

namespace imcore {

class HttpConn {
public:
	HttpConn();
	virtual ~HttpConn();
	void Request(const std::string &url, const std::string &method,
			const std::string &req, std::function<void(int, const std::string &rsp)> cb);
	void RecvData(char *base, unsigned int len);

	int HandleError();
	int ParseUrl();
	int Resolve();
	int Connect();
	int SendReq();
	int ParseRsp();

	void set_res(struct addrinfo *res) { res_ = res; }
	int req_buf_len() { return 4096; }
	struct pt *pt() { return (struct pt *)&copt_; }
	struct co_send_recv_t *copt() { return &copt_; }

private:
	struct addrinfo *res_;
	struct addrinfo hints_;
	struct sockaddr_in addr_;
	uint16_t port_;
	struct http_parser_url urlparser_;
	http_parser_settings settings_;

	std::string req_;
	std::string rsp_;
	std::string url_;
	std::string recvdata_;
	std::string schema_;
	std::string host_;
	std::string method_;
	std::string path_;
	std::string query_;

	std::function<void(int, const std::string &rsp)> cb_;

	uv_getaddrinfo_t resolver_;
	uv_tcp_t socket_;
	uv_connect_t connect_;
	//uv_timer_t timer_;

	http_parser httpparser_;
	struct co_send_recv_t copt_;

	CO_ACCESSOR(int, status)
	CO_ACCESSOR(uint64_t, body_len)
};

}
#endif
