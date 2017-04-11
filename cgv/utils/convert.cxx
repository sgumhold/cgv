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

	}
}
