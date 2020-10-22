#include <cgv/base/base.h>
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
bool shader_program::attach_files(const context& ctx, const std::vector<std::string>& file_names, std::string defines)
{
	bool no_error = true;
	for (unsigned int i = 0; i < file_names.size(); ++i)
		no_error = attach_file(ctx, file_names[i], ST_DETECT, defines) && no_error;
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
	const char* exts[] = { "glvs", "glgs", "glfs", "glcs", "pglvs", "pglfs", "pglgs", "pglcs", 0 }; 
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
	if (!content.empty() && content[0] == '§')
		content = cgv::utils::decode_base64(content.substr(1));
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
	nr_attached_geometry_shaders = 0;
}

/// call destruct method
shader_program::~shader_program()
{
	if (ctx_ptr && ctx_ptr->make_current())
		destruct(*ctx_ptr);
	else
		if (handle != 0)
			std::cerr << "could not destruct shader program properly" << std::endl;
}

/// create the shader program
bool shader_program::create(const context& ctx)
{
	state_out_of_date = true;
	nr_attached_geometry_shaders = 0;
	if (ctx_ptr)
		destruct(*ctx_ptr);
	else
		if (handle)
			destruct(ctx);
	ctx_ptr = &ctx;
	return ctx.shader_program_create(*this);
}

/// attach a compiled shader code instance that is managed outside of program
bool shader_program::attach_code(const context& ctx, const shader_code& code)
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
	ctx.shader_program_attach(*this, code);
	if (code.get_shader_type() == ST_GEOMETRY)
		++nr_attached_geometry_shaders;
	return true;
}

/// detach a shader code 
bool shader_program::detach_code(const context& ctx, const shader_code& code)
{
	if (!handle) {
		last_error = "detach_code from shader program that was not created";
		return false;
	}
	if (!code.handle) {
		last_error = "attempt to detach_code that is not created to shader program";
		return false;
	}
	ctx.shader_program_detach(*this, code);
	if (code.get_shader_type() == ST_GEOMETRY)
		--nr_attached_geometry_shaders;
	return true;
}


/// attach a shader code given as string and managed the created shader code object
bool shader_program::attach_code(const context& ctx, const std::string& source, ShaderType st)
{
	shader_code* code_ptr = new shader_code;
	if (code_ptr->set_code(ctx,source,st) && code_ptr->compile(ctx)) {
		managed_codes.push_back(code_ptr);
		return attach_code(ctx, *code_ptr);
	}
	last_error = code_ptr->last_error;
	delete code_ptr;
	return false;
}


/// read shader code from file, compile and attach to program
bool shader_program::attach_file(const context& ctx, const std::string& file_name, ShaderType st, std::string defines)
{
	shader_code* code_ptr = new shader_code;
	if (!code_ptr->read_and_compile(ctx,file_name,st,show_code_errors,defines)) {
		last_error = code_ptr->last_error;
		delete code_ptr;
		return false;
	}
	managed_codes.push_back(code_ptr);
	return attach_code(ctx, *code_ptr);
}

/// read shader code from files with the given base name, compile and attach them
bool shader_program::attach_files(const context& ctx, const std::string& base_name, std::string defines)
{
	std::vector<std::string> file_names;
	if (!collect_files(base_name,file_names))
		return false;
	return attach_files(ctx, file_names, defines);
}
/// collect shader code files from directory, compile and attach.
bool shader_program::attach_dir(const context& ctx, const std::string& dir_name, bool recursive)
{
	std::vector<std::string> file_names;
	if (!collect_dir(dir_name,recursive,file_names))
		return false;
	return attach_files(ctx, file_names);
}

