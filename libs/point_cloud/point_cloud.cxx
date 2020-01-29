#include <cgv/math/permute.h>
#include <cgv/math/det.h>
#include "point_cloud.h"
#include <cgv/utils/file.h>
#include <cgv/utils/stopwatch.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/media/mesh/obj_reader.h>
#include <fstream>

#pragma warning(disable:4996)

using namespace cgv::utils::file;
using namespace cgv::utils;
using namespace std;
using namespace cgv::media::mesh;

class point_cloud_obj_loader : public obj_reader, public point_cloud_types
{
protected:
	std::vector<Pnt>& P;
	std::vector<Nml>& N;
	std::vector<Clr>& C;
public:
	///
	point_cloud_obj_loader(std::vector<Pnt>& _P, std::vector<Nml>& _N, std::vector<Clr>& _C) : P(_P), N(_N), C(_C) {}
	/// overide this function to process a vertex
	void process_vertex(const v3d_type& p)
	{
		P.push_back(point_cloud::Pnt(p));
	}
	/// overide this function to process a normal
	void process_normal(const v3d_type& n)
	{
		N.push_back(point_cloud::Nml(n));
	}
	/// overide this function to process a color (this called for vc prefixes which is is not in the standard but for example used in pobj-files)
	void process_color(const color_type& c)
	{
		C.push_back(c);
	}

};

index_image::Idx index_image::get_index(const PixCrd& pixcrd) const
{
	return (pixcrd(1) - pixel_range.get_min_pnt()(1))*width + pixcrd(0) - pixel_range.get_min_pnt()(0);
}

index_image::index_image()
{

}

void index_image::create(const PixRng& _pixel_range, Idx _initial_value)
{
	pixel_range = _pixel_range;
	PixRng::fvec_type extent = pixel_range.get_extent();
	indices.resize((extent(0)+1)*(extent(1)+1));
	width = extent(0)+1;
	std::fill(indices.begin(), indices.end(), _initial_value);
}

index_image::Idx  index_image::operator () (const PixCrd& pixcrd) const
{
	return indices[get_index(pixcrd)];
}

index_image::Idx& index_image::operator () (const PixCrd& pixcrd)
{
	return indices[get_index(pixcrd)];
}

/// return begin point index for iteration
point_cloud::Idx point_cloud::begin_index(Idx component_index) const
{
	if (component_index == -1 || !has_components())
		return 0;
	else
		return Idx(component_point_range(component_index).index_of_first_point);
}
/// return end point index for iteration
point_cloud::Idx point_cloud::end_index(Idx component_index) const
{
	if (component_index == -1 || !has_components())
		return get_nr_points();
	else
		return Idx(component_point_range(component_index).index_of_first_point + component_point_range(component_index).nr_points);
}

point_cloud::point_cloud()
{ 
	has_clrs = false;
	has_nmls = false;
	has_texcrds = false;
	has_pixcrds = false;
	has_comps = false;
	has_comp_trans = false;
	has_comp_clrs = false;

	no_normals_contained = false;
	box_out_of_date = false;
	pixel_range_out_of_date = false;
}

point_cloud::point_cloud(const string& file_name)
{
	has_clrs = false;
	has_nmls = false;
	has_texcrds = false;
	has_pixcrds = false;
	has_comps = false;
	has_comp_trans = false;
	has_comp_clrs = false;

	no_normals_contained = false;
	box_out_of_date = false;
	pixel_range_out_of_date = false;

	read(file_name);
}

void point_cloud::clear()
{
	P.clear();
	N.clear();
	C.clear();
	T.clear();
	I.clear();

	/// container to store  one component index per point
	component_indices.clear();
	components.clear();
	component_colors.clear();
	component_rotations.clear();
	component_translations.clear();
	component_boxes.clear();
	component_pixel_ranges.clear();

	has_clrs = false;
	has_nmls = false;
	has_texcrds = false;
	has_pixcrds = false;
	has_comps = false;
	has_comp_trans = false;
	has_comp_clrs = false;

	box_out_of_date = true;
	pixel_range_out_of_date = true;
}
/// append another point cloud
void point_cloud::append(const point_cloud& pc)
{
	if (pc.get_nr_points() == 0)
		return;
	Cnt old_n = (Cnt)P.size();
	Cnt n = (Cnt)P.size() + pc.get_nr_points();
	if (has_normals())
		N.resize(n);
	if (has_colors())
		C.resize(n);
	if (has_texture_coordinates())
		T.resize(n);
	if (has_pixel_coordinates())
		I.resize(n);
	if (has_components())
		component_indices.resize(n);
	P.resize(n);
	if (has_normals() && pc.has_normals())
		std::copy(pc.N.begin(), pc.N.end(), N.begin()+old_n);
	if (has_colors() && pc.has_colors())
		std::copy(pc.C.begin(), pc.C.end(), C.begin() + old_n);
	if (has_texture_coordinates() && pc.has_texture_coordinates())
		std::copy(pc.T.begin(), pc.T.end(), T.begin() + old_n);
	if (has_pixel_coordinates() && pc.has_pixel_coordinates()) {
		std::copy(pc.I.begin(), pc.I.end(), I.begin() + old_n);
		pixel_range_out_of_date = true;
	}
	if (has_components()) {
		if (pc.has_components()) {
			int old_nc = int(get_nr_components());
			for (unsigned ci = 0; ci < pc.get_nr_components(); ++ci) {
				components.push_back(pc.component_point_range(ci));
				components.back().index_of_first_point += old_n;
				if (has_component_colors())
					component_colors.push_back(pc.has_component_colors() ? pc.component_color(ci) : RGBA(1,1,1,1));
				if (has_component_transformations()) {
					component_rotations.push_back(pc.has_component_transformations() ? pc.component_rotation(ci) : Qat(1, 0, 0, 0));
					component_translations.push_back(pc.has_component_transformations() ? pc.component_translation(ci) : Dir(0, 0, 0));
				}
				comp_box_out_of_date.push_back(pc.comp_box_out_of_date[ci]);
				component_boxes.push_back(comp_box_out_of_date.back() ? Box() : pc.box(ci));
				if (has_pixel_coordinates()) {
					comp_pixrng_out_of_date.push_back(pc.comp_pixrng_out_of_date[ci]);
					component_pixel_ranges.push_back(comp_pixrng_out_of_date.back() ? PixRng() : pc.pixel_range(ci));
				}
			}
			for (unsigned i = 0; i < pc.get_nr_points(); ++i)
				component_indices[i + old_n] = pc.component_index(i) + old_nc;
		}
		else {
			Idx ci = Idx(components.empty() ? 0 : get_nr_components() - 1);
			std::fill(component_indices.begin() + old_n, component_indices.end(), ci);
			components[ci].nr_points += pc.get_nr_points();
			comp_box_out_of_date[ci] = true;
			if (has_pixel_coordinates())
				comp_pixrng_out_of_date[ci] = true;
		}
	}
	std::copy(pc.P.begin(), pc.P.end(), P.begin() + old_n);
	box_out_of_date = true;
}

