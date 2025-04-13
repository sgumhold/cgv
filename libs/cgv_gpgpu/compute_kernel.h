#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

class CGV_API compute_kernel {
public:
	bool init(cgv::render::context& ctx, const std::string& name, const cgv::render::shader_compile_options& config, const std::string& where);

	void destruct(const cgv::render::context& ctx);

	bool enable(cgv::render::context& ctx);

	bool disable(cgv::render::context& ctx);

	template<typename T>
	bool set_argument(const cgv::render::context& ctx, const std::string& name, const T& value) {
		return prog.set_uniform(ctx, _uniforms[name], value);
	}

	void set_arguments(cgv::render::context& ctx, const uniform_argument_list& arguments);

private:
	cgv::render::shader_program _prog;
	std::map<std::string, int> _uniforms;
};

}
}

#include <cgv/config/lib_end.h>
