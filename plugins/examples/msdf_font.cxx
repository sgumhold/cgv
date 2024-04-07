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
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>
#include <cgv_g2d/trect.h>
#include <cgv_g2d/shape2d_styles.h>

using namespace cgv::render;

class msdf_font :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
protected:
	cgv::g2d::irect viewport_rect;

	cgv::g2d::canvas canvas;
	cgv::g2d::text2d_style text_style;

	cgv::g2d::msdf_text_geometry texts_light;
	cgv::g2d::msdf_text_geometry texts_regular;
	cgv::g2d::msdf_text_geometry texts_bold;
	cgv::render::TextAlignment text_align;
	float text_angle = 0.0f;

	float texts_origin = 64.0f;
	float texts_offset = 0.0f;

	struct {
		cgv::vec2 translation = cgv::vec2(0.0f);
		float scale = 1.0f;
		float angle = 0.0f;
	} view_params;

public:
	msdf_font() : cgv::base::node("MSDF Font Example") {
		viewport_rect.position = cgv::ivec2(0);
		viewport_rect.size = cgv::ivec2(-1);

		text_style.font_size = 1.0f;

		text_align = cgv::render::TA_BOTTOM_LEFT;
	}
	void stream_help(std::ostream& os) {
		return;
	}
	bool handle(cgv::gui::event& e) {
		bool handled = false;

		if(!handled) {
			unsigned et = e.get_kind();
			unsigned char modifiers = e.get_modifiers();

			if(et == cgv::gui::EID_MOUSE) {
				cgv::gui::mouse_event& me = (cgv::gui::mouse_event&)e;
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
						view_params.translation += me.get_dy() > 0 ? -0.5f * offset : offset;
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
		if(member_ptr == &text_align) {
			set_text_alignment(texts_light);
			set_text_alignment(texts_regular);
			set_text_alignment(texts_bold);
		}

		if(member_ptr == &text_angle) {
			set_text_angle(texts_light);
			set_text_angle(texts_regular);
			set_text_angle(texts_bold);
		}

		post_redraw();
		update_member(member_ptr);
	}
	std::string get_type_name() const {
		return "msdf_font_example";
	}
	void clear(cgv::render::context& ctx) {
		canvas.destruct(ctx);

		// decrease reference count of msdf font renderer singleton
		cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, -1);
		cgv::g2d::ref_msdf_font_regular(ctx, -1);
		cgv::g2d::ref_msdf_font_light(ctx, -1);
		cgv::g2d::ref_msdf_font_bold(ctx, -1);
	}
	bool init(cgv::render::context& ctx) {
		bool success = true;

		success &= canvas.init(ctx);

		// increase reference count of msdf font renderer singleton
		cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);
		cgv::g2d::ref_msdf_font_light(ctx, 1);
		cgv::g2d::ref_msdf_font_bold(ctx, 1);

		texts_light.set_msdf_font(&cgv::g2d::ref_msdf_font_light(ctx, 1));
		texts_regular.set_msdf_font(&cgv::g2d::ref_msdf_font_regular(ctx, 1));
		texts_bold.set_msdf_font(&cgv::g2d::ref_msdf_font_bold(ctx, 1));
		create_text_render_data();
		
		return success;
	}
	void init_frame(cgv::render::context& ctx) {
		cgv::ivec2 viewport_resolution(ctx.get_width(), ctx.get_height());

		if(viewport_resolution != viewport_rect.size) {
			viewport_rect.size = viewport_resolution;

			texts_offset = viewport_rect.w() - texts_bold.get_text_render_size(0, 1.0f).x() - texts_origin;
			texts_offset *= 0.5f;

			canvas.set_resolution(ctx, viewport_rect.size);
		}
	}
	void draw(cgv::render::context& ctx) {
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		canvas.push_modelview_matrix();
		canvas.mul_modelview_matrix(ctx, get_view_matrix());
		canvas.set_zoom_factor(view_params.scale);

		cgv::g2d::msdf_gl_canvas_font_renderer& font_renderer = cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 0);
		font_renderer.render(ctx, canvas, texts_light, text_style);

		canvas.push_modelview_matrix();
		canvas.mul_modelview_matrix(ctx, cgv::math::translate2h(texts_offset, 0.0f));

		font_renderer.render(ctx, canvas, texts_regular, text_style);

		canvas.mul_modelview_matrix(ctx, cgv::math::translate2h(texts_offset, 0.0f));
		font_renderer.render(ctx, canvas, texts_bold, text_style);

		canvas.pop_modelview_matrix(ctx);
		
		canvas.set_zoom_factor(1.0f);
		canvas.pop_modelview_matrix(ctx);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
	void create_text_render_data() {
		//std::vector<std::string> strs;
		std::string str = "MSDF Font";

		cgv::vec2 position(texts_origin);
		float size = 128.0f;

		texts_light.clear();
		texts_regular.clear();
		texts_bold.clear();
		for(size_t i = 0; i < 8; ++i) {
			texts_light.add_text(str, position, text_align, size);
			texts_regular.add_text(str, position, text_align, size);
			texts_bold.add_text(str, position, text_align, size);

			position.y() += 1.5f * size;
			size -= 16.0f;
			size = std::max(size, 8.0f);
		}
	}
	void set_text_alignment(cgv::g2d::msdf_text_geometry& texts) {
		for(unsigned i = 0; i < static_cast<unsigned>(texts.size()); ++i)
			texts.set_alignment(i, text_align);
	}
	void set_text_angle(cgv::g2d::msdf_text_geometry& texts) {
		for(unsigned i = 0; i < static_cast<unsigned>(texts.size()); ++i)
			texts.set_angle(i, text_angle);
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
	void create_gui() {
		add_decorator("Text Style", "heading", "level=2");
		add_gui("text_style", text_style);
		add_member_control(this, "Alignment", text_align, "dropdown", "enums='Center=0,Left=1,Right=2,Top=4,Bottom=8,Top Left=5,Top Right=6,Bottom Left=9,Bottom Right=10'");
		add_member_control(this, "Angle", text_angle, "value_slider", "min=0;max=360;step=0.1;ticks=true");
		
		add_decorator("", "separator");

		add_decorator("View Transformation", "heading", "level=2");
		add_member_control(this, "Translation X", view_params.translation[0], "value", "min=-10000;max=10000;step=0.5;ticks=true");
		add_member_control(this, "Translation Y", view_params.translation[1], "value", "min=-10000;max=10000;step=0.5;ticks=true");
		add_member_control(this, "Scale", view_params.scale, "value_slider", "min=1;max=64;step=0.1;ticks=true");
		add_member_control(this, "Angle", view_params.angle, "value_slider", "min=0;max=360;step=0.5;ticks=true");
	}
};

#include <cgv/base/register.h>

/// register a factory to create new msdf font examples
cgv::base::factory_registration<msdf_font> msdf_font_fac("New/Demo/MSDF Font Example");
