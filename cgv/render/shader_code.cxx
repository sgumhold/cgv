#include <cgv/base/base.h>
#include "shader_code.h"
#include <cgv/base/register.h>
#include <cgv/base/import.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/ppp/ph_processor.h>
#include <cgv/utils/file.h>
#include <cgv/type/variant.h>

#ifdef WIN32
#pragma warning(disable:4996)
#endif

using namespace cgv::type;
using namespace cgv::utils::file;
using namespace cgv::utils;
using namespace cgv::base;

namespace cgv {
	namespace render {

shader_config::shader_config()
{
	trace_file_names = false;
	show_file_paths = false;
}

std::string shader_config::get_type_name() const
{
	return "shader_config";
}

/// reflect the shader_path member
bool shader_config::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return 
		rh.reflect_member("shader_path", shader_path) &&
		rh.reflect_member("show_file_paths", show_file_paths);
}

/// return a reference to the current shader configuration
shader_config_ptr get_shader_config()
{
	static shader_config_ptr config;
	if (!config) {
		config = shader_config_ptr(new shader_config); 
		if (getenv("CGV_SHADER_PATH"))
			config->shader_path = getenv("CGV_SHADER_PATH");
	}
	return config;
}

/** query the last error in a way that developer environments can 
    locate errors in the source file */
std::string shader_code::get_last_error(const std::string& file_name, const std::string& last_error)
{
	std::string fn = shader_code::find_file(file_name);
	std::vector<line> lines;
	split_to_lines(last_error, lines);
	std::string formated_error;
	for (unsigned int i = 0; i<lines.size(); ++i) {
		std::string l = to_string((const token&)(lines[i]));
		if (to_upper(l.substr(0, 5)) == "ERROR") {
			std::vector<token> toks;
			tokenizer(l).set_sep(":").set_ws("").bite_all(toks);
			formated_error += fn + "(";
			if (toks.size() > 4) {
				formated_error += to_string(toks[4])+") : error G0001: ";
				if (toks.size() > 6)
					formated_error += l.substr(toks[6].begin-&l[0]);
			}
			else {
				formated_error += "1) : error G0000: ";
				if (toks.size() > 2)
					formated_error += l.substr(toks[2].begin-&l[0]);
			}
			formated_error += "\n";
		}
		else {
			std::vector<token> toks;
			tokenizer(l).set_sep(":()").set_ws("").bite_all(toks);
			unsigned int i, j, k; int n;
			for (i=0; i<toks.size(); ++i) {
				if (to_string(toks[i]) == "(")
					break;
			}
			for (j=0; j<toks.size(); ++j) {
				if (to_string(toks[j]) == ")")
					break;
			}
			for (k=0; k<toks.size(); ++k) {
				if (to_string(toks[k]) == ":")
					break;
			}
			if (k < toks.size() && i < j && j < k && is_integer(toks[i].end, toks[j].begin, n)) {
				std::string _fn = fn;
				int shader_id;
				if (i > 0 && is_integer(toks[i-1].begin, toks[i-1].end, shader_id)) {
					if (shader_id >= 1000) {
						unsigned si = shader_id-1000;
						if (si < get_shader_config()->inserted_shader_file_names.size())
							_fn = get_shader_config()->inserted_shader_file_names[si];
					}
					else {
						if ((unsigned)shader_id < get_shader_config()->shader_file_names.size())
							_fn = get_shader_config()->shader_file_names[shader_id];
					}
				}
				formated_error += _fn + l.substr(toks[i].begin - &l[0])+"\n";
			}
			else
				formated_error += l+"\n";
		}
	}
	return formated_error;
}

///create shader a shader code object
shader_code::shader_code()
{
	st = ST_DETECT;
}
/// calls the destruct method
shader_code::~shader_code()
{
	if (ctx_ptr && ctx_ptr->make_current())
		destruct(*ctx_ptr);
	else
		if (handle)
			std::cerr << "shader code not destructed correctly" << std::endl;
}

/// destruct shader
void shader_code::destruct(const context& ctx)
{
	if (!handle)
		return;
	ctx.shader_code_destruct(*this);
	handle = 0;
}

std::string shader_code::find_file(const std::string& file_name)
{
	if (file_name.substr(0, 6) == "str://" || file_name.substr(0, 6) == "res://") {
		std::map<std::string, resource_file_info>::const_iterator it = ref_resource_file_map().find(file_name.substr(6));
		if (it != ref_resource_file_map().end())
			return file_name;
		else
			return "";
	}
	if (exists(file_name))
		return file_name;
	
	std::string try_name = cgv::utils::file::get_path(ref_prog_name()) + "/" + file_name;
	if (exists(try_name))
		return try_name;

	std::map<std::string, resource_file_info>::const_iterator it = 
		ref_resource_file_map().find(file_name);
	if (it != ref_resource_file_map().end()) {
		if (it->second.file_offset == -1)
			return std::string("str://") + file_name;
		else
			return std::string("res://") + file_name;
	}
	if (get_shader_config()->shader_path.empty()) {
		try_name = std::string("glsl/") + file_name;
		if (exists(try_name))
			return try_name;
		try_name = cgv::utils::file::get_path(ref_prog_name()) + "/glsl/" + file_name;
		if (exists(try_name))
			return try_name;
		return "";
	}
	return find_in_paths(file_name, get_shader_config()->shader_path, true);
}

ShaderType shader_code::detect_shader_type(const std::string& file_name)
{
	std::string ext = to_lower(get_extension(file_name));
	ShaderType st = ST_VERTEX;
	if (ext == "glfs" || ext == "pglfs")
		st = ST_FRAGMENT;
	else if (ext == "glgs" || ext == "pglgs")
		st = ST_GEOMETRY;
	else if (ext == "glcs" || ext == "pglcs")
		st = ST_COMPUTE;
	return st;
}

/// return the shader type of this code
ShaderType shader_code::get_shader_type() const
{
	return st;
}

/// read shader code from file and return string with content or empty string if read failed
std::string shader_code::read_code_file(const std::string &file_name, std::string* last_error)
{
	std::string source;
	std::string fn = find_file(file_name);
	if (fn.empty()) {
		if (last_error) {
			*last_error = "could not find shader file ";
			*last_error += file_name;
		}
		return "";
	}
	if (!cgv::base::read_data_file(fn, source, true)) {
		if (last_error) {
			*last_error = "could not read shader file ";
			*last_error += fn;
		}
		return "";
	}
	if (get_shader_config()->show_file_paths)
		std::cout << "read shader code <" << fn << ">" << std::endl;
	if (!source.empty() && source[0] == '§')
		source = cgv::utils::decode_base64(source.substr(1));
	if (get_extension(file_name)[0] == 'p') {
		std::string code;
		get_shader_config()->inserted_shader_file_names.clear();
		std::string paths = get_path(fn);
		if (!get_shader_config()->shader_path.empty())
			paths = paths+";"+get_shader_config()->shader_path;

		cgv::ppp::ph_processor php(paths, true);
		php.configure_insert_to_shader(&get_shader_config()->inserted_shader_file_names);
		if (!php.parse_string(source))
			return "";
		if (!php.process_to_string(code))
			return "";
		cgv::ppp::clear_variables();
		source = code;
	}
	return source;
}

/// read shader code from file
bool shader_code::read_code(const context& ctx, const std::string &file_name, ShaderType st, std::string defines)
{
	if (st == ST_DETECT)
		st = detect_shader_type(file_name);

	std::string source = read_code_file(file_name, &last_error);

	if(!defines.empty()) {
		set_defines(source, defines);
	}

	if (source.empty())
		return false;
	return set_code(ctx, source, st);
}

/// set shader code from string
bool shader_code::set_code(const context& ctx, const std::string &source, ShaderType _st)
{
	st = _st;
	destruct(ctx);
	ctx_ptr = &ctx;
	return ctx.shader_code_create(*this, st, source);
}

/// set shader code defines
void shader_code::set_defines(std::string& source, const std::string& defines) {

	size_t current, previous = 0;
	current = defines.find_first_of(';');
	std::vector<std::string> tokens;
	while(current != std::string::npos) {
		tokens.push_back(defines.substr(previous, current - previous));
		previous = current + 1;
		current = defines.find_first_of(';', previous);
	}
	tokens.push_back(defines.substr(previous, current - previous));

	for(size_t i = 0; i < tokens.size(); ++i) {
		std::string token = tokens[i];
		size_t pos = token.find('=');
		if(pos == std::string::npos)
			continue;

		std::string name = token.substr(0, pos);
		std::string value = token.substr(pos + 1);
		if(name.empty())
			continue;

		size_t define_pos = source.find("#define " + name);
		if(define_pos == std::string::npos)
			continue;

		size_t overwrite_pos = define_pos + 8 + name.length() + 1; // length of: #define <NAME><SINGLE_SPACE>
		std::string first_part = source.substr(0, overwrite_pos);
		size_t new_line_pos = source.find_first_of('\n', overwrite_pos);
		if(new_line_pos != std::string::npos) {
			std::string second_part = source.substr(new_line_pos);
			source = first_part + value + second_part;
			source += "";
		}
	}
}

///compile attached source; returns true if successful
bool shader_code::compile(const context& ctx)
{
	if (!ctx.shader_code_compile(*this)) {
		user_data = 0;
		return false;
	}
	int id = 1;
	user_data = (void*&) id;
	return true;
}

/// read shader code from file, compile and print error message if necessary
bool shader_code::read_and_compile(const context& ctx, const std::string &file_name, ShaderType st, bool show_error, std::string defines)
{
	if (!read_code(ctx,file_name,st,defines))
		return false;
	if (!compile(ctx)) {
		if (show_error)
			std::cerr << get_last_error(file_name, last_error).c_str() << std::endl;
		return false;
	}
	return true;
}

/// return whether shader has been compiled successfully
bool shader_code::is_compiled() const
{
	return user_data != 0;
}


	}
}

struct shader_config_registration
{
	shader_config_registration()
	{
		register_object(cgv::render::get_shader_config(), "register global shader config");
	}
};

shader_config_registration shader_config_registration_instance;