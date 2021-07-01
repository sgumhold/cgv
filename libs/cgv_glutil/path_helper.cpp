#include "path_helper.h"

#include <cgv/base/register.h>

//#include <codecvt>
//#include <locale>
//#define NOMINMAX
//#include <windows.h>

namespace cgv {
namespace glutil {

/*std::string path_helper::ws2s(const std::wstring& wstr) {

	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}*/

std::string path_helper::get_executable_name() {

	return cgv::base::ref_prog_name();
}

std::string path_helper::get_executable_path() {

	return cgv::base::ref_prog_path_prefix();
}

/*std::string path_helper::get_executable_path() {

	TCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	return ws2s(std::wstring(buffer).substr(0, pos));
}*/

}
}
