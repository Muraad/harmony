#ifndef __MISC_UTIL_H__
#define __MISC_UTIL_H__

#include <string>
#include <vector>
#include <utility>

//#include "log/logger.h"
//using namespace im_logger;
//const std::string LOG_NAME = "imsdk_log";
//const std::string CONFIG_NAME = "imsdk_config";

//#define LOG_PREFIX "func: "__FUNCTION__  << "line:" << __LINE__
//#define LOG_FUNC_TRAC LOGGER_TRACE(LOG_PREFIX << " called!");
//#define LOG_ERROR_BUF_TOO_SHORT LOGGER_ERROR(LOG_PREFIX << " Buffer too short");

#ifndef _MSC_VER
#define CLOG(fmt, args...) printf("[%s][%d][%s]: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#else
#define CLOG(fmt, ...) printf("[%s][%d][%s]: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#endif

#define CO_LOGD(...) CLOG(__VA_ARGS__)
#define CO_LOGI(...) CLOG(__VA_ARGS__)
#define CO_LOGW(...) CLOG(__VA_ARGS__)
#define CO_LOGE(...) CLOG(__VA_ARGS__)

static char c_b2s[256][4] = {
    "00", "01", "02", "03", "04", "05", "06", "07",
    "08", "09", "0a", "0b", "0c", "0d", "0e", "0f", "10", "11", "12", "13",
    "14", "15", "16", "17", "18", "19", "1a", "1b", "1c", "1d", "1e", "1f",
    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2a", "2b",
    "2c", "2d", "2e", "2f", "30", "31", "32", "33", "34", "35", "36", "37",
    "38", "39", "3a", "3b", "3c", "3d", "3e", "3f", "40", "41", "42", "43",
    "44", "45", "46", "47", "48", "49", "4a", "4b", "4c", "4d", "4e", "4f",
    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5a", "5b",
    "5c", "5d", "5e", "5f", "60", "61", "62", "63", "64", "65", "66", "67",
    "68", "69", "6a", "6b", "6c", "6d", "6e", "6f", "70", "71", "72", "73",
    "74", "75", "76", "77", "78", "79", "7a", "7b", "7c", "7d", "7e", "7f",
    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8a", "8b",
    "8c", "8d", "8e", "8f", "90", "91", "92", "93", "94", "95", "96", "97",
    "98", "99", "9a", "9b", "9c", "9d", "9e", "9f", "a0", "a1", "a2", "a3",
    "a4", "a5", "a6", "a7", "a8", "a9", "aa", "ab", "ac", "ad", "ae", "af",
    "b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "b9", "ba", "bb",
    "bc", "bd", "be", "bf", "c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7",
    "c8", "c9", "ca", "cb", "cc", "cd", "ce", "cf", "d0", "d1", "d2", "d3",
    "d4", "d5", "d6", "d7", "d8", "d9", "da", "db", "dc", "dd", "de", "df",
    "e0", "e1", "e2", "e3", "e4", "e5", "e6", "e7", "e8", "e9", "ea", "eb",
    "ec", "ed", "ee", "ef", "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",
    "f8", "f9", "fa", "fb", "fc", "fd", "fe", "ff" };

static inline std::string bin2hexstr(const void *buf, size_t len) 
{
	if (buf == NULL || len <= 0) {
		return "";
	}
    
	std::string out;
	const unsigned char *p = (const unsigned char *) buf;
    
	for (size_t i = 0; i < len; ++i, ++p) {
		out += c_b2s[*p][0];
		out += c_b2s[*p][1];
	}
    
	return std::move(out);
}

static inline char hexdigit2byte(char c)
{
	switch (c) {
		case '0': return 0x00;
		case '1': return 0x01;
		case '2': return 0x02;
		case '3': return 0x03;
		case '4': return 0x04;
		case '5': return 0x05;
		case '6': return 0x06;
		case '7': return 0x07;
		case '8': return 0x08;
		case '9': return 0x09;
		case 'a': case 'A': return 0x0a;
		case 'b': case 'B': return 0x0b;
		case 'c': case 'C': return 0x0c;
		case 'd': case 'D': return 0x0d;
		case 'e': case 'E': return 0x0e;
		case 'f': case 'F': return 0x0f;
		default: return 0x00;
	}

	//not reachable
	return 0x00;
}

static inline std::string hexstr2bin(const std::string &src)
{
	int len = src.size() / 2;
	std::vector<char> tmp;
	tmp.resize(len);
	char *p = &tmp[0];

	for (int i = 0; i < len; ++i) {
		char c1 = hexdigit2byte(src.c_str()[2*i]);
		char c2 = hexdigit2byte(src.c_str()[2*i+1]);
		*p++ = (c1 << 4)|c2;
	}

	std::string r;
	r.assign((const char *)&tmp[0], len);

	return std::move(r);
}

#endif
