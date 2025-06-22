#pragma once

// DLL bindings
#if defined(WIN32) && !defined(CESIUM_TILES_STATIC)
	#ifdef CESIUM_TILES_BUILDING
		#define CESIUM_TILES_API __declspec(dllexport)
	#else
		#define CESIUM_TILES_API __declspec(dllimport)
	#endif
#else
#define CESIUM_TILES_API
#endif