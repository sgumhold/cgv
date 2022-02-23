#include "resources.h"
#include <iostream>

#ifdef WIN32
#include <Windows.h>

const char* resource_type_name(unsigned i)
{
	static const char* resource_type_names[] = {
	"",
	"Cursor",
	"Bitmap",
	"Icon",
	"Menu",
	"Dialog",
	"String",
	"FontDir",
	"Font",
	"Accelerator",
	"RawData",
	"MessageTable",
	"GroupCoursor",
	"",
	"GroupIcon",
	"",
	"Version",
	"DLGInclude",
	"",
	"PlugAndPlay",
	"VXD",
	"Animated cursor",
	"AnimatedIcon",
	"HTML",
	"Manifest"
	};
	return resource_type_names[i];
}
enum class RessourceType
{
	CURSOR = 1,
	BITMAP = 2,
	ICON = 3,
	MENU = 4,
	DIALOG = 5,
	STRING = 6,
	FONTDIR = 7,
	FONT = 8,
	ACCELERATOR = 9,
	RCDATA = 10,
	MESSAGETABLE = 11,
	GROUP_CURSOR = 12,
	GROUP_ICON = 14,
	VERSION = 16,
	DLGINCLUDE = 17,
	PLUGPLAY = 19,
	VXD = 20,
	ANICURSOR = 21,
	ANIICON = 22,
	HTML = 23,
	MANIFEST = 24
};

BOOL EnumLangsFunc(HMODULE hModule, LPCTSTR lpType, LPCTSTR lpName, WORD wLang, LONG lParam)
{
	HRSRC hResInfo;
	hResInfo = FindResourceEx(hModule, lpType, lpName, wLang);
	std::cout << "    Language: " << (USHORT)wLang;
	std::cout << "    ResInfo: " << hResInfo << "    Size = " << (USHORT)wLang << std::endl;
	return TRUE;
}

BOOL EnumNamesFunc(HMODULE hModule, LPCTSTR lpType, LPTSTR lpName, LONG lParam)
{
	if (IS_INTRESOURCE(lpName))
		std::cout << "  Name:" << (USHORT)lpName << std::endl;
	else
		std::cout << "  Name:" << lpName << std::endl;
	EnumResourceLanguages(hModule, lpType, lpName, (ENUMRESLANGPROC)EnumLangsFunc, 0);
	return TRUE;
}

BOOL EnumTypesFunc(HMODULE hModule, LPTSTR lpType, LONG lParam)
{
	if (IS_INTRESOURCE(lpType)) {
		USHORT us = (USHORT)lpType;
		if (us < 25)
			std::cout << resource_type_name(us) << std::endl;
		else
			std::cout << us << std::endl;
	}
	else
		std::cout << lpType << std::endl;
	EnumResourceNames(hModule, lpType, (ENUMRESNAMEPROC)EnumNamesFunc, 0);
	return TRUE;
}
namespace cgv {
	namespace os {

		void enum_resources(const std::string& prog_name)
		{
		#ifdef UNICODE
			HMODULE hi = GetModuleHandleA(prog_name.c_str());
		#else
			HMODULE hi = GetModuleHandle(prog_name.c_str());
		#endif
			EnumResourceTypes(hi, (ENUMRESTYPEPROC)EnumTypesFunc, 0);
		}
	}
}
#else
namespace cgv {
	namespace os {

		void enum_resources(const std::string& prog_name)
		{
			std::cout << "cgv::os::enum_resources() not implemented" << std::endl;
		}
	}
}
#endif