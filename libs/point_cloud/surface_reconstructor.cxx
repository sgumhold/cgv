
#include "surface_reconstructor.h"
#include <cgv/reflect/reflect_enum.h>
#include "ply_writer.h"
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/tokenizer.h>
#include <fstream>

std::ostream& operator << (std::ostream& os, const grow_event& ge)
{
	static const char* type_strs[2] = { "corner", "edge" };
	static const char* dir_strs[2] = { "backward", "forward" };

	os << type_strs[ge.type];
	if (ge.type == EDGE_GROW_EVENT)
		os << " " << dir_strs[ge.dir];
	return os << "(" << ge.vi << ":" << ge.j << ":" << ge.k << ")=" << ge.quality;
}

namespace cgv {
	namespace reflect {

		cgv::reflect::enum_reflection_traits<surface_reconstructor::DebugMode> get_reflection_traits(const surface_reconstructor::DebugMode&)
		{
			return cgv::reflect::enum_reflection_traits<surface_reconstructor::DebugMode>("NONE,SYMMETRIZE,MAKE_CONSISTENT,CYCLE_FILTER");
		}
	}
}

bool surface_reconstructor::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return 
		srh.reflect_member("use_orientation",use_orientation) &&
		srh.reflect_member("allow_intersections_in_holes", allow_intersections_in_holes) &&
		srh.reflect_member("z_weight", z_weight) &&
		srh.reflect_member("normal_quality_exp", normal_quality_exp) &&
		srh.reflect_member("noise_to_sampling_ratio", noise_to_sampling_ratio) &&
		srh.reflect_member("use_normal_weight", use_normal_weight) &&
		srh.reflect_member("max_nr_exhaustive_search", max_nr_exhaustive_search) &&
		srh.reflect_member("SR.line_width", line_width) &&
		srh.reflect_member("SR.point_size", point_size) &&
		srh.reflect_member("debug_mode", debug_mode) &&
		srh.reflect_member("debug_vi", debug_vi) &&
		srh.reflect_member("debug_j", debug_j) &&
		srh.reflect_member("compute_reference_length_from_delaunay_filter", compute_reference_length_from_delaunay_filter) &&
		srh.reflect_member("debug_events", debug_events) &&
		srh.reflect_member("valid_length_scale", valid_length_scale);
}

surface_reconstructor::surface_reconstructor() 
{
	pc = 0;
	ng = 0;
	use_normal_weight = true;
	z_weight = 0.5;
	normal_quality_exp = 3.0;
	noise_to_sampling_ratio = 0.1;
	debug_mode = DBG_NONE;
	debug_vi = -1;
	point_size = 1;
	line_width = 1;
	debug_hole = false;
	max_nr_exhaustive_search = 8;
	use_orientation = true;
	compute_reference_length_from_delaunay_filter = true;
	debug_events = false;
	debug_intersection_tests = false;
	perform_intersection_tests = true;
	allow_intersections_in_holes = false;
	valid_length_scale = 2;
	use_normal_weight = true;
}

void surface_reconstructor::analyze_holes()
{
	unsigned int hole_vi = 0;
	int hole_j = -1;
	clear_flag();
	std::vector<unsigned int> hole_lens[3];
	do {
		surface_reconstructor::HoleType ht = find_next_hole(hole_vi,hole_j,hole);
		if (ht == surface_reconstructor::HT_NONE)
			break;
		hole_lens[ht-1].push_back((int)hole.size());
		mark_hole(hole);
	} while (true);
	for (unsigned int x = 0; x < 3; ++x) {
		static const char* strs[3] = { "manifold", "non manifold", "inconsistent" };
		std::cout << "found " << hole_lens[x].size() << " " << strs[x] << " holes:";
		unsigned int i;
		for (i=0; i<hole_lens[x].size(); ++i)
			std::cout << " " << hole_lens[x][i];
		std::cout << std::endl;
	}
	clear_flag();
}

