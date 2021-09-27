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

#include <cgv_glutil/msdf_gl_font_renderer.h>
#include <cgv_glutil/generic_renderer.h>

#include <cgv_glutil/2d/draggable.h>
#include <cgv_glutil/2d/draggables_collection.h>
#include <cgv_glutil/2d/rect.h>

#include <cgv_glutil/2d/shape2d_styles.h>

using namespace cgv::render;

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

	cgv::glutil::canvas canvas;
	cgv::glutil::shape2d_style bg_style, rect_style, circle_style, text_style, draggable_style;
	cgv::glutil::line2d_style line_style, control_line_style;
	cgv::glutil::arrow2d_style arrow_style;

	bool show_background;
	cgv::render::texture background_tex;
	cgv::render::texture image_tex;
	
	std::vector<point> points;
	cgv::glutil::draggables_collection<point*> line_handles;
	cgv::glutil::draggables_collection<point*> arrow_handles;
	cgv::glutil::draggables_collection<point*> curve_handles;
	cgv::glutil::draggables_collection<point*> text_handles;

	cgv::glutil::generic_renderer line_renderer, spline_renderer, point_renderer;
	
	// TODO: find way to use ivec2 and vec2 as attribs
	DEFINE_GENERIC_RENDER_DATA_CLASS(point_geometry, 1, vec2, position);
	point_geometry draggable_points;

	DEFINE_GENERIC_RENDER_DATA_CLASS(line_geometry, 2, vec2, position, rgba, color);
	line_geometry lines, control_lines;

	DEFINE_GENERIC_RENDER_DATA_CLASS(spline_geometry, 3, vec2, position, vec2, tangent, rgba, color);
	spline_geometry curves;

	// shape appearance attributes
	const rgba light_blue = rgba(0.7f, 0.7f, 1.0f, 1.0f);
	const rgba blue = rgba(0.4f, 0.4f, 0.9f, 1.0f);

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
	struct {
		vec2 translation = vec2(0.0f);
		float scale = 1.0f;
		float angle = 0.0f;
	} view_params, model_params;

