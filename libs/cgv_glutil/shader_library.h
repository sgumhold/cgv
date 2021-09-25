#pragma once

#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil {

/// provides a shader library that handles shader loading
class CGV_API shader_library {
protected:
	/*struct shader_info {
		std::string filename = "";
		shader_program prog;
		shader_define_map defines = {};
	};*/

	typedef std::pair<cgv::render::shader_program, std::string> shader_program_pair;
	std::map<std::string, shader_program_pair> shaders;
	//std::map<std::string, shader_info> shaders;

public:
	shader_library();
	~shader_library();

	void clear(cgv::render::context& ctx);

	bool add(const std::string& name, const std::string& file);

	cgv::render::shader_program& get(const std::string& name) { return shaders.at(name).first; }

	std::map<std::string, shader_program_pair>::iterator begin() { return shaders.begin(); }
	std::map<std::string, shader_program_pair>::iterator end() { return shaders.end(); }
	
	bool load_shaders(cgv::render::context& ctx);

	bool reload(cgv::render::context& ctx, const std::string& name, const cgv::render::shader_define_map& defines = {}, const std::string& where = "");

	static bool load(cgv::render::context& ctx, cgv::render::shader_program& prog, const std::string& name, const bool reload = false, const std::string& where = "") {

		return load(ctx, prog, name, cgv::render::shader_define_map(), reload, where);
	}

	static bool load(cgv::render::context& ctx, cgv::render::shader_program& prog, const std::string& name, const cgv::render::shader_define_map& defines, const bool reload = false, const std::string& where = "") {

		if(prog.is_created()) {
			if(reload) prog.destruct(ctx);
			else return true;
		}

		std::string function_context = where == "" ? "shader_library::load_shader()" : where;

		if(!prog.is_created()) {
			bool from_program_file = name.substr(name.length() - 5) == ".glpr";

			if(from_program_file) {
				if(!prog.build_program(ctx, name, true, defines)) {
					std::cerr << "ERROR in " << function_context << " ... could not build shader program " << name << std::endl;
					return false;
				}
			} else {
				if(!prog.build_files(ctx, name, true, defines)) {
					std::cerr << "ERROR in " << function_context << " ... could not build shader files " << name << std::endl;
					return false;
				}
			}
		}
		return true;
	}
};
}
}

#include <cgv/config/lib_end.h>
