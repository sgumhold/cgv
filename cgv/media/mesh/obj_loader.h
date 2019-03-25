#pragma once

#include "obj_reader.h"
#include <map>
#include <set>
#include <cgv/math/fvec.h>

#include <cgv/media/lib_begin.h>

namespace cgv {
	namespace media {
		namespace mesh {

/** simple structure to describe a face */
struct CGV_API face_info
{
	/// degree of face
	unsigned degree;
	/// index into vertex index array
	unsigned first_vertex_index;
	/// index into texcoord index array or -1 if not specified
	int first_texcoord_index;
	/// index into normal index array or -1 if not specified
	int first_normal_index;
	/// index of group to which the face belongs
	int      group_index;
	/// material index to which the face belongs
	int      material_index;
	/// construct face info
	face_info(unsigned _nr = 0, unsigned _vi0 = 0, int _ti0 = -1, int _ni0 = -1, unsigned gi=-1, unsigned mi=-1); 
};

/** simple structure to describe a group*/
struct group_info
{
	/// name of the group
	std::string name;
	/// parameters string
	std::string parameters;
};

/** implements the virtual interface of the obj_reader and stores all 
	read information. The read information is automatically stored in 
	binary form to accelerate the second loading of the same obj file. */
template <typename T>
class CGV_API obj_loader_generic : public obj_reader_generic<T>
{
public:
	// We need to repeat the typenames here, because gcc will not inherit them in templates
	typedef typename obj_reader_generic<T>::v3d_type v3d_type;
	typedef typename obj_reader_generic<T>::v2d_type v2d_type;
	typedef obj_reader_base::color_type color_type;
	
	std::vector<v3d_type> vertices; 
    std::vector<v3d_type> normals; 
    std::vector<v2d_type> texcoords;
	std::vector<color_type> colors;

	std::vector<unsigned> vertex_indices;
	std::vector<unsigned> normal_indices;
	std::vector<unsigned> texcoord_indices;
	
	std::vector<face_info> faces; 
	std::vector<group_info> groups;
	std::vector<cgv::media::illum::obj_material> materials;
protected:
	/**@name virtual interface*/
	//@{
	/// overide this function to process a vertex
	void process_vertex(const v3d_type& p);
	/// overide this function to process a texcoord
	void process_texcoord(const v2d_type& t);
	/// overide this function to process a normal
	void process_normal(const v3d_type& n);
	/// overide this function to process a color (this called for vc prefixes which is is not in the standard but for example used in pobj-files)
	void process_color(const color_type& c);
	/// overide this function to process a face
	void process_face(unsigned vcount, int *vertices, 
					  int *texcoords = 0, int *normals=0);
	/// overide this function to process a group given by name
	void process_group(const std::string& name, const std::string& parameters);
	/// process a material definition
	void process_material(const cgv::media::illum::obj_material& mtl, unsigned idx);
	//@}
public:
	/// overloads reading to support binary file format
	bool read_obj(const std::string& file_name);
	/// read a binary version of an obj file
	bool read_obj_bin(const std::string& file_name);
	/// write the information from the last read obj file in binary format
	bool write_obj_bin(const std::string& file_name) const;
	/// use this after reading to show status information about the number of read entities
	void show_stats() const;
	/// prepare for reading another file
	void clear();
};


typedef obj_loader_generic<float>  obj_loaderf;
typedef obj_loader_generic<double> obj_loaderd;
typedef obj_loader_generic<double> obj_loader;

		}
	}
}

#include <cgv/config/lib_end.h>
