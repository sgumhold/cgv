#include "shader_library.h"

namespace cgv {
namespace glutil {

shader_library::shader_library() {

	shaders.clear();
}

shader_library::~shader_library() {

	shaders.clear();
}

void shader_library::clear(context& ctx) {

	shaders.clear();
}

bool shader_library::add(const std::string& name, const std::string& file) {

	if(shaders.find(name) == shaders.end()) {
		shader_program prog;
		shader_program_pair elem = std::make_pair(prog, file);
		shaders.insert({ name, elem });
		return true;
	}
	return false;
}

bool shader_library::load_shaders(context& ctx) {

	bool success = true;
	for(auto& elem : shaders) {
		shader_program_pair& pair = elem.second;

		success &= load_shader(ctx, pair.first, pair.second);
	}

	return success;
}

/*bool shader_library::load_shader(context& ctx, shader_program& prog, const std::string& name, const bool reload, const std::string& defines, const std::string& where) {

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
			if(!prog.build_files(ctx, name, true)) {
				std::cerr << "ERROR in " << function_context << " ... could not build shader files " << name << std::endl;
				return false;
			}
		}
	}
	return true;
}*/

}
}
