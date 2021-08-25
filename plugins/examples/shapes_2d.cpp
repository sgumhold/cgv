#include <random>
#include <unordered_map>

#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
//#include <cgv/math/ftransform.h>
#include <cgv/media/image/image.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_glutil/shader_library.h>

class shapes_2d : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider {
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

protected:
	ivec2 last_viewport_resolution;

	cgv::glutil::shader_library shaders;

	cgv::render::texture background_tex;

	plain_geometry<vec2, rgba> lines;

	// shape appearance attributes
	rgba color = rgba(0.7f, 0.7f, 1.0f, 1.0f);
	rgba border_color = rgba(0.4f, 0.4f, 0.9f, 1.0f);
	float border_width = 5.0f;
	float border_radius = 0.0f;
	float feather_width = 1.0f;
	float feather_origin = 0.5f;

	// line appearance attributes
	float line_width = 20.0f;
	float dash_length = 0.0f;

	// shape render options
	bool use_color = true;
	bool use_blending = true;
	bool use_smooth_feather = false;
	bool apply_gamma = true;

public:
	shapes_2d() : cgv::base::node("shapes 2d test") {
		last_viewport_resolution = ivec2(-1);

		// load some specific 2d shaders
		shaders.add("rectangle", "rect2d.glpr");
		shaders.add("circle", "circle2d.glpr");
		shaders.add("ellipse", "ellipse2d.glpr");
		shaders.add("polygon", "poly2d.glpr");
		shaders.add("line", "line2d.glpr");
	}
	void on_set(void* member_ptr) {
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

		cgv::data::data_format tex_format;
		cgv::media::image::image_reader image(tex_format);
		cgv::data::data_view tex_data;

		std::string file_name = "res://plus.png";
		if(!image.read_image(file_name, tex_data)) {
			std::cout << "Error: Could not read image file " << file_name << std::endl;
			success = false;
		} else {
			background_tex.create(ctx, tex_data, 0);
			background_tex.set_min_filter(cgv::render::TextureFilter::TF_LINEAR);
			background_tex.set_mag_filter(cgv::render::TextureFilter::TF_LINEAR);
			background_tex.set_wrap_s(cgv::render::TextureWrap::TW_REPEAT);
			background_tex.set_wrap_t(cgv::render::TextureWrap::TW_REPEAT);
		}
		image.close();

		lines.add(vec2(100, 500), 0.5f*color);
		lines.add(vec2(500, 600), color);
		success &= lines.create(ctx, shaders.get("line"));

		return success;
	}
	void init_frame(cgv::render::context& ctx) {
		ivec2 viewport_resolution(ctx.get_width(), ctx.get_height());

		if(viewport_resolution != last_viewport_resolution) {
			last_viewport_resolution = viewport_resolution;

			set_resolution_uniform(ctx, shaders.get("rectangle"));
			set_resolution_uniform(ctx, shaders.get("circle"));
			set_resolution_uniform(ctx, shaders.get("ellipse"));
			set_resolution_uniform(ctx, shaders.get("line"));
		}
	}
	void draw(cgv::render::context& ctx) {
		if(!background_tex.is_created())
			return;

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

		shader_program& line_prog = shaders.get("line");
		line_prog.enable(ctx);
		set_shared_uniforms(ctx, line_prog);
		line_prog.set_uniform(ctx, "width", line_width);
		line_prog.set_uniform(ctx, "dash_length", dash_length);
		line_prog.disable(ctx);
		lines.render(ctx, PT_LINES, line_prog);

		// TODO: line feather influences dash length

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	void set_resolution_uniform(cgv::render::context& ctx, cgv::render::shader_program& prog) {
		prog.enable(ctx);
		prog.set_uniform(ctx, "resolution", last_viewport_resolution);
		prog.disable(ctx);
	}
	void set_shared_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog) {
		// appearance
		prog.set_uniform(ctx, "color", color);
		prog.set_uniform(ctx, "border_color", border_color);
		prog.set_uniform(ctx, "border_width", border_width);
		prog.set_uniform(ctx, "border_radius", border_radius);
		prog.set_uniform(ctx, "feather_width", feather_width);
		prog.set_uniform(ctx, "feather_origin", feather_origin);

		// options
		prog.set_uniform(ctx, "use_color", use_color);
		prog.set_uniform(ctx, "use_blending", use_blending);
		prog.set_uniform(ctx, "use_smooth_feather", use_smooth_feather);
		prog.set_uniform(ctx, "apply_gamma", apply_gamma);
	}
	void create_gui() {
		add_decorator("Shapes 2D", "heading");

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
		add_member_control(this, "Feather Width", feather_width, "value_slider", "min=0;max=20;step=0.5;ticks=true");
		add_member_control(this, "Feather Origin", feather_origin, "value_slider", "min=0;max=1;step=0.01;ticks=true");

		add_decorator("Line Appearance", "heading", "level=3");
		add_member_control(this, "Width", line_width, "value_slider", "min=0;max=40;step=0.5;ticks=true");
		add_member_control(this, "Dash Length", dash_length, "value_slider", "min=0;max=100;step=0.5;ticks=true");
	}
};

#include <cgv/base/register.h>

/// register a factory to create new rounded cone texturing tests
cgv::base::factory_registration<shapes_2d> shapes_2d_fac("new/demo/shapes_2d");