void surface_reconstructor::init()
{
	nr_triangles_per_vertex.clear();
	nr_triangles_per_edge.clear();
	vertex_info.clear();
	directed_edge_info.clear();
	grow_events.clear();
	first_grow_event.clear();
	vertex_reference_length.clear();
	secondary_normals.clear();
	geqs.init();
	ntpv.init();
	hole.clear();
	ntpe.init();
	debug_mode = DBG_NONE;
	debug_vi = -1;
}


/// detect output type from extension
bool surface_reconstructor::write(const std::string& file_name, const std::vector<unsigned int>& T) const
{
	std::string ext = cgv::utils::to_lower(cgv::utils::file::get_extension(file_name));
	if (ext == "obj")
		return write_obj(file_name, T);
	if (ext == "ply")
		return write_ply(file_name, T);
	std::cerr << "unknown extension <." << ext << ">." << std::endl;
	return false;

}
/// write in obj format
bool surface_reconstructor::write_obj(const std::string& file_name, const std::vector<unsigned int>& T) const
{
	std::ofstream os(file_name.c_str());
	if (os.fail())
		return false;
	Idx vi;
	Cnt n = (unsigned int) pc->get_nr_points();
	for (vi = 0; vi < (Idx)n; ++vi) {
		const Pnt& p = pc->pnt(vi);
		if (pc->has_colors()) {
			const Clr& c = pc->clr(vi);
			os << "v " << p[0] << " " << p[1] << " " << p[2] << " " << c[0] << " " << c[1] << " " << c[2] << std::endl;
		}
		else
			os << "v " << p[0] << " " << p[1] << " " << p[2] << std::endl;
	}
	for (vi = 0; vi < (Idx)n; ++vi) {
		const Nml& n = pc->nml(vi);
		os << "vn " << n[0] << " " << n[1] << " " << n[2] << std::endl;
	}
	unsigned int ti;
	for (ti = 0; ti < T.size(); ti += 3) {
		os << "f " << T[ti]+1 << "//" << T[ti]+1 << " "
			        << T[ti+1]+1 << "//" << T[ti+1]+1 << " "
					  << T[ti+2]+1 << "//" << T[ti+2]+1 << std::endl;
	}
	os.close();
	return !os.fail();
}

using namespace cgv::utils;

enum ParseMode { PM_EXTERIOR, PM_POINTS, PM_FACES };


#include <cgv/media/mesh/obj_reader.h>

struct my_obj_reader : public cgv::media::mesh::obj_reader, public point_cloud_types
{
	std::vector<unsigned int>& T;
	Cnt nr_nmls, nr_clrs, nr_pnts;
	point_cloud* pc;
	std::string filename;
	my_obj_reader(const std::string& fn, point_cloud* _pc, std::vector<unsigned int>& _T) : filename(fn), pc(_pc), T(_T) 
	{
		nr_pnts = 0;
		nr_nmls = 0;
		nr_clrs = 0;
	}
	///overide this function to process a vertex
	void process_vertex(const v3d_type& p)
	{
		if (pc->get_nr_points() == nr_pnts)
			pc->add_point(p);
		else
			pc->pnt(nr_pnts) = p;
		++nr_pnts;
	}
	///overide this function to process a normal
	void process_normal(const v3d_type& n)
	{
		if (pc->get_nr_points() == nr_nmls)
			pc->add_point(Pnt(0,0,0));
		else
			pc->nml(nr_nmls) = n;
		++nr_nmls;
	}
	///overide this function to process a normal
	void process_color(const color_type& c)
	{
		if (pc->get_nr_points() == nr_clrs)
			pc->add_point(Pnt(0,0,0));
		else
			pc->clr(nr_clrs) = c;
		++nr_clrs;
	}
	///overide this function to process a face
	void process_face(unsigned vcount, int *vertices, int *texcoords, int *normals)
	{
		if (vcount != 3) {
			std::cout << "skipping non triangular face" << std::endl;
			return;
		}
		T.push_back(vertices[0]-1);
		T.push_back(vertices[1]-1);
		T.push_back(vertices[2]-1);
	}
};

