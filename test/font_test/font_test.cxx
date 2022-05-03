#include <cgv/base/node.h>
#include <cgv/base/register.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>
#include <cgv/render/drawable.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cgv/math/ftransform.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/media/color_scale.h>
#include <libs/tt_gl_font/tt_gl_font.h>
// include self reflection helpers of used types (here vec3 & rgb)
#include <libs/cgv_reflect_types/math/fvec.h>
#include <libs/cgv_reflect_types/media/color.h>

class font_test :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::base::argument_handler,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
protected:
	/// helper member to support loading individual truetype files
	std::string font_file_name;
	/// helper member to support scanning of specific font directories
	std::string font_directory;

	/// currently chosen font
	int font_idx;
	/// pointer to current font
	cgv::tt_gl_font_ptr font_ptr;
	/// font face selector
	cgv::media::font::FontFaceAttributes ffa;
	/// pointer to current font face
	cgv::tt_gl_font_face_ptr font_face_ptr;
	/// rasterization size of font
	float font_size;

	/// whether to use blending during font rendering
	bool use_blending;
	/// render style for drawing font onto cube
	cgv::render::rectangle_render_style rrs;

	/// to be drawn text
	std::string text;
	/// whether to use per character colors when drawing text
	bool use_colors;
	/// screen location in pixel coordinates
	ivec2 pixel_position;
	/// subpixel offset that allows to debug font rendering results if texels and pixels are not aligned
	vec2 subpixel_offset;

	/// whether to show lines
	bool show_lines;
	/// y-pixel coordinate to start drawing lines
	int fst_line;
	/// y-pixel offset between successive lines
	unsigned line_delta;
	
	/// whether to show cube
	bool show_cube;

	void render_textured_quads(cgv::render::context& ctx, cgv::render::rectangle_render_style& _rrs, 
		const std::vector<cgv::render::textured_rectangle>& Q, 
		const std::vector<rgba>* colors_ptr = 0,
		const std::vector<vec3>* translations_ptr = 0,
		const std::vector<quat>* rotations_ptr = 0)
	{
		auto& rr = cgv::render::ref_rectangle_renderer(ctx);
		rr.set_render_style(_rrs);
		rr.set_textured_rectangle_array(ctx, Q);
		font_face_ptr->ref_texture(ctx).enable(ctx);
		if (colors_ptr)
			rr.set_color_array(ctx, *colors_ptr);
		if (translations_ptr)
			rr.set_translation_array(ctx, *translations_ptr);
		if (rotations_ptr)
			rr.set_rotation_array(ctx, *rotations_ptr);
		GLboolean blend;
		GLenum blend_src, blend_dst, depth;
		if (use_blending) {
			blend = glIsEnabled(GL_BLEND); glEnable(GL_BLEND);
			glGetIntegerv(GL_BLEND_DST, reinterpret_cast<GLint*>(&blend_dst));
			glGetIntegerv(GL_BLEND_SRC, reinterpret_cast<GLint*>(&blend_src));
			glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&depth));
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthFunc(GL_LEQUAL);
		}
		rr.render(ctx, 0, Q.size());
		if (use_blending) {
			if (!blend)
				glDisable(GL_BLEND);
			glDepthFunc(depth);
			glBlendFunc(blend_src, blend_dst);
		}
		font_face_ptr->ref_texture(ctx).disable(ctx);
	}
