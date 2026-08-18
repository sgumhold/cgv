#pragma once

#include <tuple>
#include <vector>

#include "lib_begin.h"

namespace cgv {
namespace media {

extern CGV_API std::tuple<int, int, int>  get_tick_specification(float start, float stop, int count);

extern CGV_API std::vector<float> compute_ticks(float start, float stop, int count);

extern CGV_API std::vector<float> compute_ticks_log(float start, float stop, float base, int count);

} // namespace media
} // namespace cgv

#include <cgv/config/lib_end.h>
