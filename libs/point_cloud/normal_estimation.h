#pragma once

#include "lib_begin.h"

extern CGV_API void estimate_normal_ls(unsigned nr_points, const float* _points, float* _normal, float* _evs = 0);
extern CGV_API void estimate_normal_wls(unsigned nr_points, const float* _points, const float* _weights, float* _normal, float* _evs = 0);

#include <cgv/config/lib_end.h>
