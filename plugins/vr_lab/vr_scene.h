#include <cgv/base/node.h>
#include <cgv/base/register.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_proc/terrain_renderer.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <plugins/crg_vr_view/vr_view_interactor.h>
#include <3rd/screen_capture_lite/include/ScreenCapture.h>
#include <cg_nui/label_manager.h>
#include <cg_nui/spatial_dispatcher.h>
#include <cg_nui/vr_table.h>
#include <cg_nui/table_gizmo.h>
#include <cg_nui/label_drawable.h>

#include "lib_begin.h"

namespace vr {

/// different table types
enum TableMode
{
	TM_HIDE,
	TM_RECTANGULAR,
	TM_ROUND
};

/// different ground types
enum GroundMode {
	GM_NONE,
	GM_BOXES,
	GM_TERRAIN
};

/// different environment modes that are not yet supported
enum EnvironmentMode {
	EM_EMPTY,
	EM_SKYBOX,
	EM_PROCEDURAL
};

enum Skybox {
	SB_MOUNTAIN_AND_SEA,
	SB_BEACH_HEARTINTHESAND,
	SB_BEACH_LARNACA,
	SB_BEACH_PALMTREES,
	SB_BEACH_TENERIFE,
	SB_BEACH_TENERIFE2,
	SB_BEACH_TENERIFE3,
	SB_BEACH_TENERIFE4,
	SB_FOREST_BRUDSLOJAN,
	SB_FOREST_LANGHOLMEN2,
	SB_FOREST_LANGHOLMEN3,
	SB_FOREST_MOUNTAINPATH,
	SB_FOREST_PLANTS
};

/// support self reflection of table mode
extern CGV_API cgv::reflect::enum_reflection_traits<TableMode> get_reflection_traits(const TableMode&);

/// support self reflection of ground mode
extern CGV_API cgv::reflect::enum_reflection_traits<GroundMode> get_reflection_traits(const GroundMode&);

/// support self reflection of environment mode
extern CGV_API cgv::reflect::enum_reflection_traits<EnvironmentMode> get_reflection_traits(const EnvironmentMode&);

/// support self reflection of environment mode
extern CGV_API cgv::reflect::enum_reflection_traits<Skybox> get_reflection_traits(const Skybox&);

/// class manages static and dynamic parts of scene
class CGV_API vr_scene :
	public cgv::base::node,
	public cgv::base::registration_listener,
	public cgv::nui::label_drawable,
	public cgv::gui::event_handler,
	public cgv::nui::spatial_dispatcher,
	public cgv::gui::provider
{
	using vec2 = cgv::vec2;
	using vec3 = cgv::vec3;
	using dvec3 = cgv::dvec3;
	using rgb = cgv::rgb;
	using rgba = cgv::rgba;
	using box3 = cgv::box3;

private:
	// keep reference to vr view (initialized in init function)
	vr_view_interactor* vr_view_ptr;

	double ctrl_pointing_animation_duration = 0.5;
	// if both grabbing and pointing turned off for controller ci, detach it from focus in hid and kit attachments
	void check_for_detach(int ci, const cgv::gui::event& e);
	// keep reference to vr table
	cgv::nui::vr_table_ptr table;

	//@name boxes and table
	//@{	

	// store the static part of the scene as colored boxes with the table in the last 5 boxes
	std::vector<box3> boxes;
	std::vector<rgb> box_colors;

	// rendering style for rendering of boxes
	cgv::render::box_render_style box_style;

	cgv::render::texture skybox_tex;
	cgv::render::shader_program cubemap_prog;

	bool invert_skybox;
	std::string skybox_file_names;

	GroundMode ground_mode;

	// terrain members
	std::vector<vec2> custom_positions;
	std::vector<unsigned int> custom_indices;
	cgv::render::terrain_render_style terrain_style;
	int grid_width;
	int grid_height;
	dvec3 terrain_translation;
	double terrain_scale;

	EnvironmentMode environment_mode;

	Skybox skybox = SB_MOUNTAIN_AND_SEA;

	bool draw_room, draw_walls, draw_ceiling;
	float room_width, room_depth, room_height, wall_width;

	/// construct boxes that represent a room of dimensions w,d,h and wall width W
	void construct_room(float w, float d, float h, float W, bool walls, bool ceiling);
	/// construct boxes for environment
	void construct_ground(float s, float ew, float ed, float w, float d, float h);
	/// construct a scene with a table
	void build_scene(float w, float d, float h, float W);
	/// clear scene geometry containers
	void clear_scene();
	//@}
	/// set the common border color of labels
	void set_label_border_color(const rgba& border_color);
	/// set the common border width in percent of the minimal extent
	void set_label_border_width(float border_width);

protected:
	bool auto_grab_focus = false;
	bool draw_controller_mode;
	cgv::render::sphere_render_style srs;
	cgv::render::cone_render_style crs;
	std::vector<vec3> sphere_positions;
	std::vector<rgb> sphere_colors;
	std::vector<vec3> cone_positions;
	std::vector<rgb> cone_colors;
	void construct_hit_geometry();
public:
	/// overload to handle registration events
	void register_object(base_ptr object, const std::string& options);
	/// overload to handle unregistration events
	void unregister_object(base_ptr object, const std::string& options);
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
	/// used to communicate context height to spatial dispatcher
	void resize(unsigned int w, unsigned int h);
	/// initialization called once per frame
	void init_frame(cgv::render::context& ctx);
	/// called before context destruction to clean up GPU objects
	void clear(cgv::render::context& ctx);
	/// draw scene here
	void draw(cgv::render::context& ctx);
	//@}
	/// cgv::gui::provider function to create classic UI
	void create_gui();
};

}

#include <cgv/config/lib_end.h>