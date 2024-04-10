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
#include <cgv_gl/generic_renderer.h>
#include <cgv_g2d/canvas.h>
#include <cgv_g2d/draggable.h>
#include <cgv_g2d/draggable_collection.h>
#include <cgv_g2d/generic_2d_render_data.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>
#include <cgv_g2d/trect.h>
#include <cgv_g2d/shape2d_styles.h>

using namespace cgv::render;

class shapes_2d :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
protected:
	cgv::g2d::irect viewport_rect;

	cgv::g2d::canvas canvas;
	cgv::g2d::shape2d_style bg_style, rect_style, quad_style, draggable_style;
	cgv::g2d::circle2d_style circle_style;
	cgv::g2d::ring2d_style ring_style;
	cgv::g2d::line2d_style line_style, control_line_style;
	cgv::g2d::arrow2d_style arrow_style;
	cgv::g2d::grid2d_style grid_style;
	cgv::g2d::text2d_style text_style;
	
	bool show_background;
	cgv::render::texture background_tex;
	cgv::render::texture image_tex;
	
	std::vector<cgv::g2d::circle_draggable> points;
	cgv::g2d::draggable_collection<cgv::g2d::circle_draggable*> line_handles;
	cgv::g2d::draggable_collection<cgv::g2d::circle_draggable*> arrow_handles;
	cgv::g2d::draggable_collection<cgv::g2d::circle_draggable*> quadratic_curve_handles;
	cgv::g2d::draggable_collection<cgv::g2d::circle_draggable*> cubic_curve_handles;
	cgv::g2d::draggable_collection<cgv::g2d::circle_draggable*> text_handles;
	cgv::g2d::draggable_collection<cgv::g2d::circle_draggable*> quad_handles;

	cgv::render::generic_renderer line_renderer, quadratic_spline_renderer, cubic_spline_renderer, point_renderer;
	
	cgv::g2d::generic_render_data_vec2 draggable_points, control_lines;
	cgv::g2d::generic_render_data_vec2_rgba lines, quadratic_curves;
	
	DEFINE_GENERIC_RENDER_DATA_CLASS(cubic_spline_geometry, 3, cgv::vec2, position, cgv::vec2, tangent, cgv::rgba, color);
	cubic_spline_geometry cubic_curves;

	// shape appearance attributes
	const cgv::rgba light_blue = cgv::rgba(0.7f, 0.7f, 1.0f, 1.0f);
	const cgv::rgba blue = cgv::rgba(0.4f, 0.4f, 0.9f, 1.0f);

	cgv::rgba color = cgv::rgba(0.7f, 0.7f, 1.0f, 1.0f);
	cgv::rgba border_color = cgv::rgba(0.4f, 0.4f, 0.9f, 1.0f);
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
	cgv::render::TextAlignment text_align_h, text_align_v;
	float text_angle = 0.0f;

	cgv::g2d::msdf_text_geometry texts;

	// test variables
	struct {
		cgv::vec2 translation = cgv::vec2(0.0f);
		float scale = 1.0f;
		float angle = 0.0f;
	} view_params, model_params;

