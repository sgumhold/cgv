#pragma once

#include "Config.h"

// See: https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
int MAPTILES_API long2tilex(double lon, int z);

int MAPTILES_API lat2tiley(double lat, int z);

double MAPTILES_API tilex2long(int x, int z);

double MAPTILES_API tiley2lat(int y, int z);
