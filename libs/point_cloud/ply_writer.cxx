#include "ply_writer.h"

template <typename T>
struct PlyVertex 
{
  T x,y,z;
  T nx,ny,nz;
  unsigned char red, green, blue, alpha;
};

struct PlyFace 
{
  unsigned char nverts;
  int *verts;
};


template <typename T>
struct ply_traits {};

template <> struct ply_traits<unsigned char> { static const int type = Uint8; };
template <> struct ply_traits<unsigned short> { static const int type = Uint16; };
template <> struct ply_traits<unsigned int> { static const int type = Uint32; };
template <> struct ply_traits<char> { static const int type = Int8; };
template <> struct ply_traits<short> { static const int type = Int16; };
template <> struct ply_traits<int> { static const int type = Int32; };
template <> struct ply_traits<float> { static const int type = Float32; };
template <> struct ply_traits<double> { static const int type = Float64; };

template <typename T>
PlyProperty* construct_vertex_properties()
{
	static PlyProperty vertex_props[] = { 
		{"x", ply_traits<T>::type, ply_traits<T>::type, offsetof(PlyVertex<T>,x), 0, 0, 0, 0},
		{"y", ply_traits<T>::type, ply_traits<T>::type, offsetof(PlyVertex<T>,y), 0, 0, 0, 0},
		{"z", ply_traits<T>::type, ply_traits<T>::type, offsetof(PlyVertex<T>,z), 0, 0, 0, 0},
		{"nx", ply_traits<T>::type, ply_traits<T>::type, offsetof(PlyVertex<T>,nx), 0, 0, 0, 0},
		{"ny", ply_traits<T>::type, ply_traits<T>::type, offsetof(PlyVertex<T>,ny), 0, 0, 0, 0},
		{"nz", ply_traits<T>::type, ply_traits<T>::type, offsetof(PlyVertex<T>,nz), 0, 0, 0, 0},
		{"red", Uint8, Uint8, offsetof(PlyVertex<T>,red), 0, 0, 0, 0},
		{"green", Uint8, Uint8, offsetof(PlyVertex<T>,green), 0, 0, 0, 0},
		{"blue", Uint8, Uint8, offsetof(PlyVertex<T>,blue), 0, 0, 0, 0},
		{"alpha", Uint8, Uint8, offsetof(PlyVertex<T>,alpha), 0, 0, 0, 0},
	};
	return vertex_props;
}

static PlyProperty face_props[] = { /* list of property information for a face */
	{ "vertex_indices", Int32, Int32, offsetof(PlyFace,verts), 
	  1, Uint8, Uint8, offsetof(PlyFace,nverts) },
};

static char* propNames[] = { "vertex", "face" };

template <typename T>
typename ply_writer<T>::Nml ply_writer<T>::dummy_normal(0,0,1);

template <typename T>
typename ply_writer<T>::Clr ply_writer<T>::dummy_color(0,0,0,255);

template <typename T>
ply_writer<T>::ply_writer()
{
	ply_file = 0;
	write_mode = WM_NONE;
	have_vertex_normals = false;
	have_vertex_colors = false;
}

int to_ply_format(PlyFileFormat format)
{
	static const int ply_formats[] = { PLY_ASCII, PLY_BINARY_BE, PLY_BINARY_LE };
	return ply_formats[format];
}

template <typename T>
bool ply_writer<T>::open(const std::string& file_name, 
								 unsigned int nr_vertices, unsigned int nr_faces, 
								 bool vertex_normals, bool vertex_colors, PlyFileFormat format)
{
	PlyFile* ply_out = open_ply_for_write(file_name.c_str(), 2, propNames, format);
	if (!ply_out) return 0;
	describe_element_ply  (ply_out, "vertex", nr_vertices);
	describe_property_ply (ply_out, &construct_vertex_properties<T>()[0]);
	describe_property_ply (ply_out, &construct_vertex_properties<T>()[1]);
	describe_property_ply (ply_out, &construct_vertex_properties<T>()[2]);
	have_vertex_normals = vertex_normals;
	have_vertex_colors = vertex_colors;
	if (vertex_normals) {
		describe_property_ply (ply_out, &construct_vertex_properties<T>()[3]);
		describe_property_ply (ply_out, &construct_vertex_properties<T>()[4]);
		describe_property_ply (ply_out, &construct_vertex_properties<T>()[5]);
	}
	if (vertex_colors) {
		describe_property_ply (ply_out, &construct_vertex_properties<T>()[6]);
		describe_property_ply (ply_out, &construct_vertex_properties<T>()[7]);
		describe_property_ply (ply_out, &construct_vertex_properties<T>()[8]);
		describe_property_ply (ply_out, &construct_vertex_properties<T>()[9]);
	}
	describe_element_ply  (ply_out, "face", nr_faces);
	describe_property_ply (ply_out, &face_props[0]);
	header_complete_ply   (ply_out);
	this->ply_file = ply_out;
	return true;
}

/// write one vertex
template <typename T>
void ply_writer<T>::write_vertex(const Pnt& pt, const Nml& nml, const Clr& clr)
{
	PlyFile* ply_out = static_cast<PlyFile*>(ply_file);
	if (!ply_out)
		return;
	if (write_mode != WM_VERTEX) {
		put_element_setup_ply (ply_out, "vertex");
		write_mode = WM_VERTEX;
	}
	PlyVertex<T> pv;
	pv.x = pt[0];
	pv.y = pt[1];
	pv.z = pt[2];
	if (have_vertex_normals) {
		pv.nx = nml[0];
		pv.ny = nml[1];
		pv.nz = nml[2];
	}
	if (have_vertex_colors) {
		pv.red = clr[0];
		pv.green = clr[1];
		pv.blue = clr[2];
		pv.alpha = clr[3];
	}
	put_element_ply(ply_out, (void *)&pv);
}

/// write a triangle given by its three vertex indices
template <typename T>
void ply_writer<T>::write_triangle(int* vis)
{
	PlyFile* ply_out = static_cast<PlyFile*>(ply_file);
	if (!ply_out)
		return;
	if (write_mode != WM_FACE) {
		put_element_setup_ply (ply_out, "face");
		write_mode = WM_FACE;
	}
	PlyFace pf;
	pf.nverts = 3;
	pf.verts  = vis;
	put_element_ply(ply_out, (void *)&pf);
}

/// write a polygon given by its degree and the vertex indices
template <typename T>
void ply_writer<T>::write_polygon(unsigned char degree, int* vis)
{
	PlyFile* ply_out = static_cast<PlyFile*>(ply_file);
	if (!ply_out)
		return;
	if (write_mode != WM_FACE) {
		put_element_setup_ply (ply_out, "face");
		write_mode = WM_FACE;
	}
	PlyFace pf;
	pf.nverts = degree;
	pf.verts  = vis;
	put_element_ply(ply_out, (void *)&pf);
}

template <typename T>
void ply_writer<T>::close()
{
	PlyFile* ply_out = static_cast<PlyFile*>(ply_file);
	if (!ply_out)
		return;
	close_ply (ply_out);
	free_ply (ply_out);
	ply_file = 0;
}

#if (!defined _PLY_WRITER__MSC_TEMPLATES_DEFINED)
	template<> class ply_writer<float>;
	template<> class ply_writer<double>;
#endif
