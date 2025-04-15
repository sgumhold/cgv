#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>

#include "argument.h"

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
		return _prog.set_uniform(ctx, _uniforms[name], value);
	}

	void set_argument_locations(cgv::render::context& ctx, const std::string& prefix, uniform_arguments& arguments) const;

	void set_arguments(cgv::render::context& ctx, const uniform_arguments& arguments);
	void set_arguments(cgv::render::context& ctx, const uniform_binding_list& arguments, const std::string& prefix = "");

private:
	struct enable_guard {
		cgv::render::context& ctx;
		cgv::render::shader_program& prog;
		bool was_enabled = false;

		enable_guard(cgv::render::context& ctx, cgv::render::shader_program& prog);
		~enable_guard();
	};

	cgv::render::shader_program _prog;
	std::map<std::string, int> _uniforms;
};

}
}

#include <cgv/config/lib_end.h>
