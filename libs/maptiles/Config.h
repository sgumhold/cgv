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

	// The max number of raster tiles generated along one axis when using frustum based tile generation
	int FrustumRasterTilesCount = 10;			
	// The max distance from the camera at which a 3D tile can be generated
	// when using frustum based tile generation
	double FrustumTile3DMaxDistance = 0.045; // in degrees

	// Distance at which the tile manager will change its ReferencePoint if Auto Recentering is enabled
	// this is required to avoid floating point precision based artifacts
	double AutoRecenterDistance = 10000; // in meters

	// Detemines how many neighbours will be fetched
	// For a neighbourhood size of n, a (n * 2 + 1) x (n * 2 + 1) grid will be fetched
	int NeighbourhoodFetchSizeRasterTile = 1;
	int NeighbourhoodFetchSizeTile3D = 1;

	bool FrustumBasedTileGeneration = false;
};
