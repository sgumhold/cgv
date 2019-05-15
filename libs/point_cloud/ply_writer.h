#pragma once 

#include <vector>
#include <cgv/math/fvec.h>
#include <cgv/media/color.h>
#include "ply.h"

#include "lib_begin.h"

enum PlyFileFormat { 
	PFF_ASCII, 
	PFF_BINARY_BIG_ENDIAN, 
	PFF_BINARY_LITTLE_ENDIAN 
};

template <typename ta_coord_type>
class ply_writer
{
public:
	typedef ta_coord_type coord_type;
	typedef cgv::math::fvec<coord_type,3> Pnt;
	typedef cgv::math::fvec<coord_type,3> Nml;
	typedef cgv::media::color<unsigned char,cgv::media::RGB,cgv::media::OPACITY> Clr;
protected:
	/// keep pointer to ply file
	void* ply_file;
	/// store a dummy normal
	static Nml dummy_normal;
	static Clr dummy_color;
	///
	bool have_vertex_normals;
	///
	bool have_vertex_colors;
	/// store whether we are writing vertices or faces
	enum WriteMode { WM_NONE, WM_VERTEX, WM_FACE } write_mode;
public:
	/// standard construction
	ply_writer();
	/// open ply file for write
	bool open(const std::string& file_name, unsigned int nr_vertices, 
				 unsigned int nr_faces, bool vertex_normals = false, 
				 bool vertex_colors = false, PlyFileFormat format = PFF_BINARY_BIG_ENDIAN
				);
	/// close the ply file
	void close();
	/// write one vertex
	void write_vertex(const Pnt& pt, const Nml& nml = dummy_normal, const Clr& clr = dummy_color);
	/// write a triangle given by its three vertex indices
	void write_triangle(int* vis);
	/// write a polygon given by its degree and the vertex indices
	void write_polygon(unsigned char degree, int* vis);
};

#if _MSC_VER > 1400
#pragma warning(disable:4275)
#pragma warning(disable:4231)
#pragma warning(disable:4251)
#pragma warning(disable:4661)
CGV_TEMPLATE template class CGV_API ply_writer<unsigned char>;
CGV_TEMPLATE template class CGV_API ply_writer<unsigned short>;
CGV_TEMPLATE template class CGV_API ply_writer<unsigned int>;
CGV_TEMPLATE template class CGV_API ply_writer<char>;
CGV_TEMPLATE template class CGV_API ply_writer<short>;
CGV_TEMPLATE template class CGV_API ply_writer<int>;
CGV_TEMPLATE template class CGV_API ply_writer<float>;
CGV_TEMPLATE template class CGV_API ply_writer<double>;
#define _PLY_WRITER__MSC_TEMPLATES_DEFINED 1
#endif

#include <cgv/config/lib_end.h>