public:
	font_test() : node("font test")
	{
		font_idx = 0;
		ffa = cgv::media::font::FFA_REGULAR;
		font_size = 32.0f;
		use_blending = true;
		on_set(&font_idx); // ensure that font and font face pointers are set

		text = "This demo shows how to render text with the tt_gl_font library including <�������>!";
		pixel_position = ivec2(100, 100);
		use_colors = true;
		subpixel_offset = vec2(0.0f);

		show_lines = true;
		fst_line = 100;
		line_delta = 40;

		show_cube = true;

		rrs.surface_color = rgba(1, 0, 1, 1);
		rrs.pixel_blend = 0.0f;
		rrs.texture_mode = cgv::render::RTM_REPLACE_ALPHA;
		rrs.default_depth_offset = -0.0000001f;
		rrs.default_border_color = rgba(0, 0, 0, 0);
		rrs.map_color_to_material = cgv::render::CM_COLOR_AND_OPACITY;
		rrs.illumination_mode = cgv::render::IM_OFF;
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &font_file_name)
			read_font(font_file_name);
		
		if (member_ptr == &font_directory) {
			cgv::scan_fonts(font_directory);
			font_idx = 0;
			on_set(&font_idx);
			post_recreate_gui();
		}
		if (member_ptr == &ffa)
			font_face_ptr = font_ptr->get_font_face(ffa).up_cast<cgv::tt_gl_font_face>();
		
		if (member_ptr == &font_idx) {
			font_ptr = cgv::find_font(cgv::get_font_names()[font_idx]);
			font_ptr->set_font_size(font_size);
			bool first = true;
			std::string ffa_enum;
			static const char* ffa_names[4] = { "regular","bold","italic","bolditalic" };
			for (int _ffa = 0; _ffa < 4; ++_ffa) {
				if (font_ptr->supports_font_face(_ffa)) {
					if (first)
						first = false;
					else
						ffa_enum += ',';
					ffa_enum += ffa_names[_ffa];
				}
			}
			if (find_control(ffa)) 
				find_control(ffa)->set("enums", ffa_enum);
		
			while (!font_ptr->supports_font_face(ffa))
				ffa = cgv::media::font::FontFaceAttributes(((int)ffa + 1) % 4);
			update_member(&ffa);

			font_face_ptr = font_ptr->get_font_face(ffa).up_cast<cgv::tt_gl_font_face>();
		}
		if (font_ptr) {
			if (member_ptr == &font_size) {
				font_ptr->set_font_size(font_size);
			}
		}
		update_member(member_ptr);
		post_redraw();
	}
	bool read_font(const std::string& file_name)
	{
		cgv::tt_gl_font_face_ptr new_font_face_ptr = new cgv::tt_gl_font_face(file_name, font_size);
		if (new_font_face_ptr->is_valid()) {
			font_face_ptr = new_font_face_ptr;
			return true;
		}
		return false;
	}
	void handle_args(std::vector<std::string>& args)
	{
		for (auto a : args) {
			if (cgv::utils::file::exists(a)) {
				font_file_name = a;
				on_set(&font_file_name);
			}
			else if (cgv::utils::dir::exists(a)) {
				font_directory = a;
				on_set(&font_directory);
			}

		}
		// clear args vector as all args have been processed
		args.clear();
		post_redraw();
	}
	void stream_help(std::ostream& os)
	{
		os << "font test: toggle use_<B>lending, use_<C>olor, show_<L>ines, select font|size with left,right|up,down" << std::endl;
	}
	void stream_stats(std::ostream& os)
	{
		os << "font name = " << (font_ptr ? font_ptr->get_name() : "<empty>") << std::endl;
	}
	bool handle(cgv::gui::event& e)
	{
		if (e.get_kind() == cgv::gui::EID_KEY) {
			cgv::gui::key_event& ke = reinterpret_cast<cgv::gui::key_event&>(e);
			if (ke.get_action() == cgv::gui::KA_RELEASE)
				return false;
			switch (ke.get_key()) {
			case 'L':
				show_lines = !show_lines;
				on_set(&show_lines);
				break;
			case 'B':
				use_blending = !use_blending;
				on_set(&use_blending);
				break;
			case 'C':
				use_colors = !use_colors;
				on_set(&use_colors);
				break;
			case cgv::gui::KEY_Up:
				if (ke.get_modifiers() == 0) {
					font_size *= 1.2f;
					on_set(&font_size);
					return true;
				}
				break;
			case cgv::gui::KEY_Down:
				if (ke.get_modifiers() == 0) {
					font_size /= 1.2f;
					on_set(&font_size);
					return true;
				}
				break;
			case cgv::gui::KEY_Right:
				if (ke.get_modifiers() == 0) {
					if (font_idx + 1 < cgv::get_font_names().size()) {
						++font_idx;
						on_set(&font_idx);
					}
					return true;
				}
				break;
			case cgv::gui::KEY_Left:
				if (ke.get_modifiers() == 0) {
					if (font_idx > 0) {
						--font_idx;
						on_set(&font_idx);
					}
					return true;
				}
				break;
			}
		}
		return false;
	}
	std::string get_type_name() const { return "font_test"; }
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return 
			rh.reflect_member("font_idx", font_idx) &&
			rh.reflect_member("text", text) &&
			rh.reflect_member("use_blending", use_blending) &&
			rh.reflect_member("rrs", rrs) &&
			rh.reflect_member("font_file_name", font_file_name) &&
			rh.reflect_member("font_directory", font_directory) &&
			rh.reflect_member("font_size", font_size);
	}
	bool init(cgv::render::context& ctx)
	{
		cgv::render::ref_rectangle_renderer(ctx, 1);
		return true;
	}
	void clear(cgv::render::context& ctx)
	{
		cgv::render::ref_rectangle_renderer(ctx, -1);
	}
	void draw(cgv::render::context& ctx)
	{
		if (show_cube) {
			// render cube first
			auto& surf_prog = ctx.ref_surface_shader_program();
			surf_prog.enable(ctx);
			ctx.tesselate_unit_cube(true);
			surf_prog.disable(ctx);
			// then render numbers onto sides
			if (font_face_ptr) {
				std::vector<cgv::render::textured_rectangle> Q;
				vec2 p(0.0f);
				box2 R;
				std::vector<vec3> positions;
				std::vector<quat> rotations;
				std::vector<rgba> colors;
				for (int i = 0; i < 6; ++i) {
					std::string t(1, (char)('1' + i));
					font_face_ptr->text_to_quads(p, t, Q);
					Q.back().rectangle.translate(-Q.back().rectangle.get_center());
					R.add_axis_aligned_box(Q.back().rectangle);
					// select z axis and offset as well as other axes
					int z_axis = i / 2;
					int delta = 2 * (i % 2) - 1;
					int x_axis = (z_axis + 1) % 3;
					int y_axis = (z_axis + 2) % 3;
					// define color
					rgba col(0.5f, 0.5f, 0.5f, 1.0f);
					col[z_axis] = 0.5f * (delta+1);
					colors.push_back(col);
					// define position in center of cube face
					vec3 pos(0.0f);
					pos[z_axis] = (float)delta;
					positions.push_back(pos);
					// define rotation of x and y axes onto cube face
					mat3 rot;
					rot.zeros();
					rot(x_axis, 0) = 1.0f;
					rot(y_axis, 1) = 1.0f;
					rot(z_axis, 2) = 1.0f;
					rotations.push_back(quat(rot));
				}
				// scale down quads to fit cube face
				float scale = 1.6f / R.get_extent()[R.get_max_extent_coord_index()];
				for (auto& q : Q)
					q.rectangle.scale(scale);
				render_textured_quads(ctx, rrs, Q, &colors, &positions, & rotations);
			}
		}
		ctx.push_pixel_coords();
		if (show_lines) {
			std::vector<vec3> P;
			std::vector<rgb> C;
			int y = ctx.get_height() - 1 - fst_line;
			while (y >= 0) {
				P.push_back(vec3(0.0f, float(y), 0.0f));
				C.push_back(rgb(1, 1, 1));
				P.push_back(vec3(float(ctx.get_width()), float(y), 0.0f));
				C.push_back(rgb(1, 1, 1));
				y -= line_delta;
			}
			auto& prog = ctx.ref_default_shader_program();
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_position_index(), P);
			cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_color_index(), C);
			cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_position_index());
			cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_color_index());
			prog.enable(ctx);
			glDrawArrays(GL_LINES, 0, (GLsizei)P.size());
			prog.disable(ctx);
			cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_position_index());
			cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_color_index());
		}
		if (font_face_ptr) {
			ctx.mul_modelview_matrix(cgv::math::translate4<float>(subpixel_offset[0], subpixel_offset[1], 0.0f));

			// default approach to draw text in single color
			if (!use_colors) {
				vec2 p = pixel_position;
				p[1] = ctx.get_height() - 1 - p[1];
				ctx.set_color(rrs.surface_color);
				font_face_ptr->enable(&ctx, font_size);
				font_face_ptr->draw_text(p[0], p[1], text);
			}
			else {
				// otherwise convert to textured rectangles
				std::vector<cgv::render::textured_rectangle> Q;
				vec2 fpix_pos(pixel_position);
				fpix_pos[1] = ctx.get_height() - 1 - fpix_pos[1];
				font_face_ptr->text_to_quads(fpix_pos, text, Q);
				if (!Q.empty()) {
					// define color array
					std::vector<rgba> C(Q.size());
					for (size_t i = 0; i < C.size(); ++i) {
						float v = (float)i / (C.size() - 1);
						C[i] = cgv::media::color_scale(v, cgv::media::CS_HUE);
					}
					// render with default rectangle render style 
					render_textured_quads(ctx, cgv::ref_rectangle_render_style(), Q, &C);
				}
			}
		}
		ctx.pop_pixel_coords();
	}
	void create_gui()
	{
		add_decorator("font test", "heading", "level=1");
		add_gui("font_file_name", font_file_name, "file_name");
		add_gui("font_directory", font_directory, "directory");
		add_member_control(this, "font", (cgv::type::DummyEnum&)font_idx, "dropdown", cgv::get_font_enum_declaration());
		add_member_control(this, "font face", ffa, "dropdown", "enums='regular,bold,italic,bold+italic'");
		add_member_control(this, "font_size", font_size, "value_slider", "min=6;max=128;log=true;ticks=true");

		add_decorator("text", "heading", "level=2");
		add_member_control(this, "use_colors", use_colors, "toggle");
		add_member_control(this, "text", text);
		add_member_control(this, "default_text_color", rrs.surface_color);
		add_gui("pixel_position", pixel_position, "vector", "options='min=0;step=0.01;max=300;ticks=true'");
		add_gui("subpixel_offset", subpixel_offset, "vector", "options='min=0;step=0.001;max=1;ticks=true'");

		add_decorator("lines", "heading", "level=2");
		add_member_control(this, "show_lines", show_lines, "toggle");
		add_member_control(this, "fst_line", fst_line, "value_slider", "min=-1;max=128;ticks=true");
		add_member_control(this, "line_delta", line_delta, "value_slider", "min=5;max=128;ticks=true");

		add_decorator("cube", "heading", "level=2");
		add_member_control(this, "show_cube", show_cube, "toggle");
		add_member_control(this, "use_blending", use_blending, "toggle");
		if (begin_tree_node("rectangle", rrs, true)) {
			align("\a");
			add_gui("rectangle", rrs);
			align("\b");
			end_tree_node(rrs);
		}
	}
};

#include <cgv/base/register.h>

cgv::base::object_registration<font_test> reg_font_test("");

#ifdef CGV_FORCE_STATIC
cgv::base::registration_order_definition ro_def("stereo_view_interactor;font_test");
#endif
