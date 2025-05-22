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
		int loc = _prog.get_uniform_location(ctx, name);
		if(loc > -1)
			return _prog.set_uniform(ctx, loc, value);
		return false;
	}

	template<typename T, typename S, typename std::enable_if<!std::is_same<T, S>::value, bool>::type = true>
	bool set_argument(const cgv::render::context& ctx, const std::string& name, const S& value) {
		int loc = _prog.get_uniform_location(ctx, name);
		if(loc > -1)
			return _prog.set_uniform(ctx, loc, static_cast<T>(value));
		return false;
	}

	void set_arguments(cgv::render::context& ctx, const argument_bindings& arguments);

private:
	cgv::render::shader_program _prog;
};

}
}

#include <cgv/config/lib_end.h>
