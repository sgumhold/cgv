#pragma once

#ifdef _WIN32
	#ifdef MAPTILES_PLUGIN_EXPORTS
		#define MAPTILES_PLUGIN_API __declspec(dllexport)
	#elif !defined(MAPTILES_PLUGIN_FORCE_STATIC)
		#define MAPTILES_PLUGIN_API __declspec(dllimport)
	#else
		#define MAPTILES_PLUGIN_API
	#endif
#else
	#define MAPTILES_PLUGIN_API
#endif

class maptiles;

class maptiles_interfacer
{
private:
	static maptiles* ptr;

public:
	static void set_pointer (maptiles* _ptr) { ptr = _ptr; }
	static maptiles* get_pointer (void) { return ptr; }
	static MAPTILES_PLUGIN_API void force_draw (cgv::render::context& ctx);
};
