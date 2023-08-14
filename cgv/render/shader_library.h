#pragma once

#include "context.h"
#include "shader_program.h"

#include "lib_begin.h"

namespace cgv {
namespace render {

/// provides a shader library that handles shader loading
class CGV_API shader_library {
protected:
	struct shader_info {
		std::string filename = "";
		shader_program prog;
		shader_define_map defines = {};
	};

	typedef std::map<std::string, shader_info> shader_lib_map;
	shader_lib_map shaders;

	shader_info& get_shader_info(const std::string& name) {

		if(shaders.find(name) != shaders.end()) {
			return shaders.at(name);
		} else {
			std::cerr << "Error: shader_library::get shader with name " << name << " not found!" << std::endl;
			abort();
		}
	}

public:
	shader_library();
	~shader_library();

	void clear(context& ctx);

	bool add(const std::string& name, const std::string& file, const shader_define_map& defines = {});

	shader_program& get(const std::string& name) {
		
		return get_shader_info(name).prog;
	}

	shader_define_map& get_defines(const std::string& name) {
		
		return get_shader_info(name).defines;
	}

	bool contains(const std::string& name) const {

		return shaders.find(name) != shaders.end();
	}

	shader_lib_map::iterator begin() { return shaders.begin(); }
	shader_lib_map::iterator end() { return shaders.end(); }
	
	bool load_all(context& ctx, const std::string& where = "");

	bool reload(context& ctx, const std::string& name, const shader_define_map& defines = {}, const std::string& where = "");

	static bool load(context& ctx, shader_program& prog, const std::string& name, const bool reload = false, const std::string& where = "") {

		return load(ctx, prog, name, shader_define_map(), reload, where);
	}

	static bool load(context& ctx, shader_program& prog, const std::string& name, const shader_define_map& defines, const bool reload = false, const std::string& where = "") {

		if(prog.is_created()) {
			if(reload) prog.destruct(ctx);
			else return true;
		}

		std::string function_context = where == "" ? "shader_library::load_shader()" : where;

		if(!prog.is_created()) {
			bool from_program_file = name.length() > 4 && name.substr(name.length() - 5) == ".glpr";

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

	bool reload_all(context& ctx, const std::string& where = "");
};
}
}

#include <cgv/config/lib_end.h>
