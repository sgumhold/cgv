#include <cgv/base/base.h>
#include "shader_code.h"
#include <cgv/base/register.h>
#include <cgv/base/import.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/ppp/ph_processor.h>
#include <cgv/utils/dir.h>
#include <cgv/utils/file.h>
#include <cgv/type/variant.h>

#ifdef WIN32
#pragma warning(disable:4996)
#endif

using namespace cgv::base;
using namespace cgv::type;
using namespace cgv::utils;

namespace cgv {
	namespace render {

std::map<std::string, std::string> shader_code::code_cache;

std::map<std::string, std::string> shader_code::shader_file_name_map;

bool shader_code::shader_file_name_map_initialized = false;

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
				std::string(getenv("CGV_DIR")) + "/libs/holo_disp;" +
				std::string(getenv("CGV_DIR")) + "/plugins/examples";
	}
	return config;
}

void shader_code::decode_if_base64(std::string& content) {
//#ifdef _WIN32
	if (!content.empty()) {
		// test if the first character is equal to the base64 prefix (ANSI 'paragraph' char with hexcode A7)
		if(content[0] == char(0xA7))
			content = decode_base64(content.substr(1));
	}
/*#else
	if (content.size() > 1) {
		if ((uint8_t&)content[0]==0xC2 && (uint8_t&)content[1]==0xA7) // UTF-8 for '§'
			content = decode_base64(content.substr(2));
		else if (
			content.size() > 2 &&
			(int)content[0] == -17 &&
			(int)content[1] == -65 &&
			(int)content[2] == -67) {
			content = decode_base64(content.substr(3));
		}
	}
#endif*/
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

std::string shader_code::find_file(const std::string& file_name, bool search_exhaustive)
{
	if (file_name.substr(0, 6) == "str://" || file_name.substr(0, 6) == "res://") {
		std::map<std::string, resource_file_info>::const_iterator it = ref_resource_file_map().find(file_name.substr(6));
		if (it != ref_resource_file_map().end())
			return file_name;
		else
			return "";
	}
	if (file::exists(file_name))
		return file_name;
	
	std::string try_name = file::get_path(ref_prog_name()) + "/" + file_name;
	if (file::exists(try_name))
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
		if (file::exists(try_name))
			return try_name;
		try_name = file::get_path(ref_prog_name()) + "/glsl/" + file_name;
		if (file::exists(try_name))
			return try_name;
		return "";
	}

	if(!shader_file_name_map_initialized) {
		std::string path_list = get_shader_config()->shader_path;

		size_t pos = 0;
		do {
			size_t end_pos = path_list.find_first_of(';', pos);
			std::string path;
			if(end_pos == std::string::npos) {
				path = path_list.substr(pos);
				pos = path_list.length();
			} else {
				path = path_list.substr(pos, end_pos - pos);
				pos = end_pos + 1;
			}
			
			std::vector<std::string> file_names;
			dir::glob(path, file_names, "*gl*", true);

			for(const auto& file_name : file_names) {
				std::string ext = file::get_extension(file_name);
				if(ext.length() > 2) {
					if(ext[0] == 'g' && ext[1] == 'l' ||
					   ext[0] == 'p' && ext[1] == 'g' && ext[2] == 'l')
						shader_file_name_map.emplace(file::get_file_name(file_name), file_name);
				}
			}
		} while(pos < path_list.length());

		shader_file_name_map_initialized = true;
	}

	std::map<std::string, std::string>::const_iterator file_name_map_it = shader_file_name_map.find(file_name);
	if(file_name_map_it != shader_file_name_map.end()) {
		try_name = file_name_map_it->second;
		if(file::exists(try_name))
		   return try_name;
	} else if(!search_exhaustive) {
		return "";
	}

	return file::find_in_paths(file_name, get_shader_config()->shader_path, true);
}

std::string shader_code::retrieve_code(const std::string& file_name, bool use_cache, std::string* _last_error) {

	std::string source = "";

	if(use_cache) {
		auto it = code_cache.find(file_name);
		if(it != code_cache.end()) {
			source = it->second;
		}
	} else {
		code_cache.clear();
	}

	if(source.empty())
		source = read_code_file(file_name, _last_error);

	if(use_cache)
		code_cache.emplace(file_name, source);

	return source;
}

