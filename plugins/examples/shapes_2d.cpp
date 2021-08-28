#include <random>
#include <unordered_map>

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/media/image/image.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/utils/file.h>
#include <cgv/utils/tokenizer.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_glutil/shader_library.h>

#include <cgv_glutil/overlay.h>

class shapes_2d :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
private:
	// a helper class to store geometry in vertex buffers and manage binding to an attribute array
	template<typename PosType, typename ColType>
	struct plain_geometry {
		struct vertex_type {
			PosType pos;
			ColType col;
		};

		std::vector<vertex_type> vertices;

		type_descriptor pos_type_descriptor = cgv::render::element_descriptor_traits<PosType>::get_type_descriptor(PosType());
		type_descriptor col_type_descriptor = cgv::render::element_descriptor_traits<ColType>::get_type_descriptor(ColType());

		vertex_buffer vb;
		attribute_array_binding aab;

		size_t size() { return vertices.size(); }

		void clear(context& ctx) {
			vertices.clear();

			if(vb.is_created())
				vb.destruct(ctx);
			if(aab.is_created())
				aab.destruct(ctx);
		}

		bool create(context& ctx, const shader_program& prog) {
			bool success = true;
			success &= vb.create(ctx, &(vertices[0]), vertices.size());
			success &= aab.create(ctx);
			success &= aab.set_attribute_array(ctx, prog.get_position_index(), pos_type_descriptor, vb, 0, vertices.size(), sizeof(vertex_type));
			success &= aab.set_attribute_array(ctx, prog.get_color_index(), col_type_descriptor, vb, sizeof(PosType), vertices.size(), sizeof(vertex_type));
			return success;
		}

		void add(const PosType& pos, const ColType& col) {
			vertices.push_back({ pos, col });
		}

		void render(context& ctx, PrimitiveType type, shader_program& prog) {
			render(ctx, type, 0, size(), prog);
		}

		void render(context& ctx, PrimitiveType type, int offset, size_t count, shader_program& prog) {
			if(aab.is_created()) {
				prog.enable(ctx);
				aab.enable(ctx);
				GLenum mode = gl::map_to_gl(type);
				glDrawArrays(mode, (GLint)offset, (GLsizei)count);
				aab.disable(ctx);
				prog.disable(ctx);
			}
		}
	};

	struct spline_geometry {
		struct vertex_type {
			vec2 pos;
			vec2 tan;
			rgba col;
		};

		std::vector<vertex_type> vertices;

		type_descriptor pos_type_descriptor = cgv::render::element_descriptor_traits<vec2>::get_type_descriptor(vec2());
		type_descriptor col_type_descriptor = cgv::render::element_descriptor_traits<rgba>::get_type_descriptor(rgba());

		vertex_buffer vb;
		attribute_array_binding aab;

		size_t size() { return vertices.size(); }

		void clear(context& ctx) {
			vertices.clear();

			if(vb.is_created())
				vb.destruct(ctx);
			if(aab.is_created())
				aab.destruct(ctx);
		}

		bool create(context& ctx, const shader_program& prog) {
			bool success = true;
			success &= vb.create(ctx, &(vertices[0]), vertices.size());
			success &= aab.create(ctx);
			success &= aab.set_attribute_array(ctx, prog.get_position_index(), pos_type_descriptor, vb, 0, vertices.size(), sizeof(vertex_type));
			success &= aab.set_attribute_array(ctx, prog.get_attribute_location(ctx, "tangent"), pos_type_descriptor, vb, sizeof(vec2), vertices.size(), sizeof(vertex_type));
			success &= aab.set_attribute_array(ctx, prog.get_color_index(), col_type_descriptor, vb, 2*sizeof(vec2), vertices.size(), sizeof(vertex_type));
			return success;
		}

		void add(const vec2& pos, const vec2& tan, const rgba& col) {
			vertices.push_back({ pos, tan, col });
		}

		void render(context& ctx, shader_program& prog) {
			render(ctx, 0, size(), prog);
		}

		void render(context& ctx, int offset, size_t count, shader_program& prog) {
			if(aab.is_created()) {
				prog.enable(ctx);
				aab.enable(ctx);
				GLenum mode = gl::map_to_gl(PT_LINES);
				glDrawArrays(mode, (GLint)offset, (GLsizei)count);
				aab.disable(ctx);
				prog.disable(ctx);
			}
		}
	};









	struct text_geometry {
		struct vertex_type {
			vec4 position;
			vec4 texcoord;
		};

		struct rope_option {
			int offset;
			int count;
			ivec2 position;
			vec2 size;
			cgv::render::TextAlignment alignment;
		};

		std::vector<vertex_type> vertices;
		std::vector<rope_option> rope_options;

		//type_descriptor type_descriptor = cgv::render::element_descriptor_traits<vec4>::get_type_descriptor(vec4());
		//
		//vertex_buffer vb;
		//attribute_array_binding aab;

		GLuint ssbo;

		size_t size() { return vertices.size(); }

		void clear(context& ctx) {
			vertices.clear();
			rope_options.clear();

			if(ssbo != 0) {
				glDeleteBuffers(1, &ssbo);
				ssbo = 0;
			}

			//if(vb.is_created())
			//	vb.destruct(ctx);
			//if(aab.is_created())
			//	aab.destruct(ctx);
		}

		bool create(context& ctx, const shader_program& prog) {

			if(ssbo != 0) {
				glDeleteBuffers(1, &ssbo);
				ssbo = 0;
			}

			glGenBuffers(1, &ssbo);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
			glBufferData(GL_SHADER_STORAGE_BUFFER, size() * sizeof(vertex_type), vertices.data(), GL_STATIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		
			return true;
		}

		void add(const vec4& pos, const vec4& txc) {
			vertices.push_back({ pos, txc });
		}

		void end_rope(const ivec2& position, const vec2& size, const cgv::render::TextAlignment alignment) {
			rope_option ro;

			if(rope_options.size() > 0) {
				const rope_option& lro = *rope_options.rbegin();
				ro.offset = lro.offset + lro.count;
				ro.count = vertices.size() - ro.offset;
			} else {
				ro.offset = 0;
				ro.count = vertices.size();
			}

			ro.position = position;
			ro.size = size;
			ro.alignment = alignment;
			rope_options.push_back(ro);
		}

		void render(context& ctx, shader_program& prog) {
			prog.enable(ctx);
			
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

			for(const auto& ro : rope_options) {

				vec2 position(ro.position);
				vec2 size = ro.size;

				/*
				TA_NONE = 0,
				TA_LEFT = 1,    // center of left edge of text bounds
				TA_RIGHT = 2,   // center of right edge of text bounds
				TA_TOP = 4,     // center of top edge of text bounds
				TA_BOTTOM = 8,  // center of bottom edge of text bounds
				TA_TOP_LEFT = TA_LEFT + TA_TOP,    // top left corner of text bounds
				TA_TOP_RIGHT = TA_RIGHT + TA_TOP,  // top right corner of text bounds
				TA_BOTTOM_LEFT = TA_LEFT + TA_BOTTOM,   // bottom left corner of text bounds
				TA_BOTTOM_RIGHT = TA_RIGHT + TA_BOTTOM  // bottom right corner of text bounds
				*/

				position -= 0.5f * size;

				if(ro.alignment != TA_NONE) {
					//position = vec2(0.0f);

					if(ro.alignment & cgv::render::TA_LEFT)
						position.x() += 0.5f * size.x();
					else if(ro.alignment & cgv::render::TA_RIGHT)
						position.x() -= 0.5f * size.x();

					if(ro.alignment & cgv::render::TA_TOP)
						position.y() -= 0.5f * size.y();
					else if(ro.alignment & cgv::render::TA_BOTTOM)
						position.y() += 0.5f * size.y();
				}

				prog.set_uniform(ctx, "position", ivec2(round(position)));
				//glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, size());
				glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP, 0, 4, ro.count, ro.offset);
			}
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			prog.disable(ctx);
		}
	};











	struct draggable {
		vec2 pos;
		vec2 size;

		enum ConstrainReference {
			CR_CENTER,
			CR_MIN_POINT,
			CR_MAX_POINT,
			CR_FULL_SIZE
		} constrain_reference;

		bool position_is_center;

		draggable() {
			position_is_center = false;
			constrain_reference = CR_FULL_SIZE;
		}

		vec2 center() const {

			if(position_is_center)
				return pos;
			else
				return pos + size;
		}

		void apply_constraint(const cgv::glutil::overlay::rect& area) {

			vec2 min_pnt = vec2(area.box.get_min_pnt());
			vec2 max_pnt = vec2(area.box.get_max_pnt());

			switch(constrain_reference) {
			case CR_MIN_POINT:
				min_pnt += size;
				max_pnt += size;
				break;
			case CR_MAX_POINT:
				min_pnt -= size;
				max_pnt -= size;
				break;
			case CR_FULL_SIZE:
				min_pnt += size;
				max_pnt -= size;
				break;
			case CR_CENTER:
			default:
				break;
			}

			if(!position_is_center) {
				min_pnt -= size;
				max_pnt -= size;
			}

			pos = cgv::math::clamp(pos, min_pnt, max_pnt);
		}

		virtual bool is_inside(const ivec2& p) const {

			vec2 a = pos;
			vec2 b = pos + size;
			return
				p.x() >= a.x() && p.x() <= b.x() &&
				p.y() >= a.y() && p.y() <= b.y();
		}
	};

	struct point : public draggable {
		point(const ivec2& pos) {
			this->pos = pos;
			size = vec2(8.0f);
			position_is_center = true;
			constrain_reference = CR_FULL_SIZE;
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
	cgv::glutil::overlay::rect viewport_rect;

	cgv::glutil::shader_library shaders;

	bool show_background;
	cgv::render::texture background_tex;
	cgv::render::texture image_tex;

	std::vector<point> points;
	point* dragged_point;
	point* selected_point;
	ivec2 drag_offset_pos;

	plain_geometry<vec2, rgba> lines;
	spline_geometry curves;
	plain_geometry<vec2, rgba> control_lines;

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




	struct glyph {
		float advance;
		vec4 plane_bounds;
		vec4 atlas_bounds;
	};

	std::vector<glyph> glyphs;

	text_geometry text;
	float font_size = 32.0f;

public:
	shapes_2d() : cgv::base::node("shapes 2d test") {
		viewport_rect.set_pos(ivec2(0));
		viewport_rect.set_size(ivec2(-1));
		
		show_background = true;

		dragged_point = nullptr;
		selected_point = nullptr;
		drag_offset_pos = ivec2(0);

		// load some specific 2d shaders
		shaders.add("rectangle", "rect2d.glpr");
		shaders.add("circle", "circle2d.glpr");
		shaders.add("ellipse", "ellipse2d.glpr");
		shaders.add("arrow", "arrow2d.glpr");
		shaders.add("line", "line2d.glpr");
		shaders.add("spline", "cubic_spline2d.glpr");
		shaders.add("text", "sdf_font2d.glpr");
	}
	void stream_help(std::ostream& os) {
		return;
	}
	bool handle(cgv::gui::event& e) {
		unsigned et = e.get_kind();
		unsigned char modifiers = e.get_modifiers();

		if(et == cgv::gui::EID_MOUSE) {
			cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
			cgv::gui::MouseAction ma = me.get_action();

			ivec2 mpos(me.get_x(), me.get_y());
			//mpos.y() = last_viewport_resolution.y() - mpos.y();
			mpos.y() = viewport_rect.size().y() - mpos.y();

			if(me.get_button() == cgv::gui::MB_LEFT_BUTTON) {
				if(ma == cgv::gui::MA_RELEASE) {
					if(dragged_point) {
						selected_point = dragged_point;
						dragged_point = nullptr;
					} else {
						selected_point = get_hit_point(mpos);
					}
					post_redraw();
				}
			}

			if(me.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
				if(dragged_point) {
					dragged_point->pos = mpos + drag_offset_pos;
					dragged_point->apply_constraint(viewport_rect);
					cgv::render::context* ctx_ptr = get_context();
					if(ctx_ptr) {
						create_line_render_data(*ctx_ptr);
						create_curve_render_data(*ctx_ptr);
					}
				} else {
					if(ma == cgv::gui::MA_PRESS) {
						dragged_point = get_hit_point(mpos);
						if(dragged_point)
							drag_offset_pos = dragged_point->pos - mpos;
						selected_point = dragged_point;
						post_recreate_gui();
					}
				}
				post_redraw();
				return true;
			}

			return false;
		} else {
			return false;
		}
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
			cgv::render::context* ctx_ptr = get_context();
			if(ctx_ptr) {
				create_line_render_data(*ctx_ptr);
				create_curve_render_data(*ctx_ptr);
			}
		}

		if(member_ptr == &font_size) {
			cgv::render::context* ctx_ptr = get_context();
			if(ctx_ptr)
				create_text_render_data(*ctx_ptr);
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
	}
	bool init(cgv::render::context& ctx) {
		
		bool success = true;
		success &= shaders.load_shaders(ctx);

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
			cgv::data::data_format tex_format;
			cgv::media::image::image_reader image(tex_format);
			cgv::data::data_view tex_data;

			// TODO: png images are flipped in y direction
			std::string file_name = "res://segoeui2.png";
			if(!image.read_image(file_name, tex_data)) {
				std::cout << "Error: Could not read image file " << file_name << std::endl;
				success = false;
			} else {
				image_tex.create(ctx, tex_data, 0);
				image_tex.set_min_filter(cgv::render::TextureFilter::TF_LINEAR);
				image_tex.set_mag_filter(cgv::render::TextureFilter::TF_LINEAR);
				image_tex.set_wrap_s(cgv::render::TextureWrap::TW_REPEAT);
				image_tex.set_wrap_t(cgv::render::TextureWrap::TW_REPEAT);
			}
			image.close();
		}

		success &= read_glyph_atlas();
		
		// add two control points for the arrow
		points.push_back(point(ivec2(600, 600)));
		points.push_back(point(ivec2(700, 600)));
		// add two control points for the line
		points.push_back(point(vec2(100, 500)));
		points.push_back(point(vec2(500, 600)));
		// add 4 control points for the curve
		points.push_back(point(vec2(600, 300)));
		points.push_back(point(vec2(650, 400)));
		points.push_back(point(vec2(700, 250)));
		points.push_back(point(vec2(800, 300)));
		
		success &= create_line_render_data(ctx);
		success &= create_curve_render_data(ctx);
		success &= create_text_render_data(ctx);

		return success;
	}
	void init_frame(cgv::render::context& ctx) {
		ivec2 viewport_resolution(ctx.get_width(), ctx.get_height());

		//if(viewport_resolution != last_viewport_resolution) {
		if(viewport_resolution != viewport_rect.size()) {
			//last_viewport_resolution = viewport_resolution;
			viewport_rect.set_size(viewport_resolution);

			set_resolution_uniform(ctx, shaders.get("rectangle"));
			set_resolution_uniform(ctx, shaders.get("circle"));
			set_resolution_uniform(ctx, shaders.get("ellipse"));
			set_resolution_uniform(ctx, shaders.get("arrow"));
			set_resolution_uniform(ctx, shaders.get("line"));
			set_resolution_uniform(ctx, shaders.get("spline"));
			set_resolution_uniform(ctx, shaders.get("text"));
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
		arrow_prog.set_uniform(ctx, "position_a", ivec2(points[0].pos));
		arrow_prog.set_uniform(ctx, "position_b", ivec2(points[1].pos));
		arrow_prog.set_uniform(ctx, "stem_width", stem_width);
		arrow_prog.set_uniform(ctx, "head_width", head_width);
		arrow_prog.set_uniform(ctx, "head_length", head_length_is_relative ? relative_head_length : absolute_head_length);
		arrow_prog.set_uniform(ctx, "head_length_is_relative", head_length_is_relative);
		set_shared_uniforms(ctx, arrow_prog);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		arrow_prog.disable(ctx);

		shader_program& line_prog = shaders.get("line");
		line_prog.enable(ctx);
		set_shared_uniforms(ctx, line_prog);
		line_prog.set_uniform(ctx, "width", line_width);
		line_prog.set_uniform(ctx, "dash_length", dash_length);
		line_prog.set_uniform(ctx, "dash_ratio", dash_ratio);
		line_prog.disable(ctx);
		lines.render(ctx, PT_LINES, line_prog);

		shader_program& spline_prog = shaders.get("spline");
		spline_prog.enable(ctx);
		set_shared_uniforms(ctx, spline_prog);
		spline_prog.set_uniform(ctx, "width", line_width);
		spline_prog.set_uniform(ctx, "dash_length", dash_length);
		spline_prog.set_uniform(ctx, "dash_ratio", dash_ratio);
		spline_prog.disable(ctx);
		curves.render(ctx, spline_prog);

		draw_control_lines(ctx);
		draw_draggables(ctx);

		shader_program& text_prog = shaders.get("text");
		text_prog.enable(ctx);
		text_prog.set_uniform(ctx, "position", ivec2(500, 150));
		text_prog.set_uniform(ctx, "font_size", font_size);
		text_prog.set_uniform(ctx, "use_color", false);
		//text_prog.set_uniform(ctx, "position", ivec2(0, viewport_rect.size().y() - 256));
		//text_prog.set_uniform(ctx, "size", ivec2(256, 256));
		set_shared_uniforms(ctx, text_prog);
		text_prog.disable(ctx);
		text.render(ctx, shaders.get("text"));

		image_tex.disable(ctx);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	void draw_background(cgv::render::context& ctx) {
		shader_program& rect_prog = shaders.get("rectangle");
		rect_prog.enable(ctx);

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
		shader_program& line_prog = shaders.get("line");
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

		control_lines.render(ctx, PT_LINES, line_prog);
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
			//point_prog.set_uniform(ctx, "color",
			//	selected_point == &p ? vec4(0.5f, 0.5f, 0.5f, 1.0f) : vec4(0.9f, 0.9f, 0.9f, 1.0f)
			//);
			point_prog.set_uniform(ctx, "color", vec4(0.9f, 0.9f, 0.9f, 1.0f));
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		point_prog.disable(ctx);
	}
	bool create_line_render_data(cgv::render::context& ctx) {
		if(points.size() < 4)
			return false;
		lines.clear(ctx);
		for(unsigned  i = 0; i < 2; ++i) {
			unsigned idx = i + 2;
			float brightness = static_cast<float>(i) / 1;
			lines.add(points[idx].pos, brightness * color);
		}
		return lines.create(ctx, shaders.get("line"));
	}
	bool create_curve_render_data(cgv::render::context& ctx) {
		if(points.size() < 8)
			return false;
		curves.clear(ctx);
		control_lines.clear(ctx);
		for(unsigned i = 0; i < 2; ++i) {
			unsigned idx = 2*i + 4;
			unsigned si = idx;
			unsigned ei = idx + 1;
			unsigned pi = (i % 2) ? ei : si;
			vec2 tangent = 3.0f * (points[ei].pos - points[si].pos);
			
			curves.add(points[pi].pos, tangent, color);
			control_lines.add(points[si].pos, rgba(0.7f, 0.2f, 0.2f, 1.0f));
			control_lines.add(points[ei].pos, rgba(0.7f, 0.2f, 0.2f, 1.0f));
		}
		return
			curves.create(ctx, shaders.get("spline")) &&
			control_lines.create(ctx, shaders.get("line"));
	}
	bool create_text_render_data(cgv::render::context& ctx) {
		text.clear(ctx);

		std::vector<std::string> texts;
		texts.push_back("Hello World!");
		texts.push_back("CGV Framework");
		std::vector<ivec2> positions;
		positions.push_back(ivec2(750, 150));
		positions.push_back(ivec2(500, 450));

		//std::string str = "Hello World!";
		//vec2 pos(500, 50);

		for(unsigned i = 0; i < 2; ++i) {
			std::string str = texts[i];

			float acc_advance = 0.0f;
			vec2 atlas_size(image_tex.get_width(), image_tex.get_height());

			vec2 text_size(0.0f, font_size);

			for(char c : str) {
				auto& g = glyphs[c];
				vec4 texcoord;
				texcoord = g.atlas_bounds / vec4(atlas_size.x(), atlas_size.y(), atlas_size.x(), atlas_size.y());

				vec2 pa(g.plane_bounds.x(), g.plane_bounds.y());
				vec2 pb(g.plane_bounds.z(), g.plane_bounds.w());

				vec2 ps = pb - pa;

				vec2 min_pos = pa * font_size + vec2(acc_advance, 0.0f);
				vec2 max_pos = ps * font_size; // the size of this glyph quad

				text_size.x() = acc_advance + max_pos.x();
				acc_advance += g.advance * font_size;

				text.add(vec4(min_pos.x(), min_pos.y(), max_pos.x(), max_pos.y()), texcoord);
			}
			text.end_rope(positions[i], text_size, cgv::render::TA_NONE);
		}

		return text.create(ctx, shaders.get("text"));
	}
	void set_resolution_uniform(cgv::render::context& ctx, cgv::render::shader_program& prog) {
		prog.enable(ctx);
		//prog.set_uniform(ctx, "resolution", last_viewport_resolution);
		prog.set_uniform(ctx, "resolution", viewport_rect.size());
		prog.set_uniform(ctx, "tex", 0);
		prog.disable(ctx);
	}
	void set_shared_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog) {
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
	bool read_glyph_atlas() {
		
		glyphs.resize(256);

		std::string filename = "D:/develop/cgv/plugins/examples/res/segoeui2.csv";
		if(!cgv::utils::file::exists(filename))
			return false;

		std::string content;

		if(!cgv::utils::file::read(filename, content, false))
			return false;

		if(content.length() > 0) {
			bool read_lines = true;
			size_t split_pos = content.find_first_of('\n');
			size_t line_offset = 0;

			while(read_lines) {
				std::string line = "";

				if(split_pos == std::string::npos) {
					read_lines = false;
					line = content.substr(line_offset, std::string::npos);
				} else {
					size_t next_line_offset = split_pos;
					line = content.substr(line_offset, next_line_offset - line_offset);
					line_offset = next_line_offset + 1;
					split_pos = content.find_first_of('\n', line_offset);
				}

				if(!line.empty()) {
					std::vector<cgv::utils::token> tokens;

					cgv::utils::tokenizer tknzr(line);
					tknzr.set_ws(",");
					//tknzr.set_sep(",");
					tknzr.bite_all(tokens);
						
					if(tokens.size() == 10) {
						int id = std::strtol(to_string(tokens[0]).c_str(), 0, 10);

						if(id > 0 && id < 256) {
							auto& g = glyphs[id];
							g.advance = std::strtof(to_string(tokens[1]).c_str(), 0);
							
							g.plane_bounds.x() = std::strtof(to_string(tokens[2]).c_str(), 0);
							g.plane_bounds.y() = std::strtof(to_string(tokens[3]).c_str(), 0);
							g.plane_bounds.z() = std::strtof(to_string(tokens[4]).c_str(), 0);
							g.plane_bounds.w() = std::strtof(to_string(tokens[5]).c_str(), 0);

							g.atlas_bounds.x() = std::strtof(to_string(tokens[6]).c_str(), 0);
							g.atlas_bounds.y() = std::strtof(to_string(tokens[7]).c_str(), 0);
							g.atlas_bounds.z() = std::strtof(to_string(tokens[8]).c_str(), 0);
							g.atlas_bounds.w() = std::strtof(to_string(tokens[9]).c_str(), 0);
						}
					}
				}
			}
		}
		
		return true;
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
	}
};

#include <cgv/base/register.h>

/// register a factory to create new rounded cone texturing tests
cgv::base::factory_registration<shapes_2d> shapes_2d_fac("new/demo/shapes_2d");