bool surface_reconstructor::read_obj(const std::string& file_name, point_cloud* pc, std::vector<unsigned int>& T)
{
	T.clear();
	pc->clear();
	my_obj_reader r(file_name, pc, T);
	return r.read_obj(file_name);
}

bool surface_reconstructor::read_vrml_1(const std::string& file_name, point_cloud* pc, std::vector<unsigned int>& T)
{
	std::string content;
	if (!cgv::utils::file::read(file_name, content, true))
		return false;
	T.clear();
	pc->clear();
	std::vector<token> toks;
	bite_all(tokenizer(content).set_ws(" \t\n,").set_sep("[](){}"), toks);
	ParseMode mode = PM_EXTERIOR;
	int step = 0;
	int pos = 0;
	double pnt[3];
	std::vector<int> face;
	for (unsigned int i = 0; i < toks.size(); ++i) {
		switch (mode) {
		case PM_EXTERIOR:
			if (to_string(toks[i]) == "Coordinate3") {
				step = 0;
				mode = PM_POINTS;
			}
			else if (to_string(toks[i]) == "IndexedFaceSet") {
				step = 0;
				mode = PM_FACES;
			}
			break;
		case PM_POINTS:
			if (step == 0) {
				if (to_string(toks[i]) == "{")
					++step;
			}
			else if (step == 1) {
				if (to_string(toks[i]) == "point")
					++step;
			}
			else if (step == 2) {
				if (to_string(toks[i]) == "[") {
					++step;
					pos = 0;
				}
			}
			else {
				if (is_double(toks[i].begin,toks[i].end,pnt[pos])) {
					if (++pos == 3) {
						pc->add_point(Pnt((Crd)pnt[0],(Crd)pnt[1],(Crd)pnt[2]));
						pos = 0;
					}
				}
				else if (to_string(toks[i]) == "]") {
					mode = PM_EXTERIOR;
				}
			}
			break;
		case PM_FACES:
			if (step == 0) {
				if (to_string(toks[i]) == "{")
					++step;
			}
			else if (step == 1) {
				if (to_string(toks[i]) == "coordIndex")
					++step;
			}
			else if (step == 2) {
				if (to_string(toks[i]) == "[") {
					++step;
					pos = 0;
				}
			}
			else {
				face.resize(pos+1);
				if (is_integer(toks[i].begin,toks[i].end,face[pos])) {
					if (face[pos] == -1) {
						if (pos == 3) {
							T.push_back(face[0]);
							T.push_back(face[1]);
							T.push_back(face[2]);
						}
						else {
							std::cout << "non triangular faces not implemented" << std::endl;
						}
						pos = 0;
					}
					else
						++pos;
				}
				else if (to_string(toks[i]) == "]") {
					mode = PM_EXTERIOR;
				}
			}
			break;
		};
	}
	return true;
}

/// write in ply format
bool surface_reconstructor::write_ply(const std::string& file_name, const std::vector<unsigned int>& T) const
{
	ply_writer<Crd> pw;
	if (!pw.open(file_name, (unsigned int) pc->get_nr_points(), (unsigned int) T.size()/3, true, pc->has_colors()))
		return false;
	unsigned int n = (unsigned int) pc->get_nr_points();
	if (pc->has_colors()) {
		for (unsigned int vi = 0; vi < n; ++vi)
			pw.write_vertex(pc->pnt(vi), pc->nml(vi), pc->clr(vi));
	}
	else {
		for (unsigned int vi = 0; vi < n; ++vi)
			pw.write_vertex(pc->pnt(vi), pc->nml(vi));
	}
	unsigned int ti;
	for (ti = 0; ti < T.size(); ti += 3)
		pw.write_triangle((int*)&T[ti]);
	pw.close();
	return true;
}
