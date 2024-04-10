#pragma once

#include <cgv/math/fvec.h>
#include <cgv/media/illum/obj_material.hh>
#include <cgv/utils/tokenizer.h>
#include <set>
#include <map>
#include <vector>
#include <string>

#include <cgv/media/lib_begin.h>

namespace cgv {
	namespace media {
		namespace mesh {

/** base class for obj reader with implementation that is independent of coordinate type.*/
class CGV_API obj_reader_base
{
public:
	/// type used for rgba colors
	typedef illum::obj_material::color_type color_type;
protected:
	/// keep track of the current group
	unsigned group_index;
	/// number of groups
	unsigned nr_groups;
	/// keep track of current material index
	unsigned material_index;
	/// number of materials
	unsigned nr_materials;
	/// mapping from material names to material indices
	std::map<std::string, unsigned> material_index_lut;
	/**@name helpers for reading*/
	//@{
	/// parse a color, if alpha not given it defaults to 1
	color_type parse_color(const std::vector<cgv::utils::token>& t, unsigned off = 0) const;
	int minus;
	unsigned nr_normals, nr_texcoords;
	bool have_default_material;
	std::set<std::string> mtl_lib_files;
	void parse_face(const std::vector<cgv::utils::token>& tokens, bool is_line = false);
	void parse_material(const std::vector<cgv::utils::token>& tokens);
	virtual void parse_and_process_vertex(const std::vector<cgv::utils::token>& tokens) = 0;
	virtual void parse_and_process_normal(const std::vector<cgv::utils::token>& tokens) = 0;
	virtual void parse_and_process_texcoord(const std::vector<cgv::utils::token>& tokens) = 0;
	//@}

	/**@name status info during reading*/
	//@{
	/// store the path name
	std::string path_name;
	/// return the index of the currently selected group or -1 if no group is defined
	unsigned get_current_group() const;
	/// return the index of the currently selected material or -1 if no material is defined
	unsigned get_current_material() const;
	//@}
	/**@name virtual interface*/
	//@{
	/// overide this function to process a comment
	virtual void process_comment(const std::string& comment);
	/// overide this function to process a color (this called for vc prefixes which is is not in the standard but for example used in pobj-files)
	virtual void process_color(const color_type& c);
	/// convert negative indices to positive ones by adding the number of elements
	void convert_to_positive(unsigned vcount, int *vertices,
		int *texcoords, int *normals,
		unsigned v, unsigned n, unsigned t);
	/// overide this function to process a line strip, the indices start with 0
	virtual void process_line(unsigned vcount, int *vertices,
		int *texcoords = 0, int *normals = 0);
	/// overide this function to process a face, the indices start with 0
	virtual void process_face(unsigned vcount, int* vertices,
		int* texcoords = 0, int* normals = 0);
	/// overide this function to process a group given by name and parameter string
	virtual void process_group(const std::string& name, const std::string& parameters);
	/// process a material definition. If a material with a certain name is overwritten, it will receive the same index
	virtual void process_material(const cgv::media::illum::obj_material& mtl, unsigned idx);
	//@}
public:
	///
	obj_reader_base();
	/// parse the content of an obj file already read to memory, where path_name is used to find material files
	virtual bool parse_obj(const std::string& content, const std::string path_name = "");
	/// read an obj file
	virtual bool read_obj(const std::string& file_name);
	/// read a material file
	virtual bool read_mtl(const std::string& file_name);
	/// clear the reader such that a new file can be read
	virtual void clear();
};

/** implements the pure reading of an obj file and calls virtual callback
    functions to allow a derived class to handle the read information. 
	The default implementations of the processing methods are implemented
	to simply ignore the read information. */
template <typename T>
class CGV_API obj_reader_generic : public obj_reader_base
{
public:
	/// type of coordinates
	typedef T crd_type;
	/// type used to store texture coordinates
	typedef cgv::math::fvec<T,2> v2d_type;
	/// type used to store positions and normal vectors
	typedef cgv::math::fvec<T,3> v3d_type;
	///
	static bool is_double(const char* begin, const char* end, crd_type& value);
protected:
	/**@name template type dependent helpers for reading*/
	//@{
	/// parse 2d vector
	v2d_type   parse_v2d(const std::vector<cgv::utils::token>& t) const;
	/// parse 3d vector
	v3d_type   parse_v3d(const std::vector<cgv::utils::token>& t) const;
	///
	void parse_and_process_vertex(const std::vector<cgv::utils::token>& tokens);
	///
	void parse_and_process_normal(const std::vector<cgv::utils::token>& tokens);
	///
	void parse_and_process_texcoord(const std::vector<cgv::utils::token>& tokens);
	//@}

	/**@name virtual interface*/
	//@{
	/// overide this function to process a vertex
	virtual void process_vertex(const v3d_type& p);
	/// overide this function to process a texcoord
	virtual void process_texcoord(const v2d_type& t);
	/// overide this function to process a normal
	virtual void process_normal(const v3d_type& n);
	//@}
public:
	/// default constructor
	obj_reader_generic();
};

typedef obj_reader_generic<float>  obj_readerf;
typedef obj_reader_generic<double> obj_readerd;
typedef obj_reader_generic<double> obj_reader;

		}
	}
}

#include <cgv/config/lib_end.h>
