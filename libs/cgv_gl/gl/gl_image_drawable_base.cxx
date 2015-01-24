#include "gl_image_drawable_base.h"

#include <cgv/media/image/image_reader.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/base/import.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv_gl/gl/gl.h>

using namespace cgv::base;
using namespace cgv::data;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::media::image;

namespace cgv {
	namespace render {
		namespace gl {

gl_image_drawable_base::gl_image_drawable_base()
{
	aspect = 1;
	start_time = -2;
	use_blending = false;
	animate = true;
	show_rectangle = false;
	x = 100;
	y = 50;
	w = 200;
	h = 100;
	W = 1;
	H = 1;
}

void gl_image_drawable_base::timer_event(double t, double dt)
{
	if (tex_ids.size() > 1 && start_time != -2) {
		if (animate) {
			if (start_time == -1) {
				start_time = t;
				anim_time = 0;
				for (unsigned i = 0; i<current_image; ++i) {
					start_time -= durations[i];
					anim_time += durations[i];
				}
			}
			else {
				dt = t-start_time;
				if (dt > anim_time+durations[current_image]) {
					while (dt > anim_time+durations[current_image]) {
						anim_time += durations[current_image];
						++current_image;
						if (current_image >= tex_ids.size()) {
							dt -= anim_time;
							start_time += anim_time;
							anim_time = 0;
							current_image = 0;
						}
					}
					post_redraw();
				}
			}
		}
		else 
			start_time = -1;
	}
}

bool gl_image_drawable_base::read_image(const std::string& _file_name)
{
	if (!cgv::render::gl::read_image_to_textures(_file_name, tex_ids, durations, true, &aspect, &use_blending))
		return false;
	data_format df;
	image_reader ir(df);
	ir.open(_file_name);
	W = df.get_width();
	H = df.get_height();
	x = 0;
	y = 0;
	w = W;
	h = H;
	current_image = 0;
	start_time = -2;
	file_name = _file_name;
	files.clear();
	post_redraw();
	return true;
}

bool gl_image_drawable_base::read_images(const std::string& _file_name, const std::vector<std::string>& _files)
{
	for (unsigned i=0; i<_files.size(); ++i) {
		unsigned tex_id = cgv::render::gl::read_image_to_texture(_file_name+"/"+_files[i],
			true, &aspect, &use_blending);
		if (tex_id == -1)
			return false;
		tex_ids.push_back(tex_id);
		durations.push_back(0.04f);
		std::cout << "read image " << _file_name+"/"+_files[i] << std::endl;
	}
	if (!files.empty()) {
		data_format df;
		image_reader ir(df);
		ir.open(_file_name+"/"+_files[0]);
		W = df.get_width();
		H = df.get_height();
		w = W;
		h = H;
		x = 0;
		y = 0;
	}
	current_image = 0;
	start_time = -2;
	file_name = _file_name;
	files = _files;
	post_redraw();
	return true;
}

bool gl_image_drawable_base::save_images(const std::string& output_file_name)
{
	// crop update rectangle
	if (x+w > W)
		w = W-x;
	if (y+h > H)
		h = H-y;
	bool crop = (w > 0 && h > 0 && (w < W || h < H));

	// prepare output
	image_writer iw(output_file_name);

	unsigned nr = 1;
	if (files.size() > nr)
		nr = (unsigned)files.size();

	for (unsigned i=0; i<nr; ++i) {
		std::string input_file_name = file_name;
		if (i < files.size()) {
			input_file_name += '/';
			input_file_name += files[i];
		}

		std::vector<data_format> palette_formats;
		std::vector<data_view> palettes;
		data_view dv;
		data_format df;
		image_reader ir(df, &palette_formats);
		// open image
		if (!ir.open(input_file_name)) {
			std::cerr << ir.get_last_error().c_str() << std::endl;
			return false;
		}
		palettes.resize(palette_formats.size());

		data_format* df_out = &df;
		data_view* dv_out = &dv;
		if (crop) {
			df_out = new data_format(df);
			df_out->set_width(w);
			df_out->set_height(h);
			dv_out = new data_view(df_out);
		}
		// query number of images
		unsigned nr = ir.get_nr_images();

		for (unsigned i=0; i<nr; ++i) {
			// query palettes
			for (unsigned j=0; j<palette_formats.size(); ++j) {
				if (!ir.read_palette(j, palettes[j]))
					return false;
			}
			double duration = ir.get_image_duration();
			if (!ir.read_image(dv)) {
				std::cerr << ir.get_last_error() << std::endl;
				return false;
			}
			if (crop) {
				unsigned char* src_ptr = dv.get_ptr<unsigned char>() + x*dv.get_step_size(1) + y*dv.get_step_size(0);
				unsigned char* dst_ptr = dv_out->get_ptr<unsigned char>();
				for (int j=0; j<h; ++j) {
					memcpy(dst_ptr, src_ptr, dv_out->get_step_size(0));
					dst_ptr += dv_out->get_step_size(0);
					src_ptr += dv.get_step_size(0);
				}
			}
			if (!iw.write_image(*dv_out, (std::vector<const_data_view>*)(&palettes))) {
				std::cerr << iw.get_last_error() << std::endl;
				return false;
			}
		}
		if (crop) {
			delete dv_out;
			delete df_out;
		}
	}
	return true;
}

void gl_image_drawable_base::draw(context& )
{
	glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT);
	glDisable(GL_LIGHTING);
	glColor3d(1,0,0);
	glPushMatrix();
	glScaled(aspect,1,1);
	if (show_rectangle) {
		glPushMatrix();
			glTranslated(-1,-1,0);
			glScaled(2.0/W, 2.0/H, 1);
			glBegin(GL_LINE_STRIP);
			glVertex2i(x,y);
			glVertex2i(x+w,y);
			glVertex2i(x+w,y+h);
			glVertex2i(x,y+h);
			glVertex2i(x,y);
			glEnd();
		glPopMatrix();
	}
	glColor3d(1,1,0);
	if (tex_ids.size()>0) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,tex_ids[current_image]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		if (start_time == -2)
			start_time = -1;
	}
	if (use_blending) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glDisable(GL_CULL_FACE);
		glBegin(GL_QUADS);
			glNormal3d(0,0,1);
			glTexCoord2d(0,0);
			glVertex3d(-1,-1,0);
			glTexCoord2d(1,0);
			glVertex3d(1,-1,0);
			glTexCoord2d(1,1);
			glVertex3d(1,1,0);
			glTexCoord2d(0,1);
			glVertex3d(-1,1,0);
		glEnd();
	if (use_blending)
		glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	glPopAttrib();
}
		}
	}
}