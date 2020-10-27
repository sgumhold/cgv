//========= Copyright Valve Corporation ============//
#include "sharedlibtools_public.h"
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(POSIX)
#include <dlfcn.h>
#endif
#ifdef _WIN32
#ifdef UNICODE
#include <xstring>
#endif
#endif

SharedLibHandle SharedLib_Load(const char* pchPath)
{
#ifdef _WIN32
#ifdef UNICODE
	std::string s(pchPath);
	std::wstring ws;
	ws.resize(s.size());
	if (!s.empty()) {
		int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &ws[0], (int)ws.size());
		ws.resize(n);
	}
	return (SharedLibHandle)LoadLibraryEx(ws.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#else
	return (SharedLibHandle)LoadLibraryEx(pchPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
#endif
#elif defined(POSIX)
	return (SharedLibHandle)dlopen(pchPath, RTLD_LOCAL|RTLD_NOW);
#endif
}

void *SharedLib_GetFunction( SharedLibHandle lib, const char *pchFunctionName)
{
#if defined( _WIN32)
	return (void*)GetProcAddress( (HMODULE)lib, pchFunctionName );
#elif defined(POSIX)
	return dlsym( lib, pchFunctionName );
#endif
}


void SharedLib_Unload( SharedLibHandle lib )
{
	if ( !lib )
		return;
#if defined( _WIN32)
	FreeLibrary( (HMODULE)lib );
#elif defined(POSIX)
	dlclose( lib );
#endif
}


