#pragma once

// DLL bindings
#if defined(WIN32) && !defined(MAPTILES_STATIC)
	#ifdef MAPTILES_BUILDING
		#define MAPTILES_API __declspec(dllexport)
	#else
		#define MAPTILES_API __declspec(dllimport)
	#endif
#else
#define MAPTILES_API
#endif

/// The global config struct
struct GlobalConfig
{
	struct ReferencePointWGS84
	{
		double lat;
		double lon;
	};

	ReferencePointWGS84 ReferencePoint = { 0, 0 };
	double Tile3DSize = 0.01;	// in degrees

	// Detemines how many neighbours will be fetched
	// For a neighbourhood size of n, a (n * 2 + 1) x (n * 2 + 1) grid will be fetched
	int NeighbourhoodFetchSizeRasterTile = 1;
	int NeighbourhoodFetchSizeTile3D = 1;
};