/// clip on box
void point_cloud::clip(const Box clip_box)
{
	Idx j=0;
	for (Idx i=0; i<(Idx)get_nr_points(); ++i) {
		if (clip_box.inside(transformed_pnt(i))) {
			if (j < i) {
				pnt(j) = pnt(i);
				if (has_colors())
					clr(j) = clr(i);
				if (has_normals())
					nml(j) = nml(i);
				if (has_texture_coordinates())
					texcrd(j) = texcrd(i);
				if (has_pixel_coordinates())
					pixcrd(j) = pixcrd(i);
				if (has_components())
					component_index(j) = component_index(i);
			}
			++j;
		}
	}
	if (j != get_nr_points()) {
		if (has_colors())
			C.resize(j);
		if (has_normals())
			N.resize(j);
		if (has_texture_coordinates())
			T.resize(j);
		if (has_pixel_coordinates())
			I.resize(j);
		if (has_components()) {
			component_indices.resize(j);
			if (get_nr_components() > 0) {
				// recompute components
				Idx ci = -1;
				for (Idx i = 0; i < j; ++i) {
					while (ci < int(component_index(i)))
						components[++ci] = component_info(i, 0);
					++components[ci].nr_points;
				}
				while (ci+1 < int(get_nr_components()))
					components[++ci] = component_info(j, 0);
			}
		}
		P.resize(j);
	}
	box_out_of_date = true;
	if (has_pixel_coordinates())
		pixel_range_out_of_date = true;
}

/// permute points
void point_cloud::permute(std::vector<Idx>& perm, bool permute_component_indices)
{
	cgv::math::permute_vector(P, perm);
	if (has_normals())
		cgv::math::permute_vector(N, perm);
	if (has_colors())
		cgv::math::permute_vector(C, perm);
	if (has_texture_coordinates())
		cgv::math::permute_vector(T, perm);
	if (has_pixel_coordinates())
		cgv::math::permute_vector(I, perm);
	if (permute_component_indices && has_components())
		cgv::math::permute_vector(component_indices, perm);
}

/// translate by direction
void point_cloud::translate(const Dir& dir, Idx ci)
{
	for (Idx e = end_index(ci), i = begin_index(ci); i < e; ++i)
		pnt(i) += dir;
	if (ci == -1 || !has_components()) {
		B.ref_min_pnt() += dir;
		B.ref_max_pnt() += dir;
	}
	else {
		component_boxes[ci].ref_min_pnt() += dir;
		component_boxes[ci].ref_max_pnt() += dir;
		box_out_of_date = true;
	}
}

/// translate by direction
void point_cloud::rotate(const Qat& qat, Idx ci)
{
	for (Idx e = end_index(ci), i = begin_index(ci); i < e; ++i)
		pnt(i) = qat.apply(pnt(i));
	if (has_normals()) {
		for (Idx e = end_index(ci), i = begin_index(ci); i < e; ++i)
			nml(i) = qat.apply(nml(i));
	}
	box_out_of_date = true;
	if (ci != -1 && has_components())
		comp_box_out_of_date[ci] = true;
}

/// transform with linear transform 
void point_cloud::transform(const Mat& mat)
{
	for (Idx i=0; i<(Idx)get_nr_points(); ++i) {
		pnt(i) = mat*pnt(i);
	}
	box_out_of_date = true;
}

/// transform with affine transform 
void point_cloud::transform(const AMat& amat)
{
	HVec h(0,0,0,1);
	for (Idx i=0; i<(Idx)get_nr_points(); ++i) {
		(Dir&)h=pnt(i);
		pnt(i) = amat*h;
	}
	box_out_of_date = true;
}

/// transform with homogeneous transform and w-clip
void point_cloud::transform(const HMat& hmat)
{
	HVec h0(0,0,0,1), h1;
	for (Idx i=0; i<(Idx)get_nr_points(); ++i) {
		(Dir&)h0=pnt(i);
		h1 = hmat*h0;
		pnt(i) = (1/h1(3))*(const Dir&)h1;
	}
	box_out_of_date = true;
}

/// add a point and allocate normal and color if necessary
size_t point_cloud::add_point(const Pnt& p)
{
	if (has_normals())
		N.resize(N.size()+1);
	if (has_colors())
		C.resize(P.size() + 1);
	if (has_texture_coordinates())
		T.resize(P.size() + 1);
	if (has_pixel_coordinates())
		I.resize(P.size() + 1);
	if (has_components()) {
		if (components.empty())
			components.push_back(component_info(0, p.size()));
		component_indices.push_back(unsigned(components.size() - 1));
		++components.back().nr_points;
	}
	size_t idx = P.size();
	P.push_back(p);
	return idx;
}

/// resize the point cloud
void point_cloud::resize(size_t nr_points)
{
	if (has_normals())
		N.resize(nr_points);
	if (has_colors())
		C.resize(nr_points);
	if (has_texture_coordinates())
		T.resize(nr_points);
	if (has_pixel_coordinates())
		I.resize(nr_points);
	if (has_components())
		component_indices.resize(nr_points);
	P.resize(nr_points);
}


bool point_cloud::read(const string& _file_name)
{
	string ext = to_lower(get_extension(_file_name));
	bool success = false;
	if (ext == "bpc")
		success = read_bin(_file_name);
	if (ext == "xyz")
		success = read_xyz(_file_name);
	if (ext == "pct")
		success = read_pct(_file_name);
	if (ext == "points")
		success = read_points(_file_name);
	if (ext == "wrl")
		success = read_wrl(_file_name);
	if (ext == "apc" || ext == "pnt")
		success = read_ascii(_file_name);
	if (ext == "obj" || ext == "pobj")
	success = read_obj(_file_name);
	if (ext == "ply")
	success = read_ply(_file_name);
	if (success) {
		if (N.size() > 0)
			has_nmls = true;
		else if (P.size() > 0)
			has_nmls = false;

		if (C.size() > 0)
			has_clrs = true;
		else if (P.size() > 0)
			has_clrs = false;

		if (T.size() > 0)
			has_texcrds = true;
		else if (P.size() > 0)
			has_texcrds = false;

		if (I.size() > 0)
			has_pixcrds = true;
		else if (P.size() > 0)
			has_pixcrds = false;

		if (has_comps = components.size() > 0) {
			has_comp_clrs = component_colors.size() > 0;
			has_comp_trans = component_rotations.size() > 0;
			component_boxes.resize(get_nr_components());
			component_pixel_ranges.resize(get_nr_components());
			comp_box_out_of_date.resize(get_nr_components());
			std::fill(comp_box_out_of_date.begin(), comp_box_out_of_date.end(), true);
			comp_pixrng_out_of_date.resize(get_nr_components());
			std::fill(comp_pixrng_out_of_date.begin(), comp_pixrng_out_of_date.end(), true);
		}

		box_out_of_date = true;
		if (has_pixel_coordinates())
			pixel_range_out_of_date = true;
	}
	else {
		cerr << "unknown extension <." << ext << ">." << endl;
	}
	if (has_normals() && P.size() != N.size()) {
		cerr << "ups different number of normals: " << N.size() << " instead of " << P.size() << endl;
		N.resize(P.size());
	}
	if (has_colors() && P.size() != C.size()) {
		cerr << "ups different number of colors: " << C.size() << " instead of " << P.size() << endl;
		C.resize(P.size());
	}
	if (has_texture_coordinates() && P.size() != T.size()) {
		cerr << "ups different number of texture coordinates: " << T.size() << " instead of " << P.size() << endl;
		T.resize(P.size());
	}
	if (has_pixel_coordinates() && P.size() != I.size()) {
		cerr << "ups different number of pixel coordinates: " << I.size() << " instead of " << P.size() << endl;
		I.resize(P.size());
	}
	if (has_components() && P.size() != component_indices.size()) {
		cerr << "ups different number of component indices: " << component_indices.size() << " instead of " << P.size() << endl;
		component_indices.resize(P.size());
	}
	return success;
}