/// collect shader code files declared in shader program file, compile and attach them
bool shader_program::attach_program(const context& ctx, const std::string& file_name, bool show_error, std::string defines)
{
	std::string fn = shader_code::find_file(file_name);
	if (fn.empty()) {
		last_error = "could not find shader program file "+file_name;
		if (show_error)
			std::cerr << last_error << std::endl;
		return false;
	}
	std::string content;
	if (!cgv::base::read_data_file(fn, content, true)) {
		last_error = "could not read shader program file "+file_name;
		if (show_error)
			std::cerr << last_error << std::endl;
		return false;
	}
	if (!content.empty() && content[0] == '§')
		content = cgv::utils::decode_base64(content.substr(1));
	if (get_shader_config()->show_file_paths)
		std::cout << "read shader program <" << fn << ">" << std::endl;
	static std::vector<line> lines;
	lines.clear();
	split_to_lines(content, lines);
	std::string old_shader_path = get_shader_config()->shader_path;
	std::string path = file::get_path(file_name);
	if (!path.empty())
		get_shader_config()->shader_path = path+";"+get_shader_config()->shader_path; 

	bool no_error = true;
	std::string error = "2 : attach command failed";
	for (unsigned int i=0; i<lines.size(); ++i) {
		token tok = lines[i];
		while (tok.begin < tok.end && cgv::utils::is_space(*tok.begin))
			++tok.begin;
		std::string l = to_string(tok);

		bool success = true;
		// ignore empty lines
		if (l.empty())
			continue;
		// ignore comments
		if (l[0] == '/')
			continue;
		if (l.substr(0,5) == "file:")
			success = attach_file(ctx, l.substr(5), ST_DETECT, defines);
		else if (l.substr(0,12) == "vertex_file:")
			success = attach_file(ctx, l.substr(12), ST_VERTEX, defines);
		else if (l.substr(0,14) == "geometry_file:")
			success = attach_file(ctx, l.substr(14), ST_GEOMETRY, defines);
		else if (l.substr(0,14) == "fragment_file:")
			success = attach_file(ctx, l.substr(14), ST_FRAGMENT, defines);
		else if(l.substr(0, 14) == "compute_file:")
			success = attach_file(ctx, l.substr(14), ST_COMPUTE, defines);
		else if (l.substr(0,6) == "files:")
			success = attach_files(ctx, l.substr(6), defines);
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
	if (show_error && !no_error)
		std::cerr << last_error << std::endl;
	return no_error;
}

/// successively calls create, attach_files and link.
bool shader_program::build_files(const context& ctx, const std::string& base_name, bool show_error)
{
	return (is_created() || create(ctx)) && 
		    attach_files(ctx, base_name) && link(ctx, show_error);
}
/// successively calls create, attach_dir and link.
bool shader_program::build_dir(const context& ctx, const std::string& dir_name, bool recursive, bool show_error)
{
	return (is_created() || create(ctx)) && 
			 attach_dir(ctx, dir_name, recursive) && link(ctx, show_error);
}

/// successively calls create, attach_program and link.
bool shader_program::build_program(const context& ctx, const std::string& file_name, bool show_error, std::string defines)
{
	if (!(is_created() || create(ctx)))
		return false;
	if (!attach_program(ctx, file_name, show_error, defines))
		return false;
	if (!link(ctx, show_error)) {
		if (show_error) {
			std::string fn = shader_code::find_file(file_name);
			std::vector<line> lines;
			split_to_lines(last_error, lines);
			std::string formated_error;
			for (unsigned int i = 0; i < lines.size(); ++i) {
				formated_error += fn + "(1) : error G0002: " + to_string(lines[i]) + "\n";
			}
			std::cerr << formated_error.c_str() << std::endl;
		}
		return false;
	}
	return true;
}

/// return the maximum number of output vertices of a geometry shader
unsigned int shader_program::get_max_nr_geometry_shader_output_vertices(const context& ctx)
{
	return ctx.query_integer_constant(MAX_NR_GEOMETRY_SHADER_OUTPUT_VERTICES);
}

/// ensure that the state has been set in the context
void shader_program::update_state(const context& ctx)
{
	if (state_out_of_date) {
		if (nr_attached_geometry_shaders > 0) {
			if (geometry_shader_output_count < 1)
				geometry_shader_output_count = get_max_nr_geometry_shader_output_vertices(ctx);
			ctx.shader_program_set_state(*this);
		}
		state_out_of_date = false;
	}
}
///link shaders to an executable program
bool shader_program::link(const context& ctx, bool show_error)
{
	update_state(ctx);
	if (ctx.shader_program_link(*this)) {
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
		ctx.error("attempt to enable shader_program that is not created", this);
		return false;
	}
	if (!is_linked()) {
		ctx.error("attempt to enable shader_program that is not linked", this);
		return false;
	}
	if (is_enabled()) {
		ctx.error("attempt to enable shader_program that is already enabled or was not disabled properly", this);
		return false;
	}
	update_state(ctx);
	bool res = ctx.shader_program_enable(*this);
	if (res)
		shader_program_base::is_enabled = true;
	return res;
}

/// disable shader program and restore fixed functionality
bool shader_program::disable(context& ctx)
{
	if (!is_enabled()) {
		ctx.error("attempt to disable shader_program that is not enabled", this);
		return false;
	}
	bool res = ctx.shader_program_disable(*this);
	shader_program_base::is_enabled = false;
	return res;
}

/// query location index of an uniform
int shader_program::get_uniform_location(const context& ctx, const std::string& name) const
{
	return ctx.get_uniform_location(*this, name);
}
/// set a uniform of type material
bool shader_program::set_material_uniform(const context& ctx, const std::string& name, const cgv::media::illum::surface_material& material, bool generate_error)
{
	return
		set_uniform(ctx, name + ".brdf_type", (int)material.get_brdf_type(), generate_error) &&
		set_uniform(ctx, name + ".diffuse_reflectance", material.get_diffuse_reflectance(), generate_error) &&
		set_uniform(ctx, name + ".roughness", material.get_roughness(), generate_error) &&
		set_uniform(ctx, name + ".ambient_occlusion", material.get_ambient_occlusion(), generate_error) &&
		set_uniform(ctx, name + ".emission", material.get_emission(), generate_error) &&
		set_uniform(ctx, name + ".specular_reflectance", material.get_specular_reflectance(), generate_error) &&
		set_uniform(ctx, name + ".roughness_anisotropy", material.get_roughness_anisotropy(), generate_error) &&
		set_uniform(ctx, name + ".roughness_orientation", material.get_roughness_orientation(), generate_error) &&
		set_uniform(ctx, name + ".propagation_slow_down", cgv::math::fvec<float, 2>(material.get_propagation_slow_down().real(), material.get_propagation_slow_down().imag()), generate_error) &&
		set_uniform(ctx, name + ".transparency", material.get_transparency(), generate_error) &&
		set_uniform(ctx, name + ".metalness", material.get_metalness(), generate_error);
}

/// set a uniform of type textured_material
bool shader_program::set_textured_material_uniform(const context& ctx, const std::string& name, const textured_material& material, bool generate_error)
{
	const char* texture_names[] = {
		"tex0", "tex1", "tex2", "tex3"
	};
	for (int i = 0; i < (int)material.get_nr_image_files(); ++i)
		if (!set_uniform(ctx, texture_names[i], i, generate_error))
			return false;
	return
		set_material_uniform(ctx, name, material, generate_error) &&
		set_uniform(ctx, "sRGBA_textures", material.get_sRGBA_textures(), generate_error) &&
		set_uniform(ctx, "diffuse_index", material.get_diffuse_index(), generate_error) &&
		set_uniform(ctx, "roughness_index", material.get_roughness_index(), generate_error) &&
		set_uniform(ctx, "metalness_index", material.get_metalness_index(), generate_error) &&
		set_uniform(ctx, "ambient_index", material.get_ambient_index(), generate_error) &&
		set_uniform(ctx, "emission_index", material.get_emission_index(), generate_error) &&
		set_uniform(ctx, "transparency_index", material.get_transparency_index(), generate_error) &&
		set_uniform(ctx, "bump_index", material.get_bump_index(), generate_error) &&
		set_uniform(ctx, "specular_index", material.get_specular_index(), generate_error);
}



/// set a uniform of type light source
bool shader_program::set_light_uniform(const context& ctx, const std::string& name, const cgv::media::illum::light_source& L, bool generate_error)
{
	if (!set_uniform(ctx, name + ".light_source_type", static_cast<int>(L.get_type()), generate_error))
		return false;
	if (!set_uniform(ctx, name + ".position", L.get_position(), generate_error))
		return false;
	if (!set_uniform(ctx, name + ".emission", L.get_emission(), generate_error))
		return false;
	if (!set_uniform(ctx, name + ".ambient_scale", L.get_ambient_scale(), generate_error))
		return false;
	if (!set_uniform(ctx, name + ".spot_direction", L.get_spot_direction(), generate_error))
		return false;
	if (!set_uniform(ctx, name + ".spot_exponent", L.get_spot_exponent(), generate_error))
		return false;
	if (!set_uniform(ctx, name + ".spot_cos_cutoff", cos(0.01745329252f*L.get_spot_cutoff()), generate_error))
		return false;
	if (!set_uniform(ctx, name + ".constant_attenuation", L.get_constant_attenuation(), generate_error))
		return false;
	if (!set_uniform(ctx, name + ".linear_attenuation", L.get_linear_attenuation(), generate_error))
		return false;
	if (!set_uniform(ctx, name + ".quadratic_attenuation", L.get_quadratic_attenuation(), generate_error))
		return false;
	return true;
}

/// query location index of an attribute
int shader_program::get_attribute_location(const context& ctx, const std::string& name) const
{
	return ctx.get_attribute_location(*this, name);
}

/// destruct shader program
void shader_program::destruct(const context& ctx)
{
	while (managed_codes.size() > 0) {
		delete managed_codes.back();
		managed_codes.pop_back();
	}
	if (handle) {
		ctx.shader_program_destruct(*this);
		handle = 0;
	}
	linked = false;
	state_out_of_date = true;
	nr_attached_geometry_shaders = 0;
}

	}
}