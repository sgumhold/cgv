#pragma once

#include <cgv/data/time_stamp.h>
#include <cgv/media/color_scale.h>
#include <cgv/render/texture.h>
#include <cgv/utils/convert_string.h>
#include <cgv/utils/number_format.h>
#include <cgv_app/themed_canvas_overlay.h>
#include <cgv_g2d/generic_2d_renderer.h>
#include <cgv_g2d/generic_2d_render_data.h>
#include <cgv_g2d/msdf_text_geometry.h>

#include "lib_begin.h"

namespace cgv {
namespace app {

class CGV_API color_scale_legend : public themed_canvas_overlay {
public:
	enum class Orientation {
		kHorizontal,
		kVertical
	};

protected:
	struct layout_attributes {
		int padding = 0;
		int label_space = 12;
		int x_label_size = 0;
		int title_space = 0;

		ivec2 total_size;

		Orientation orientation = Orientation::kHorizontal;
		AlignmentOption label_alignment = AO_END;

		// dependent members
		cgv::g2d::irect color_ramp_rect;
	} layout;

	bool invert_color = false;
	bool flip_texture = false;

	cgv::render::texture tex = { "uint8[R,G,B,A]" };

	std::string title;
	AlignmentOption title_alignment = AO_START;

	unsigned num_ticks = 10;
	bool nice_ticks = true;
	bool auto_precision = true;
	cgv::utils::number_format label_format;

	bool show_opacity = true;

	// general appearance
	cgv::g2d::shape2d_style border_style, color_ramp_style, tick_style;
	cgv::g2d::grid2d_style background_style;

	// text appearance
	cgv::g2d::text2d_style text_style;
	cgv::g2d::msdf_text_geometry label_geometry;

	cgv::g2d::generic_2d_renderer tick_renderer;
	cgv::g2d::generic_render_data_vec2_vec2 tick_geometry;

	cgv::data::time_stamp build_time;

	std::shared_ptr<const cgv::media::color_scale> color_scale;

	void init_styles() override;
	void update_layout(const ivec2& parent_size);
	void create_texture();
	bool create_ticks(const cgv::render::context& ctx);

	void create_gui_impl() override;

public:
	color_scale_legend();
	std::string get_type_name() const override { return "color_scale_legend"; }

	void clear(cgv::render::context& ctx) override;

	void handle_member_change(cgv::data::informed_ptr ptr) override;

	bool init(cgv::render::context& ctx) override;
	void init_frame(cgv::render::context& ctx) override;
	void draw_content(cgv::render::context& ctx) override;

	void set_color_scale(std::shared_ptr<const cgv::media::color_scale> color_scale);
	
	void set_width(size_t w);
	void set_height(size_t h);

	void set_title(const std::string& t);

	Orientation get_orientation() const { return layout.orientation; }
	void set_orientation(Orientation orientation);

	AlignmentOption get_label_alignment() const { return layout.label_alignment; }
	void set_label_alignment(AlignmentOption alignment);

	void set_invert_color(bool flag);

	unsigned get_num_ticks() { return num_ticks; }
	void set_num_ticks(unsigned n);

	void set_label_precision(unsigned p);
	void set_label_auto_precision(bool enabled);
	void set_label_prune_trailing_zeros(bool enabled);
	void set_label_integer_mode(bool enabled);
	void set_show_opacity(bool enabled);
};

typedef cgv::data::ref_ptr<color_scale_legend> color_scale_legend_ptr;

}
}

#include <cgv/config/lib_end.h>
