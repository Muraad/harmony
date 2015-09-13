#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "http_encode.h"
#include "misc_util.h"

using namespace std;

namespace imcore {

HTTPRequest::HTTPRequest()
{
	method_ = "POST";
	url_ = "/";

	//GET / HTTP/1.1\r\n
	size_ = (int)(method_.size() + 1 + url_.size() + 1 + 10);
}

int HTTPRequest::size()
{
	return size_;
}

int HTTPRequest::add_header(const std::string &key, const std::string &value)
{
	headers_[key] = value;
	//key: value\r\n
	size_ += (int)(key.size() + 1 + 1 + value.size() + 2);

	return 0;
}

int HTTPRequest::set_uri(const std::string &uri)
{
	size_ -= (int)url_.size();
	size_ += (int)uri.size();

	url_ = uri;

	return 0;
}

int HTTPRequest::set_post_body(const std::string & payload)
{
	if (payload_.empty()) {
		size_ += 2;
	} else {
		size_ -= (int)payload_.size();
		size_ += payload.size();
	}

	payload_ = payload;

	return 0;
}

int HTTPRequest::set_method(const std::string & method)
{
	size_ -= (int)method_.size();
	size_ += (int)method.size();

	method_ = method;

	return 0;
}

int HTTPRequest::encode(char *buffer, int & len)
{
	int offset = snprintf(buffer, len, "%s %s HTTP/1.1\r\n", method_.c_str(), url_.c_str());
	if (offset <= 0 || offset >= len) {
		CO_LOGE("here");
		return -10;
	}

	if (method_ == "POST") {
		char sContentLength[20] = {};
		snprintf(sContentLength, sizeof(sContentLength), "%lu", payload_.size());
		headers_["Content-Length"] = sContentLength;
	}

	int out_len = 0;

	for (std::map<std::string, std::string>::iterator i = headers_.begin(); i!= headers_.end(); i++) {
		out_len = snprintf(buffer + offset, len - offset, "%s: %s\r\n", i->first.c_str(), i->second.c_str());
		if (out_len <= 0 || out_len >= len - offset) {
		CO_LOGE("here");
			return -11;
		}
		offset += out_len;
	}

	if (method_ == "POST") {
		out_len = snprintf(buffer + offset, len - offset, "\r\n");
		memcpy(buffer + offset + out_len, payload_.data(), payload_.size());
		out_len += payload_.size();

		//LOGGER_INFO("out_len: " << out_len);
	} else {
		out_len = snprintf(buffer + offset, len - offset, "\r\n");
	}

	if (out_len <= 0 || out_len >= len - offset) {
		return -12;
	}

	offset += out_len;
	len = offset;

	//CO_LOGD("\n%s", buffer);

	return 0;
}

}
