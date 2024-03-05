#pragma once

#include <cgv/render/drawable.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cg_nui/label_manager.h>
#include <vr/vr_state.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
/// class manages static and dynamic parts of scene
class CGV_API label_drawable : public cgv::render::drawable
{
public:
	/// different alignments
	enum class label_alignment
	{
		center,
		left,
		right,
		bottom,
		top
	};
	/// different coordinate systems used to place labels
	enum class coordinate_system
	{
		lab = 0,
		table,
		head,
		left_controller,
		right_controller,
		TOTAL
	};
protected:
	///
	cgv::nui::label_manager lm;
	/// store poses of different coordinate systems. These are computed in init_frame() function
	mat34 pose[static_cast<int>(coordinate_system::TOTAL)];
	/// store whether poses are valid
	bool valid[static_cast<int>(coordinate_system::TOTAL)];
	/// store label placements for rectangle renderer
	std::vector<vec3> label_positions;
	std::vector<quat> label_orientations;
	std::vector<vec2> label_extents;
	std::vector<vec4> label_texture_ranges;
	///
	float pixel_scale = 0.001f;
	// label visibility
	std::vector<int> label_visibilities;
	/// for each label coordinate system
	std::vector<coordinate_system> label_coord_systems;
	/// for rectangle renderer a rectangle_render_style is needed
	cgv::render::rectangle_render_style rrs;
	/// attribute array manager for rectangle renderer
	cgv::render::attribute_array_manager aam;
public:
	/// add a new label without placement information and return its index
	uint32_t add_label(const std::string& text, const rgba& bg_clr, int _border_x = 4, int _border_y = 4, int _width = -1, int _height = -1);
	/// update label text
	void update_label_text(uint32_t li, const std::string& text);
	/// update label size in texel
	void update_label_size(uint32_t li, int w, int h);
	/// update label background color
	void update_label_background_color(uint32_t li, const rgba& bgclr);
	/// fix the label size based on the font metrics even when text is changed later on
	void fix_label_size(uint32_t li);
	/// place a label relative to given coordinate system
	void place_label(uint32_t li, const vec3& pos, const quat& ori = quat(),
		coordinate_system coord_system = coordinate_system::lab,
		label_alignment align = label_alignment::center, float scale = 1.0f);
	/// hide a label
	void hide_label(uint32_t li);
	/// show a label
	void show_label(uint32_t li);
	/// set the common border color of labels
	virtual void set_label_border_color(const rgba& border_color);
	/// set the common border width in percent of the minimal extent
	virtual void set_label_border_width(float border_width);
	//@}

public:
	/// standard constructor for scene
	label_drawable();
	/// set coordinate systems from optional vr state and optional table pose
	void set_coordinate_systems(const vr::vr_kit_state* state_ptr = 0, const mat34* table_pose_ptr = 0);
	/// initialization called once per context creation
	bool init(cgv::render::context& ctx);
	/// initialization called once per frame
	void init_frame(cgv::render::context& ctx);
	/// called before context destruction to clean up GPU objects
	void clear(cgv::render::context& ctx);
	/// draw scene here
	void draw(cgv::render::context& ctx);
	/// draw transparent part here
	void finish_frame(cgv::render::context& ctx);
	/// check whether coordinate system is available
	bool is_coordsystem_valid(coordinate_system cs) const;
	/// provide access to coordinate system - check validity with is_coordsystem_valid() before
	const mat34& get_coordsystem(coordinate_system cs) const;
};

	}
}

#include <cgv/config/lib_end.h>