#pragma once

#include <set>
#include "element_traits.h"
#include "shader_code.h"
#include "textured_material.h"

namespace cgv {
	namespace media {
		namespace illum {
			class surface_material;
			class light_source;
		}
	}
}

#include "lib_begin.h"

namespace cgv {
	namespace utils {
		struct line;
	}
	namespace render {

/** a shader program combines several shader code fragments
    to a complete definition of the shading pipeline. */
class CGV_API shader_program : public shader_program_base
{
protected:
	/// maps used to cache program file contents and valid file names indexed by their respective file name
	static std::map<std::string, std::string> program_file_cache;
	static std::map<std::string, std::vector<std::string>> files_cache;

	bool show_code_errors : 1;
	bool linked : 1;
	bool state_out_of_date : 1;
	int  nr_attached_geometry_shaders : 13;

	std::vector<shader_code*> managed_codes;
	/// attach a list of files
	bool attach_files(const context& ctx, const std::vector<std::string>& file_names, const shader_define_map& defines = shader_define_map());
	/// ensure that the state has been set in the context
	void update_state(const context& ctx);
	/// common code necessary to open file
	static bool open_program_file(std::string& file_name, bool use_cache, std::string& content, std::vector<cgv::utils::line>& lines, std::string* last_error_ptr = 0);
public:
	
	
	/// resolve file name with shader_code::find_file and add file to list if found
	static bool collect_files_from_cache(const std::string& name, std::vector<std::string>& file_names, bool& added_files);