/// read component transformations from ascii file with 12 numbers per line (9 for rotation matrix and 3 for translation vector)
bool point_cloud::read_component_transformations(const std::string& file_name)
{
	if (!has_components()) {
		std::cerr << "ERROR in point_cloud::read_component_transformations: no components available" << std::endl;
		return false;
	}
	string content;
	if (!cgv::utils::file::read(file_name, content, true)) {
		std::cerr << "ERROR in point_cloud::read_component_transformations: could not read file " << file_name << std::endl;
		return false;
	}
	string ext = cgv::utils::file::get_extension(file_name);

	vector<line> lines;
	split_to_lines(content, lines);
	Cnt ci = 0;
	for (unsigned li = 0; li < lines.size(); ++li) {
		if (lines[li].empty())
			continue;
		Mat R;
		Dir t;
		char tmp = lines[li].end[0];
		content[lines[li].end - content.c_str()] = 0;
		int count = sscanf(lines[li].begin, "%f %f %f %f %f %f %f %f %f %f %f %f",
			&R(0, 0), &R(1, 0), &R(2, 0),
			&R(0, 1), &R(1, 1), &R(2, 1),
			&R(0, 2), &R(1, 2), &R(2, 2),
			&t(0), &t(1), &t(2));

		if (count == 12) {
			float D = cgv::math::det_33(
				R(0, 0), R(1, 0), R(2, 0),
				R(0, 1), R(1, 1), R(2, 1),
				R(0, 2), R(1, 2), R(2, 2)
			);
			if (fabs(fabs(D) - 1.0f) > 0.0001f) {
				std::cerr << "C" << ci << "(" << component_name(ci) << "): rotation matrix not normalized, det = " << D << std::endl;
			}
			if (ext == "som") {
				if (D < 0) {
					std::cerr << "C" << ci << "(" << component_name(ci) << "): negative determinant of rotation matrix = " << D << std::endl;
					R.transpose();
					t = R * t;
					R = -R;
				}
			}
			Qat q(R);
			if (fabs(q.length() - 1.0f) > 0.0001f) {
				std::cerr << "C" << ci << "(" << component_name(ci) << "): quaternion of not unit length " << q.length() << std::endl;
			}
			Mat R1;
			q.put_matrix(R1);
			if ((R-R1).frobenius_norm() > 0.0001f) {
				std::cerr << "C" << ci << "(" << component_name(ci) << "): matrix could not be reconstructed " << R << " vs " << R1 << std::endl;
				q = Qat(R);
			}

			if (fabs(q.length() - 1.0f) > 0.0001f) {
				std::cerr << "C" << ci << "(" << component_name(ci) << "): quaternion of not unit length " << q.length() << std::endl;
			}
			component_rotation(ci) = q;
			component_translation(ci) = t;
			comp_box_out_of_date[ci] = true;
			++ci;
		}
		else if (count == 7) {
			component_rotation(ci) = Qat(R(0,0), R(1, 0), R(2, 0), R(0, 1));
			component_translation(ci) = Dir(R(1, 1), R(2, 1), R(0, 2));
			comp_box_out_of_date[ci] = true;
			++ci;
		}
		if (ci >= get_nr_components())
			break;
		content[lines[li].end - content.c_str()] = tmp;
	}
	box_out_of_date = true;
	std::cout << "read " << ci << " transformation (have " << get_nr_components() << " components)" << std::endl;
	return true;
}

bool point_cloud::write(const string& _file_name)
{
	string ext = to_lower(get_extension(_file_name));
	if (ext == "bpc")
		return write_bin(_file_name);
	if (ext == "apc" || ext == "pnt")
		return write_ascii(_file_name, (ext == "apc") && has_normals());
	if (ext == "obj" || ext == "pobj")
		return write_obj(_file_name);
	if (ext == "ply")
		return write_ply(_file_name);
	cerr << "unknown extension <." << ext << ">." << endl;
	return false;
}

/// write component transformations to ascii file with 12 numbers per line (9 for rotation matrix and 3 for translation vector)
bool point_cloud::write_component_transformations(const std::string& file_name, bool as_matrices) const
{
	if (!has_components())
		return false;
	std::ofstream os(file_name);
	if (os.fail())
		return false;

	for (Idx ci = 0; ci < (Idx)get_nr_components(); ++ci) {
		const Qat& q = component_rotation(ci);
		const Dir& t = component_translation(ci);
		if (as_matrices) {
			Mat R;
			q.put_matrix(R);
			os << R(0, 0) << " " << R(1, 0) << " " << R(2, 0) << " " 
			   << R(0, 1) << " " << R(1, 1) << " " << R(2, 1) << " " 
			   << R(0, 2) << " " << R(1, 2) << " " << R(2, 2);
		}
		else
			os << q.re() << " " << q.x() << " " << q.y() << " " << q.z();
		os << " " << t(0) << " " << t(1) << " " << t(2) << std::endl;
	}
	return true;
	std::cerr << "write_component_transformations not implemented" << std::endl;
	return false;
}



/// read ascii file with lines of the form i j x y z I, where ij are pixel coordinates, xyz coordinates and I the intensity
bool point_cloud::read_pct(const std::string& file_name)
{
	string content;
	// double time;
	// {
		// cgv::utils::stopwatch watch(&time);
		if (!cgv::utils::file::read(file_name, content, true))
			return false;
		// std::cout << "read from disk in " << watch.get_elapsed_time() << " sec" << std::endl; watch.restart();
		clear();
		vector<line> lines;
		split_to_lines(content, lines);
		// std::cout << "split to " << lines.size() << " lines in " << watch.get_elapsed_time() << " sec" << std::endl; watch.restart();

		bool do_parse = false;
		for (unsigned li = 1; li < lines.size(); ++li) {
			if (lines[li].empty())
				continue;
			Pnt p;
			int i, j, Intensity;
			char tmp = lines[li].end[0];
			content[lines[li].end - content.c_str()] = 0;
			sscanf(lines[li].begin, "%d %d %f %f %f %d", &i, &j, &p[2], &p[0], &p[1], &Intensity);
			content[lines[li].end - content.c_str()] = tmp;
			P.push_back(p);
			C.push_back(Clr(byte_to_color_component(Intensity), byte_to_color_component(Intensity), byte_to_color_component(Intensity)));
			I.push_back(PixCrd(i, j));

			if ((P.size() % 100000) == 0)
				cout << "read " << P.size() << " points" << endl;
		}
		// std::cout << "parsed in " << watch.get_elapsed_time() << " sec" << std::endl;
	// }
	return true;
}


