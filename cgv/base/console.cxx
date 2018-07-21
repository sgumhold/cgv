#include "console.h"
#include <cgv/type/variant.h>

using namespace cgv::type;

#ifdef _WIN32
#include <windows.h>
#endif

namespace cgv {
	namespace base {

/// hide constructor for singleton protection
console::console()
{
}
/// return console singleton
console_ptr console::get_console()
{
	static console_ptr cp(new console);
	return cp;
}

/// return "console"
std::string console::get_type_name() const
{
	return "console";
}
/// supported properties are x:int32;y:int32;w:int32;h:int32;show:bool;title:string
std::string console::get_property_declarations()
{
	return "x:int32;y:int32;w:int32;h:int32;show:bool;title:string";
}
///
bool console::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
#if defined(_MSC_VER)
	if (property == "x") {
		int32_type x;
		get_variant(x, value_type, value_ptr);
		HWND console = GetConsoleWindow();
		RECT rect;
		GetWindowRect(console, &rect);
		SetWindowPos(console,HWND_BOTTOM,x,rect.top,0,0,SWP_NOSIZE|SWP_NOZORDER);
		return true;
	}
	if (property == "y") {
		int32_type y;
		get_variant(y, value_type, value_ptr);
		HWND console = GetConsoleWindow();
		RECT rect;
		GetWindowRect(console, &rect);
		SetWindowPos(console, HWND_BOTTOM, rect.left, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		return true;
	}
	if (property == "w") {
		int32_type w;
		get_variant(w, value_type, value_ptr);
		HWND console = GetConsoleWindow();
		RECT rect;
		GetWindowRect(console, &rect);
		SetWindowPos(console, HWND_BOTTOM, 0, 0, w, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
		return true;
	}
	if (property == "h") {
		int32_type h;
		get_variant(h, value_type, value_ptr);
		HWND console = GetConsoleWindow();
		RECT rect;
		GetWindowRect(console, &rect);
		SetWindowPos(console, HWND_BOTTOM, 0, 0, rect.right - rect.left, h, SWP_NOMOVE | SWP_NOZORDER);
		return true;
	}
#endif
	return false;
}
///
bool console::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	return false;
}
	}
}