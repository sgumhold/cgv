#pragma once

#ifndef _WIN32
#ifdef CESIUM_TILES_PLUGIN_EXPORTS
#define CESIUM_TILES_PLUGIN_API __declspec(dllexport)
#elif !defined(CESIUM_TILES_PLUGIN_FORCE_STATIC)
#define CESIUM_TILES_PLUGIN_API __declspec(dllimport)
#else
#define CESIUM_TILES_PLUGIN_API
#endif
#else
#define CESIUM_TILES_PLUGIN_API
#endif