/// read ascii file with lines of the form x y z r g b I colors and intensity values, where intensity values are ignored
bool point_cloud::read_xyz(const std::string& file_name)
{
	string content;
	cgv::utils::stopwatch watch;
	if (!cgv::utils::file::read(file_name, content, true))
		return false;
	std::cout << "read data from disk "; watch.add_time();
	clear();
	vector<line> lines;
	split_to_lines(content, lines);
	std::cout << "split data into " << lines.size() << " lines. ";	watch.add_time();

	bool do_parse = false;
	unsigned i;
	for (i = 0; i<lines.size(); ++i) {
		if (lines[i].empty())
			continue;

		if (true) {
			Pnt p;
			int c[3], I;
			char tmp = lines[i].end[0];
			content[lines[i].end - content.c_str()] = 0;
			sscanf(lines[i].begin, "%f %f %f %d %d %d %d", &p[0], &p[1], &p[2], c, c + 1, c + 2, &I);
			content[lines[i].end - content.c_str()] = tmp;
			P.push_back(p);
			C.push_back(Clr(byte_to_color_component(c[0]), byte_to_color_component(c[1]), byte_to_color_component(c[2])));
		}
		else {

			vector<token> numbers;
			tokenizer(lines[i]).bite_all(numbers);
			double values[7];
			unsigned n = min(7, (int)numbers.size());
			unsigned j;
			for (j = 0; j < n; ++j) {
				if (!is_double(numbers[j].begin, numbers[j].end, values[j]))
					break;
			}
			if (j >= 3)
				P.push_back(Pnt((Crd)values[0], (Crd)values[1], (Crd)values[2]));
			if (j >= 6)
				C.push_back(Clr(float_to_color_component(values[3]), float_to_color_component(values[4]), float_to_color_component(values[5])));
		}
		if ((P.size() % 100000) == 0)
			cout << "read " << P.size() << " points" << endl;
	}
	watch.add_time();
	return true;
}

bool point_cloud::read_points(const std::string& file_name)
{
	string content;
	if (!cgv::utils::file::read(file_name, content, true))
		return false;
	clear();
	vector<line> lines;
	split_to_lines(content, lines);
	bool do_parse = false;
	unsigned i;
	for (i=0; i<lines.size(); ++i) {
		if (do_parse) {
			if (lines[i].empty())
				continue;
			vector<token> numbers;
			tokenizer(lines[i]).bite_all(numbers);
			double values[9];
			unsigned n = min(9,(int)numbers.size());
			unsigned j;
			for (j=0; j<n; ++j) {
				if (!is_double(numbers[j].begin, numbers[j].end, values[j]))
					break;
			}
			if (j >= 3)
				P.push_back(Pnt((Crd)values[0],(Crd)values[1],(Crd)values[2]));
			if (j >= 6)
				N.push_back(Nml((Crd)values[3],(Crd)values[4],(Crd)values[5]));
			if (j >= 9)
				C.push_back(Clr(float_to_color_component(values[6]), float_to_color_component(values[7]), float_to_color_component(values[8])));

			if ( (P.size() % 10000) == 0)
				cout << "read " << P.size() << " points" << endl;
		}
		if (lines[i] == "#Data:") {
			do_parse = true;
			cout << "starting to parse after line " << i << endl;
		}
	}
	return true;
}

bool point_cloud::read_wrl(const std::string& file_name)
{
	string content;
	if (!cgv::utils::file::read(file_name, content, true))
		return false;
	clear();
	vector<line> lines;
	split_to_lines(content, lines);
	int parse_mode = 0;
	unsigned i, j;
	for (i=0; i<lines.size(); ++i) {
		if (lines[i].empty())
			continue;
		vector<token> toks;
		tokenizer(lines[i]).set_sep("[]{},").bite_all(toks);
		switch (parse_mode) {
		case 0 :
			for (j=0; j<toks.size(); ++j)
				if (toks[j] == "Shape")
					++parse_mode;
			break;
		case 1 :
			for (j=0; j<toks.size(); ++j) {
				if (toks[j] == "point")
					++parse_mode;
				else if (toks[j] == "normal")
					parse_mode += 2;
				else if (toks[j] == "color")
					parse_mode += 3;
				else if (toks[j] == "[") {
					if (parse_mode > 1 && parse_mode < 5)
						parse_mode += 3;
				}
			}
			break;
		case 2 :
		case 3 :
		case 4 :
			for (j=0; j<toks.size(); ++j) {
				if (toks[j] == "[") {
					parse_mode += 3;
					break;
				}
			}
			break;
		case 5 :
		case 6 :
		case 7 :
			if (toks[0] == "]")
				parse_mode = 1;
			else {
				if (toks.size() >= 3) {
					double x,y,z;
					if (is_double(toks[0].begin, toks[0].end, x) && 
						is_double(toks[1].begin, toks[1].end, y) && 
						is_double(toks[2].begin, toks[2].end, z)) {
							switch (parse_mode) {
							case 5 : 
								P.push_back(Pnt((Crd)x,(Crd)y,(Crd)z)); 
								if ( (P.size() % 10000) == 0)
									cout << "read " << P.size() << " points" << endl;
								break;
							case 6 : 
								N.push_back(Nml((Crd)x,(Crd)y,(Crd)z)); 
								if ( (N.size() % 10000) == 0)
									cout << "read " << N.size() << " normals" << endl;
								break;
							case 7 : 
								C.push_back(Clr(float_to_color_component(x), float_to_color_component(y), float_to_color_component(z)));
								if ( (C.size() % 10000) == 0)
									cout << "read " << C.size() << " colors" << endl;
								break;
							}
					}
				}
			}
			break;
		}
	}
	return true;
}

enum BPCFlags
{
	BPC_HAS_CLRS = 1,
	BPC_HAS_NMLS = 2,
	BPC_HAS_TCS = 4,
	BPC_HAS_PIXCRDS = 8,
	BPC_HAS_COMPS = 16,
	BPC_HAS_COMP_CLRS = 32,
	BPC_HAS_COMP_TRANS = 64,
	BPC_HAS_BYTE_CLRS = 128
};

