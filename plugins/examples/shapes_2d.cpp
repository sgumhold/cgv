#include <random>
#include <unordered_map>

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/math/ftransform.h>
#include <cgv/media/image/image.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_glutil/shader_library.h>

#include <cgv_glutil/msdf_gl_font_renderer.h>
#include <cgv_glutil/generic_renderer.h>

#include <cgv_glutil/2d/draggable.h>
#include <cgv_glutil/2d/draggables_collection.h>
#include <cgv_glutil/2d/rect.h>



class shapes_2d :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
private:
	/** Define a helper struct for a circle-shaped draggable control point.
	*/
	struct point : public cgv::glutil::draggable {
		point(const ivec2& pos) {
			this->pos = pos;
			size = vec2(8.0f);
			position_is_center = true;
			constraint_reference = CR_FULL_SIZE;
		}

		bool is_inside(const vec2& mp) const {

			float dist = length(mp - center());
			return dist <= size.x();
		}

		ivec2 get_render_position() const {
			return ivec2(pos + 0.5f);
		}

		ivec2 get_render_size() const {
			return 2 * ivec2(size);
		}
	};

protected:
	cgv::glutil::rect viewport_rect;

	cgv::glutil::shader_library shaders;

	bool show_background;
	cgv::render::texture background_tex;
	cgv::render::texture image_tex;
	
	std::vector<point> points;
	cgv::glutil::draggables_collection<point*> line_handles;
	cgv::glutil::draggables_collection<point*> arrow_handles;
	cgv::glutil::draggables_collection<point*> curve_handles;
	cgv::glutil::draggables_collection<point*> text_handles;

	cgv::glutil::generic_renderer line_renderer;
	cgv::glutil::generic_renderer spline_renderer;

	DEFINE_GENERIC_RENDER_DATA_CLASS(line_geometry, 2, vec2, position, rgba, color);
	line_geometry lines, control_lines;

	DEFINE_GENERIC_RENDER_DATA_CLASS(spline_geometry, 3, vec2, position, vec2, tangent, rgba, color);
	spline_geometry curves;

	// shape appearance attributes
	rgba color = rgba(0.7f, 0.7f, 1.0f, 1.0f);
	rgba border_color = rgba(0.4f, 0.4f, 0.9f, 1.0f);
	float border_width = 5.0f;
	float border_radius = 0.0f;
	float ring_width = 0.0f;
	float feather_width = 1.0f;
	float feather_origin = 0.5f;

	// arrow appearance attributes
	float stem_width = 20.0f;
	float head_width = 40.0f;
	float absolute_head_length = 50.0f;
	float relative_head_length = 0.25f;
	bool head_length_is_relative = false;

	// line appearance attributes
	float line_width = 20.0f;
	float dash_length = 0.0f;
	float dash_ratio = 0.5f;

	// shape render options
	bool use_color = true;
	bool use_blending = true;
	bool use_smooth_feather = false;
	bool apply_gamma = true;

	// text appearance
	float font_size = 32.0f;
	cgv::render::TextAlignment text_align_h, text_align_v;

	cgv::glutil::msdf_font msdf_font;
	cgv::glutil::msdf_text_geometry texts;
	cgv::glutil::msdf_gl_font_renderer font_renderer;

	// test variables
	vec2 translation = vec2(0.0f);
	float scale = 1.0f;
	float angle = 0.0f;

