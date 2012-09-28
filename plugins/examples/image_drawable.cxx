#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/render/drawable.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/base/import.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_image_drawable_base.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::data;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::media::image;

class image_drawable : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::render::gl::gl_image_drawable_base,     /// derive from drawable for drawing the cube
	public provider
{
public:
	image_drawable() : node("image drawable")
	{
		connect(get_animation_trigger().shoot, this, &image_drawable::timer_event);
	}
	void timer_event(double t, double dt)
	{
		unsigned old_current_image = current_image;
		gl_image_drawable_base::timer_event(t, dt);
		if (old_current_image != current_image) {
			update_member(&current_image);
			post_redraw();
		}
	}
	void configure_gui()
	{
		if (find_control(current_image)) {
			find_control(current_image)->set("max", durations.size()-1);
			find_control(animate)->set("active", tex_ids.size() > 1);
			find_control(current_image)->set("active", tex_ids.size() > 1);
		}
		if (find_control(x)) {
			find_control(x)->set("max", W);
			find_control(y)->set("max", H);
			find_control(w)->set("max", W);
			find_control(h)->set("max", H);
		}
		update_member(&x);
		update_member(&y);
		update_member(&w);
		update_member(&h);
		update_member(&file_name);
		update_member(&current_image);
		update_member(&use_blending);
	}
	void create_gui()
	{
		add_decorator("image drawable", "heading");
		add_view("file_name", file_name);
		connect_copy(add_button("&open", "shortcut='O'")->click, rebind(this, &image_drawable::open));
		connect_copy(add_button("o&pen files", "shortcut='P'")->click, rebind(this, &image_drawable::open_files));
		
		connect_copy(add_control("use_blending", use_blending, "check")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		
		add_control("&animate", animate, "check", "shortcut='A'");
		connect_copy(add_control("current_image", current_image, "value_slider", "min=0;max=0;ticks=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));

		connect_copy(add_button("&save", "shortcut='S'")->click, rebind(this, &image_drawable::save));

		connect_copy(add_control("show_rectangle", show_rectangle, "check", "shortcut='R'")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("x", x, "value_slider", "min=0;ticks=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("y", y, "value_slider", "min=0;ticks=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("w", w, "value_slider", "min=0;ticks=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("h", h, "value_slider", "min=0;ticks=true")->value_change,
			rebind(static_cast<drawable*>(this), &drawable::post_redraw));

		configure_gui();
	}
	std::string get_type_name() const 
	{ 
		return "image_drawable"; 
	}
	void stream_help(std::ostream& os)
	{
		os << "image_drawable: open new image with 'O', save current image with 'S'\n\n";
	}
	void open()
	{
		get_context()->make_current();
		if (tex_ids.size() > 0) {
			glDeleteTextures(tex_ids.size(),&tex_ids.front());
			durations.clear();
			tex_ids.clear();
		}
		std::string file_name = cgv::gui::file_open_dialog("open image file", image_reader::construct_filter_string(), cgv::base::ref_data_path_list().empty()?"":cgv::base::ref_data_path_list()[0]+"/Regular/Images");
		if (!file_name.empty()) {
			if (!read_image(file_name))
				read_image("res://cgv_logo.png");
			configure_gui();
			post_redraw();
		}
	}
	void open_files()
	{
		get_context()->make_current();
		if (tex_ids.size() > 0) {
			glDeleteTextures(tex_ids.size(),&tex_ids.front());
			durations.clear();
			tex_ids.clear();
		}
		std::vector<std::string> file_names;
		std::string file_path = cgv::gui::files_open_dialog(file_names, "open image files as animation", image_reader::construct_filter_string(), cgv::base::ref_data_path_list().empty()?"":cgv::base::ref_data_path_list()[0]+"/Regular/Images");
//		std::cout << "path = " << file_path << std::endl;
//		for (unsigned i=0; i<file_names.size(); ++i)
//			std::cout << "  file " << i << " = " << file_names[i] << std::endl;
		read_images(file_path, file_names);
		configure_gui();
		post_redraw();
	}
	void save()
	{
		std::string file_name = cgv::gui::file_save_dialog("save image file", image_writer::construct_filter_string(), cgv::base::ref_data_path_list().empty()?"":cgv::base::ref_data_path_list()[0]+"/Regular/Images");
		if (!file_name.empty())
			if (save_images(file_name))
				std::cout << "successfully saved " << this->file_name << " to " << file_name << std::endl;			
	}
	bool init(context& )
	{		
		return read_image("res://tree.gif");
	}
};

#include <cgv/base/register.h>

extern factory_registration<image_drawable> image_drawable_factory_registration("new/image drawable", 'I', true);

