#include "shader_library.h"

namespace cgv {
namespace render {

shader_library::shader_library() {

	shaders.clear();
}

shader_library::~shader_library() {

	shaders.clear();
}

void shader_library::clear(context& ctx) {

	shaders.clear();
}

bool shader_library::add(const std::string& name, const std::string& file, const shader_define_map& defines) {

	if(shaders.find(name) == shaders.end()) {
		shader_info elem;
		elem.filename = file;
		elem.defines = defines;
		shaders.insert({ name, elem });
		return true;
	}
	return false;
}

bool shader_library::load_all(context& ctx, const std::string& where) {

	bool success = true;
	for(auto& elem : shaders) {
		shader_info& si = elem.second;
		success &= load(ctx, si.prog, si.filename, si.defines, true, where);
	}

	return success;
}

bool shader_library::reload(context& ctx, const std::string& name, const shader_define_map& defines, const std::string& where) {
	
	auto it = shaders.find(name);
	if(it != shaders.end()) {
		shader_info& si = (*it).second;
		si.defines = defines;
		return load(ctx, si.prog, si.filename, si.defines, true, where);
	}
	return false;
}

bool shader_library::reload_all(context& ctx, const std::string& where) {

	bool success = true;
	for(auto& elem : shaders) {
		shader_info& si = elem.second;
		success &= load(ctx, si.prog, si.filename, si.defines, true, where);
	}

	return success;
}

}
}
