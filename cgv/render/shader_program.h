#pragma once

#include <set>
#include "context.h"
#include "shader_code.h"
#include <cgv/math/vec.h>
#include <cgv/math/mat.h>
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/type/info/type_id.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {

		/// different offsets for compound types
		enum TypeIdOffset {
			/// base types
			TO_VEC = 0x01000, // cgv::math::vec<T> of standard type T
			TO_FVEC2 = 0x02000, // cgv::math::vec2<T,2> of standard type T
			TO_FVEC3 = 0x03000, // cgv::math::vec3<T,3> of standard type T
			TO_FVEC4 = 0x04000, // cgv::math::vec4<T,4> of standard type T
			TO_MAT = 0x05000, // cgv::math::mat<T> of standard type T
			TO_FMAT2 = 0x06000, // cgv::math::fmat<T,2,2> of standard type T
			TO_FMAT3 = 0x07000, // cgv::math::fmat<T,3,3> of standard type T
			TO_FMAT4 = 0x08000, // cgv::math::fmat<T,4,4> of standard type T
			TO_BASE_MASK = 0x0F000, // mask to filter out base type
			TO_DIM_OFFSET = 0x01000,

			// array types
			TO_VECTOR = 0x10000, // std::vector<T> where T is standard type or one of base types
			TO_VEC_OF = 0x20000, // cgv::math::vec<T> where T is standard type or one of base types
			TO_POINTER = 0x30000, // T*  where T is standard type or one of base types
			TO_ARRAY_MASK = 0xF0000, // mask to filter out array type
		};


		/// extend cgv::type::info::type_id<T> by vector and matrix types that can be used as arguments to set_uniform
		template <typename T>
		struct type_id_offset : public cgv::type::info::type_id<T>
		{
		};

		/// specialization for cgv::math::vec 
		template <typename T>
		struct type_id_offset<cgv::math::vec<T> >
		{
			static int get_id() {
				return cgv::type::info::type_id<T>::get_id() + TO_VEC;
			}
		};
		/// specialization for cgv::math::fvec
		template <typename T, unsigned i>
		struct type_id_offset<cgv::math::fvec<T, i> >
		{
			static int get_id() {
				return cgv::type::info::type_id<T>::get_id() + TO_FVEC2 + (i - 2)*TO_DIM_OFFSET;
			}
		};

		/// specialization for cgv::math::mat 
		template <typename T>
		struct type_id_offset<cgv::math::mat<T> >
		{
			static int get_id() {
				return cgv::type::info::type_id<T>::get_id() + TO_MAT;
			}
		};

		/// specialization for cgv::math::fmat
		template <typename T, unsigned i>
		struct type_id_offset<cgv::math::fmat<T, i, i> >
		{
			static int get_id() {
				return cgv::type::info::type_id<T>::get_id() + TO_FMAT2 + (i - 2)*TO_DIM_OFFSET;
			}
		};

		/// specialization for std::vector
		template <typename T>
		struct type_id_offset<std::vector<T> >
		{
			static int get_id() {
				return type_id_offset<T>::get_id() + TO_VECTOR;
			}
		};

		/// specialization for nested cgv::math::vec types [cgv::math::vec]
		template <typename T>
		struct type_id_offset<cgv::math::vec<cgv::math::vec<T> > >
		{
			static int get_id() {
				return type_id_offset<cgv::math::vec<T> >::get_id() + TO_VEC_OF;
			}
		};
		/// specialization for nested cgv::math::vec types [cgv::math::fvec]
		template <typename T, unsigned i>
		struct type_id_offset<cgv::math::vec<cgv::math::fvec<T, i> > >
		{
			static int get_id() {
				return type_id_offset<cgv::math::fvec<T, i> >::get_id() + TO_VEC_OF;
			}
		};

		/// specialization for nested cgv::math::vec types [cgv::math::vec]
		template <typename T>
		struct type_id_offset<cgv::math::vec<cgv::math::mat<T> > >
		{
			static int get_id() {
				return type_id_offset<cgv::math::mat<T> >::get_id() + TO_VEC_OF;
			}
		};
		/// specialization for nested cgv::math::vec types [cgv::math::fvec]
		template <typename T, unsigned i>
		struct type_id_offset<cgv::math::vec<cgv::math::fmat<T, i, i> > >
		{
			static int get_id() {
				return type_id_offset<cgv::math::fmat<T, i, i> >::get_id() + TO_VEC_OF;
			}
		};

		/// specialization for pointer
		template <typename T>
		struct type_id_offset<T*>
		{
			static int get_id() {
				return type_id_offset<T>::get_id() + TO_POINTER;
			}
		};

/** a shader program combines several shader code fragments
    to a complete definition of the shading pipeline. */
class CGV_API shader_program : public shader_program_base
{
protected:
	bool show_code_errors : 1;
	bool linked : 1;
	bool state_out_of_date : 1;
	bool is_enabled : 1;
	int  nr_attached_geometry_shaders : 13;

	std::vector<shader_code*> managed_codes;
	std::set<unsigned> attribute_locations;
	///
	bool add_attribute_location(context& ctx, int loc);
	/// attach a list of files
	bool attach_files(context& ctx, const std::vector<std::string>& file_names);
	/// ensure that the state has been set in the context
	void update_state(context& ctx);
public:
	/// resolve file name with shader_code::find_file and add file to list if found
	static bool collect_file(const std::string& file_name, std::vector<std::string>& file_names);
	/** collect shader code files that extent the given base name. 
	    Returns true if at least one shader code file has been collected.*/
	static bool collect_files(const std::string& base_name, std::vector<std::string>& file_names);
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
	static bool collect_program(const std::string& file_name, std::vector<std::string>& file_names);
	/// return the maximum number of output vertices of a geometry shader
	static unsigned int get_max_nr_geometry_shader_output_vertices(context& ctx);
	/** create empty shader program and set the option whether errors during 
	    shader code attachment should be printed to std::cerr */
	shader_program(bool _show_code_errors = false);
	/// call destruct method
	~shader_program();
	/// create the shader program
	bool create(context& ctx);
	/// attach a compiled shader code instance that is managed outside of program
	bool attach_code(context& ctx, const shader_code& code);
	/// detach a shader code 
	bool detach_code(context& ctx, const shader_code& code);
	/// attach a shader code given as string and managed the created shader code object
	bool attach_code(context& ctx, const std::string& source, ShaderType st);
	/// read shader code from file, compile and attach to program
	bool attach_file(context& ctx, const std::string& file_name, ShaderType st = ST_DETECT);
	/// read shader code from files with the given base name, compile and attach them
	bool attach_files(context& ctx, const std::string& base_name);
	/// collect shader code files from directory, compile and attach.
	bool attach_dir(context& ctx, const std::string& dir_name, bool recursive);
	/// collect shader code files declared in shader program file, compile and attach them
	bool attach_program(context& ctx, const std::string& file_name, bool show_error = false);
	/// link shaders to an executable program
	bool link(context& ctx, bool show_error = false);
	/// return whether program is linked
	bool is_linked() const;
	/// successively calls create, attach_files and link.
	bool build_files(context& ctx, const std::string& base_name, bool show_error = false);
	/// successively calls create, attach_dir and link.
	bool build_dir(context& ctx, const std::string& dir_name, bool recursive = false, bool show_error = false);
	/// successively calls create, attach_program and link.
	bool build_program(context& ctx, const std::string& file_name, bool show_error = false);
	/// configure the geometry shader, if count < 1 set it to get_max_nr_geometry_shader_output_vertices
	void set_geometry_shader_info(PrimitiveType input_type, PrimitiveType output_type, int max_output_count = 0);
	/// enable the shader program
	bool enable(context& ctx);
	/** Set the value of a uniform by name, where the type can be any of int, unsigned, float, vec<int>, vec<unsigned>,
	vec<float>, mat<float> and the vectors are of dimension 2,
	3 or 4 and the matrices of dimensions 2, 3 or 4. */
	template <typename T>
	bool set_uniform(context& ctx, const std::string& name, const T& value, bool dimension_independent = false) {
		return ctx.set_uniform_void(*this, name, type_id_offset<T>::get_id(), dimension_independent, &value);
	}
	/// Set a vertex attribute to a single value or an array of values through the cgv::math::vec or std::vector classes.
	template <typename T>
	bool set_attribute(context& ctx, const std::string& name, const T& value, bool force_array = false) {
		int loc = ctx.get_attribute_location(*this, name);
		if (loc == -1) {
			ctx.error(std::string("shader_program::set_attribute() attribute <") + name + "> not found", this);
			return false;
		}
		return ctx.set_attribute_void(*this, loc, type_id_offset<T>::get_id(), force_array, &value) && add_attribute_location(ctx, loc);
	}
	/// Set a vertex attribute to a single value or an array of values through the cgv::math::vec or std::vector classes.
	template <typename T>
	bool set_attribute_array(context& ctx, const std::string& name, const T* value, unsigned size = 0, unsigned stride = 0) {
		int loc = ctx.get_attribute_location(*this, name);
		if (loc == -1) {
			ctx.error(std::string("shader_program::set_attribute_array() attribute <") + name + "> not found", this);
			return false;
		}
		return ctx.set_attribute_void(*this, loc, type_id_offset<T*>::get_id(), false, value, stride, size) && add_attribute_location(ctx, loc);
	}
	/// disable shader program and restore fixed functionality
	bool disable(context& ctx);
	/// destruct shader program
	void destruct(context& ctx);
};

	}
}

#include <cgv/config/lib_end.h>
