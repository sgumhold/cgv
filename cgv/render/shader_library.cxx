#include "shader_library.h"

namespace cgv {
namespace render {

void shader_library::clear(context& ctx) {
	for(auto& entry : shaders)
		entry.second.prog.destruct(ctx);

	shaders.clear();
}

bool shader_library::add(const std::string& name, const std::string& file, const shader_compile_options& options) {
	if(shaders.find(name) == shaders.end()) {
		shader_info info;
		info.filename = file;
		info.options = options;
		shaders.insert({ name, info });
		return true;
	}
	return false;
}

bool shader_library::contains(const std::string& name) const {
	return shaders.find(name) != shaders.end();
}

shader_program& shader_library::get(const std::string& name) {
	return get_shader_info(name).prog;
}

shader_define_map& shader_library::get_defines(const std::string& name) {
	return get_shader_info(name).options.defines;
}

shader_compile_options& shader_library::get_compile_options(const std::string& name) {
	return get_shader_info(name).options;
}

bool shader_library::load(context& ctx, shader_program& prog, const std::string& name, const shader_compile_options& options, const std::string& where) {
	if(prog.is_created())
		prog.destruct(ctx);

	const std::string function_context = where == "" ? "shader_library::load_shader()" : where;

	if(!prog.is_created()) {
		bool from_program_file = name.length() > 4 && name.substr(name.length() - 5) == ".glpr";

		if(from_program_file) {
			if(!prog.build_program(ctx, name, options, true)) {
				std::cerr << "ERROR in " << function_context << " ... could not build shader program " << name << std::endl;
				return false;
			}
		} else {
			if(!prog.build_files(ctx, name, options, true)) {
				std::cerr << "ERROR in " << function_context << " ... could not build shader files " << name << std::endl;
				return false;
			}
		}
	}
	return true;
}

bool shader_library::load(context& ctx, shader_program& prog, const std::string& name, const std::string& where) {
	return load(ctx, prog, name, {}, where);
}

bool shader_library::load_all(context& ctx, const std::string& where) {
	bool success = true;
	for(auto& entry : shaders) {
		shader_info& info = entry.second;
		success &= load(ctx, info.prog, info.filename, info.options, where);
	}
	return success;
}

bool shader_library::reload(context& ctx, const std::string& name, const shader_compile_options& options, const std::string& where) {
	auto it = shaders.find(name);
	if(it != shaders.end()) {
		shader_info& info = (*it).second;
		info.options = options;
		return load(ctx, info.prog, info.filename, info.options, where);
	}
	return false;
}

shader_library::shader_info& shader_library::get_shader_info(const std::string& name) {
	if(shaders.find(name) != shaders.end()) {
		return shaders.at(name);
	} else {
		std::cerr << "Error: shader_library::get shader with name " << name << " not found!" << std::endl;
		abort();
	}
}

}
}
