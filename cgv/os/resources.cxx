#include "resources.h"
#include <iostream>

#ifdef WIN32
#include <Windows.h>

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
	if (IS_INTRESOURCE(lpType))
		std::cout << (USHORT)lpType << std::endl;
	else
		std::cout << lpType << std::endl;
	EnumResourceNames(hModule, lpType, (ENUMRESNAMEPROC)EnumNamesFunc, 0);
	return TRUE;
}
namespace cgv {
	namespace os {

		void enum_resources(const std::string& prog_name)
		{
			HMODULE hi = GetModuleHandle(prog_name.c_str());
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