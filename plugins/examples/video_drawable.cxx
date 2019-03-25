#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/media/video/video_reader.h>
#include <cgv/base/import.h>
#include <cgv/utils/file.h>
#include <cgv/utils/convert.h>
#include <cgv/media/image/image_writer.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/gui/trigger.h>

using namespace cgv::base;
using namespace cgv::data;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::signal;
using namespace cgv::utils;
using namespace cgv::utils;
using namespace cgv::media::video;
using namespace cgv::media::image;

class video_drawable : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public event_handler,
	public cgv::render::drawable,     /// derive from drawable for drawing the cube
	public cgv::gui::provider
{
public:
	unsigned int tex_id;
	double aspect;
	bool use_blending;
	bool running;
	bool dublicate;
	double time;
	double speed;
	video_reader reader;
	cgv::data::data_format df;
	cgv::data::data_view dv;
	std::string file_name;
	unsigned int frame_index;
	float fps;

	video_drawable() : node("video drawable"), reader(df)
	{
		running = false;
		dublicate = false;
		fps = 25;
		tex_id = -1;
		aspect = 1;
		use_blending = false;
		frame_index = 0;
		time = 0;
		speed = 1;
		connect(get_animation_trigger().shoot, this, &video_drawable::timer_event);
	}
	void create_gui()
	{
		connect_copy(add_control("animate", running, "check")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("use_blending", use_blending, "check")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("dublicate", dublicate, "check")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("speed", speed, "value_slider", "min=0.01;max=10;log=true;ticks=true")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	}
	void timer_event(double t, double dt)
	{
		if (!running)
			return;
		double T = t-time;
		int fi = (int)(speed*fps*T);
		if (fi > (int)frame_index) {
			read_frame();
			post_redraw();
		}
	}
	std::string get_type_name() const 
	{ 
		return "video_drawable"; 
	}
	void stream_help(std::ostream& os)
	{
		os << "video_drawable: open new video with 'O'\n\n";
	}
	bool init(context& )
	{
		tex_id = -1;
		return true;
	}
	void open_interactive(bool interactive)
	{
		if (interactive)
			file_name = file_open_dialog("open video file", video_reader::construct_filter_string());
		else
			file_name = "D://test.avi";
		reader.close();
		if (!reader.open(file_name)) {
			std::cerr << reader.get_last_error().c_str() << std::endl;
			return;
		}
		fps = reader.get_fps();
		frame_index = 0;
		if (running)
			time = get_animation_trigger().get_current_time();
		else 
			time = 0;
		if (tex_id != -1)
			glDeleteTextures(1, &tex_id);
		aspect = (float) df.get_width() / df.get_height();
		dv.~data_view();
		new (&dv) cgv::data::data_view(&df);
		if (reader.read_frame(dv))
			tex_id = cgv::render::gl::create_texture(dv, false);
	}
	void read_frame()
	{
		if (tex_id == -1)
			return;
		if (!reader.read_frame(dv)) {
			reader.set("frame", 0);
			frame_index = 0;
		}
		glBindTexture(GL_TEXTURE_2D, tex_id);
		cgv::render::gl::replace_texture(dv);
		++frame_index;
	}

	void write_frame()
	{
		if (file_name.empty())
			return;
		std::string fn = file::drop_extension(file_name)+"_"+to_string(frame_index)+".png";
		image_writer wr(fn);
		wr.write_image(dv);
	}

	void draw(context& )
	{
		glColor3d(1,1,0);
		if (tex_id != (unsigned int) -1) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,tex_id);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}
		if (use_blending) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		glDisable(GL_CULL_FACE);
			if (dublicate)
				glTranslated(-aspect,0,0);
			glBegin(GL_QUADS);
				glNormal3d(0,0,1);
				glTexCoord2d(0,0);
				glVertex3d(-aspect,-1,0);
				glTexCoord2d(1,0);
				glVertex3d(aspect,-1,0);
				glTexCoord2d(1,1);
				glVertex3d(aspect,1,0);
				glTexCoord2d(0,1);
				glVertex3d(-aspect,1,0);
			glEnd();
			if (dublicate) {
				glTranslated(2*aspect,0,0);
				glBegin(GL_QUADS);
					glTexCoord2d(0,0);
					glVertex3d(-aspect,-1,0);
					glTexCoord2d(1,0);
					glVertex3d(aspect,-1,0);
					glTexCoord2d(1,1);
					glVertex3d(aspect,1,0);
					glTexCoord2d(0,1);
					glVertex3d(-aspect,1,0);
				glEnd();
				glTranslated(-aspect,0,0);
			}
		if (use_blending)
			glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glDisable(GL_TEXTURE_2D);
	}
	bool handle(event& e)
	{
		if (e.get_kind() != EID_KEY)
			return false;
		key_event& ke = static_cast<key_event&>(e);
		if (ke.get_action() != KA_PRESS)
			return false;
		switch (ke.get_key()) {
		case 'O' :
			open_interactive(true);
			post_redraw();
			return true;
		case 'C' :
			read_frame();
			post_redraw();
			return true;
		case 'N' :
			read_frame();
			post_redraw();
			return true;
		case 'S' :
			write_frame();
			return true;
		case 'P' :
			if (frame_index > 1) {
				frame_index -= 2;
				reader.set("frame", frame_index);
				read_frame();
			}
			post_redraw();
			return true;
		case 'D' :
			dublicate=!dublicate;
			post_redraw();
			return true;
		case 'A' :
			if (!running)
				time = get_animation_trigger().get_current_time()+time;
			else
				time = time-get_animation_trigger().get_current_time();
			running = !running;
			return true;
		}
		return false;
	}
};

#include <cgv/base/register.h>

extern factory_registration<video_drawable> video_drawable_fac("new/media/video drawable", 'V', true);

