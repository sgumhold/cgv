#pragma once

#include "context.h"
#include "shader_program.h"

#include "lib_begin.h"

namespace cgv {
namespace render {

/// provides a shader library that handles shader loading and stores shaders
class CGV_API shader_library {
	struct shader_info {
		std::string filename;
		shader_program prog;
		shader_compile_options options;
	};

	using shader_lib_map = std::map<std::string, shader_info>;

public:
	void clear(context& ctx);

	bool add(const std::string& name, const std::string& file, const shader_compile_options& options = {});

	bool contains(const std::string& name) const;

	shader_program& get(const std::string& name);

	shader_define_map& get_defines(const std::string& name);

	shader_compile_options& get_compile_options(const std::string& name);

	shader_lib_map::iterator begin() { return shaders.begin(); }
	shader_lib_map::iterator end() { return shaders.end(); }
	
	static bool load(context& ctx, shader_program& prog, const std::string& name, const shader_compile_options& options, const std::string& where = "");

	static bool load(context& ctx, shader_program& prog, const std::string& name, const std::string& where = "");

	bool load_all(context& ctx, const std::string& where = "");

	bool reload(context& ctx, const std::string& name, const shader_compile_options& options = {}, const std::string& where = "");

private:
	shader_info& get_shader_info(const std::string& name);

	shader_lib_map shaders;
};

} // namespace render
} // namespace cgv

#include <cgv/config/lib_end.h>
