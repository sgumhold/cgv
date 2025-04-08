#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_library.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv_gl/gl/gl.h>

#include <iostream>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/** Definition of base functionality for highly parallel gpu algorithms. */
class CGV_API algorithm {
public:
	algorithm() {}
	~algorithm() {}

	bool is_initialized() const { return _is_initialized; }

protected:
	bool _is_initialized = false;

	// TODO: Move this functionality to the shader_program class.
	std::map<std::string, int> get_program_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog) const;
	
	void dispatch_compute(unsigned num_groups_x, unsigned num_groups_y, unsigned num_groups_z) {
		glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
	}
};

}
}

#include <cgv/config/lib_end.h>
