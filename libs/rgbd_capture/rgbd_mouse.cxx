#include <omp.h>
#include "rgbd_mouse.h"
#include <cgv/utils/file.h>
#include <cgv/math/mat.h>
#include <cgv/math/diag_mat.h>
#include <cgv/math/inv.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>
#include <fstream>

namespace rgbd {
///
rgbd_mouse::rgbd_mouse() : last_pos(-1,-1,0), p_min(-1,-1,0), p_max(1,1,1)
{
	pitch = 0;
	z_click = 0.5f;

	inlier_radius = 0.1f;
	weight_exp = 0.0f;
	nr_cached_images = 0;
	nr_pred = 3;
	nr_cached = 5;

	reduction_factor = 1;
	nr_patches_w = 1;
	nr_patches_h = 1;

	last_pos_idx = 0;
	f = 0.99f;
}

///
void rgbd_mouse::set_config(const vec3& p_min, const vec3& p_max, float z_click, float pitch)
{
	this->p_min = p_min;
	this->p_max = p_max;
	this->z_click = z_click;
	this->pitch = pitch;
}

///
bool rgbd_mouse::save_config(const std::string& file_name) const
{
	std::ofstream os(file_name.c_str());
	if (os.fail())
		return false;
	os << "x_min=" << p_min(0) << "\n";
	os << "y_min=" << p_min(1) << "\n";
	os << "z_min=" << p_min(2) << "\n";
	os << "x_max=" << p_max(0) << "\n";
	os << "y_max=" << p_max(1) << "\n";
	os << "z_max=" << p_max(2) << "\n";
	os << "z_click=" << z_click << "\n";
	os << "pitch=" << pitch << "\n";
	os << "inlier_radius=" << inlier_radius << "\n";
	os << "nr_cached=" << nr_cached<< "\n";
	os << "weight_exp=" << weight_exp;
	return !os.fail();
}

bool scan_value(const std::vector<cgv::utils::token>& toks, unsigned i, float& f, float std_value)
{
	if (i >= toks.size()) {
		f = std_value;
		return true;
	}
	double d;
	if (!cgv::utils::is_double(toks[i].begin, toks[i].end, d))
		return false;
	f = float(d);
	return true;
}

bool scan_value(const std::vector<cgv::utils::token>& toks, unsigned i, unsigned& f, unsigned std_value)
{
	if (i >= toks.size()) {
		f = std_value;
		return true;
	}
	int v;
	if (!cgv::utils::is_integer(toks[i].begin, toks[i].end, v))
		return false;
	f = unsigned(v);
	return true;
}

///
bool rgbd_mouse::load_config(const std::string& file_name)
{
	std::string content;
	if (!cgv::utils::file::read(file_name, content, true))
		return false;
	std::vector<cgv::utils::token> toks;
	cgv::utils::tokenizer(content).set_ws("= \t\n").bite_all(toks);
	return 
		scan_value(toks, 1, p_min(0), -1.0f) &&
		scan_value(toks, 3,p_min(1), -1.0f) &&
		scan_value(toks, 5,p_min(2), 0.0f) &&
		scan_value(toks, 7,p_max(0), 1.0f) &&
		scan_value(toks, 9,p_max(1), 1.0f) &&
		scan_value(toks, 11,p_max(2), 1.0f) &&
		scan_value(toks, 13,z_click, 0.4f) &&
		scan_value(toks, 15,pitch, 0.0f) &&
		scan_value(toks, 17,inlier_radius, 0.1f) &&
		scan_value(toks, 19,nr_cached, 5) &&
		scan_value(toks, 21,weight_exp, 0.0f);
}

///
rgbd_mouse::vec3 rgbd_mouse::track_rectangle(int x0, int y0, int x1, int y1, int d0, int d1) const
{
	int w = data_ptr->get_format()->get_width();
	int h = data_ptr->get_format()->get_height();
	int wp = x1-x0;
	int hp = y1-y0;
	if (wp < 1 || hp < 1)
		return vec3(0,0,1);
	int line_delta = w - wp;
	const unsigned short* d = data_ptr->get_ptr<unsigned short>()+y0*w+x0;
	unsigned short min_dp = d1+1;
	unsigned min_ip = -1;
	int ip = 0;
	for (int yp=0; yp<hp; ++yp) {
		for (int xp=0; xp<wp; ++xp, ++ip, ++d) {
			if (d0 <= *d && *d < min_dp) {
				min_dp = *d;
				min_ip = ip;
			}
		}
		d += line_delta;
	}
	int min_x = min_ip%wp;
	int min_y = min_ip/wp;
	return vec3(2.0f*min_x/(wp-1)-1.0f,2.0f*min_y/(hp-1)-1.0f,min_dp*1.0f/2047);
}


void rgbd_mouse::filter_depth_image(cgv::data::data_view& depth_data)
{
	while (last_images.size() > nr_cached_images) {
		delete [] last_images.front();
		last_images.pop_front();
	}

	if (nr_cached_images == 0)
		return;
	unsigned n = unsigned(depth_data.get_format()->get_nr_bytes());
	unsigned short* last_img = new unsigned short[n];
	unsigned short* img = depth_data.get_ptr<unsigned short>();
	memcpy(last_img, img, n*sizeof(unsigned short));
	last_images.push_back(last_img);

	#pragma omp parallel for

	for (int i=0; i<int(n); ++i) {
		unsigned v = 0;
		unsigned count = 0;
		for (unsigned j=0; j<last_images.size(); ++j) {
			if (last_images[j][i] != 2047) {
				v += last_images[j][i];
				++count;
			}
		}
		if (count == 0)
			img[i] = 2047;
		else
			img[i] = unsigned short(v/count);
	}
}

void rgbd_mouse::filter_tracked_pos()
{
	while (last_poses.size() >= nr_cached)
		last_poses.pop_front();
	last_poses.push_back(last_pos);

	if (last_poses.size() < 3)
		return;

	unsigned i,j, max_count = 0;
	unsigned max_count_i = 0;

	for (i=0; i<last_poses.size(); ++i) {

		unsigned count = 1;
		for (j=0; j<last_poses.size(); ++j) {
			if (j != i) {
				float l = (last_poses[i]-last_poses[j]).length();
				if (l <= inlier_radius)
					++count;
			}
		}
		if (count >= max_count) {
			max_count = count;
			max_count_i = i;
		}
	}

	float total_weight = 0;
	last_pos = vec3(0,0,0);
	for (j=0; j<last_poses.size(); ++j) {
		float l = (last_poses[max_count_i]-last_poses[j]).length();
		if (l <= inlier_radius) {
			float weight = pow(float(j+1),weight_exp);
			last_pos += weight*last_poses[j];
			total_weight += weight;
		}
	}
	last_pos *= 1.0f/total_weight;
}

void rgbd_mouse::filter_tracked_pos_linear()
{
	while (last_poses.size() >= nr_cached)
		last_poses.pop_front();
	last_poses.push_back(last_pos);

	if (last_poses.size() < 4)
		return;

	unsigned i,j,k,max_count = 0;
	unsigned max_count_i = 0;
	unsigned max_count_j = 0;

	for (i=0; i<last_poses.size(); ++i) {
		for (j=i+1; j<last_poses.size(); ++j) {
			if (j != i) {
				vec3 v = (1.0f/(j-i))*(last_poses[j] - last_poses[i]);
				vec3 p0 = last_poses[i] - float(i)*v;
				unsigned count = 1;
				for (k=0;k<last_poses.size(); ++k) {
					float l = (last_poses[k]-(p0+float(k)*v)).length();
					if (l <= inlier_radius)
						++count;
				}
				if (count >= max_count) {
					max_count = count;
					max_count_i = i;
					max_count_j = j;
				}
			}
		}
	}
	std::cout << "max_count = " << max_count << " at(" << max_count_i << "," << max_count_j << ")" << std::endl;
	vec3 v = (1.0f/(max_count_j-max_count_i))*(last_poses[max_count_j] - last_poses[max_count_i]);
	vec3 p0 = last_poses[max_count_i] - float(max_count_i)*v;

	last_pos = p0;
/*
	float total_weight = 0;
	last_pos = vec3(0,0,0);
	for (j=0; j<last_poses.size(); ++j) {
		float l = (last_poses[max_count_i]-last_poses[j]).length();
		if (l <= inlier_radius) {
			float weight = pow(float(j+1),weight_exp);
			last_pos += weight*last_poses[j];
			total_weight += weight;
		}
	}
	last_pos *= 1.0f/total_weight;*/
}

///
rgbd_mouse::vec3 rgbd_mouse::track(cgv::data::data_view& depth_data)
{
//	reduce(depth_data);
	filter_depth_image(depth_data);
	data_ptr = &depth_data;
	int w = data_ptr->get_format()->get_width();
	int h = data_ptr->get_format()->get_height();
	int x0 = int(0.5*(1-p_max(0))*(w-1));
	int x1 = int(0.5*(1-p_min(0))*(w-1));
	int y0 = int(0.5*(1-p_max(1))*(h-1));
	int y1 = int(0.5*(1-p_min(1))*(h-1));
	int d0 = int(p_min(2)*2047);
	int d1 = int(p_max(2)*2047);
	last_pos = track_rectangle(x0,y0,x1,y1,d0,d1);
//	if (weight_exp > 0) 
		filter_tracked_pos();
//	else
//		filter_tracked_pos_linear();
	return vec3(-last_pos(0),-last_pos(1),last_pos(2));
}


void rgbd_mouse::reduce(const cgv::data::const_data_view& depth_data)
{
/*	if (reduction_factor == 1) {
		data_ptr = &depth_data;
		return;
	}

	int w = depth_data.get_format()->get_width();
	int h = depth_data.get_format()->get_height();
	int n=w*h;

	int wr = w/reduction_factor;
	int hr = h/reduction_factor;
	int nr =wr*hr;

	if (reduced_df.get_width() != wr) {
		reduced_df = *depth_data.get_format();
		reduced_df.set_width(wr);
		reduced_df.set_height(hr);
		reduced_data = cgv::data::data_view(&reduced_df);
	}
	data_ptr     = (cgv::data::const_data_view*)&reduced_data;

	int x,y;
	const unsigned short *p  = depth_data.get_ptr<unsigned short>();
	unsigned short *pr = reduced_data.get_ptr<unsigned short>();
	std::fill(pr, pr+nr, 0);

	std::vector<int> c(wr, 0);
	int* cp = &c.front();
	for (y=0; y<h; ++y) {
		for (x=0; x<w; ++x) {
			if (*p < 2047) {
				*pr += *p;
				++(*cp);
			}
			++p;
			if ((x % reduction_factor)==reduction_factor-1) {
				++pr;
				++cp;
			}
		}
		cp -= wr;
		pr -= wr;
		if ((y % reduction_factor) == reduction_factor-1) {
			for (int i=0; i<wr; ++i, ++pr, ++cp)
				*pr = *cp == 0 ? 2047 : *pr / *cp;
			cp = &c.front();
			std::fill(c.begin(), c.end(), 0);
		}
	}
	*/
}

///
rgbd_mouse::vec3 rgbd_mouse::track_patch(int x, int y, unsigned nr_patches_w, unsigned nr_patches_h) const
{
	int w = data_ptr->get_format()->get_width();
	int h = data_ptr->get_format()->get_height();
	int wp = w/nr_patches_w;
	int hp = h/nr_patches_h;
	int line_delta = w - wp;
	const unsigned short* d = data_ptr->get_ptr<unsigned short>()+y*hp*w+x*wp;
	unsigned short min_dp = -1;
	unsigned min_ip = -1;
	int ip = 0;
	for (int yp=0; yp<hp; ++yp) {
		for (int xp=0; xp<wp; ++xp, ++ip, ++d) {
			if (*d < min_dp) {
				min_dp = *d;
				min_ip = ip;
			}
		}
		d += line_delta;
	}
	int min_x = x*wp + min_ip%wp;
	int min_y = y*hp + min_ip/wp;
	return vec3(2.0f*min_x/(w-1)-1.0f,2.0f*min_y/(h-1)-1.0f,min_dp*1.0f/2047);
}

}