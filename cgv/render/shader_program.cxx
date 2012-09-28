#include "shader_program.h"
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/base/import.h>

using namespace cgv::utils;

namespace cgv {
	namespace render {

/// attach a list of files
bool shader_program::attach_files(context& ctx, const std::vector<std::string>& file_names)
{
	bool no_error = true;
	for (unsigned int i = 0; i < file_names.size(); ++i)
		no_error = attach_file(ctx, file_names[i]) && no_error;
	return no_error;
}

bool shader_program::collect_file(const std::string& file_name, std::vector<std::string>& file_names)
{
	std::string fn = shader_code::find_file(file_name);
	if (!fn.empty()) {
		file_names.push_back(fn);
		return true;
	}
	return false;
}

bool shader_program::collect_files(const std::string& base_name, std::vector<std::string>& file_names)
{
	const char* exts[] = { "glvs", "glgs", "glfs", "pglvs", "pglfs", "pglgs", 0 }; 
	const char** iter = exts;
	bool added_file = false;
	while (*iter) {
		std::string fn = shader_code::find_file(base_name+"."+*iter);
		if (!fn.empty()) {
			file_names.push_back(fn);
			added_file = true;
		}
		++iter;
	}
	return added_file;
}

bool shader_program::collect_dir(const std::string& dir_name, bool recursive, std::vector<std::string>& file_names)
{
	std::string dn = dir_name;
	if (!dir::exists(dn)) {
		if (get_shader_config()->shader_path.empty()) {
			return false;
		}
		dn = file::find_in_paths(dir_name, get_shader_config()->shader_path, true);
		if (dn.empty()) {
			return false;
		}
	}
	void* handle = file::find_first(dn+"/*.gl*");
	if (!handle)
		return false;
	while (handle) {
		file_names.push_back(dir_name+"/"+file::find_name(handle));
		handle = file::find_next(handle);
	}
	return true;
}

bool shader_program::collect_program(const std::string& file_name, std::vector<std::string>& file_names)
{
	std::string fn = shader_code::find_file(file_name);
	if (fn.empty())
		return false;
	std::string content;
	if (!cgv::base::read_data_file(fn, content, true))
		return false;
	std::vector<line> lines;
	split_to_lines(content, lines);
	bool added_file = false;
	std::string old_shader_path = get_shader_config()->shader_path;
	std::string path = file::get_path(file_name);
	if (!path.empty())
		get_shader_config()->shader_path = path+";"+get_shader_config()->shader_path; 

	for (unsigned int i=0; i<lines.size(); ++i) {
		std::string l = to_string((const token&)lines[i]);
		if (l.substr(0,5) == "file:")
			added_file = collect_file(l.substr(5), file_names) || added_file;
		else if (l.substr(0,12) == "vertex_file:")
			added_file = collect_file(l.substr(12), file_names) || added_file;
		else if (l.substr(0,14) == "geometry_file:")
			added_file = collect_file(l.substr(14), file_names) || added_file;
		else if (l.substr(0,14) == "fragment_file:")
			added_file = collect_file(l.substr(14), file_names) || added_file;
		else if (l.substr(0,6) == "files:")
			added_file = collect_files(l.substr(6), file_names) || added_file;
		else if (l.substr(0,4) == "dir:")
			added_file = collect_dir(l.substr(4), false, file_names) || added_file;
		else if (l.substr(0,8) == "rec_dir:")
			added_file = collect_dir(l.substr(8), true, file_names) || added_file;
		else if (l.substr(0,8) == "program:")
			added_file = collect_program(l.substr(8), file_names) || added_file;
	}

	get_shader_config()->shader_path = old_shader_path;
	return added_file;
}	

///create empty shader program
shader_program::shader_program(bool _show_code_errors)
{
	show_code_errors = _show_code_errors;
	linked = false;
	state_out_of_date = true;
	has_geometry_shader = false;
}

/// call destruct method
shader_program::~shader_program()
{
	if (ctx_ptr)
		destruct(*ctx_ptr);
	else
		if (handle != 0)
			std::cerr << "could not destruct shader program properly" << std::endl;
}

/// create the shader program
bool shader_program::create(context& ctx)
{
	state_out_of_date = true;
	has_geometry_shader = false;
	if (ctx_ptr)
		destruct(*ctx_ptr);
	else
		if (handle)
			destruct(ctx);
	ctx_ptr = &ctx;
	return ctx.shader_program_create(handle, last_error);
}

/// attach a compiled shader code instance that is managed outside of program
bool shader_program::attach_code(context& ctx, const shader_code& code)
{
	if (!handle) {
		last_error = "attach_code to shader program that was not created";
		return false;
	}
	if (!code.handle) {
		last_error = "attempt to attach_code that is not created to shader program";
		return false;
	}
	if (!code.is_compiled()) {
		last_error = "attempt to attach_code that is not compiled to shader program";
		return false;
	}
	ctx.shader_program_attach(handle, code.handle);
	if (code.get_shader_type() == ST_GEOMETRY)
		has_geometry_shader = true;
	return true;
}

/// attach a shader code given as string and managed the created shader code object
bool shader_program::attach_code(context& ctx, const std::string& source, ShaderType st)
{
	shader_code* code_ptr = new shader_code;
	if (code_ptr->set_code(ctx,source,st) && code_ptr->compile(ctx)) {
		managed_codes.push_back(code_ptr);
		attach_code(ctx, *code_ptr);
		return true;
	}
	last_error = code_ptr->last_error;
	delete code_ptr;
	return false;
}


/// read shader code from file, compile and attach to program
bool shader_program::attach_file(context& ctx, const std::string& file_name, ShaderType st)
{
	shader_code* code_ptr = new shader_code;
	if (!code_ptr->read_and_compile(ctx,file_name,st,show_code_errors)) {
		last_error = code_ptr->last_error;
		delete code_ptr;
		return false;
	}
	managed_codes.push_back(code_ptr);
	attach_code(ctx, *code_ptr);
	return true;
}

/// read shader code from files with the given base name, compile and attach them
bool shader_program::attach_files(context& ctx, const std::string& base_name)
{
	std::vector<std::string> file_names;
	if (!collect_files(base_name,file_names))
		return false;
	return attach_files(ctx, file_names);
}
/// collect shader code files from directory, compile and attach.
bool shader_program::attach_dir(context& ctx, const std::string& dir_name, bool recursive)
{
	std::vector<std::string> file_names;
	if (!collect_dir(dir_name,recursive,file_names))
		return false;
	return attach_files(ctx, file_names);
}

/// collect shader code files declared in shader program file, compile and attach them
bool shader_program::attach_program(context& ctx, const std::string& file_name, bool show_error)
{
	std::string fn = shader_code::find_file(file_name);
	if (fn.empty()) {
		last_error = "could not find shader program file "+file_name;
		return false;
	}
	std::string content;
	if (!cgv::base::read_data_file(fn, content, true)) {
		last_error = "could not read shader program file "+file_name;
		return false;
	}	
	std::vector<line> lines;
	split_to_lines(content, lines);
	std::string old_shader_path = get_shader_config()->shader_path;
	std::string path = file::get_path(file_name);
	if (!path.empty())
		get_shader_config()->shader_path = path+";"+get_shader_config()->shader_path; 

	bool no_error = true;
	std::string error = "2 : attach command failed";
	for (unsigned int i=0; i<lines.size(); ++i) {
		std::string l = to_string((const token&)lines[i]);
		bool success = true;
		if (l.substr(0,5) == "file:")
			success = attach_file(ctx, l.substr(5));
		else if (l.substr(0,12) == "vertex_file:")
			success = attach_file(ctx, l.substr(12), ST_VERTEX);
		else if (l.substr(0,14) == "geometry_file:")
			success = attach_file(ctx, l.substr(14), ST_GEOMETRY);
		else if (l.substr(0,14) == "fragment_file:")
			success = attach_file(ctx, l.substr(14), ST_FRAGMENT);
		else if (l.substr(0,6) == "files:")
			success = attach_files(ctx, l.substr(6));
		else if (l.substr(0,4) == "dir:")
			success = attach_dir(ctx, l.substr(4), false);
		else if (l.substr(0,8) == "rec_dir:")
			success = attach_dir(ctx, l.substr(8), true);
		else if (l.substr(0,8) == "program:")
			success = attach_program(ctx, l.substr(8));
		else if (l.substr(0,21) == "geometry_shader_info:") {
			std::vector<token> toks;
			std::string l1 = l.substr(21);
			tokenizer(l1).set_ws(";").bite_all(toks);
			if (toks.size() == 3) {
				PrimitiveType i_pt = PT_UNDEF, o_pt = PT_UNDEF;
				int pi, count = 0;
				for (pi = PT_UNDEF+1; pi < PT_LAST; ++pi) {
					PrimitiveType pt = (PrimitiveType)pi;
					std::string s = to_string(pt);
					if (s == to_string(toks[0]))
						i_pt = pt;
					if (s == to_string(toks[1]))
						o_pt = pt;
				}
				if (i_pt == PT_UNDEF) {
					error = "4 : unknown input_type for geometry shader <";
					error += to_string(toks[0])+">";
					success = false;
				}
				else if (i_pt == PT_UNDEF) {
					error = "5 : unknown ouput_type for geometry shader <";
					error += to_string(toks[1])+">";
					success = false;
				}
				else if (!is_integer(toks[2].begin,toks[2].end,count)) {
					error = "6 : max_output_count of geometry shader must be an integer but received <";
					error += to_string(toks[2])+">";
					success = false;
				}
				else {
					set_geometry_shader_info(i_pt, o_pt, count);
				}
			}
			else {
				success = false;
				error = "3 : geometry_shader_info takes three arguments separated by colons";
			}
		} else {
			if (show_error) {
				std::cerr << fn.c_str() << " (" << i+1 
					       << "): warning G0001 : syntax error in line '" 
							 << l.c_str() << "'" << std::endl;
			}
		}
		if (!success) {
			if (show_error) {
				std::cerr << fn.c_str() << " (" << i+1 
					       << "): error G000" << error.c_str() << std::endl;
			}
			no_error = false;
		}
	}

	get_shader_config()->shader_path = old_shader_path;
	return no_error;
}

/// successively calls create, attach_files and link.
bool shader_program::build_files(context& ctx, const std::string& base_name, bool show_error)
{
	return (is_created() || create(ctx)) && 
		    attach_files(ctx, base_name) && link(ctx, show_error);
}
/// successively calls create, attach_dir and link.
bool shader_program::build_dir(context& ctx, const std::string& dir_name, bool recursive, bool show_error)
{
	return (is_created() || create(ctx)) && 
			 attach_dir(ctx, dir_name, recursive) && link(ctx, show_error);
}

/// successively calls create, attach_program and link.
bool shader_program::build_program(context& ctx, const std::string& file_name, bool show_error)
{
	return (is_created() || create(ctx)) && 
			 attach_program(ctx, file_name, show_error) && link(ctx, show_error);
}

/// return the maximum number of output vertices of a geometry shader
unsigned int shader_program::get_max_nr_geometry_shader_output_vertices(context& ctx)
{
	return ctx.query_integer_constant(MAX_NR_GEOMETRY_SHADER_OUTPUT_VERTICES);
}

/// ensure that the state has been set in the context
void shader_program::update_state(context& ctx)
{
	if (state_out_of_date) {
		if (has_geometry_shader) {
			if (geometry_shader_output_count < 1)
				geometry_shader_output_count = get_max_nr_geometry_shader_output_vertices(ctx);
			ctx.shader_program_set_state(*this);
		}
		state_out_of_date = false;
	}
}
///link shaders to an executable program
bool shader_program::link(context& ctx, bool show_error)
{
	update_state(ctx);
	if (ctx.shader_program_link(handle,last_error)) {
		linked = true;
		return true;
	}
	else {
		linked = false;
		if (show_error)
			std::cerr << "link error:\n" << last_error.c_str() << std::endl;
		return false;
	}
}
/// return whether program is linked
bool shader_program::is_linked() const
{
	return linked;
}

/// configure the geometry shader
void shader_program::set_geometry_shader_info(PrimitiveType input_type, PrimitiveType output_type, int max_output_count)
{
	geometry_shader_input_type = input_type;
	geometry_shader_output_type = output_type;
	geometry_shader_output_count = max_output_count;
	state_out_of_date = true;
}

/// enable the shader program
bool shader_program::enable(context& ctx)
{
	if (!is_created()) {
		last_error = "attempt to enable shader_program that is not created";
		return false;
	}
	if (!is_linked()) {
		last_error = "attempt to enable shader_program that is not linked";
		return false;
	}
	update_state(ctx);
	return ctx.shader_program_enable(*this);
}

/// disable shader program and restore fixed functionality
bool shader_program::disable(context& ctx)
{
	return ctx.shader_program_disable(*this);
}

/// destruct shader program
void shader_program::destruct(context& ctx)
{
	while (managed_codes.size() > 0) {
		delete managed_codes.back();
		managed_codes.pop_back();
	}
	if (handle) {
		ctx.shader_program_destruct(handle);
		handle = 0;
	}
	linked = false;
	state_out_of_date = true;
	has_geometry_shader = false;
}

	}
}