public:
	shapes_2d() : cgv::base::node("shapes 2d test") {
		viewport_rect.set_pos(ivec2(0));
		viewport_rect.set_size(ivec2(-1));
		
		show_background = true;

		canvas.register_shader("rectangle", "rect2d.glpr");
		canvas.register_shader("circle", "circle2d.glpr");
		canvas.register_shader("ellipse", "ellipse2d.glpr");
		canvas.register_shader("arrow", "arrow2d.glpr");

		line_renderer = cgv::glutil::generic_renderer("line2d.glpr");
		spline_renderer = cgv::glutil::generic_renderer("cubic_spline2d.glpr");
		point_renderer = cgv::glutil::generic_renderer("circle2d.glpr");

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

		mat3 M = get_view_matrix() * get_model_matrix();
		arrow_handles.set_transformation(M);
		line_handles.set_transformation(M);
		curve_handles.set_transformation(M);
		text_handles.set_transformation(M);

		handled |= arrow_handles.handle(e, viewport_rect.size());
		handled |= line_handles.handle(e, viewport_rect.size());
		handled |= curve_handles.handle(e, viewport_rect.size());
		handled |= text_handles.handle(e, viewport_rect.size());

		if(!handled) {
			unsigned et = e.get_kind();
			unsigned char modifiers = e.get_modifiers();

			if(et == cgv::gui::EID_MOUSE) {
				cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
				cgv::gui::MouseAction ma = me.get_action();

				if(ma == cgv::gui::MA_DRAG && me.get_button_state() & cgv::gui::MB_MIDDLE_BUTTON) {
					view_params.translation += vec2(me.get_dx(), -me.get_dy());
				}

				if(ma == cgv::gui::MA_WHEEL) {
					ivec2 mpos(me.get_x(), me.get_y());
					mpos.y() = viewport_rect.size().y() - mpos.y();

					vec2 origin = viewport_rect.box.get_center();
					vec2 offset = origin - mpos + view_params.translation;

					float scale = view_params.scale;
					scale *= me.get_dy() > 0 ? 0.5 : 2.0;

					scale = cgv::math::clamp(scale, 0.5f, 64.0f);

					if(view_params.scale != scale) {
						view_params.translation += me.get_dy() > 0 ? -0.5f*offset : offset;
						view_params.scale = scale;
						update_member(&view_params.scale);
						update_member(&view_params.translation[0]);
						update_member(&view_params.translation[1]);
					}
					handled = true;
				}
			}
		}

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
		//shaders.clear(ctx);
		canvas.destruct(ctx);
		background_tex.destruct(ctx);

		msdf_font.destruct(ctx);
		font_renderer.destruct(ctx);
	}
	bool init(cgv::render::context& ctx) {
		bool success = true;

		success &= canvas.init(ctx);

		success &= line_renderer.init(ctx);
		success &= spline_renderer.init(ctx);
		success &= point_renderer.init(ctx);

		set_default_styles();

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

			canvas.set_resolution(ctx, viewport_rect.size());

			set_resolution_uniform(ctx, line_renderer.ref_prog());
			set_resolution_uniform(ctx, spline_renderer.ref_prog());
			set_resolution_uniform(ctx, point_renderer.ref_prog());

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

		canvas.push_modelview_matrix();
		canvas.mul_modelview_matrix(ctx, get_view_matrix());
		canvas.mul_modelview_matrix(ctx, get_model_matrix());
		canvas.set_feather_scale(1.0f / view_params.scale);

		image_tex.enable(ctx, 0);

		auto& rect_prog = canvas.enable_shader(ctx, "rectangle");
		rect_style.apply(ctx, rect_prog);
		canvas.draw_shape(ctx, ivec2(100, 100), ivec2(200, 100), rgba(1, 0, 0, 1));
		canvas.disable_current_shader(ctx);

		auto& circle_prog = canvas.enable_shader(ctx, "circle");
		circle_style.apply(ctx, circle_prog);
		// size defines the diameter, both components must be set to the same value
		canvas.draw_shape(ctx, ivec2(500, 100), ivec2(100), rgba(0, 1, 0, 1));
		canvas.disable_current_shader(ctx);

		auto& ellipse_prog = canvas.enable_shader(ctx, "ellipse");
		circle_style.apply(ctx, ellipse_prog);
		// size defines the two diameters
		canvas.draw_shape(ctx, ivec2(200, 300), ivec2(200, 100), rgba(0, 1, 1, 1));
		canvas.disable_current_shader(ctx);

		auto& arrow_prog = canvas.enable_shader(ctx, "arrow");
		arrow_style.apply(ctx, arrow_prog);
		canvas.draw_shape2(ctx, arrow_handles[0]->pos, arrow_handles[1]->pos, rgba(1.0f, 0.0f, 1.0f, 1.0f), rgba(0.0f, 0.0f, 1.0f, 1.0f));
		canvas.disable_current_shader(ctx);

		shader_program& line_prog = line_renderer.ref_prog();
		line_prog.enable(ctx);
		line_style.apply(ctx, line_prog);
		canvas.set_view(ctx, line_prog);
		line_prog.disable(ctx);
		line_renderer.render(ctx, PT_LINES, lines);

		shader_program& spline_prog = spline_renderer.ref_prog();
		spline_prog.enable(ctx);
		canvas.set_view(ctx, spline_prog);
		line_style.apply(ctx, spline_prog);
		spline_prog.disable(ctx);
		spline_renderer.render(ctx, PT_LINES, curves);

		image_tex.disable(ctx);

		// TODO: use style as a parameter in the font renderer render method
		auto& font_prog = font_renderer.ref_prog();
		font_prog.enable(ctx);
		text_style.apply(ctx, font_prog);
		canvas.set_view(ctx, font_prog);
		font_prog.disable(ctx);
		font_renderer.render(ctx, viewport_rect.size(), texts);
		
		draw_control_lines(ctx);
		draw_draggables(ctx);

		canvas.set_feather_scale(1.0f);
		canvas.pop_modelview_matrix(ctx);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	void draw_background(cgv::render::context& ctx) {
		auto& rect_prog = canvas.enable_shader(ctx, "rectangle");
		bg_style.texcoord_scaling = vec2(viewport_rect.size()) / 20.0f;
		bg_style.apply(ctx, rect_prog);

		background_tex.enable(ctx, 0);
		canvas.draw_shape(ctx, ivec2(0), viewport_rect.size());
		background_tex.disable(ctx);

		canvas.disable_current_shader(ctx);
	}
	void draw_control_lines(cgv::render::context& ctx) {
		shader_program& line_prog = line_renderer.ref_prog();
		line_prog.enable(ctx);
		control_line_style.apply(ctx, line_prog);
		line_prog.disable(ctx);

		line_renderer.render(ctx, PT_LINES, control_lines);
	}
	void draw_draggables(cgv::render::context& ctx) {
		// TODO: move creation of render data to own function and call only when necessary
		draggable_points.clear();
		ivec2 render_size;

		for(unsigned i = 0; i < points.size(); ++i) {
			const point& p = points[i];
			draggable_points.add(p.get_render_position());
			render_size = p.get_render_size();
		}
		
		draggable_points.set_out_of_date();

		shader_program& point_prog = point_renderer.ref_prog();
		point_prog.enable(ctx);
		canvas.set_view(ctx, point_prog);
		draggable_style.apply(ctx, point_prog);
		point_prog.set_attribute(ctx, "size", vec2(render_size));
		point_prog.disable(ctx);
		point_renderer.render(ctx, PT_POINTS, draggable_points);
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

		const rgba colors[2] = {
			rgba(1.0f, 0.0f, 0.0f, 1.0f),
			rgba(1.0f, 1.0f, 0.0f, 1.0f)
		};

		curves.clear();
		control_lines.clear();
		for(unsigned i = 0; i < 2; ++i) {
			unsigned idx = 2*i;
			unsigned si = idx;
			unsigned ei = idx + 1;
			unsigned pi = (i % 2) ? ei : si;
			vec2 tangent = 3.0f * (control_points[ei]->pos - control_points[si]->pos);
			
			curves.add(control_points[pi]->pos, tangent, colors[i]);
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
		prog.disable(ctx);
	}
	void set_default_styles() {
		// set background style
		bg_style.feather_width = 0.0f;
		bg_style.use_texture = true;
		bg_style.use_blending = false;

		// set control line style
		control_line_style.use_fill_color = false;
		control_line_style.width = 2.0f;
		control_line_style.border_width = 0.0f;
		control_line_style.dash_length = 10.0f;
		control_line_style.dash_ratio = 0.75f;
		control_line_style.use_blending = true;

		// set draggable point style
		draggable_style.position_is_center = true;
		draggable_style.fill_color = rgba(0.9f, 0.9f, 0.9f, 1.0f);
		draggable_style.border_color = rgba(0.2f, 0.2f, 0.2f, 1.0f);
		draggable_style.border_width = 1.5f;
		draggable_style.use_blending = true;
		
		// set default style of all shapes
		set_default_shape_style(rect_style);
		set_default_shape_style(circle_style);
		set_default_shape_style(text_style);
		set_default_shape_style(line_style);
		set_default_shape_style(arrow_style);
		line_style.width = 20.0f;
		arrow_style.stem_width = 20.0f;
		arrow_style.head_width = 40.0f;
	}
	void set_default_shape_style(cgv::glutil::shape2d_style& s) {
		s.fill_color = light_blue;
		s.border_color = blue;
		s.border_width = 5.0f;
		s.use_blending = true;
	}
	mat3 get_view_matrix() {
		mat3 T0 = cgv::math::translate2h(vec2(-viewport_rect.box.get_center()));
		mat3 T1 = cgv::math::translate2h(vec2(viewport_rect.box.get_center()));
		mat3 T = cgv::math::translate2h(vec2(view_params.translation));
		mat3 S = cgv::math::scale2h(vec2(view_params.scale));
		mat3 R = cgv::math::rotate2h(view_params.angle);
		//return T * S * R; // pivot is in lower left corner
		return T * T1 * S * R * T0; // pivot is in viewport center
	}
	mat3 get_model_matrix() {
		mat3 T = cgv::math::translate2h(vec2(model_params.translation));
		mat3 S = cgv::math::scale2h(vec2(model_params.scale));
		mat3 R = cgv::math::rotate2h(model_params.angle);
		return T * S * R;
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

		if(begin_tree_node("Rectangle Style", rect_style, false)) {
			align("\a");
			add_gui("rect_style", rect_style);
			align("\b");
			end_tree_node(rect_style);
		}

		if(begin_tree_node("Circle Style", circle_style, false)) {
			align("\a");
			add_gui("circle_style", circle_style);
			align("\b");
			end_tree_node(circle_style);
		}

		if(begin_tree_node("Line Style", line_style, false)) {
			align("\a");
			add_gui("line_style", line_style);
			align("\b");
			end_tree_node(line_style);
		}

		if(begin_tree_node("Arrow Style", arrow_style, false)) {
			align("\a");
			add_gui("arrow_style", arrow_style);
			align("\b");
			end_tree_node(arrow_style);
		}

		if(begin_tree_node("Text Style", text_style, false)) {
			align("\a");
			add_gui("text_style", text_style);
			add_member_control(this, "Font Size", font_size, "value_slider", "min=1;max=256;step=0.5;ticks=true");
			add_member_control(this, "Horizontal Alignment", text_align_h, "dropdown", "enums='Center=0,Left=1,Right=2'");
			add_member_control(this, "Vertical Alignment", text_align_v, "dropdown", "enums='Center=0,Top=4,Botom=8'");
			align("\b");
			end_tree_node(text_style);
		}

		add_decorator("Model Transformation", "heading", "level=3");
		add_member_control(this, "Translation X", model_params.translation[0], "value_slider", "min=-100;max=100;step=0.5;ticks=true");
		add_member_control(this, "Translation Y", model_params.translation[1], "value_slider", "min=-100;max=100;step=0.5;ticks=true");
		add_member_control(this, "Scale", model_params.scale, "value_slider", "min=1;max=5;step=0.1;ticks=true");
		add_member_control(this, "Angle", model_params.angle, "value_slider", "min=0;max=360;step=0.5;ticks=true");

		add_decorator("View Transformation", "heading", "level=3");
		add_member_control(this, "Translation X", view_params.translation[0], "value", "min=-10000;max=10000;step=0.5;ticks=true");
		add_member_control(this, "Translation Y", view_params.translation[1], "value", "min=-10000;max=10000;step=0.5;ticks=true");
		add_member_control(this, "Scale", view_params.scale, "value_slider", "min=1;max=64;step=0.1;ticks=true");
		add_member_control(this, "Angle", view_params.angle, "value_slider", "min=0;max=360;step=0.5;ticks=true");
	}
};

#include <cgv/base/register.h>

/// register a factory to create new rounded cone texturing tests
cgv::base::factory_registration<shapes_2d> shapes_2d_fac("new/demo/shapes_2d");
