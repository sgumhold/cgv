#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_library.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv_gl/gl/gl.h>

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {



// TODO: Move this functionality to the shader_program class?
extern CGV_API std::map<std::string, int> get_program_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog);







struct uniform_argument {
	std::string name;
	cgv::render::type_descriptor desc;
	void* addr = nullptr;

	uniform_argument() {}

	template<typename T>
	uniform_argument(const std::string& name, T value) : name(name) {
		desc = cgv::render::element_descriptor_traits<T>::get_type_descriptor(value);
		addr = cgv::render::element_descriptor_traits<T>::get_address(value);
	}
};

using uniform_argument_list = std::vector<uniform_argument>;





struct compute_kernel {
	bool init(cgv::render::context& ctx, const std::string& name, const cgv::render::shader_compile_options& config, const std::string& where) {
		bool res = cgv::render::shader_library::load(ctx, prog, name, config, true, where);
		if(res)
			uniforms = get_program_uniforms(ctx, prog);
		return res;
	}

	void destruct(const cgv::render::context& ctx) {
		prog.destruct(ctx);
		uniforms.clear();
	}

	bool enable(cgv::render::context& ctx) {
		return prog.enable(ctx);
	}

	bool disable(cgv::render::context& ctx) {
		return prog.disable(ctx);
	}

	template<typename T>
	bool set_argument(const cgv::render::context& ctx, const std::string& name, const T& value) {
		return prog.set_uniform(ctx, uniforms[name], value);
	}

	void set_arguments(cgv::render::context& ctx, const uniform_argument_list& arguments) {
		bool was_enabled = prog.is_enabled();
		if(!was_enabled)
			prog.enable(ctx);

		for(const uniform_argument& arg : arguments) {
			auto it = uniforms.find(arg.name);
			if(it != uniforms.end()) {
				int loc = it->second;
				prog.set_uniform(ctx, loc, arg.desc, arg.addr);
			}
		}

		if(!was_enabled)
			prog.disable(ctx);
	}

	cgv::render::shader_program prog;
	std::map<std::string, int> uniforms;
};












/** Definition of base functionality for highly parallel gpu algorithms. */
class CGV_API algorithm {
public:
	// TODO: Remove default constructor.
	algorithm() {}
	algorithm(const std::string& type_name) : _type_name(type_name) {}
	
	bool is_initialized() const {
		return _is_initialized;
	}

protected:
	std::string get_debug_context() const {
		return "cgv::gpgpu::" + _type_name;
	}

	void register_kernel(compute_kernel& kernel, const std::string& name) {
		_kernel_registrations.push_back({ &kernel, name });
	};
	
	bool init_kernels(cgv::render::context& ctx, const cgv::render::shader_compile_options& config) {
		const std::string debug_context = get_debug_context();
		bool success = true;
		for(const auto& info : _kernel_registrations)
			success &= info.kernel->init(ctx, info.name, config, debug_context);
		return success;
	}

	// TODO: Add destruct_kernels and handle setting of _is_initialized member in init and destruct kernels methods.
	
	std::string _type_name;
	bool _is_initialized = false;
	
	void dispatch_compute(unsigned num_groups_x, unsigned num_groups_y, unsigned num_groups_z) {
		glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
	}

private:
	struct compute_kernel_info {
		compute_kernel* kernel = nullptr;
		std::string name;
	};

	std::vector<compute_kernel_info> _kernel_registrations;
};

}
}

#include <cgv/config/lib_end.h>
