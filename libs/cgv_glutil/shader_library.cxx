#include "shader_library.h"

namespace cgv {
namespace glutil {

shader_library::shader_library() {

	shaders.clear();
}

shader_library::~shader_library() {

	shaders.clear();
}

void shader_library::clear(cgv::render::context& ctx) {

	shaders.clear();
}

bool shader_library::add(const std::string& name, const std::string& file) {

	if(shaders.find(name) == shaders.end()) {
		cgv::render::shader_program prog;
		shader_program_pair elem = std::make_pair(prog, file);
		shaders.insert({ name, elem });
		return true;
	}
	return false;
}

bool shader_library::load_shaders(cgv::render::context& ctx) {

	bool success = true;
	for(auto& elem : shaders) {
		shader_program_pair& pair = elem.second;

		success &= load(ctx, pair.first, pair.second);
	}

	return success;
}

bool shader_library::reload(cgv::render::context& ctx, const std::string& name, const cgv::render::shader_define_map& defines, const std::string& where) {
	
	auto it = shaders.find(name);
	if(it != shaders.end()) {
		shader_program_pair& pair = (*it).second;
		return load(ctx, pair.first, pair.second, defines, true, where);
	}
	return false;
}

}
}