bool point_cloud::read_bin(const string& file_name)
{
	FILE* fp = fopen(file_name.c_str(), "rb");
	if (!fp)
		return false;
	Cnt n, m;
	cgv::type::uint32_type flags = 0;

	bool success =
		fread(&n,sizeof(Cnt),1,fp) == 1 &&
		fread(&m,sizeof(Cnt),1,fp) == 1;

	if (success) {
		clear();
		if (n == 0) {
			n = m;
			success = fread(&flags, sizeof(flags), 1, fp) == 1;
		}
		else {
			flags += (m >= 2 * n) ? BPC_HAS_CLRS : 0;
			if (flags & BPC_HAS_CLRS) {
				m = m - 2 * n;
			}
			flags += (m > 0) ? BPC_HAS_NMLS : 0;
		}
	}

	if (success) {
		P.resize(n);
		success = fread(&P[0][0], sizeof(Pnt), n, fp) == n;
		if (flags & BPC_HAS_NMLS) {
			N.resize(m);
			success = success && (fread(&N[0][0], sizeof(Nml), m, fp) == m);
		}
		if (flags & BPC_HAS_CLRS) {
			bool byte_colors_in_file = (flags & BPC_HAS_BYTE_CLRS) != 0;
#ifdef BYTE_COLORS
			bool byte_colors_in_pc = true;
#else
			bool byte_colors_in_pc = false;
#endif
			C.resize(n);
			if (byte_colors_in_file == byte_colors_in_pc)
				success = success && fread(&C[0][0], sizeof(Clr), n, fp) == n;
			else {
				if (byte_colors_in_file) {
					std::vector<cgv::media::color<cgv::type::uint8_type> > tmp;
					tmp.resize(n);
					success = success && fread(&tmp[0][0], sizeof(cgv::media::color<cgv::type::uint8_type>), n, fp) == n;
					if (success) {
						for (size_t i = 0; i < n; ++i)
							C[i] = Clr(byte_to_color_component(tmp[i][0]), byte_to_color_component(tmp[i][1]), byte_to_color_component(tmp[i][2]));
					}
				}
				else {
					std::vector<cgv::media::color<float> > tmp;
					tmp.resize(n);
					success = success && fread(&tmp[0][0], sizeof(cgv::media::color<float>), n, fp) == n;
					if (success) {
						for (size_t i = 0; i < n; ++i)
							C[i] = Clr(float_to_color_component(tmp[i][0]), float_to_color_component(tmp[i][1]), float_to_color_component(tmp[i][2]));
					}

				}
			}
		}
		if (flags & BPC_HAS_TCS) {
			T.resize(n);
			success = success && fread(&T[0][0], sizeof(TexCrd), n, fp) == n;
		}
		if (flags & BPC_HAS_PIXCRDS) {
			I.resize(n);
			success = success && fread(&I[0][0], sizeof(PixCrd), n, fp) == n;
		}
		if (flags & BPC_HAS_COMPS) {
			cgv::type::uint32_type nr_comps;
			success = success && fread(&nr_comps, sizeof(nr_comps), 1, fp) == 1;
			if (success) {
				components.resize(nr_comps);
				success = success && fread(&components[0], sizeof(component_info), nr_comps, fp) == nr_comps;
				component_indices.resize(n);
				for (unsigned i = 0; i < nr_comps; ++i)
					for (unsigned j = unsigned(components[i].index_of_first_point); j < components[i].index_of_first_point + components[i].nr_points; ++j)
						component_indices[j] = i;
				if (flags & BPC_HAS_COMP_CLRS) {
					component_colors.resize(nr_comps);
					success = success && fread(&component_colors[0], sizeof(RGBA), nr_comps, fp) == nr_comps;
				}
				if (flags & BPC_HAS_COMP_TRANS) {
					component_rotations.resize(nr_comps);
					component_translations.resize(nr_comps);
					success = success && fread(&component_rotations[0], sizeof(Qat), nr_comps, fp) == nr_comps;
					success = success && fread(&component_translations[0], sizeof(Dir), nr_comps, fp) == nr_comps;
				}
			}
		}
	}
	return fclose(fp) == 0 && success;
}

bool point_cloud::read_obj(const string& _file_name) 
{
	point_cloud_obj_loader pc_obj(P,N,C);
	if (!exists(_file_name))
		return false;
	clear();
	if (!pc_obj.read_obj(_file_name))
		return false;
	return true;
}
#include "ply.h"

struct PlyVertex 
{
  float x,y,z;             /* the usual 3-space position of a vertex */
  float nx, ny, nz;
  unsigned char red, green, blue, alpha;
};

static PlyProperty vert_props[] = { /* list of property information for a vertex */
  {"x", Float32, Float32, offsetof(PlyVertex,x), 0, 0, 0, 0},
  {"y", Float32, Float32, offsetof(PlyVertex,y), 0, 0, 0, 0},
  {"z", Float32, Float32, offsetof(PlyVertex,z), 0, 0, 0, 0},
  {"nx", Float32, Float32, offsetof(PlyVertex,nx), 0, 0, 0, 0},
  {"ny", Float32, Float32, offsetof(PlyVertex,ny), 0, 0, 0, 0},
  {"nz", Float32, Float32, offsetof(PlyVertex,nz), 0, 0, 0, 0},
  {"red", Uint8, Uint8, offsetof(PlyVertex,red), 0, 0, 0, 0},
  {"green", Uint8, Uint8, offsetof(PlyVertex,green), 0, 0, 0, 0},
  {"blue", Uint8, Uint8, offsetof(PlyVertex,blue), 0, 0, 0, 0},
  {"alpha", Uint8, Uint8, offsetof(PlyVertex,alpha), 0, 0, 0, 0},
  {"intensity", Uint8, Uint8, offsetof(PlyVertex,red), 0, 0, 0, 0},
};

typedef struct PlyFace {
  unsigned char nverts;
  int *verts;
} PlyFace;

static PlyProperty face_props[] = { /* list of property information for a face */
{"vertex_indices", Int32, Int32, offsetof(PlyFace,verts), 1, Uint8, Uint8, offsetof(PlyFace,nverts)},
};

static char* propNames[] = { "vertex", "face" };