ShaderType shader_code::detect_shader_type(const std::string& file_name)
{
	std::string ext = to_lower(file::get_extension(file_name));
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

std::string shader_code::resolve_includes(const std::string& source, bool use_cache, std::set<std::string>& included_file_names, std::string* _last_error)
{
	const std::string identifier = "#include ";

	std::string resolved_source = "";

	std::vector<line> lines;
	split_to_lines(source, lines);

	for(size_t i = 0; i < lines.size(); ++i) {
		std::string current_line = to_string(lines[i]);

		// search for the include identifier
		size_t identifier_pos = current_line.find(identifier);
		if(identifier_pos != std::string::npos) {
			// remove identifier and all content before; an include directive must be the first and only statement on a line
			current_line.erase(0, identifier_pos + identifier.length());
			
			// trim whitespace
			trim(current_line);

			if(current_line.length() > 0) {
				// remove quotation marks, leaving only the include path
				std::string include_file_name = current_line.substr(1, current_line.length() - 2);
				std::string include_source = retrieve_code(include_file_name, use_cache, _last_error);

				// check whether this file was already included and skip if this is the case
				if(included_file_names.find(include_file_name) == included_file_names.end()) {
					included_file_names.insert(include_file_name);
					current_line = resolve_includes(include_source, use_cache, included_file_names, _last_error);
				} else {
					current_line = "";
				}
			}

			// skip this line if nothing needs to be included (removes the include statement from the code)
			if(current_line == "")
				continue;
		}

		resolved_source += current_line + '\n';
	}

	return resolved_source;
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

	decode_if_base64(source);

	if (file::get_extension(file_name)[0] == 'p') {
		std::string code;
		get_shader_config()->inserted_shader_file_names.clear();
		std::string paths = file::get_path(fn);
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

	// get source code from cache or read file
	std::string source = retrieve_code(file_name, ctx.is_shader_file_cache_enabled(), &last_error);

	source = resolve_includes(source, ctx.is_shader_file_cache_enabled());

	if(!defines.empty())
		set_defines(source, defines);
	
	if (st == ST_VERTEX && ctx.get_gpu_vendor_id() == GPUVendorID::GPU_VENDOR_AMD)
		set_vertex_attrib_locations(source);

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
		token tok;
		int location = -1;
		std::string type = "";
		std::string name = "";

		vertex_attribute(const token& tok) : tok(tok) {}

		std::string to_string() {
			std::string str = "layout (location = ";
			str += std::to_string(location);
			str += ") in ";
			str += type + " ";
			str += name + ";";
			return str;
		}
	};

	std::vector<token> parts;
	std::vector<token> tokens;
	std::vector<vertex_attribute> attribs;
	std::vector<bool> attrib_flags;
	size_t version_idx = 0;
	int version_number = 0;
	bool is_core = false;
	bool no_upgrade = false;

	source = strip_cpp_comments(source);

	split_to_tokens(source, parts, "", true, "", "", ";\n");

	attrib_flags.resize(parts.size(), false);

	size_t part_idx = 0;

	// First read the version. The only thing allowed before the version statement is comments and empty lines.
	// Both get removed bevore splitting the source into parts, so the first part must be the version.
	split_to_tokens(parts[part_idx], tokens, "", true, "", "", " \t");

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
	split_to_tokens(parts[part_idx], tokens, "", true, "", "", " \t");

	if(tokens.size() > 1 && tokens[0] == "#define") {
		if(tokens[1] == "NO_UPGRADE") {
			no_upgrade = true;
			++part_idx;
		}
	}

	// return if the shader should not be upgraded
	if(no_upgrade)
		return;

	// now get all vertex attributes
	for(; part_idx < parts.size(); ++part_idx) {
		auto& tok = parts[part_idx];

		while(tok.begin < tok.end && is_space(*tok.begin))
			++tok.begin;

		if(tok.size() > 2) {
			int parentheses_count = 0;
			bool is_new_word = true;
			bool was_new_word = true;

			for(unsigned i = 0; i < tok.size() - 2; ++i) {
				char c = tok[i];

				if(c == '(')
					++parentheses_count;

				if(c == ')')
					--parentheses_count;

				is_new_word = is_space(c) || c == ')';

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
		split_to_tokens(attrib.tok, tokens, "", true, "", "", " \t()");

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