	/// resolve file name with shader_code::find_file and add file to list if found
	static bool collect_file(const std::string& file_name, bool use_cache, std::vector<std::string>& file_names);
	/** collect shader code files that extent the given base name. 
	    Returns true if at least one shader code file has been collected.*/
	static bool collect_files(const std::string& base_name, bool use_cache, std::vector<std::string>& file_names);
	/** collect shader code files from directory. If the directory does not exist
		 search it in the shader path of the shader configuration returned
		 by get_shader_config(). Returns true if at least one shader code 
		 file has been collected.*/
	static bool collect_dir(const std::string& dir_name, bool recursive, std::vector<std::string>& file_names);
	/** collect shader code files declared in a shader program file.
	    Program files have the extension glpr and contain lines of the form
		 command:argument. The following commands can be used
		 - file:file_name ... calls attach_file(ctx,file_name)
		 - vertex_file:file_name ... calls attach_file(ctx,file_name,ST_VERTEX)
		 - geometry_file:file_name ... calls attach_file(ctx,file_name,ST_GEOMETRY)
		 - fragment_file:file_name ... calls attach_file(ctx,file_name,ST_FRAGMENT)
		 - files:base_name ... calls attach_files(ctx,base_name)
		 - dir:dir_name ... calls attach_dir(ctx,dir_name,false)
		 - rec_dir:dir_name ... calls attach_dir(ctx,dir_name,true)
		 - program:file_name ... calls attach_program(file_name) recursively
		                         take care, not to generate cyclic includes.
		 - geometry_shader_info:input_type;output_type;max_output_count ... calls set_geometry_shader_info 
		           with input_type, output_type and max_output_count as arguments, where the primitive types
		           are one out of points,lines,lines_adjacency,line_strip,line_strip_adjacency,line_loop,
					  triangles,triangles_adjacency,triangle_strip,triangle_strip_adjacency,triangle_fan,
					  quads,quad_strip,polygon.
		 Returns true if at least one shader code file has been collected.*/
	static bool collect_program(const std::string& file_name, bool use_cache, std::vector<std::string>& file_names);
	/// return the maximum number of output vertices of a geometry shader
	static unsigned int get_max_nr_geometry_shader_output_vertices(const context& ctx);
	/** create empty shader program and set the option whether errors during 
	    shader code attachment should be printed to std::cerr */
	shader_program(bool _show_code_errors = false);
	/// call destruct method
	~shader_program();
	/// create the shader program
	bool create(const context& ctx);
	/// destruct shader program
	void destruct(const context& ctx);
	/// attach a compiled shader code instance that is managed outside of program
	bool attach_code(const context& ctx, const shader_code& code);
	/// detach a shader code 
	bool detach_code(const context& ctx, const shader_code& code);
	/// attach a shader code given as string and managed the created shader code object
	bool attach_code(const context& ctx, const std::string& source, ShaderType st);
	/// read shader code from file, compile and attach to program
	bool attach_file(const context& ctx, const std::string& file_name, ShaderType st = ST_DETECT, const shader_define_map& defines = shader_define_map());
	/// read shader code from files with the given base name, compile and attach them
	bool attach_files(const context& ctx, const std::string& base_name, const shader_define_map& defines = shader_define_map());
	/// collect shader code files from directory, compile and attach.
	bool attach_dir(const context& ctx, const std::string& dir_name, bool recursive);
	/// collect shader code files declared in shader program file, compile and attach them
	bool attach_program(const context& ctx, std::string file_name, bool show_error = false, const shader_define_map& defines = shader_define_map());
	/// find and parse all instance definitions in a shader program file
	static std::vector<shader_define_map> extract_instances(std::string file_name);
	/// link shaders to an executable program
	bool link(const context& ctx, bool show_error = false);
	/// return whether program is linked
	bool is_linked() const;
	/// successively calls create, attach_files and link.
	bool build_files(const context& ctx, const std::string& base_name, bool show_error = false, const shader_define_map& defines = shader_define_map());
	/// successively calls create, attach_dir and link.
	bool build_dir(const context& ctx, const std::string& dir_name, bool recursive = false, bool show_error = false);
	/// successively calls create, attach_program and link.
	bool build_program(const context& ctx, const std::string& file_name, bool show_error = false, const shader_define_map& defines = shader_define_map());
	/// configure the geometry shader, if count < 1 set it to get_max_nr_geometry_shader_output_vertices
	void set_geometry_shader_info(PrimitiveType input_type, PrimitiveType output_type, int max_output_count = 0);
	/// enable the shader program
	bool enable(context& ctx);
	/// disable shader program and restore fixed functionality
	bool disable(context& ctx);
	/// check whether program is currently enabled
	bool is_enabled() const { return shader_program_base::is_enabled; }
	/// query location index of an uniform
	int get_uniform_location(const context& ctx, const std::string& name) const;
	/// set a uniform of type material
	bool set_material_uniform(const context& ctx, const std::string& name, const cgv::media::illum::surface_material& material, bool generate_error = false);
	/// set a uniform of type textured_material
	bool set_textured_material_uniform(const context& ctx, const std::string& name, const textured_material& material, bool generate_error = false);
	/// set a uniform of type light source
	bool set_light_uniform(const context& ctx, const std::string& name, const cgv::media::illum::light_source& light, bool generate_error = false);
	/** Set the value of a uniform by name, where the type can be any of int, unsigned, float, vec<int>, vec<unsigned>,
	vec<float>, mat<float> and the vectors are of dimension 2,
	3 or 4 and the matrices of dimensions 2, 3 or 4. */
	template <typename T>
	bool set_uniform(const context& ctx, const std::string& name, const T& value, bool generate_error = false) {
		int loc = ctx.get_uniform_location(*this, name);
		if (loc == -1 && generate_error) {
			ctx.error(std::string("shader_program::set_uniform() uniform <") + name + "> not found", this);
			return false;
		}
		return ctx.set_uniform_void(*this, loc, element_descriptor_traits<T>::get_type_descriptor(value), element_descriptor_traits<T>::get_address(value));
	}
	/// set uniform array from array \c array where number elements can be derived from array through \c array_descriptor_traits; supported array types include cgv::math::vec and std::vector
	template <typename T>
	bool set_uniform_array(const context& ctx, const std::string& name, const T& array) {
		int loc = ctx.get_uniform_location(*this, name);
		if (loc == -1) {
			ctx.error(std::string("shader_program::set_uniform_array() uniform <") + name + "> not found", this);
			return false;
		}
		return ctx.set_uniform_array_void(*this, loc, array_descriptor_traits<T>::get_type_descriptor(array), array_descriptor_traits<T>::get_address(array), array_descriptor_traits<T>::get_nr_elements(array));
	}
	/// set uniform array from an array with \c nr_elements elements of type T pointed to by \c array
	template <typename T>
	bool set_uniform_array(const context& ctx, const std::string& name, const T* array, size_t nr_elements, bool generate_error = false) {
		int loc = ctx.get_uniform_location(*this, name);
		if (loc == -1 && generate_error) {
			ctx.error(std::string("shader_program::set_uniform_array() uniform <") + name + "> not found", this);
			return false;
		}
		return ctx.set_uniform_array_void(*this, loc, type_descriptor(element_descriptor_traits<T>::get_type_descriptor(array[0]), true), array, nr_elements);
	}
	/** Set the value of a uniform by name, where the type can be any of int, unsigned, float, vec<int>, vec<unsigned>,
	vec<float>, mat<float> and the vectors are of dimension 2,
	3 or 4 and the matrices of dimensions 2, 3 or 4. */
	template <typename T>
	bool set_uniform(const context& ctx, int loc, const T& value) {
		return ctx.set_uniform_void(*this, loc, element_descriptor_traits<T>::get_type_descriptor(value), element_descriptor_traits<T>::get_address(value));
	}
	/// set uniform array from array \c array where number elements can be derived from array through \c array_descriptor_traits; supported array types include cgv::math::vec and std::vector
	template <typename T>
	bool set_uniform_array(const context& ctx, int loc, const T& array) {
		return ctx.set_uniform_array_void(*this, loc, array_descriptor_traits<T>::get_type_descriptor(), array_descriptor_traits<T>::get_address(array), array_descriptor_traits<T>::get_nr_elements(array));
	}
	/// set uniform array from an array with \c nr_elements elements of type T pointed to by \c array
	template <typename T>
	bool set_uniform_array(const context& ctx, int loc, const T* array, size_t nr_elements) {
		return ctx.set_uniform_array_void(*this, loc, type_descriptor(element_descriptor_traits<T>::get_type_descriptor(array), true), array, nr_elements);
	}
	/// query location index of an attribute
	int get_attribute_location(const context& ctx, const std::string& name) const;
	/// set constant default value of a vertex attribute by attribute name, if name does not specify an attribute, an error message is generated
	template <typename T>
	bool set_attribute(const context& ctx, const std::string& name, const T& value) {
		int loc = ctx.get_attribute_location(*this, name);
		if (loc == -1) {
			ctx.error(std::string("shader_program::set_attribute() attribute <") + name + "> not found", this);
			return false;
		}
		return ctx.set_attribute_void(*this, loc, element_descriptor_traits<T>::get_type_descriptor(value), element_descriptor_traits<T>::get_address(value));
	}
	/// set constant default value of a vertex attribute by location index
	template <typename T>
	bool set_attribute(const context& ctx, int loc, const T& value) {
		if (loc == -1) {
			ctx.error("shader_program::set_attribute() called with loc=-1", this);
			return false;
		}
		return ctx.set_attribute_void(*this, loc, element_descriptor_traits<T>::get_type_descriptor(value), element_descriptor_traits<T>::get_address(value));
	}
};

	}
}

#include <cgv/config/lib_end.h>
