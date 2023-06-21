#include "convert.h"
#ifdef _WIN32
#include <windows.h>
#include <Winnls.h>
#endif
#include <iostream>

namespace cgv {
	namespace utils {

std::wstring str2wstr(const std::string& s)
{
	std::wstring ws;
	ws.resize(s.size());
#ifdef _WIN32
	if (!s.empty()) {
		int n = MultiByteToWideChar(CP_UTF8,0,s.c_str(),(int)s.size(),&ws[0],(int)ws.size());
		ws.resize(n);
	}
#else
	std::cerr << "str2wstr(const std::string& s) not implemented" << std::endl;
#endif
	return ws;
}

std::string wstr2str(const std::wstring& ws)
{
	std::string s;
	s.resize(4*ws.size());
#ifdef _WIN32
	if (!ws.empty()) {
		// BOOL default_char_used;
		char default_char = '#';
		int n = WideCharToMultiByte(CP_UTF8,0,ws.c_str(), (int)ws.size(),&s[0],(int)s.size(),NULL/*&default_char*/, NULL/*&default_char_used*/);
		if (n == 0) {
			switch (GetLastError()) {
			case ERROR_INSUFFICIENT_BUFFER : std::cerr << "A supplied buffer size was not large enough, or it was incorrectly set to NULL." << std::endl; break;
			case ERROR_INVALID_FLAGS : std::cerr << "The values supplied for flags were not valid." << std::endl; break;
			case ERROR_INVALID_PARAMETER : std::cerr << "Any of the parameter values was invalid." << std::endl; break;
			case ERROR_NO_UNICODE_TRANSLATION : std::cerr << "Invalid Unicode was found in a string." << std::endl; break;
			}
		}
		s.resize(n);
	}
#else
	std::cerr << "wstr2str(const std::wstring& ws) not implemented" << std::endl;
#endif
	return s;
}

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string encode_base64(const std::string& s) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	const char* bytes_to_encode = s.c_str();
	size_t in_len = s.length();
	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;

}

std::string decode_base64(std::string const& encoded_string) {
	int in_len = (int)encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

	}
}
