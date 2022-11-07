#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv/media/volume/volume_io.h>
#include <cgv/render/drawable.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/base/import.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/image_drawable.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::data;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::media::image;

class image_view : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::render::gl::image_drawable,     /// derive from drawable for drawing the cube
	public cgv::gui::event_handler,
	public provider
{
protected:
	vec2 range;
	size_t slice_size = 0;
	cgv::data::data_format df;
	cgv::data::data_view dv;
	bool view_vox_file = false;
public:
	image_view() : node("image view"), range(0,1)
	{
		connect(get_animation_trigger().shoot, this, &image_view::timer_event);
	}
	void timer_event(double t, double dt)
	{
		unsigned old_current_image = current_image;
		image_drawable::timer_event(t, dt);
		if (old_current_image != current_image) {
			on_set(&current_image);
			post_redraw();
		}
	}
	bool handle(event& e)
	{
		if (e.get_kind() != EID_KEY)
			return false;
		auto& ke = reinterpret_cast<key_event&>(e);
		if (ke.get_action() == KA_RELEASE)
			return false;
		switch (ke.get_char()) {
		case '+' :
			if (tex_ids.size() > 1) {
				if (++current_image == tex_ids.size())
					current_image = 0;
				on_set(&current_image);
			}
			return true;
		case '-' :
			if (tex_ids.size() > 1) {
				if (current_image == 0)
					current_image = (unsigned)tex_ids.size() - 1;
				else
					--current_image;
				on_set(&current_image);
			}
			return true;
		}
		switch (ke.get_key()) {
		case 'S': spherical = !spherical; on_set(&spherical); return true;
		case 'O' :
			if (ke.get_modifiers() == cgv::gui::EM_CTRL) {
				open();
				return true;
			}
			break;
		case KEY_Left:  pan_tilt[0] -= scale * 0.1f; on_set(&pan_tilt[0]); return true;
		case KEY_Right: pan_tilt[0] += scale * 0.1f; on_set(&pan_tilt[0]); return true;
		case KEY_Down:  pan_tilt[1] += scale * 0.1f; on_set(&pan_tilt[1]); return true;
		case KEY_Up:    pan_tilt[1] -= scale * 0.1f; on_set(&pan_tilt[1]); return true;
		case KEY_Page_Down:  scale *= 1.1f; on_set(&scale); return true;
		case KEY_Page_Up:    scale /= 1.1f; on_set(&scale); return true;
		}
		return false;
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &range[0]) {
			if (find_control(min_value[0])) {
				for (unsigned i = 0; i < 4; ++i) {
					find_control(min_value[i])->set("min", range[0]);
					find_control(max_value[i])->set("min", range[0]);
				}
			}
		}
		if (member_ptr == &range[1]) {
			if (find_control(max_value[0])) {
				for (unsigned i = 0; i < 4; ++i) {
					find_control(min_value[i])->set("max", range[1]);
					find_control(max_value[i])->set("max", range[1]);
				}
			}
		}
		if (view_vox_file && member_ptr == &current_image) {
			vox_read_slice(current_image);
		}
		update_member(member_ptr);
		post_redraw();
	}
	void configure_gui()
	{
		if (find_control(current_image)) {
			find_control(current_image)->set("max", durations.size()-1);
			find_control(animate)->set("active", tex_ids.size() > 1);
			find_control(current_image)->set("active", tex_ids.size() > 1);
		}
		if (find_control(selection.ref_min_pnt()(0))) {
			find_control(selection.ref_min_pnt()(0))->set("max", W);
			find_control(selection.ref_max_pnt()(0))->set("max", W);
			find_control(selection.ref_min_pnt()(1))->set("max", H);
			find_control(selection.ref_max_pnt()(1))->set("max", H);
		}
		update_member(&selection.ref_min_pnt()(0));
		update_member(&selection.ref_max_pnt()(0));
		update_member(&selection.ref_min_pnt()(1));
		update_member(&selection.ref_max_pnt()(1));
		update_member(&file_name);
		update_member(&current_image);
		update_member(&use_blending);
	}
	void create_gui()
	{
		add_decorator("image view", "heading");

		add_control("&animate", animate, "check", "shortcut='A'");
		add_member_control(this, "current_image", current_image, "value_slider", "min=0;max=0;ticks=true");

		if (begin_tree_node("file io", file_name, true)) {
			align("\a");
			add_view("file_name", file_name);
			connect_copy(add_button("&open", "shortcut='O'")->click, rebind(this, &image_view::open));
			connect_copy(add_button("o&pen files", "shortcut='P'")->click, rebind(this, &image_view::open_files));
			connect_copy(add_button("&save", "shortcut='S'")->click, rebind(this, &image_view::save));
			align("\b");
			end_tree_node(file_name);
		}

		if (begin_tree_node("distortion", k1)) {
			align("\a");
			add_member_control(this, "tess_level", tess_level, "value_slider", "min=1;max=64;log=true;ticks=true");
			add_member_control(this, "checker_lambda", checker_lambda, "value_slider", "min=0.0;max=1.0;log=true;ticks=true");
			add_member_control(this, "s", s, "value_slider", "min=0.1;max=10;log=true;ticks=true");
			add_member_control(this, "cx", cx, "value_slider", "min=-1.0;max=1.0;log=true;ticks=true");
			add_member_control(this, "cy", cy, "value_slider", "min=-1.0;max=1.0;log=true;ticks=true");
			add_member_control(this, "k1", k1, "value_slider", "min=-1.0;max=1.0;log=true;ticks=true");
			add_member_control(this, "k2", k2, "value_slider", "min=-1.0;max=1.0;log=true;ticks=true");
			add_member_control(this, "k3", k3, "value_slider", "min=-1.0;max=1.0;log=true;ticks=true");
			add_member_control(this, "k4", k4, "value_slider", "min=-1.0;max=1.0;log=true;ticks=true");
			add_member_control(this, "k5", k5, "value_slider", "min=-1.0;max=1.0;log=true;ticks=true");
			add_member_control(this, "k6", k6, "value_slider", "min=-1.0;max=1.0;log=true;ticks=true");
			add_member_control(this, "p1", p1, "value_slider", "min=-1.0;max=1.0;log=true;ticks=true");
			add_member_control(this, "p2", p2, "value_slider", "min=-1.0;max=1.0;log=true;ticks=true");
			align("\b");
			end_tree_node(k1);
		}
		if (begin_tree_node("rendering", use_blending)) {
			align("\a");
			add_member_control(this, "use_blending", use_blending, "check");
			add_member_control(this, "wireframe", wireframe, "check");
			add_gui("gamma4", gamma4, "vector", "main_label='heading';components='rgba';options='min=0.01;max=100;ticks=true;log=true'");
			add_gui("min_value", min_value, "vector", "main_label='heading';components='rgba';options='min=0;max=1;ticks=true;step=0.00001;log=true'");
			add_gui("max_value", max_value, "vector", "main_label='heading';components='rgba';options='min=0;max=1;ticks=true;step=0.00001;log=true'");
			add_gui("range", range, "ascending", "main_label='heading';components='nx';options='min=0;max=1;ticks=true;step=0.00001;log=true'");
			add_member_control(this, "spherical", spherical, "check");
			add_member_control(this, "pan", pan_tilt[0], "value_slider", "min=-2;max=2;ticks=true");
			add_member_control(this, "tilt", pan_tilt[1], "value_slider", "min=-2;max=2;ticks=true");
			add_member_control(this, "scale", scale, "value_slider", "min=0.01;max=100;ticks=true;log=true");
			align("\b");
			end_tree_node(use_blending);
		}

		if (begin_tree_node("selection", show_selection)) {
			align("\a");	
			add_member_control(this, "show", show_selection, "check", "shortcut='R'");
			add_gui("rectangle", selection, "", "min_size=0.1;main_label='first';gui_type='value_slider';options='min=0;max=1;ticks=true;step=1'");
			align("\b");
			end_tree_node(show_selection);
		}
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
	void vox_read_slice(unsigned slide)
	{
		get_context()->make_current();
		if (cgv::utils::file::read(file_name, dv.get_ptr<char>(), slice_size, false, current_image*slice_size)) {
			glBindTexture(GL_TEXTURE_2D, tex_ids.front());
			cgv::render::gl::replace_texture(dv);
		}
	}
	bool open_volume(const std::string& _file_name, std::string ext)
	{
		cgv::media::volume::volume_info V;
		if (ext == "vox")
			ext = "hd";
		else if (ext == "qim")
			ext = "qha";

		if (!cgv::media::volume::read_header(cgv::utils::file::drop_extension(_file_name)+"."+ext, V))
			return false;
		size_t w = V.dimensions(0);
		size_t h = V.dimensions(1);
		size_t voxel_size = cgv::type::info::get_type_size(V.type_id);
		df = cgv::data::data_format(unsigned(w), unsigned(h), V.type_id, cgv::data::CF_L);
		dv = cgv::data::data_view(&df);
		if (!cgv::utils::file::read(_file_name, dv.get_ptr<char>(), w * h * voxel_size))
			return false;
		get_context()->make_current();
		if (tex_ids.size() > 0) {
			glDeleteTextures(GLsizei(tex_ids.size()), &tex_ids.front());
			durations.clear();
			tex_ids.clear();
		}
		durations.resize(V.dimensions(2), 0.05f);
		tex_ids.resize(V.dimensions(2), cgv::render::gl::create_texture(dv, false));
		W = int(w);
		H = int(h);
		aspect = float(W) / H;
		slice_size = w*h*voxel_size;
		selection.invalidate();
		selection.add_point(vec2i(0, 0));
		selection.add_point(vec2i(W, H));
		current_image = 0;
		start_time = -2;
		file_name = _file_name;
		files.clear();
		view_vox_file = true;
		post_redraw();
		return true;
	}
	void open()
	{
		get_context()->make_current();
		if (tex_ids.size() > 0) {
			glDeleteTextures(GLsizei(tex_ids.size()),&tex_ids.front());
			durations.clear();
			tex_ids.clear();
		}
		std::string filter_string = image_reader::construct_filter_string()+";*.vox;*.qim";
		std::string file_name = cgv::gui::file_open_dialog("open image file", filter_string, cgv::base::ref_data_path_list().empty()?"":cgv::base::ref_data_path_list()[0]+"/Regular/Images");
		if (!file_name.empty()) {
			std::string ext = cgv::utils::to_lower(cgv::utils::file::get_extension(file_name));
			if (ext == "vox" || ext == "qim") {
				open_volume(file_name, ext);
			}
			else {
				bool success = read_image(file_name) || read_image("res://cgv_logo.png");
				if (success)
					view_vox_file = false;
			}
			configure_gui();
			post_redraw();
		}
	}
	void open_files()
	{
		get_context()->make_current();
		if (tex_ids.size() > 0) {
			glDeleteTextures(GLsizei(tex_ids.size()),&tex_ids.front());
			durations.clear();
			tex_ids.clear();
		}
		std::vector<std::string> file_names;
		std::string filter_string = image_reader::construct_filter_string();
		std::string file_path = cgv::gui::files_open_dialog(file_names, "open image files as animation", filter_string, cgv::base::ref_data_path_list().empty()?"":cgv::base::ref_data_path_list()[0]+"/Regular/Images");
//		std::cout << "path = " << file_path << std::endl;
//		for (unsigned i=0; i<file_names.size(); ++i)
//			std::cout << "  file " << i << " = " << file_names[i] << std::endl;
		read_images(file_path, file_names);
		view_vox_file = false;
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
	bool init(context& ctx)
	{		
		return image_drawable::init(ctx) && read_image("res://alhambra.png");
	}
};

#include <cgv/base/register.h>

factory_registration<image_view> image_drawable_factory_registration("New/Media/Image View", "shortcut='Alt-Ctrl-I'", true);