bool point_cloud::read_ply(const string& _file_name) 
{
	PlyFile* ply_in =  open_ply_for_read(const_cast<char*>(_file_name.c_str()));
	if (!ply_in)
		return false;
	clear();
	for (int elementType = 0; elementType < ply_in->num_elem_types; ++elementType) {
		int nrVertices;
		char* elem_name = setup_element_read_ply (ply_in, elementType, &nrVertices);
		if (strcmp("vertex", elem_name) == 0) {
			PlyElement* elem = ply_in->elems[elementType];
			bool has_P[3] = { false, false, false };
			bool has_N[3] = { false, false, false };
			bool has_C[4] = { false, false, false, false };
			bool is_intensity = false;
			for (int pi = 0; pi < elem->nprops; ++pi) {
				if (strcmp("x", elem->props[pi]->name) == 0)
					has_P[0] = true;
				if (strcmp("y", elem->props[pi]->name) == 0)
					has_P[1] = true;
				if (strcmp("z", elem->props[pi]->name) == 0)
					has_P[2] = true;
				if (strcmp("nx", elem->props[pi]->name) == 0)
					has_N[0] = true;
				if (strcmp("ny", elem->props[pi]->name) == 0)
					has_N[1] = true;
				if (strcmp("nz", elem->props[pi]->name) == 0)
					has_N[2] = true;
				if (strcmp("red", elem->props[pi]->name) == 0)
					has_C[0] = true;
				if (strcmp("green", elem->props[pi]->name) == 0)
					has_C[1] = true;
				if (strcmp("blue", elem->props[pi]->name) == 0)
					has_C[2] = true;
				if (strcmp("alpha", elem->props[pi]->name) == 0)
					has_C[3] = true;
				if (strcmp("intensity", elem->props[pi]->name) == 0) {
					has_C[0] = has_C[1] = has_C[2] = true;
					is_intensity = true;
				}
			}
			if (!(has_P[0] && has_P[1] && has_P[2]))
				std::cerr << "ply file " << _file_name << " has no complete position property!" << std::endl;
			P.resize(nrVertices);
			has_nmls = has_N[0] && has_N[1] && has_N[2];
			has_clrs = has_C[0] && has_C[1] && has_C[2];
			if (has_nmls)
				N.resize(nrVertices);
			if (has_clrs)
				C.resize(nrVertices);
			int p;
			for (p=0; p<6; ++p) 
				setup_property_ply(ply_in, &vert_props[p]);
			if (is_intensity)
				setup_property_ply(ply_in, &vert_props[10]);
			else {
				for (p = 0; p < 4; ++p)
					if (has_C[p])
						setup_property_ply(ply_in, &vert_props[6+p]);
			}
			for (int j = 0; j < nrVertices; j++) {
				PlyVertex vertex;
				get_element_ply(ply_in, (void *)&vertex);
				P[j].set(vertex.x, vertex.y, vertex.z);
				if (has_nmls)
					N[j].set(vertex.nx, vertex.ny, vertex.nz);
				if (has_clrs) {
					C[j][0] = byte_to_color_component(vertex.red);
					C[j][1] = byte_to_color_component(is_intensity ? vertex.red : vertex.green);
					C[j][2] = byte_to_color_component(is_intensity ? vertex.red : vertex.blue);
				}
			}
		}
	}
	/* close the PLY file */
	close_ply (ply_in);
	free_ply (ply_in);
	return true;
}

bool point_cloud::write_ply(const std::string& file_name) const
{
	PlyFile* ply_out = open_ply_for_write(file_name.c_str(), 2, propNames, PLY_BINARY_LE);
	if (!ply_out) 
		return false;
	describe_element_ply (ply_out, "vertex", (int)P.size());
	for (int p=0; p<10; ++p) 
		describe_property_ply (ply_out, &vert_props[p]);
	describe_element_ply (ply_out, "face", 0);
	describe_property_ply (ply_out, &face_props[0]);
	header_complete_ply(ply_out);

	put_element_setup_ply (ply_out, "vertex");

	for (int j = 0; j < (int)P.size(); j++) {
		PlyVertex vertex;
		vertex.x = P[j][0];
		vertex.y = P[j][1];
		vertex.z = P[j][2];
		if (N.size() == P.size()) {
			vertex.nx = N[j][0];
			vertex.ny = N[j][1];
			vertex.nz = N[j][2];
		}
		else {
			vertex.nx = 0.0f;
			vertex.ny = 0.0f;
			vertex.nz = 1.0f;
		}
		if (C.size() == P.size()) {
			vertex.red = (unsigned char)(C[j][0]*255);
			vertex.green = (unsigned char)(C[j][1]*255);
			vertex.blue = (unsigned char)(C[j][2]*255);
		}
		else {
			vertex.red   = 255;
			vertex.green = 255;
			vertex.blue  = 255;
		}
		vertex.alpha = 255;
		put_element_ply(ply_out, (void *)&vertex);
	}
	put_element_setup_ply (ply_out, "face");
	close_ply (ply_out);
	free_ply (ply_out);
	return true;
}

bool point_cloud::read_ascii(const string& file_name)
{
	ifstream is(file_name.c_str());
	if (is.fail()) 
		return false;
	clear();
	while (!is.eof()) {
		char buffer[4096];
		is.getline(buffer,4096);
		float x, y, z, nx, ny, nz, r, g, b;
		unsigned int n = sscanf(buffer, "%f %f %f %f %f %f %f %f %f", &x, &y, &z, &nx, &ny, &nz, &r, &g, &b);
		if (n == 3 || n == 6 || n == 9)
			P.push_back(Pnt(x,y,z));
		if (n == 6) {
			if (no_normals_contained)
				C.push_back(Clr(float_to_color_component(nx), float_to_color_component(ny), float_to_color_component(nz)));
			else
				N.push_back(Nml(nx,ny,nz));
		}
		if (n == 9)
			C.push_back(Clr(float_to_color_component(r), float_to_color_component(g), float_to_color_component(b)));
	}
	return true;
}


bool point_cloud::write_ascii(const std::string& file_name, bool write_nmls) const
{
	ofstream os(file_name.c_str());
	if (os.fail()) 
		return false;
	for (unsigned int i=0; i<P.size(); ++i) {
		os << P[i][0] << " " << P[i][1] << " " << P[i][2];
		if (write_nmls) 
			os << " " << N[i][0] << " " << N[i][1] << " " << N[i][2];
		os << endl;
	}
	return !os.fail();
}

bool point_cloud::write_bin(const std::string& file_name) const
{
	FILE* fp = fopen(file_name.c_str(), "wb");
	if (!fp)
		return false;
	Cnt n = (Cnt)P.size();
	Cnt m = (Cnt)N.size();
	Cnt m1 = m;
	Cnt flags = has_colors() ? BPC_HAS_CLRS : 0;
	flags += has_normals() ? BPC_HAS_NMLS : 0;
	flags += has_texture_coordinates() ? BPC_HAS_TCS : 0;
	flags += has_pixel_coordinates() ? BPC_HAS_PIXCRDS : 0;
	flags += has_components() ? BPC_HAS_COMPS : 0;
	flags += has_component_colors() ? BPC_HAS_COMP_CLRS : 0;
	flags += has_component_transformations() ? BPC_HAS_COMP_TRANS : 0;
#ifdef BYTE_COLORS
	flags += BPC_HAS_BYTE_CLRS;
#endif
	if (has_colors() && C.size() == n)
		m1 = 2*n+m;
	bool success;
	if (false) {
		success =
			fwrite(&n, sizeof(Cnt), 1, fp) == 1 &&
			fwrite(&m1, sizeof(Cnt), 1, fp) == 1 &&
			fwrite(&P[0][0], sizeof(Pnt), n, fp) == n;
		if (has_normals())
			success = success && (fwrite(&N[0][0], sizeof(Nml), m, fp) == m);
		if (has_colors() && C.size() == n)
			success = success && (fwrite(&C[0][0], sizeof(Clr), n, fp) == n);
	}
	else {
		m = 0;
		success =
			fwrite(&m, sizeof(Cnt), 1, fp) == 1 &&
			fwrite(&n, sizeof(Cnt), 1, fp) == 1 &&
			fwrite(&flags, sizeof(Cnt), 1, fp) == 1 &&
			fwrite(&P[0][0], sizeof(Pnt), n, fp) == n;
		if (has_normals())
			success = success && (fwrite(&N[0][0], sizeof(Nml), n, fp) == n);
		if (has_colors() && C.size() == n)
			success = success && (fwrite(&C[0][0], sizeof(Clr), n, fp) == n);

		if (has_texture_coordinates()) 
			success = success && (fwrite(&T[0][0], sizeof(TexCrd), n, fp) == n);
		if (has_pixel_coordinates())
			success = success && (fwrite(&I[0][0], sizeof(PixCrd), n, fp) == n);
		if (has_components()) {
			Cnt nr_comps = Cnt(get_nr_components());
			success = success && (fwrite(&nr_comps, sizeof(Cnt), 1, fp) == 1) && (fwrite(&components[0], sizeof(component_info), nr_comps, fp) == nr_comps);
		}
		if (has_component_colors())
			success = success && (fwrite(&component_colors[0][0], sizeof(RGBA), get_nr_components(), fp) == get_nr_components());
		if (has_component_transformations()) {
			success = success && (fwrite(&component_rotations[0][0], sizeof(Qat), get_nr_components(), fp) == get_nr_components());
			success = success && (fwrite(&component_translations[0][0], sizeof(Dir), get_nr_components(), fp) == get_nr_components());
		}
	}
	return fclose(fp) == 0 && success;
}

