#include <cgv/base/node.h>
#include <cgv/base/register.h>

#include <cgv/utils/file.h>
#include <cgv/utils/advanced_scan.h> // support to split string into tokens

#include <cgv/math/ftransform.h> // provides support for typical transformation matrices

#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>

#include <cgv_gl/gl/gl.h> // use this include to have opengl commands available
#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv/render/shader_program.h>


/// different drag and drop modes
enum DNDMode
{
	DND_OFF,
	DND_LEFT,
	DND_RIGHT
};

/// simple drawable for stereo images
class stereo_image_view : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::render::drawable,     /// derive from drawable for drawing the cube
	public cgv::gui::event_handler,
	public cgv::base::argument_handler,
	public cgv::gui::provider
{
private:
	DNDMode dnd_mode;
	std::string dnd_file_name;
protected:
	std::string file_name_left, file_name_right;
	cgv::render::texture tex_left, tex_right;
	/// whether to swap left and right image
	bool swap_sides;
	/// flag storing whether to preserve the aspect ratio of stereo image
	bool preserve_aspect;
	/// whether to fill the whole viewport when preserving the aspect ratio
	bool fill_viewport;
	/// read texture from file and return whether this was successful
	bool read_texture(cgv::render::texture& tex, const std::string& file_name)
	{
		// ensure that gpu context is current
		if (!get_context())
			return false;
		cgv::render::context& ctx = *get_context();
		if (!ctx.make_current())
			return false;
		// read texture form image
		tex.destruct(ctx);
		return tex.create_from_image(ctx, file_name);
	}
public:
	/// init member variables here
	stereo_image_view() : node("my_image_view") 
	{
		swap_sides = false;
		preserve_aspect = true;
		fill_viewport = false;
		dnd_mode = DND_OFF;
	}
	/// handle command line arguments
	void handle_args(std::vector<std::string>& args)
	{
		bool next_is_left = true;
		for (size_t ai = 0; ai < args.size(); )
			if (cgv::utils::file::exists(args[ai]) &&
				read_texture(next_is_left ? tex_left : tex_right, args[ai])) {
				args.erase(args.begin() + ai);
				next_is_left = false;
			}
			else
				++ai;
	}
	/// handle key and mouse drag and drop (dnd) events
	bool handle(cgv::gui::event& e)
	{
		if (e.get_kind() == cgv::gui::EID_KEY) {
			cgv::gui::key_event& ke = reinterpret_cast<cgv::gui::key_event&>(e);
			if (ke.get_action() == cgv::gui::KA_RELEASE)
				return false;
			switch (ke.get_key()) {
			//case cgv::gui::KEY_Space:
			case 'S':
				swap_sides = !swap_sides;
				on_set(&swap_sides);
				return true;
			case 'A':
				preserve_aspect = !preserve_aspect;
				on_set(&preserve_aspect);
				return true;
			case 'F':
				fill_viewport = !fill_viewport;
				on_set(&fill_viewport);
				return true;
			}
		}
		else if (e.get_kind() == cgv::gui::EID_MOUSE) {
			cgv::gui::mouse_event& me = static_cast<cgv::gui::mouse_event&>(e);
			if ((me.get_flags() & cgv::gui::EF_DND) != 0) { // select drag and drop events only
				switch (me.get_action()) {
				case cgv::gui::MA_ENTER: // store drag and drop text on dnd enter event (it is not available in drag events)
					dnd_file_name = me.get_dnd_text();
				case cgv::gui::MA_DRAG: // during dragging check for drop side and update dnd_mode
					dnd_mode = me.get_x() < (int)get_context()->get_width() / 2 ? DND_LEFT : DND_RIGHT;
					post_redraw(); // ensure to redraw in case dnd_mode changes
					return true;
				case cgv::gui::MA_LEAVE: // when mouse leaves window, we cancel drag and drop 
					dnd_mode = DND_OFF;
					dnd_file_name.clear();
					post_redraw(); // ensure to redraw to reflect change in dnd_mode 
					return true;
				case cgv::gui::MA_RELEASE: // release corresponds to drop
					dnd_mode = me.get_x() < (int)get_context()->get_width() / 2 ? DND_LEFT : DND_RIGHT;
					dnd_file_name = me.get_dnd_text();
					// always interpret drop object as file name and try to read texture
					if (dnd_mode == DND_LEFT) {
						if (read_texture(tex_left, dnd_file_name)) {
							// in case of success update file name
							file_name_left = dnd_file_name;
							update_member(&file_name_left);
						}
					}
					else {
						if (read_texture(tex_right, dnd_file_name)) {
							// in case of success update file name
							file_name_right = dnd_file_name;
							update_member(&file_name_right);
						}
					}
					dnd_mode = DND_OFF;
					dnd_file_name.clear();
					post_redraw();
					return true;
				}
				return false;
			}
		}
		return false;
	}
	/// react to changes in member variables
	void on_set(void* member_ptr)
	{
		if (member_ptr == &file_name_left)
			read_texture(tex_left, file_name_left);
		if (member_ptr == &file_name_right)
			read_texture(tex_right, file_name_right);

		update_member(member_ptr);
		post_redraw();
	}
	/// reflect members to allow configuration through config file
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return
			rh.reflect_member("swap_sides", swap_sides) &&
			rh.reflect_member("preserve_aspect", preserve_aspect) &&
			rh.reflect_member("file_name_left", file_name_left) &&
			rh.reflect_member("file_name_right", file_name_right);
	}
	/// create simple UI
	void create_gui()
	{
		add_decorator("stereo image view", "heading");
		if (begin_tree_node("file io", file_name_left, true)) {
			align("\a");
			add_gui("file_name_left", file_name_left, "file_name", "title='open left image';filter='images (bmp,jpg,png,tif):*.jpg;*.png;*.tif;*.bmp|all files:*.*'");
			add_gui("file_name_right", file_name_right, "file_name", "title='open right image';filter='images (bmp,jpg,png,tif):*.jpg;*.png;*.tif;*.bmp|all files:*.*'");
			align("\b");
			end_tree_node(file_name_left);
		}
		add_member_control(this, "swap_sides", swap_sides, "toggle");
		add_member_control(this, "preserve_aspect", preserve_aspect, "toggle");
		add_member_control(this, "fill_viewport", fill_viewport, "toggle");
	}
	/// return type name
	std::string get_type_name() const 
	{ 
		return "stereo_image_view"; 
	}
	/// stream help on short cuts
	void stream_help(std::ostream& os)
	{
		os << "stereo_image_view: toggle <S>wap_sides, preserve <A>spect ratio, <F>ill viewport\n";
	}
	/// nothing to be initialized
	bool init(cgv::render::context& ctx)
	{		
		return true;
	}
	/// destruct left/right eye textures
	void clear(cgv::render::context& ctx)
	{
		tex_left.destruct(ctx);
		tex_right.destruct(ctx);
	}
	/// draw stereo image here
	void draw(cgv::render::context& ctx)
	{
		// determine which texture to render
		// in main rendering pass, we should render for the right eye and in stereo pass for left eye
		// in case of swap_sides we exchange left and right texture
		cgv::render::texture* tex_ptr = 0;
		if (!swap_sides) {
			if (ctx.get_render_pass() == cgv::render::RP_MAIN)
				tex_ptr = &tex_right;
			else if (ctx.get_render_pass() == cgv::render::RP_STEREO)
				tex_ptr = &tex_left;
		}
		else {
			if (ctx.get_render_pass() == cgv::render::RP_MAIN)
				tex_ptr = &tex_left;
			else if (ctx.get_render_pass() == cgv::render::RP_STEREO)
				tex_ptr = &tex_right;
		}
		// check whether texture is available
		if (tex_ptr && !tex_ptr->is_created())
			return;
		// ignore modelview and projection matrix in order to fill viewport completely
		ctx.push_modelview_matrix();
		ctx.set_modelview_matrix(cgv::math::identity4<float>());
		ctx.push_projection_matrix();
		ctx.set_projection_matrix(cgv::math::identity4<float>());
		// draw texture only if it is available
		if (preserve_aspect) {
			float viewport_aspect = (float)ctx.get_width() / ctx.get_height();
			float tex_aspect = (float)tex_ptr->get_width() / tex_ptr->get_height();
			if (viewport_aspect > tex_aspect)
				if (fill_viewport)
					ctx.mul_modelview_matrix(cgv::math::scale4<float>(1.0f, viewport_aspect / tex_aspect, 1.0f));
				else
					ctx.mul_modelview_matrix(cgv::math::scale4<float>(tex_aspect / viewport_aspect, 1.0f, 1.0f));
			else if (tex_aspect > viewport_aspect)
				if (fill_viewport)
					ctx.mul_modelview_matrix(cgv::math::scale4<float>(tex_aspect / viewport_aspect, 1.0f, 1.0f));
				else
					ctx.mul_modelview_matrix(cgv::math::scale4<float>(1.0f, viewport_aspect / tex_aspect, 1.0f));
		}
		// construct geometry of quad
		tex_ptr->enable(ctx);
		// use default shader program with texture support for rendering
		auto& prog = ctx.ref_default_shader_program(true);
		prog.enable(ctx);
		// default shader program multiplies color to colors looked up in texture
		ctx.set_color(rgb(1, 1, 1));
		// draw unit square [-1,1]² with texture coordinates [0,1]²
		ctx.tesselate_unit_square();
		// recover previous state
		prog.disable(ctx);
		tex_ptr->disable(ctx);
		ctx.pop_projection_matrix();
		ctx.pop_modelview_matrix();
	}
	// draw information for drag and drop illustration after the regular draw process
	void finish_frame(cgv::render::context& ctx)
	{
		// do nothing if not in drag and drop mode
		if (dnd_mode == DND_OFF)
			return;
		// construct to be drawn text as a list of lines
		std::vector<std::string> text_lines;
		// start with heading telling into which image to drag and drop
		text_lines.push_back((dnd_mode == DND_LEFT) ? "LEFT" : "RIGHT");
		std::vector<cgv::utils::token> toks;
		// interpret slashes '/' or '\' as white spaces to split file name into one token per directory eliminating all slashes
		cgv::utils::split_to_tokens(dnd_file_name, toks, "", false, "", "", "\\/");
		for (const auto& t : toks)
			if (&t == &toks.back()) // check for last loop iteration
				text_lines.push_back(to_string(t));
			else
				text_lines.push_back(to_string(t) + "/"); // add slash back to all directories

		// determine position to start drawing
		unsigned w_div_2 = ctx.get_width()/2;
		int x = (dnd_mode == DND_LEFT) ? w_div_2 / 2 : 3 * w_div_2 / 2;
		int y = unsigned(ctx.get_height()/2 - 0.5f * text_lines.size() * ctx.get_current_font_size());

		// disable depth test to draw over previously drawn stuff
		glDisable(GL_DEPTH_TEST);
		// set projection and modelview such that one can use pixel coordinates for positioning
		ctx.push_pixel_coords();
			ctx.set_color(rgb(1, 1, 1));
			for (const auto& txt : text_lines) {
				ctx.set_cursor(vecn(float(x), float(y)), txt, cgv::render::TA_BOTTOM);
				ctx.output_stream() << txt;
				std::cout << txt;
				y -= int(1.5f * ctx.get_current_font_size());
				if (&txt == &text_lines.front()) { // check for first iteration
					ctx.output_stream().flush(); // flush to ensure that white color is used for first line
					ctx.set_color(rgb(1, 1, 0));
				}
			}
			ctx.output_stream().flush();
		// recover previous state of GPU context
		ctx.pop_pixel_coords();
		glEnable(GL_DEPTH_TEST);
	}
};

#ifdef CGV_FORCE_STATIC
cgv::base::registration_order_definition ro_def("stereo_view_interactor;stereo_image_view");
#endif

cgv::base::object_registration<stereo_image_view> stereo_image_view_registration("stereo image view");