public:
	shapes_2d() : cgv::base::node("Shapes 2D Test") {
		viewport_rect.position = cgv::ivec2(0);
		viewport_rect.size = cgv::ivec2(-1);

		show_background = true;

		canvas.register_shader("rectangle", cgv::g2d::shaders::rectangle);
		canvas.register_shader("circle", cgv::g2d::shaders::circle);
		canvas.register_shader("ring", cgv::g2d::shaders::ring);
		canvas.register_shader("ellipse", cgv::g2d::shaders::ellipse);
		canvas.register_shader("quad", cgv::g2d::shaders::quad);
		canvas.register_shader("arrow", cgv::g2d::shaders::arrow);
		canvas.register_shader("grid", cgv::g2d::shaders::grid);

		line_renderer = cgv::render::generic_renderer(cgv::g2d::shaders::line);
		quadratic_spline_renderer = cgv::render::generic_renderer(cgv::g2d::shaders::quadratic_spline);
		cubic_spline_renderer = cgv::render::generic_renderer(cgv::g2d::shaders::cubic_spline);
		point_renderer = cgv::render::generic_renderer(cgv::g2d::shaders::circle);

		text_align_h = text_align_v = cgv::render::TA_NONE;

		// set callbacks for changes to draggable control points
		line_handles.set_drag_callback(std::bind(&shapes_2d::create_line_render_data, this));
		quadratic_curve_handles.set_drag_callback(std::bind(&shapes_2d::create_curve_render_data, this));
		cubic_curve_handles.set_drag_callback(std::bind(&shapes_2d::create_curve_render_data, this));
		text_handles.set_drag_callback(std::bind(&shapes_2d::set_text_positions, this));
	}
	void stream_help(std::ostream& os) {
		return;
	}
	bool handle(cgv::gui::event& e) {
		bool handled = false;

		cgv::mat3 M = get_view_matrix() * get_model_matrix();
		arrow_handles.set_transformation(M);
		line_handles.set_transformation(M);
		quadratic_curve_handles.set_transformation(M);
		cubic_curve_handles.set_transformation(M);
		text_handles.set_transformation(M);
		quad_handles.set_transformation(M);

		handled |= arrow_handles.handle(e, viewport_rect.size);
		handled |= line_handles.handle(e, viewport_rect.size);
		handled |= quadratic_curve_handles.handle(e, viewport_rect.size);
		handled |= cubic_curve_handles.handle(e, viewport_rect.size);
		handled |= text_handles.handle(e, viewport_rect.size);
		handled |= quad_handles.handle(e, viewport_rect.size);

		if(!handled) {
			unsigned et = e.get_kind();
			unsigned char modifiers = e.get_modifiers();

			if(et == cgv::gui::EID_MOUSE) {
				cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
				cgv::gui::MouseAction ma = me.get_action();

				if(ma == cgv::gui::MA_DRAG && me.get_button_state() & cgv::gui::MB_RIGHT_BUTTON) {
					view_params.translation += cgv::vec2((float)me.get_dx(), (float)-me.get_dy());
				}

				if(ma == cgv::gui::MA_WHEEL) {
					cgv::ivec2 mpos(me.get_x(), me.get_y());
					mpos.y() = viewport_rect.h() - mpos.y() - 1;

					cgv::vec2 origin = viewport_rect.center();
					cgv::vec2 offset = origin - mpos + view_params.translation;

					float scale = view_params.scale;
					scale *= ((float)me.get_dy()) > 0 ? 0.5f : 2.0f;

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
			for(unsigned i=0; i<(unsigned)texts.size(); ++i)
				texts.set_alignment(i, static_cast<cgv::render::TextAlignment>(text_align_h | text_align_v));
		}

		if(member_ptr == &text_angle) {
			for(unsigned i = 0; i < (unsigned)texts.size(); ++i)
				texts.set_angle(i, text_angle);
		}

		post_redraw();
		update_member(member_ptr);
	}
	std::string get_type_name() const {
		return "shapes_2d";
	}
	void clear(cgv::render::context& ctx) {
		canvas.destruct(ctx);
		background_tex.destruct(ctx);

		// decrease reference count of msdf font and renderer singletons, potentially causing them to be destructed
		cgv::g2d::ref_msdf_font_regular(ctx, -1);
		cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);
	}
	bool init(cgv::render::context& ctx) {
		bool success = true;

		success &= canvas.init(ctx);

		success &= line_renderer.init(ctx);
		success &= quadratic_spline_renderer.init(ctx);
		success &= cubic_spline_renderer.init(ctx);
		success &= point_renderer.init(ctx);

		set_default_styles();

		// TODO: png images are flipped in y direction, when reading with an image reader first and then creating a texture from the data view

		// create a checkerboard texture to use as the background
		{
			cgv::rgb a(0.85f);
			cgv::rgb b(0.95f);
			std::vector<cgv::rgb> bg_data = { a, b, b, a };

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
		
		// increase reference count of msdf font and renderer singletons, potentially causing them to be initialized
		cgv::g2d::msdf_font_regular& msdf_font = cgv::g2d::ref_msdf_font_regular(ctx, 1);
		cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);

		if(msdf_font.is_initialized())
			texts.set_msdf_font(&msdf_font);

		const auto& setup_point = [](const cgv::vec2 & position) {
			cgv::g2d::circle_draggable p(position, cgv::vec2(16.0f));
			p.position_is_center = true;
			p.constraint_reference = cgv::g2d::draggable::ConstraintReference::CR_FULL_SIZE;
			return p;
		};
		
		// add 2 control points for the arrow
		points.push_back(setup_point({ 600, 600 }));
		points.push_back(setup_point({ 700, 600 }));
		// add 2 control points for the line
		points.push_back(setup_point({ 100, 500 }));
		points.push_back(setup_point({ 500, 600 }));

		// add 3 control points for the quadratic curve
		points.push_back(setup_point({ 300, 200 }));
		points.push_back(setup_point({ 350, 300 }));
		points.push_back(setup_point({ 400, 150 }));

		// add 4 control points for the cubic curve
		points.push_back(setup_point({ 600, 300 }));
		points.push_back(setup_point({ 650, 400 }));
		points.push_back(setup_point({ 700, 250 }));
		points.push_back(setup_point({ 800, 300 }));
		// add 2 control points for the texts
		points.push_back(setup_point({ 750, 150 }));
		points.push_back(setup_point({ 500, 500 }));
		// add 2 control points for the quad
		points.push_back(setup_point({ 350, 350 }));
		points.push_back(setup_point({ 550, 450 }));
		
		// put pointers to the control points into their respective draggables collection
		int idx = 0;
		arrow_handles.add(&points[idx++]);
		arrow_handles.add(&points[idx++]);

		line_handles.add(&points[idx++]);
		line_handles.add(&points[idx++]);

		quadratic_curve_handles.add(&points[idx++]);
		quadratic_curve_handles.add(&points[idx++]);
		quadratic_curve_handles.add(&points[idx++]);

		cubic_curve_handles.add(&points[idx++]);
		cubic_curve_handles.add(&points[idx++]);
		cubic_curve_handles.add(&points[idx++]);
		cubic_curve_handles.add(&points[idx++]);

		text_handles.add(&points[idx++]);
		text_handles.add(&points[idx++]);

		quad_handles.add(&points[idx++]);
		quad_handles.add(&points[idx++]);

		create_line_render_data();
		create_curve_render_data();
		create_text_render_data();

		return success;
	}
	void init_frame(cgv::render::context& ctx) {
		cgv::ivec2 viewport_resolution(ctx.get_width(), ctx.get_height());

		if(viewport_resolution != viewport_rect.size) {
			viewport_rect.size = viewport_resolution;

			canvas.set_resolution(ctx, viewport_rect.size);

			set_resolution_uniform(ctx, line_renderer.ref_prog());
			set_resolution_uniform(ctx, quadratic_spline_renderer.ref_prog());
			set_resolution_uniform(ctx, cubic_spline_renderer.ref_prog());
			set_resolution_uniform(ctx, point_renderer.ref_prog());

			// update the constraint for all draggables
			arrow_handles.set_constraint(viewport_rect);
			line_handles.set_constraint(viewport_rect);
			quadratic_curve_handles.set_constraint(viewport_rect);
			cubic_curve_handles.set_constraint(viewport_rect);
			text_handles.set_constraint(viewport_rect);
			quad_handles.set_constraint(viewport_rect);
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
		canvas.set_zoom_factor(view_params.scale);

		image_tex.enable(ctx, 0);

		auto& rect_prog = canvas.enable_shader(ctx, "rectangle");
		rect_style.apply(ctx, rect_prog);
		canvas.draw_shape(ctx, cgv::ivec2(100, 100), cgv::ivec2(200, 100), cgv::rgba(1, 0, 0, 1));
		canvas.disable_current_shader(ctx);

		auto& circle_prog = canvas.enable_shader(ctx, "circle");
		circle_style.apply(ctx, circle_prog);
		// size defines the diameter, both components must be set to the same value
		canvas.draw_shape(ctx, cgv::ivec2(500, 100), cgv::ivec2(100), cgv::rgba(0, 1, 0, 1));
		canvas.disable_current_shader(ctx);

		auto& ellipse_prog = canvas.enable_shader(ctx, "ellipse");
		circle_style.apply(ctx, ellipse_prog);
		// size defines the two diameters
		canvas.draw_shape(ctx, cgv::ivec2(100, 300), cgv::ivec2(200, 100), cgv::rgba(0, 1, 1, 1));
		canvas.disable_current_shader(ctx);

		auto& ring_prog = canvas.enable_shader(ctx, "ring");
		ring_style.apply(ctx, ring_prog);
		// size defines the diamater in the x coordinate and the completeness in [0,1] in the y-coordinate
		canvas.draw_shape(ctx, cgv::vec2(700.0f, 450.0f), cgv::vec2(100.0f, 0.6f), cgv::rgba(0, 0, 0, 1));
		canvas.disable_current_shader(ctx);

		auto& quad_prog = canvas.enable_shader(ctx, "quad");
		quad_style.apply(ctx, quad_prog);
		// takes 4 positions (must be convex)
		canvas.draw_shape4(ctx, cgv::ivec2(400, 300), cgv::ivec2(480, 300), static_cast<cgv::ivec2>(points[13].position), static_cast<cgv::ivec2>(points[14].position), cgv::rgba(1, 1, 0, 1));
		canvas.disable_current_shader(ctx);

		auto& arrow_prog = canvas.enable_shader(ctx, "arrow");
		arrow_style.apply(ctx, arrow_prog);
		canvas.draw_shape2(ctx, arrow_handles[0]->position, arrow_handles[1]->position, cgv::rgba(1.0f, 0.0f, 1.0f, 1.0f), cgv::rgba(0.0f, 0.0f, 1.0f, 1.0f));
		canvas.disable_current_shader(ctx);

		shader_program& line_prog = line_renderer.ref_prog();
		line_prog.enable(ctx);
		line_style.apply(ctx, line_prog);
		canvas.set_view(ctx, line_prog);
		line_prog.disable(ctx);
		line_renderer.render(ctx, PT_LINES, lines);

		shader_program& quadratic_spline_prog = quadratic_spline_renderer.ref_prog();
		quadratic_spline_prog.enable(ctx);
		canvas.set_view(ctx, quadratic_spline_prog);
		line_style.apply(ctx, quadratic_spline_prog);
		quadratic_spline_prog.disable(ctx);
		quadratic_spline_renderer.render(ctx, PT_TRIANGLES, quadratic_curves);

		shader_program& cubic_spline_prog = cubic_spline_renderer.ref_prog();
		cubic_spline_prog.enable(ctx);
		canvas.set_view(ctx, cubic_spline_prog);
		line_style.apply(ctx, cubic_spline_prog);
		cubic_spline_prog.disable(ctx);
		cubic_spline_renderer.render(ctx, PT_LINES, cubic_curves);

		image_tex.disable(ctx);

		cgv::g2d::msdf_gl_canvas_font_renderer& font_renderer = cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 0);
		font_renderer.render(ctx, canvas, texts, text_style);
		
		draw_control_lines(ctx);
		draw_draggables(ctx);

		canvas.set_zoom_factor(1.0f);
		canvas.pop_modelview_matrix(ctx);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	void draw_background(cgv::render::context& ctx) {
		auto& grid_prog = canvas.enable_shader(ctx, "grid");
		grid_style.texcoord_scaling = cgv::vec2(viewport_rect.size) / 20.0f;
		grid_style.apply(ctx, grid_prog);
		canvas.draw_shape(ctx, cgv::ivec2(0), viewport_rect.size);
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
		draggable_points.clear();
		cgv::vec2 render_size;

		for(unsigned i = 0; i < points.size(); ++i) {
			const auto& p = points[i];
			draggable_points.add(p.position);
			render_size = p.size;
		}
		
		draggable_points.set_out_of_date();

		shader_program& point_prog = point_renderer.ref_prog();
		point_prog.enable(ctx);
		canvas.set_view(ctx, point_prog);
		draggable_style.apply(ctx, point_prog);
		point_prog.set_attribute(ctx, "size", render_size);
		point_prog.disable(ctx);
		point_renderer.render(ctx, PT_POINTS, draggable_points);
	}
	void create_line_render_data() {
		lines.clear();
		for(unsigned  i = 0; i < 2; ++i) {
			float brightness = static_cast<float>(i) / 1;
			lines.add(line_handles[i]->position, brightness * color);
		}
		lines.set_out_of_date();
	}
	void create_curve_render_data() {
		control_lines.clear();

		const cgv::rgba colors[3] = {
			cgv::rgba(1.0f, 0.0f, 0.0f, 1.0f),
			cgv::rgba(1.0f, 1.0f, 0.0f, 1.0f),
			cgv::rgba(0.0f, 1.0f, 0.0f, 1.0f)
		};

		{
			auto& control_points = cubic_curve_handles.ref_draggables();

			cubic_curves.clear();
			
			for(unsigned i = 0; i < 2; ++i) {
				unsigned idx = 2 * i;
				unsigned si = idx;
				unsigned ei = idx + 1;
				unsigned pi = (i % 2) ? ei : si;
				cgv::vec2 tangent = 3.0f * (control_points[ei]->position - control_points[si]->position);

				cubic_curves.add(control_points[pi]->position, tangent, colors[i]);
				control_lines.add(control_points[si]->position);
				control_lines.add(control_points[ei]->position);
			}

			cubic_curves.set_out_of_date();
		}
		{
			auto& control_points = quadratic_curve_handles.ref_draggables();

			quadratic_curves.clear();
			for(unsigned i = 0; i < 3; ++i)
				quadratic_curves.add(control_points[i]->position, colors[i]);

			control_lines.add(control_points[0]->position);
			control_lines.add(control_points[1]->position);
			control_lines.add(control_points[1]->position);
			control_lines.add(control_points[2]->position);

			quadratic_curves.set_out_of_date();
		}
		
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
		for(unsigned i=0; i<2; ++i) {
			std::string str = labels[i];
			texts.add_text(str, text_handles[i]->position, static_cast<cgv::render::TextAlignment>(text_align_h | text_align_v));
		}
	}
	void set_text_positions() {
		for(unsigned i=0; i<2; ++i)
			texts.set_position(i, text_handles[i]->position);

		cgv::ivec2 p(text_handles[0]->position);
		std::string pos_str = "(";
		pos_str += std::to_string(p.x());
		pos_str += ", ";
		pos_str += std::to_string(p.y());
		pos_str += ")";
		texts.set_text(0, pos_str);
	}
	void set_resolution_uniform(cgv::render::context& ctx, cgv::render::shader_program& prog) {
		prog.enable(ctx);
		prog.set_uniform(ctx, "resolution", viewport_rect.size);
		prog.disable(ctx);
	}
	void set_default_styles() {
		// set background style
		bg_style.feather_width = 0.0f;
		bg_style.use_texture = true;
		bg_style.use_blending = false;

		grid_style.fill_color = cgv::rgba(1.0f);
		grid_style.border_color = cgv::rgba(0.9f, 0.9f, 0.9f, 1.0f);
		grid_style.pattern = cgv::g2d::grid2d_style::GP_CHECKER;
		grid_style.scale = 0.5f;

		// set control line style
		control_line_style.use_fill_color = true;
		control_line_style.fill_color = cgv::rgba(0.7f, 0.2f, 0.2f, 1.0f);
		control_line_style.width = 2.0f;
		control_line_style.border_width = 0.0f;
		control_line_style.dash_length = 10.0f;
		control_line_style.dash_ratio = 0.75f;
		control_line_style.use_blending = true;

		// set draggable point style
		draggable_style.position_is_center = true;
		draggable_style.fill_color = cgv::rgba(0.9f, 0.9f, 0.9f, 1.0f);
		draggable_style.border_color = cgv::rgba(0.2f, 0.2f, 0.2f, 1.0f);
		draggable_style.border_width = 1.5f;
		draggable_style.use_blending = true;
		
		// set default style of all shapes
		set_default_shape_style(rect_style);
		set_default_shape_style(circle_style);
		set_default_shape_style(ring_style);
		set_default_shape_style(quad_style);
		set_default_shape_style(text_style);
		set_default_shape_style(line_style);
		set_default_shape_style(arrow_style);

		text_style.feather_origin = 0.5f;
		line_style.width = 20.0f;
		line_style.use_fill_color = false;
		arrow_style.stem_width = 20.0f;
		arrow_style.head_width = 40.0f;
		arrow_style.use_fill_color = false;
	}
	void set_default_shape_style(cgv::g2d::shape2d_style& s) {
		s.fill_color = light_blue;
		s.border_color = blue;
		s.border_width = 5.0f;
		s.use_blending = true;
	}
	cgv::mat3 get_view_matrix() {
		cgv::mat3 T0 = cgv::math::translate2h(cgv::vec2(-viewport_rect.center()));
		cgv::mat3 T1 = cgv::math::translate2h(cgv::vec2(viewport_rect.center()));
		cgv::mat3 T = cgv::math::translate2h(cgv::vec2(view_params.translation));
		cgv::mat3 S = cgv::math::scale2h(cgv::vec2(view_params.scale));
		cgv::mat3 R = cgv::math::rotate2h(view_params.angle);
		//return T * S * R; // pivot is in lower left corner
		return T * T1 * S * R * T0; // pivot is in viewport center
	}
	cgv::mat3 get_model_matrix() {
		cgv::mat3 T = cgv::math::translate2h(cgv::vec2(model_params.translation));
		cgv::mat3 S = cgv::math::scale2h(cgv::vec2(model_params.scale));
		cgv::mat3 R = cgv::math::rotate2h(model_params.angle);
		return T * S * R;
	}
	cgv::g2d::circle_draggable* get_hit_point(const cgv::ivec2& pos) {
		cgv::g2d::circle_draggable* hit = nullptr;
		for(unsigned i = 0; i < points.size(); ++i) {
			auto& p = points[i];
			if(p.contains(pos))
				hit = &p;
		}
		return hit;
	}
	void create_gui() {
		add_decorator("Shapes 2D", "heading");

		add_decorator("Example Settings", "heading", "level=3");
		add_member_control(this, "Show Background", show_background, "check");

		if(begin_tree_node("Grid Style", grid_style)) {
			align("\a");
			add_gui("grid_style", grid_style);
			align("\b");
			end_tree_node(grid_style);
		}

		if(begin_tree_node("Rectangle Style", rect_style)) {
			align("\a");
			add_gui("rect_style", rect_style);
			align("\b");
			end_tree_node(rect_style);
		}

		if(begin_tree_node("Circle Style", circle_style)) {
			align("\a");
			add_gui("circle_style", circle_style);
			align("\b");
			end_tree_node(circle_style);
		}

		if(begin_tree_node("Ring Style", ring_style)) {
			align("\a");
			add_gui("ring_style", ring_style);
			align("\b");
			end_tree_node(ring_style);
		}

		if(begin_tree_node("Quad Style", quad_style)) {
			align("\a");
			add_gui("quad_style", quad_style);
			align("\b");
			end_tree_node(quad_style);
		}

		if(begin_tree_node("Line Style", line_style)) {
			align("\a");
			add_gui("line_style", line_style);
			align("\b");
			end_tree_node(line_style);
		}

		if(begin_tree_node("Arrow Style", arrow_style)) {
			align("\a");
			add_gui("arrow_style", arrow_style);
			align("\b");
			end_tree_node(arrow_style);
		}

		if(begin_tree_node("Text Style", text_style)) {
			align("\a");
			add_gui("text_style", text_style);
			add_member_control(this, "Horizontal Alignment", text_align_h, "dropdown", "enums='Center=0,Left=1,Right=2'");
			add_member_control(this, "Vertical Alignment", text_align_v, "dropdown", "enums='Center=0,Top=4,Bottom=8'");
			add_member_control(this, "Angle", text_angle, "value_slider", "min=0;max=360;step=0.1;ticks=true");
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

/// register a factory to create new shapes_2d demos
cgv::base::factory_registration<shapes_2d> shapes_2d_fac("New/Demo/2D Shapes");
