#pragma once

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
enum UniformTypeIdOffset {
	UTO_DIV         = 0x01000,
	UTO_VEC         = 0x01000,
	UTO_VECTOR      = 0x02000,
	UTO_MAT         = 0x03000,
	UTO_FVEC        = 0x04000, // offset for fvec<T,2 ..i.. 4>,   where i = (offset - UTO_FVEC) / UTO_DIV + 2
	UTO_FMAT        = 0x07000,  // offset for fmat<T,2 ..i.. 4,i>, where i = (offset - UTO_FMAT) / UTO_DIV + 2
	UTO_VECTOR_MAT  = 0x0A000,  // offset for std::vector<mat<T,2 ..i.. 4,i> >
	UTO_VECTOR_FMAT = 0x0B000,  // offset for std::vector<fmat<T,2 ..i.. 4,i> >, where i = (offset - UTO_VECTOR_FMAT) / UTO_DIV + 2
};

/// extend cgv::type::info::type_id<T> by vector and matrix types that can be used as arguments to set_uniform
template <typename T>
struct uniform_type_id : public cgv::type::info::type_id<T>
{
};

/// specialization for cgv::math::vec 
template <typename T>
struct uniform_type_id<cgv::math::vec<T> >
{
	static int get_id() {
		return cgv::type::info::type_id<T>::get_id() + UTO_VEC;
	}
};

/// specialization for std::vector
template <typename T>
struct uniform_type_id<std::vector<T> >
{
	static int get_id() {
		return cgv::type::info::type_id<T>::get_id() + UTO_VECTOR;
	}
};

/// specialization for cgv::math::mat 
template <typename T>
struct uniform_type_id<cgv::math::mat<T> >
{
	static int get_id() {
		return cgv::type::info::type_id<T>::get_id() + UTO_MAT;
	}
};

/// specialization for std::vector<cgv::math::mat>
// FIXME: Changed UTO_VEC_MAT to UTO_VECTOR_MAT as UTO_VEC_MAT is not defined.
//        Can we really do this?
template <typename T>
struct uniform_type_id<std::vector<cgv::math::mat<T> > >
{
	static int get_id() {
		return cgv::type::info::type_id<T>::get_id() + UTO_VECTOR_MAT;
	}
};

/// specialization for cgv::math::fvec
template <typename T, unsigned i>
struct uniform_type_id<cgv::math::fvec<T,i> >
{
	static int get_id() {
		return cgv::type::info::type_id<T>::get_id() + UTO_FVEC + (i - 2)*UTO_DIV;
	}
};

/// specialization for cgv::math::fmat
template <typename T, unsigned i>
struct uniform_type_id<cgv::math::fmat<T,i,i> >
{
	static int get_id() {
		return cgv::type::info::type_id<T>::get_id() + UTO_FMAT + (i - 2)*UTO_DIV;
	}
};

/// specialization for std::vector<cgv::math::fmat<T,i,i> >
// FIXME: Changed UTO_VEC_MAT to UTO_VECTOR_MAT as UTO_VEC_MAT is not defined.
//        Can we really do this?
template <typename T, unsigned i>
struct uniform_type_id<std::vector<cgv::math::fmat<T,i,i> > >
{
	static int get_id() {
		return cgv::type::info::type_id<T>::get_id() + UTO_VECTOR_FMAT + (i - 2)*UTO_DIV;
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
	bool has_geometry_shader : 1;

	std::vector<shader_code*> managed_codes;
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
	    Program files have the extension glsl and contain lines of the form
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
	/** Set the value of a uniform by name, where the type can be any of int, float, vec<int>, 
		 vec<float>, mat<float> and the vectors are of dimension 2, 
		 3 or 4 and the matrices of dimensions 2, 3 or 4. */
	template <typename T>
	bool set_uniform(context& ctx, const std::string& name, const T& value, bool dimension_independent = false) {
		return ctx.set_uniform_void(handle, name, uniform_type_id<T>::get_id(), dimension_independent,
											 &value, last_error);
	}
	/// disable shader program and restore fixed functionality
	bool disable(context& ctx);
	/// destruct shader program
	void destruct(context& ctx);
};

	}
}

#include <cgv/config/lib_end.h>
