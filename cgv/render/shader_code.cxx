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
		else if (getenv("CGV_DIR"))
			config->shader_path = 
				std::string(getenv("CGV_DIR"))+"/libs/cgv_gl/glsl;"+
				std::string(getenv("CGV_DIR"))+"/libs/plot/glsl;"+
				std::string(getenv("CGV_DIR")) + "/libs/cgv_app/glsl;" +
				std::string(getenv("CGV_DIR")) + "/libs/cgv_g2d/glsl;" +
				std::string(getenv("CGV_DIR")) + "/libs/cgv_gpgpu/glsl;" +
				std::string(getenv("CGV_DIR")) + "/plugins/examples";
	}
	return config;
}

void shader_code::decode_if_base64(std::string& content) {
	// TODO: on some occasions the encoded string starts with other characters than § (why is this the case?)
#ifdef _WIN32
	if (!content.empty()) {
		// test if the first character is equal to the base64 prefix (ANSI 'paragraph' char with hexcode A7)
		if(content[0] == char(0xA7)) {
			content = cgv::utils::decode_base64(content.substr(1));
		}

		/*if(content[0] == '§')
			content = cgv::utils::decode_base64(content.substr(1));
		else if (
			content.size() > 2 &&
			(int)content[0] == -17 &&
			(int)content[1] == -65 &&
			(int)content[2] == -67) {
			content = cgv::utils::decode_base64(content.substr(3));
		}*/
	}
#else
	if (content.size() > 1) {
		if ((uint8_t&)content[0]==0xC2 && (uint8_t&)content[1]==0xA7) // UTF-8 for '§'
			content = cgv::utils::decode_base64(content.substr(2));
		else if (
			content.size() > 2 &&
			(int)content[0] == -17 &&
			(int)content[1] == -65 &&
			(int)content[2] == -67) {
			content = cgv::utils::decode_base64(content.substr(3));
		}
	}
#endif
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
	else if (ext == "gltc" || ext == "pgltc")
		st = ST_TESS_CONTROL;
	else if (ext == "glte" || ext == "pglte")
		st = ST_TESS_EVALUATION;
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
#if WIN32
	decode_if_base64(source);
#endif
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
bool shader_code::read_code(const context& ctx, const std::string &file_name, ShaderType st, const shader_define_map& defines)
{
	if (st == ST_DETECT)
		st = detect_shader_type(file_name);

	std::string source = read_code_file(file_name, &last_error);

	if(!defines.empty()) {
		set_defines(source, defines);
	}

	if (st == ST_VERTEX && ctx.get_gpu_vendor_id() == GPUVendorID::GPU_VENDOR_AMD) {
		set_vertex_attrib_locations(source);
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
void shader_code::set_defines(std::string& source, const shader_define_map& defines)
{
	for (const auto &entry : defines) {
		std::string name = entry.first;
		std::string value = entry.second;
		
		if(name.empty())
			continue;

		size_t define_pos = source.find("#define " + name);
		if(define_pos == std::string::npos)
			continue;

		size_t offset = 1;
		if(source[define_pos + 8 + name.length()] == '\n')
			offset = 0; // set search offset to zero if define has empty string as default value

		size_t overwrite_pos = define_pos + 8 + name.length() + offset; // length of: #define <NAME><SINGLE_SPACE/NO-SPACE0>
		std::string first_part = source.substr(0, overwrite_pos);
		size_t new_line_pos = source.find_first_of('\n', overwrite_pos);
		if(new_line_pos != std::string::npos) {
			std::string second_part = source.substr(new_line_pos);
			source = first_part + (offset == 0 ? " " : "") + value + second_part;
			source += "";
		}
	}
}

/// set shader code vertex attribute locations (a hotfix for AMD driver behaviour on vertex shaders)
void shader_code::set_vertex_attrib_locations(std::string& source)
{
	struct vertex_attribute {
		cgv::utils::token tok;
		int location = -1;
		std::string type = "";
		std::string name = "";

		vertex_attribute(const cgv::utils::token& tok) : tok(tok) {}

		std::string to_string() {
			std::string str = "layout (location = ";
			str += std::to_string(location);
			str += ") in ";
			str += type + " ";
			str += name + ";";
			return str;
		}
	};

	std::vector<cgv::utils::token> parts;
	std::vector<cgv::utils::token> tokens;
	std::vector<vertex_attribute> attribs;
	std::vector<bool> attrib_flags;
	size_t version_idx = 0;
	int version_number = 0;
	bool is_core = false;
	bool no_upgrade = false;

	source = cgv::utils::strip_cpp_comments(source);

	cgv::utils::split_to_tokens(source, parts, "", true, "", "", ";\n");

	attrib_flags.resize(parts.size(), false);

	size_t part_idx = 0;

	// First read the version. The only thing allowed before the version statement is comments and empty lines.
	// Both get removed bevore splitting the source into parts, so the first part must be the version.
	cgv::utils::split_to_tokens(parts[part_idx], tokens, "", true, "", "", " \t");

	if(tokens.size() > 1 && tokens[0] == "#version") {
		version_idx = part_idx;

		std::string number_str = to_string(tokens[1]);
		char* p_end;
		const long num = std::strtol(number_str.c_str(), &p_end, 10);
		if(number_str.c_str() != p_end)
			version_number = static_cast<int>(num);

		if(tokens.size() == 3 && tokens[2] == "core")
			is_core = true;
	}

	++part_idx;

	// Search for the optional NO_UPGRADE define, which must come directly after the version statement.
	tokens.clear();
	cgv::utils::split_to_tokens(parts[part_idx], tokens, "", true, "", "", " \t");

	if(tokens.size() > 1 && tokens[0] == "#define") {
		if(tokens[1] == "NO_UPGRADE") {
			no_upgrade = true;
			++part_idx;
		}
	}

	// return if the shader shall not be upgraded
	if(no_upgrade)
		return;

	// now get all vertex attributes
	for(part_idx; part_idx < parts.size(); ++part_idx) {
		auto& tok = parts[part_idx];

		while(tok.begin < tok.end && cgv::utils::is_space(*tok.begin))
			++tok.begin;

		if(tok.size() > 2) {
			int parentheses_count = 0;
			bool is_new_word = true;
			bool was_new_word = true;

			for(size_t i = 0; i < tok.size() - 2; ++i) {
				char c = tok[i];

				if(c == '(')
					++parentheses_count;

				if(c == ')')
					--parentheses_count;

				is_new_word = cgv::utils::is_space(c) || c == ')';

				if(c == 'i' && tok[i + 1] == 'n' && tok[i + 2] == ' ') {
					if(was_new_word && parentheses_count == 0) {
						attribs.push_back(vertex_attribute(tok));
						attrib_flags[part_idx] = true;
						break;
					}
				}

				was_new_word = is_new_word;
			}
		}
	}

	// return if no vertex attributes were found
	if(attribs.size() == 0)
		return;

	int max_location = -1;
	for(size_t i = 0; i < attribs.size(); ++i) {
		auto& attrib = attribs[i];

		tokens.clear();
		cgv::utils::split_to_tokens(attrib.tok, tokens, "", true, "", "", " \t()");

		size_t size = tokens.size();

		if(tokens.size() > 2) {
			// last two entries must be type and name
			attrib.type = to_string(tokens[size - 2]);
			attrib.name = to_string(tokens[size - 1]);

			// find location if present
			size_t equals_idx = -1;
			for(size_t j = 0; j < size; ++j) {
				auto& tok = tokens[j];
				if(tok.size() == 1 && tok[0] == '=')
					equals_idx = j;
			}

			if(equals_idx != -1 && equals_idx > 0 && equals_idx < size - 1) {
				if(to_string(tokens[equals_idx - 1]) == "location") {
					std::string val_str = to_string(tokens[equals_idx + 1]);
					char* p_end;
					const long num = std::strtol(val_str.c_str(), &p_end, 10);
					if(val_str.c_str() != p_end) {
						attrib.location = static_cast<int>(num);
						max_location = std::max(max_location, attrib.location);
					}
				}
			}
		}
	}

	for(size_t i = 0; i < attribs.size(); ++i) {
		auto& attrib = attribs[i];
		if(attrib.location < 0)
			attrib.location = ++max_location;
	}

	size_t attrib_idx = 0;
	size_t accumulate_offset = 0;
	size_t content_offset = reinterpret_cast<size_t>(source.data());

	for(size_t i = 0; i < parts.size(); ++i) {
		auto& tok = parts[i];

		if(i == version_idx || attrib_flags[i]) {
			size_t token_begin = reinterpret_cast<size_t>(tok.begin);
			size_t token_end = reinterpret_cast<size_t>(tok.end);

			size_t offset = token_begin - content_offset;
			size_t length = token_end - token_begin;

			std::string str = "";

			if(attrib_flags[i]) {
				auto& attrib = attribs[attrib_idx];
				++attrib_idx;
				str = attrib.to_string() + "\n";
			} else {
				version_number = std::max(version_number, 330);
				str = "#version " + std::to_string(version_number) + (is_core ? " core" : "");
			}

			offset += accumulate_offset;
			accumulate_offset += str.length() - length - 1;

			std::string first_part = source.substr(0, offset);
			std::string second_part = source.substr(offset + length + 1);

			source = first_part + str + second_part;
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
bool shader_code::read_and_compile(const context& ctx, const std::string &file_name, ShaderType st, bool show_error, const shader_define_map& defines)
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
