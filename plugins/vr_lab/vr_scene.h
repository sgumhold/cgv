#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <vr_view_interactor.h>
#include "label_manager.h"

#include "lib_begin.h"

namespace vr {

/// different table types
enum TableMode
{
	TM_HIDE,
	TM_RECTANGULAR,
	TM_ROUND
};

/// support self reflection of table mode
extern CGV_API cgv::reflect::enum_reflection_traits<TableMode> get_reflection_traits(const TableMode&);

/// class manages static and dynamic parts of scene
class vr_scene :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
private:
	// keep reference to vr view (initialized in init function)
	vr_view_interactor* vr_view_ptr;
protected:
	//@name boxes and table
	//@{	

	// store the static part of the scene as colored boxes with the table in the last 5 boxes
	std::vector<box3> boxes;
	std::vector<rgb> box_colors;

	// rendering style for rendering of boxes
	cgv::render::box_render_style box_style;

	// use cones for the turn table
	std::vector<vec4> cone_vertices;
	std::vector<rgb> cone_colors;

	// rendering style for rendering of cones
	cgv::render::cone_render_style cone_style;

	/**@name ui parameters for table construction*/
	//@{
	/// table mode
	TableMode table_mode;
	/// global sizes 
	union {
		/// width of rectangular table
		float table_width;
		/// top radius of round table
		float table_top_radius;
	};
	union {
		/// depth of rectangular table
		float table_depth;
		/// bottom radius of round table
		float table_bottom_radius;
	};
	/// height of table measured from ground to top face
	float table_height;
	/// width of legs of rectangular table or radius of central leg of round table
	float leg_width;
	/// offset of legs relative to table width/radius 
	float percentual_leg_offset;
	/// color of table top and legs
	rgb table_color, leg_color;
	//@}

	bool draw_environment, draw_room, draw_walls, draw_ceiling;
	float room_width, room_depth, room_height, wall_width;

	/// construct boxes that represent a rectangular table of dimensions tw,td,th, leg width tW, percentual leg offset and table/leg colors
	void construct_rectangular_table(float tw, float td, float th, float tW, float tpO, rgb table_clr, rgb leg_clr);
	/// construct cones that represent a round table of dimensions top/bottom radius ttr/tbr, height th, leg width tW, percentual leg offset and table/leg colors
	void construct_round_table(float ttr, float tbr, float th, float tW, float tpO, rgb table_clr, rgb leg_clr);
	/// construct boxes that represent a room of dimensions w,d,h and wall width W
	void construct_room(float w, float d, float h, float W, bool walls, bool ceiling);
	/// construct boxes for environment
	void construct_environment(float s, float ew, float ed, float w, float d, float h);
	/// construct a scene with a table
	void build_scene(float w, float d, float h, float W);
	/// clear scene geometry containers
	void clear_scene();
	/// update labels in ui that change based on table type
	void update_table_labels();
	//@}


	//@name labels
	//@{	
	/// use label manager to organize labels in texture
	label_manager lm;

	/// store label placements for rectangle renderer
	std::vector<vec3> label_positions;
	std::vector<quat> label_orientations;
	std::vector<vec2> label_extents;
	std::vector<vec4> label_texture_ranges;

	// label visibility
	std::vector<int> label_visibilities;

	/// for rectangle renderer a rectangle_render_style is needed
	cgv::render::rectangle_render_style rrs;
	/// attribute array manager for rectangle renderer
	cgv::render::attribute_array_manager aam;
public:
	/// different coordinate systems used to place labels
	enum CoordinateSystem
	{
		CS_LAB,
		CS_TABLE,
		CS_HEAD,
		CS_LEFT_CONTROLLER,
		CS_RIGHT_CONTROLLER
	};
	/// different alignments
	enum LabelAlignment
	{
		LA_CENTER,
		LA_LEFT,
		LA_RIGHT,
		LA_BOTTOM,
		LA_TOP
	};
	/// for each label coordinate system
	std::vector<CoordinateSystem> label_coord_systems;
	/// size of pixel in meters
	float pixel_scale;
	/// add a new label without placement information and return its index
	uint32_t add_label(const std::string& text, const rgba& bg_clr, int _border_x = 4, int _border_y = 4, int _width = -1, int _height = -1) {
		uint32_t li = lm.add_label(text, bg_clr, _border_x, _border_y, _width, _height);
		label_positions.push_back(vec3(0.0f));
		label_orientations.push_back(quat());
		label_extents.push_back(vec2(1.0f));
		label_texture_ranges.push_back(vec4(0.0f));
		label_coord_systems.push_back(CS_LAB);
		label_visibilities.push_back(1);
		return li;
	}
	/// update label text
	void update_label_text(uint32_t li, const std::string& text) { lm.update_label_text(li, text); }
	/// update label size in texel
	void update_label_size(uint32_t li, int w, int h) { lm.update_label_size(li, w, h); }
	/// update label background color
	void update_label_background_color(uint32_t li, const rgba& bgclr) { lm.update_label_background_color(li, bgclr); }
	/// fix the label size based on the font metrics even when text is changed later on
	void fix_label_size(uint32_t li) { lm.fix_label_size(li); }
	/// place a label relative to given coordinate system
	void place_label(uint32_t li, const vec3& pos, const quat& ori = quat(),
		CoordinateSystem coord_system = CS_LAB, LabelAlignment align = LA_CENTER, float scale = 1.0f) {
		label_extents[li] = vec2(scale * pixel_scale * lm.get_label(li).get_width(), scale * pixel_scale * lm.get_label(li).get_height());
		static vec2 offsets[5] = { vec2(0.0f,0.0f), vec2(0.5f,0.0f), vec2(-0.5f,0.0f), vec2(0.0f,0.5f), vec2(0.0f,-0.5f) };
		label_positions[li] = pos + ori.get_rotated(vec3(offsets[align] * label_extents[li], 0.0f));
		label_orientations[li] = ori;
		label_coord_systems[li] = coord_system;
	}
	/// hide a label
	void hide_label(uint32_t li) { label_visibilities[li] = 0; }
	/// show a label
	void show_label(uint32_t li) { label_visibilities[li] = 1; }
	/// set the common border color of labels
	void set_label_border_color(const rgba& border_color);
	/// set the common border width in percent of the minimal extent
	void set_label_border_width(float border_width);
	//@}

public:
	/// standard constructor for scene
	vr_scene();
	/// return type name
	std::string get_type_name() const { return "vr_scene"; }
	/// reflect member variables
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	/// callback on member updates to keep data structure consistent
	void on_set(void* member_ptr);
	//@name cgv::gui::event_handler interface
	//@{
	/// provide interaction help to stream
	void stream_help(std::ostream& os);
	/// handle events
	bool handle(cgv::gui::event& e);
	//@}

	//@name cgv::render::drawable interface
	//@{
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
	//@}
	/// provide access to table dimensions
	vec3 get_table_extent() const { return vec3(table_width, table_height, table_depth); }
	/// cgv::gui::provider function to create classic UI
	void create_gui();
};

}

#include <cgv/config/lib_end.h>