public:
	shapes_2d() : cgv::base::node("shapes 2d test") {
		viewport_rect.set_pos(ivec2(0));
		viewport_rect.set_size(ivec2(-1));
		
		show_background = true;

		// load some specific 2d shaders
		shaders.add("rectangle", "rect2d.glpr");
		shaders.add("circle", "circle2d.glpr");
		shaders.add("ellipse", "ellipse2d.glpr");
		shaders.add("arrow", "arrow2d.glpr");
		
		line_renderer = cgv::glutil::generic_renderer("line2d.glpr");
		spline_renderer = cgv::glutil::generic_renderer("cubic_spline2d.glpr");

		text_align_h = text_align_v = cgv::render::TA_NONE;

		// set callbacks for changes to draggable control points
		line_handles.set_drag_callback(std::bind(&shapes_2d::create_line_render_data, this));
		curve_handles.set_drag_callback(std::bind(&shapes_2d::create_curve_render_data, this));
		text_handles.set_drag_callback(std::bind(&shapes_2d::set_text_positions, this));
	}
	void stream_help(std::ostream& os) {
		return;
	}
	bool handle(cgv::gui::event& e) {
		bool handled = false;
		handled |= arrow_handles.handle(e, viewport_rect.size());
		handled |= line_handles.handle(e, viewport_rect.size());
		handled |= curve_handles.handle(e, viewport_rect.size());
		handled |= text_handles.handle(e, viewport_rect.size());

		if(handled)
			post_redraw();

		return handled;
	}
	void on_set(void* member_ptr) {
		if(
			member_ptr == &color[0] ||
			member_ptr == &color[1] ||
			member_ptr == &color[2] ||
			member_ptr == &color[3] ||
			member_ptr == &border_color[0] ||
			member_ptr == &border_color[1] ||
			member_ptr == &border_color[2] ||
			member_ptr == &border_color[3]
		) {
			create_line_render_data();
			create_curve_render_data();
		}

		if(member_ptr == &text_align_h || member_ptr == &text_align_v) {
			for(size_t i = 0; i < texts.size(); ++i)
				texts.set_alignment(i, static_cast<cgv::render::TextAlignment>(text_align_h | text_align_v));
		}

		if(member_ptr == &font_size) {
			texts.set_font_size(font_size);
		}

		post_redraw();
		update_member(member_ptr);
	}
	std::string get_type_name() const {
		return "shapes_2d";
	}
	void clear(cgv::render::context& ctx) {
		shaders.clear(ctx);
		background_tex.destruct(ctx);

		msdf_font.destruct(ctx);
		font_renderer.destruct(ctx);
	}
	bool init(cgv::render::context& ctx) {
		bool success = true;
		success &= shaders.load_shaders(ctx);
		success &= line_renderer.init(ctx);
		success &= spline_renderer.init(ctx);

		// TODO: png images are flipped in y direction, when reading with an image reader first and then creating a texture from the data view

		// create a checkerboard texture to use as the background
		{
			rgb a(0.85f);
			rgb b(0.95f);
			std::vector<rgb> bg_data = { a, b, b, a };

			background_tex.destruct(ctx);
			cgv::data::data_view bg_dv = cgv::data::data_view(new cgv::data::data_format(2, 2, TI_FLT32, cgv::data::CF_RGB), bg_data.data());
			background_tex = texture("flt32[R,G,B]", TF_NEAREST, TF_NEAREST, TW_REPEAT, TW_REPEAT);
			background_tex.create(ctx, bg_dv, 0);
		}

		// load an image to use as a texture
		{
			cgv::data::data_format image_format;
			cgv::data::data_view image_data;
			image_tex.create_from_image(image_format, image_data, ctx, "res://alhambra.png", (unsigned char*)0, 0);
		}
		
		success &= msdf_font.init(ctx);
		success &= font_renderer.init(ctx);
		texts.set_msdf_font(&msdf_font);

		// add 2 control points for the arrow
		points.push_back(point(ivec2(600, 600)));
		points.push_back(point(ivec2(700, 600)));
		// add 2 control points for the line
		points.push_back(point(vec2(100, 500)));
		points.push_back(point(vec2(500, 600)));
		// add 4 control points for the curve
		points.push_back(point(vec2(600, 300)));
		points.push_back(point(vec2(650, 400)));
		points.push_back(point(vec2(700, 250)));
		points.push_back(point(vec2(800, 300)));
		// add 2 control points for the texts
		points.push_back(point(ivec2(750, 150)));
		points.push_back(point(ivec2(500, 450)));
		
		// put pointers to the control points into their respective draggables collection
		arrow_handles.add(&points[0]);
		arrow_handles.add(&points[1]);

		line_handles.add(&points[2]);
		line_handles.add(&points[3]);

		curve_handles.add(&points[4]);
		curve_handles.add(&points[5]);
		curve_handles.add(&points[6]);
		curve_handles.add(&points[7]);

		text_handles.add(&points[8]);
		text_handles.add(&points[9]);

		create_line_render_data();
		create_curve_render_data();
		create_text_render_data();

		return success;
	}
	void init_frame(cgv::render::context& ctx) {
		ivec2 viewport_resolution(ctx.get_width(), ctx.get_height());

		if(viewport_resolution != viewport_rect.size()) {
			viewport_rect.set_size(viewport_resolution);

			set_resolution_uniform(ctx, shaders.get("rectangle"));
			set_resolution_uniform(ctx, shaders.get("circle"));
			set_resolution_uniform(ctx, shaders.get("ellipse"));
			set_resolution_uniform(ctx, shaders.get("arrow"));
			set_resolution_uniform(ctx, line_renderer.ref_prog());
			set_resolution_uniform(ctx, spline_renderer.ref_prog());

			// update the constraint for all draggables
			arrow_handles.set_constraint(viewport_rect);
			line_handles.set_constraint(viewport_rect);
			curve_handles.set_constraint(viewport_rect);
			text_handles.set_constraint(viewport_rect);
		}
	}
	void draw(cgv::render::context& ctx) {
		if(!background_tex.is_created())
			return;

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(show_background)
			draw_background(ctx);

		image_tex.enable(ctx, 0);

		shader_program& rect_prog = shaders.get("rectangle");
		rect_prog.enable(ctx);
		rect_prog.set_uniform(ctx, "position", ivec2(100, 100));
		rect_prog.set_uniform(ctx, "size", ivec2(200, 100));
		set_shared_uniforms(ctx, rect_prog);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		rect_prog.disable(ctx);

		shader_program& circle_prog = shaders.get("circle");
		circle_prog.enable(ctx);
		circle_prog.set_uniform(ctx, "position", ivec2(500, 150));
		circle_prog.set_uniform(ctx, "size", ivec2(100)); // size defines the diameter, both components must be set to the same value
		circle_prog.set_uniform(ctx, "position_is_center", true);
		set_shared_uniforms(ctx, circle_prog);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		circle_prog.disable(ctx);

		shader_program& ellipse_prog = shaders.get("ellipse");
		ellipse_prog.enable(ctx);
		ellipse_prog.set_uniform(ctx, "position", ivec2(200, 300));
		ellipse_prog.set_uniform(ctx, "size", ivec2(200, 100));
		ellipse_prog.set_uniform(ctx, "position_is_center", true);
		set_shared_uniforms(ctx, ellipse_prog);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		ellipse_prog.disable(ctx);

		shader_program& arrow_prog = shaders.get("arrow");
		arrow_prog.enable(ctx);
		arrow_prog.set_uniform(ctx, "position_a", ivec2(arrow_handles[0]->pos));
		arrow_prog.set_uniform(ctx, "position_b", ivec2(arrow_handles[1]->pos));
		arrow_prog.set_uniform(ctx, "stem_width", stem_width);
		arrow_prog.set_uniform(ctx, "head_width", head_width);
		arrow_prog.set_uniform(ctx, "head_length", head_length_is_relative ? relative_head_length : absolute_head_length);
		arrow_prog.set_uniform(ctx, "head_length_is_relative", head_length_is_relative);
		set_shared_uniforms(ctx, arrow_prog);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		arrow_prog.disable(ctx);

		shader_program& line_prog = line_renderer.ref_prog();
		line_prog.enable(ctx);
		set_shared_uniforms(ctx, line_prog);
		line_prog.set_uniform(ctx, "width", line_width);
		line_prog.set_uniform(ctx, "dash_length", dash_length);
		line_prog.set_uniform(ctx, "dash_ratio", dash_ratio);
		line_prog.disable(ctx);
		line_renderer.render(ctx, PT_LINES, lines);

		shader_program& spline_prog = spline_renderer.ref_prog();
		spline_prog.enable(ctx);
		set_shared_uniforms(ctx, spline_prog);
		spline_prog.set_uniform(ctx, "width", line_width);
		spline_prog.set_uniform(ctx, "dash_length", dash_length);
		spline_prog.set_uniform(ctx, "dash_ratio", dash_ratio);
		spline_prog.disable(ctx);
		spline_renderer.render(ctx, PT_LINES, curves);

		image_tex.disable(ctx);

		set_shared_uniforms(ctx, font_renderer.ref_prog());
		font_renderer.render(ctx, viewport_rect.size(), texts);
		
		draw_control_lines(ctx);
		draw_draggables(ctx);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	void draw_background(cgv::render::context& ctx) {
		shader_program& rect_prog = shaders.get("rectangle");
		rect_prog.enable(ctx);

		//mat2 MM = cgv::math::rotate2(angle);
		//rect_prog.set_uniform(ctx, "model_matrix", MM);

		rect_prog.set_uniform(ctx, "position", ivec2(0));
		rect_prog.set_uniform(ctx, "size", viewport_rect.size());
		rect_prog.set_uniform(ctx, "border_width", 0.0f);
		rect_prog.set_uniform(ctx, "border_radius", 0.0f);
		rect_prog.set_uniform(ctx, "ring_width", 0.0f);
		rect_prog.set_uniform(ctx, "feather_width", 0.0f);
		rect_prog.set_uniform(ctx, "tex_scaling", vec2(viewport_rect.size()) / 20.0f);
		rect_prog.set_uniform(ctx, "tex", 0);

		rect_prog.set_uniform(ctx, "use_color", false);
		rect_prog.set_uniform(ctx, "use_blending", false);
		rect_prog.set_uniform(ctx, "apply_gamma", true);
		
		background_tex.enable(ctx, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		background_tex.disable(ctx);

		rect_prog.set_uniform(ctx, "tex_scaling", vec2(1.0f));

		rect_prog.disable(ctx);
	}
	void draw_control_lines(cgv::render::context& ctx) {
		shader_program& line_prog = line_renderer.ref_prog();
		line_prog.enable(ctx);

		line_prog.set_uniform(ctx, "width", 2.0f);
		line_prog.set_uniform(ctx, "border_width", 0.0f);
		line_prog.set_uniform(ctx, "dash_length", 10.0f);
		line_prog.set_uniform(ctx, "dash_ratio", 0.75f);
		line_prog.set_uniform(ctx, "feather_width", 1.0f);

		line_prog.set_uniform(ctx, "use_color", true);
		line_prog.set_uniform(ctx, "use_blending", true);
		line_prog.set_uniform(ctx, "apply_gamma", true);
		line_prog.disable(ctx);

		line_renderer.render(ctx, PT_LINES, control_lines);
	}
	void draw_draggables(cgv::render::context& ctx) {
		shader_program& point_prog = shaders.get("circle");
		point_prog.enable(ctx);

		point_prog.set_uniform(ctx, "position_is_center", true);
		point_prog.set_uniform(ctx, "border_color", rgba(0.2f, 0.2f, 0.2f, 1.0f));
		point_prog.set_uniform(ctx, "border_width", 1.5f);
		point_prog.set_uniform(ctx, "border_radius", 0.0f);
		point_prog.set_uniform(ctx, "ring_width", 0.0f);
		point_prog.set_uniform(ctx, "feather_width", 1.0f);
		point_prog.set_uniform(ctx, "feather_origin", 0.5f);

		point_prog.set_uniform(ctx, "use_color", true);
		point_prog.set_uniform(ctx, "use_blending", true);
		point_prog.set_uniform(ctx, "apply_gamma", true);

		for(unsigned i = 0; i < points.size(); ++i) {
			const point& p = points[i];
			point_prog.set_uniform(ctx, "position", p.get_render_position());
			point_prog.set_uniform(ctx, "size", p.get_render_size());
			point_prog.set_uniform(ctx, "color", vec4(0.9f, 0.9f, 0.9f, 1.0f));
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		point_prog.disable(ctx);
	}
	void create_line_render_data() {
		lines.clear();
		for(unsigned  i = 0; i < 2; ++i) {
			float brightness = static_cast<float>(i) / 1;
			lines.add(line_handles[i]->pos, brightness * color);
		}
		lines.set_out_of_date();
	}
	void create_curve_render_data() {
		auto& control_points = curve_handles.ref_draggables();

		curves.clear();
		control_lines.clear();
		for(unsigned i = 0; i < 2; ++i) {
			unsigned idx = 2*i;
			unsigned si = idx;
			unsigned ei = idx + 1;
			unsigned pi = (i % 2) ? ei : si;
			vec2 tangent = 3.0f * (control_points[ei]->pos - control_points[si]->pos);
			
			curves.add(control_points[pi]->pos, tangent, color);
			control_lines.add(control_points[si]->pos, rgba(0.7f, 0.2f, 0.2f, 1.0f));
			control_lines.add(control_points[ei]->pos, rgba(0.7f, 0.2f, 0.2f, 1.0f));
		}
		curves.set_out_of_date();
		control_lines.set_out_of_date();
	}
	void create_text_render_data() {
		cgv::render::context* ctx_ptr = get_context();
		if(!ctx_ptr)
			return;
		cgv::render::context& ctx = *ctx_ptr;

		std::vector<std::string> labels;
		labels.push_back("Hello World!");
		labels.push_back("CGV Framework");

		texts.clear();
		for(unsigned i = 0; i < 2; ++i) {
			std::string str = labels[i];
			texts.add_text(str, text_handles[i]->pos, static_cast<cgv::render::TextAlignment>(text_align_h | text_align_v));
		}
	}
	void set_text_positions() {
		for(size_t i = 0; i < 2; ++i)
			texts.set_position(i, text_handles[i]->pos);

		ivec2 p(text_handles[0]->pos);
		std::string pos_str = "(";
		pos_str += std::to_string(p.x());
		pos_str += ", ";
		pos_str += std::to_string(p.y());
		pos_str += ")";
		texts.set_text(0, pos_str);
	}
	void set_resolution_uniform(cgv::render::context& ctx, cgv::render::shader_program& prog) {
		prog.enable(ctx);
		prog.set_uniform(ctx, "resolution", viewport_rect.size());
		prog.set_uniform(ctx, "tex", 0);
		prog.disable(ctx);
	}
	void set_shared_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog) {

		/*vec2 vs(viewport_rect.size());
		mat4 PM = cgv::math::ortho4(0.0f, vs.x(), 0.0f, vs.y(), 0.0f, 10.0f);
		prog.set_uniform(ctx, "projection_matrix", PM);
		
		//mat2 T = cgv::math::translate2(vec2(translation));
		mat2 S = cgv::math::scale2(vec2(scale));
		mat2 R = cgv::math::rotate2(angle);

		mat2 MM = S * R;
		
		prog.set_uniform(ctx, "model_matrix", MM);*/

		// appearance
		prog.set_uniform(ctx, "color", color);
		prog.set_uniform(ctx, "border_color", border_color);
		prog.set_uniform(ctx, "border_width", border_width);
		prog.set_uniform(ctx, "border_radius", border_radius);
		prog.set_uniform(ctx, "ring_width", ring_width);
		prog.set_uniform(ctx, "feather_width", feather_width);
		prog.set_uniform(ctx, "feather_origin", feather_origin);

		// options
		prog.set_uniform(ctx, "use_color", use_color);
		prog.set_uniform(ctx, "use_blending", use_blending);
		prog.set_uniform(ctx, "use_smooth_feather", use_smooth_feather);
		prog.set_uniform(ctx, "apply_gamma", apply_gamma);
	}
	point* get_hit_point(const ivec2& pos) {
		point* hit = nullptr;
		for(unsigned i = 0; i < points.size(); ++i) {
			point& p = points[i];
			if(p.is_inside(pos))
				hit = &p;
		}
		return hit;
	}
	void create_gui() {
		add_decorator("Shapes 2D", "heading");

		add_decorator("Example Settings", "heading", "level=3");
		add_member_control(this, "Show Background", show_background, "check");

		add_decorator("Render Options", "heading", "level=3");
		add_member_control(this, "Use Color", use_color, "check");
		add_member_control(this, "Use Blending", use_blending, "check");
		add_member_control(this, "Use Smooth Feather", use_smooth_feather, "check");
		add_member_control(this, "Apply Gamma", apply_gamma, "check");

		add_decorator("Appearance", "heading", "level=3");
		add_member_control(this, "Color", color, "");
		add_member_control(this, "Border Color", border_color, "");
		add_member_control(this, "Border Width", border_width, "value_slider", "min=0;max=20;step=0.5;ticks=true");
		add_member_control(this, "Border Radius", border_radius, "value_slider", "min=0;max=20;step=0.5;ticks=true");
		add_member_control(this, "Ring Width", ring_width, "value_slider", "min=0;max=20;step=0.5;ticks=true");
		add_member_control(this, "Feather Width", feather_width, "value_slider", "min=0;max=20;step=0.5;ticks=true");
		add_member_control(this, "Feather Origin", feather_origin, "value_slider", "min=0;max=1;step=0.01;ticks=true");

		add_decorator("Arrow Appearance", "heading", "level=3");
		add_member_control(this, "Stem Width", stem_width, "value_slider", "min=0;max=100;step=0.5;ticks=true");
		add_member_control(this, "Head Width", head_width, "value_slider", "min=0;max=100;step=0.5;ticks=true");
		add_member_control(this, "Absolute Head Length", absolute_head_length, "value_slider", "min=0;max=200;step=0.5;ticks=true");
		add_member_control(this, "Relative Head Length", relative_head_length, "value_slider", "min=0;max=1;step=0.01;ticks=true");
		add_member_control(this, "Head Length is Relative", head_length_is_relative, "check");
			
		add_decorator("Line Appearance", "heading", "level=3");
		add_member_control(this, "Width", line_width, "value_slider", "min=0;max=40;step=0.5;ticks=true");
		add_member_control(this, "Dash Length", dash_length, "value_slider", "min=0;max=100;step=0.5;ticks=true");
		add_member_control(this, "Dash Ratio", dash_ratio, "value_slider", "min=0;max=1;step=0.01;ticks=true");

		add_decorator("Text Appearance", "heading", "level=3");
		add_member_control(this, "Font Size", font_size, "value_slider", "min=1;max=256;step=0.5;ticks=true");
		add_member_control(this, "Horizontal Alignment", text_align_h, "dropdown", "enums='Center=0,Left=1,Right=2'");
		add_member_control(this, "Vertical Alignment", text_align_v, "dropdown", "enums='Center=0,Top=4,Botom=8'");

		add_decorator("Test Variables", "heading", "level=3");
		add_member_control(this, "Translation X", translation[0], "value_slider", "min=-100;max=100;step=0.5;ticks=true");
		add_member_control(this, "Translation Y", translation[1], "value_slider", "min=-100;max=100;step=0.5;ticks=true");
		add_member_control(this, "Scale", scale, "value_slider", "min=1;max=5;step=0.1;ticks=true");
		add_member_control(this, "Angle", angle, "value_slider", "min=0;max=360;step=0.5;ticks=true");
	}
};

#include <cgv/base/register.h>

/// register a factory to create new rounded cone texturing tests
cgv::base::factory_registration<shapes_2d> shapes_2d_fac("new/demo/shapes_2d");
