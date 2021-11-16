#pragma once

#include "gpu_sorter.h"

#include "lib_begin.h"

namespace cgv {
namespace glutil {

/** GPU sorting routine implmented using a prefix-sum based radix sort. */
class CGV_API radix_sort_4way : public gpu_sorter {
protected:
	bool load_shader_programs(context& ctx);

public:
	radix_sort_4way() : gpu_sorter() {}

	bool init(context& ctx, size_t count);
	void sort(context& ctx, GLuint data_buffer, GLuint value_buffer, const vec3& eye_pos, const vec3& view_dir, GLuint auxiliary_buffer = 0);
};

}
}

#include <cgv/config/lib_end.h>
