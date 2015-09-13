#ifndef __HTTP_ENCODE_H__
#define __HTTP_ENCODE_H__

#include <string>
#include <map>

namespace imcore {

class HTTPRequest {
public:
	HTTPRequest();
	int add_header(const std::string & key, const std::string & value);
	int set_uri(const std::string & uri);
	int set_post_body(const std::string & payload);
	int set_method(const std::string & method);
	int encode(char *buffer, int & len);
	int size();

private:
	std::string method_;
	std::string url_;
	std::string payload_;
	std::map<std::string, std::string> headers_;
	int size_;
};

}

#endif // DB_HTTP_H