bool point_cloud::write_obj(const std::string& file_name) const
{
	ofstream os(file_name.c_str());
	if (os.fail()) 
		return false;
	unsigned int i;
	for (i=0; i<P.size(); ++i) {
		if (has_colors())
			os << "v " << P[i][0] << " " << P[i][1] << " " << P[i][2] << " " << color_component_to_float(C[i][0]) 
			<< " " << color_component_to_float(C[i][1]) << " " << color_component_to_float(C[i][2]) << endl;
		else
			os << "v " << P[i][0] << " " << P[i][1] << " " << P[i][2] << endl;
	}
	for (i=0; i<N.size(); ++i)
		os << "vn " << N[i][0] << " " << N[i][1] << " " << N[i][2] << endl;
	return !os.fail();
}

bool point_cloud::has_colors() const
{
	return has_clrs;
}

///
void point_cloud::create_colors()
{
	has_clrs = true;
	C.resize(P.size());
}

///
void point_cloud::destruct_colors()
{
	has_clrs = false;
	C.clear();
}

/// return whether the point cloud has texture coordinates
bool point_cloud::has_texture_coordinates() const
{
	return has_texcrds;
}
/// allocate texture coordinates if not already allocated
void point_cloud::create_texture_coordinates()
{
	if (has_texture_coordinates())
		return;

	T.resize(get_nr_points());

	has_texcrds = true;
}
/// deallocate texture coordinates
void point_cloud::destruct_texture_coordinates()
{
	if (!has_texture_coordinates())
		return;

	T.clear();

	has_texcrds = false;
}

/// return whether the point cloud has pixel coordinates
bool point_cloud::has_pixel_coordinates() const
{
	return has_pixcrds;
}
/// allocate pixel coordinates if not already allocated
void point_cloud::create_pixel_coordinates()
{
	if (has_pixel_coordinates())
		return;
	
	I.resize(get_nr_points());

	has_pixcrds = true;
}
/// deallocate pixel coordinates
void point_cloud::destruct_pixel_coordinates()
{
	if (!has_pixel_coordinates())
		return;

	I.clear();

	has_pixcrds = false;
}


bool point_cloud::has_normals() const 
{
	return has_nmls; 
}

///
void point_cloud::create_normals()
{
	has_nmls = true;
	N.resize(P.size());
}

///
void point_cloud::destruct_normals()
{
	has_nmls = false;
	N.clear();
}

/// return number of components
size_t point_cloud::get_nr_components() const
{
	return components.size();
}

/// add a new component
point_cloud::Idx point_cloud::add_component()
{
	if (!has_components())
		create_components();
	components.push_back(component_info(get_nr_points(), 0));
	if (has_component_colors())
		component_colors.push_back(RGBA(1, 1, 1, 1));
	if (has_component_transformations()) {
		component_translations.push_back(Dir(0, 0, 0));
		component_rotations.push_back(Qat(1, 0, 0, 0));
	}
	comp_box_out_of_date.push_back(true);
	component_boxes.push_back(Box());
	if (has_pixel_coordinates()) {
		comp_pixrng_out_of_date.push_back(true);
		component_pixel_ranges.push_back(PixRng());
	}
	return Idx(components.size() - 1);
}

/// return whether the point cloud has component indices and point ranges
bool point_cloud::has_components() const
{
	return has_comps;
}

/// allocate component indices and point ranges if not already allocated
void point_cloud::create_components()
{
	if (has_components())
		return;
	component_indices.resize(get_nr_points());
	std::fill(component_indices.begin(), component_indices.end(), 0);
	components.resize(1);
	components[0] = component_info(0, get_nr_points());
	has_comps = true;
	if (has_component_colors()) {
		component_colors.resize(1);
		component_colors[0] = RGBA(1, 1, 1, 1);
	}
	if (has_component_transformations()) {
		component_rotations.resize(1);
		component_rotation(0) = Qat(1, 0, 0, 0);
		component_translations.resize(1);
		component_translation(0) = Dir(0, 0, 0);
	}
	component_boxes.resize(1);
	component_pixel_ranges.resize(1);
	comp_box_out_of_date.resize(1);
	comp_box_out_of_date[0] = true;
	comp_pixrng_out_of_date.resize(1);
	comp_pixrng_out_of_date[0] = true;
}

/// deallocate component indices and point ranges
void point_cloud::destruct_components()
{
	component_colors.clear();
	component_rotations.clear();
	component_translations.clear();
	components.clear();
	component_boxes.clear();
	component_pixel_ranges.clear();
	comp_box_out_of_date.clear();
	comp_pixrng_out_of_date.clear();
	has_comps = false;
	has_comp_clrs = false;
	has_comp_trans = false;
}

/// return whether the point cloud has component colors
bool point_cloud::has_component_colors() const
{
	return has_comp_clrs;
}
/// allocate component colors if not already allocated
void point_cloud::create_component_colors()
{
	if (has_component_colors())
		return;
	if (!has_components())
		create_components();
	
	component_colors.resize(get_nr_components());
	std::fill(component_colors.begin(), component_colors.end(), RGBA(1, 1, 1, 1));
	has_comp_clrs = true;
}

/// deallocate colors
void point_cloud::destruct_component_colors()
{
	if (!has_component_colors())
		return;
	component_colors.clear();
	has_comp_clrs = false;
}

/// return whether the point cloud has component tranformations
bool point_cloud::has_component_transformations() const
{
	return has_comp_trans;
}

