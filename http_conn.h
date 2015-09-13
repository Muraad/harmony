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
	void Recv(char *base, unsigned int len);
	void SetAddr(uint32_t ip) 
	{ 
		addr_.sin_family = AF_INET;
		addr_.sin_addr.s_addr = ip; 
		addr_.sin_port = htons(port_);
	}
	struct sockaddr_in *addr() { return &addr_; }
	uv_tcp_t *socket() { return &socket_; }
	uv_connect_t *connect() { return &connect_; }
	http_parser *httpparser() { return &httpparser_; }
	uv_getaddrinfo_t *resolver() { return &resolver_; }

	int req_buf_len() { return 4096; }
	std::function<void(int, const std::string &rsp)> cb() { return cb_; }
	struct pt *pt() { return (struct pt *)&copt_; }

	CO_ACCESSOR(uint64_t, body_len)
	CO_ACCESSOR(int, complete)
	CO_ACCESSOR(std::string, recvdata)
	CO_ACCESSOR(std::string, rsp)
	CO_ACCESSOR(std::string, req)
	CO_ACCESSOR(std::string, host)
	CO_ACCESSOR(std::string, method)
	CO_ACCESSOR(std::string, path)
	CO_ACCESSOR(std::string, query)

private:
	int ParseUrl(const std::string &url);

	struct addrinfo hints_;
	struct sockaddr_in addr_;
	std::string url_;
	std::string schema_;
	uint16_t port_;
	struct http_parser_url urlparser_;
	http_parser_settings settings_;

	std::function<void(int, const std::string &rsp)> cb_;

	uv_getaddrinfo_t resolver_;
	uv_tcp_t socket_;
	uv_connect_t connect_;
	//uv_timer_t timer_;

	http_parser httpparser_;
	struct co_send_recv_t copt_;
};

}
#endif