/// allocate component tranformations if not already allocated
void point_cloud::create_component_tranformations()
{
	if (has_component_transformations())
		return;
	if (!has_components())
		create_components();
	component_translations.resize(get_nr_components());
	component_rotations.resize(get_nr_components());
	has_comp_trans = true;
	reset_component_transformation();
}
/// deallocate tranformations
void point_cloud::destruct_component_tranformations()
{
	if (!has_component_transformations())
		return;

	component_translations.clear();
	component_rotations.clear();

	has_comp_trans = false;
}
/// apply transformation of given component (or all of component index is -1) to influenced points
void point_cloud::apply_component_transformation(Idx component_index)
{
	if (!has_component_transformations())
		return;
}

/// set the component transformation of given component (or all of component index is -1) to identity
void point_cloud::reset_component_transformation(Idx component_index)
{
	if (!has_component_transformations())
		return;
	if (component_index == -1) {
		std::fill(component_translations.begin(), component_translations.end(), Dir(0, 0, 0));
		std::fill(component_rotations.begin(), component_rotations.end(), Qat(1, 0, 0, 0));
	}
	else {
		component_translations[component_index] = Dir(0, 0, 0);
		component_rotations[component_index] = Qat(1, 0, 0, 0);
	}
}

/// return the i_th point, in case components and component transformations are created, transform point with its compontent's transformation before returning it 
point_cloud::Pnt point_cloud::transformed_pnt(size_t i) const
{
	if (!has_components() || !has_component_transformations())
		return pnt(i);
	Idx ci = component_index(i);
	return component_rotation(ci).apply(pnt(i)) + component_translation(ci);
}

/// return box
const point_cloud::Box& point_cloud::box(Idx ci) const
{
	if (ci == -1) {
		if (box_out_of_date) {
			B.invalidate();
			for (Idx i = 0; i < (Idx)get_nr_points(); ++i)
				B.add_point(transformed_pnt(i));
			box_out_of_date = false;
		}
		return B;
	}
	else {
		if (comp_box_out_of_date[ci]) {
			component_boxes[ci].invalidate();
			for (Idx e = end_index(ci), i = begin_index(ci); i < e; ++i)
				component_boxes[ci].add_point(pnt(i));
			comp_box_out_of_date[ci] = false;
		}
		return component_boxes[ci];
	}
}

/// return the range of the stored pixel coordinates
const point_cloud::PixRng& point_cloud::pixel_range(Idx ci) const
{
	if (ci == -1) {
		if (pixel_range_out_of_date) {
			PR.invalidate();
			for (Idx i = 0; i < (Idx)get_nr_points(); ++i)
				PR.add_point(pixcrd(i));
			pixel_range_out_of_date = false;
		}
		return PR;
	}
	else {
		if (comp_pixrng_out_of_date[ci]) {
			component_pixel_ranges[ci].invalidate();
			for (Idx e = end_index(ci), i = begin_index(ci); i < e; ++i)
				component_pixel_ranges[ci].add_point(pixcrd(i));
			comp_pixrng_out_of_date[ci] = false;
		}
		return component_pixel_ranges[ci];
	}
}

/// compute an image with a point index stored per pixel
void point_cloud::compute_index_image(index_image& img, unsigned border_size, Idx ci)
{
	if (!has_pixel_coordinates())
		return;

	PixRng rng = pixel_range(ci);
	rng.add_point(rng.get_max_pnt() + PixCrd(border_size, border_size));
	rng.add_point(rng.get_min_pnt() - PixCrd(border_size, border_size));
	img.create(rng);
	for (Idx e = end_index(ci), i = begin_index(ci); i < e; ++i) 
		img(pixcrd(i)) = i;
}

index_image::PixCrd index_image::image_neighbor_offset(int i) 
{
	static int di[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };
	static int dj[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };
	return PixCrd(di[i], dj[i]);
}

/// compute the range of direct neighbor distances
void point_cloud::compute_image_neighbor_distance_statistic(const index_image& img, cgv::utils::statistics& distance_stats, Idx ci)
{
	if (!has_pixel_coordinates())
		return;

	// computing normals
	for (Idx e = end_index(ci), i = begin_index(ci); i < e; ++i) {
		for (int j = 1; j < 8; j += 2) {
			int ni = img(pixcrd(i) + img.image_neighbor_offset(j));
			if (ni != -1)
				distance_stats.update((pnt(i) - pnt(ni)).length());
		}
	}
}
/// collect the indices of the neighbor points of point pi
point_cloud::Cnt point_cloud::collect_valid_image_neighbors(size_t pi, const index_image& img, std::vector<size_t>& Ni, Crd distance_threshold) const
{
	Ni.clear();
	for (int j = 0; j < 8; ++j) {
		int ni = img(pixcrd(pi) + img.image_neighbor_offset(j));
		if (ni != -1) {
			if (distance_threshold == 0.0f || (pnt(pi) - pnt(ni)).length() < distance_threshold)
				Ni.push_back(ni);
		}
	}
	return Cnt(Ni.size());
}

void point_cloud::estimate_normals(const index_image& img, Crd distance_threshold, Idx ci, int* nr_isolated, int* nr_iterations, int* nr_left_over)
{
	if (!has_pixel_coordinates())
		return;

	if (!has_normals())
		create_normals();

	// computing normals
	std::vector<int> not_set_normals;
	std::vector<int> isolated_normals;
	std::vector<size_t> Ni;
	for (Idx e = end_index(ci), i = begin_index(ci); i < e; ++i) {
		collect_valid_image_neighbors(i, img, Ni, distance_threshold);
		if (Ni.size() < 3) {
			if (Ni.size() == 0)
				isolated_normals.push_back(i);
			else
				not_set_normals.push_back(i);
			N[i].set(0, 0, 0);
			continue;
		}

		// compute cross product normal relative to current point
		int prev = Ni.back();
		Nml nml(0, 0, 0);
		for (int j = 0; j < int(Ni.size()); ++j) {
			nml += cross(P[Ni[j]] - P[i], P[prev] - P[i]);
			prev = Ni[j];
		}
		nml.normalize();
		N[i] = nml;
	}
	if (nr_isolated)
		*nr_isolated = int(isolated_normals.size());

	int iter = 1;
	if (!not_set_normals.empty()) {
		std::vector<int> not_set_normals_old;
		do {
			not_set_normals_old = not_set_normals;
			not_set_normals.clear();
			for (int i = 0; i < int(not_set_normals_old.size()); ++i) {
				for (int j = 0; j < 8; ++j) {
					int ni = img(pixcrd(not_set_normals_old[i]) + img.image_neighbor_offset(j));
					if (ni != -1) {
						if (N[ni].length() > 0)
							N[not_set_normals_old[i]] += N[ni];
					}
				}
				if (N[not_set_normals_old[i]].length() == 0) {
					not_set_normals.push_back(not_set_normals[i]);
					continue;
				}
				N[not_set_normals_old[i]].normalize();
			}
			++iter;
		} while (not_set_normals_old.size() > not_set_normals.size());
	}
	if (nr_iterations)
		*nr_iterations = iter;
	if (nr_left_over)
		*nr_left_over = int(not_set_normals.size());
}
