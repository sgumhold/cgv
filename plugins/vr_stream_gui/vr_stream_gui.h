#pragma once
#include <cgv/base/group.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/provider.h>
#include <cg_nui/nui_primitive_node.h>
#include <cg_nui/nui_mesh_node.h>
#include <cg_nui/label_manager.h>
#include <cg_nui/controller_tool.h>
#include <cgv/gui/event_handler.h>
#include <cgv_gl/box_renderer.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv/render/frame_buffer.h>

///@ingroup VR
///@{

/**@file
   example plugin for vr usage
*/

// these are the vr specific headers
#include <vr/vr_driver.h>
#include <cg_vr/vr_server.h>
#include <vr_view_interactor.h>
#include <vr_render_helpers.h>

#include "virtual_display.h"
#include "screen_texture_manager.h"

#include "ray_rect_intersection.h"
#include "vive_mouse_controller.h"

using namespace trajectory;

class vr_stream_gui :
	public cgv::base::group,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider {
protected:
	cgv::nui::label_manager lm;
	cgv::nui::nui_node_ptr scene;
	cgv::nui::nui_primitive_node_ptr node, lab;
	cgv::nui::nui_mesh_node_ptr mesh_node;
	cgv::nui::controller_tool_ptr tools[2];
	std::shared_ptr<vr::room::virtual_display> vt_display;
	std::shared_ptr<util::screen_texture_manager> screen_tex_manager;
	util::vive_mouse_controller vm_controller;
	bool switch_left_and_right_hand = false;

	bool vt_display_reconfiguring = false;
	int vt_display_reconfigure_state = 0; // 0 = await first corner, 1 = await second corner
	vec3 vt_display_reconfigure_first_pos;
	vec3 vt_display_reconfigure_second_pos;
	bool stream_screen = false;

	vec3 ray_origin;
	vec3 ray_dir;
	vr::room::virtual_display::intersection intersection;

	// test members
	bool draw_debug = false;
	bool prohibit_vr_input = false;

	void set_stream_screen();
	void set_draw_debug();

	// update position of virtual screen
	// call twice, one time with upper left, one time with lower right corner
	void reconfigure_virtual_display(vec3 pos);

	// general font information
	std::vector<const char*> font_names;
	std::string font_enum_decl;

	// store handle to vr kit of which left deadzone and precision is configured
	void* last_kit_handle;

	// keep reference to vr_view_interactor
	vr_view_interactor* vr_view_ptr;

	int nr_cameras;
	int frame_width, frame_height;
	int frame_split;
	float seethrough_gamma;
	mat4 camera_to_head_matrix[2];
	cgv::math::fmat<float, 4, 4> camera_projection_matrix[4];
	vec2 focal_lengths[4];
	vec2 camera_centers[4];
	cgv::render::texture camera_tex;
	cgv::render::shader_program seethrough;
	GLuint camera_tex_id;
	bool undistorted;
	bool shared_texture;
	bool max_rectangle;
	float camera_aspect;
	bool show_seethrough;
	bool use_matrix;
	float background_distance;
	float background_extent;
	vec2 extent_texcrd;
	vec2 center_left;
	vec2 center_right;

public:
	void init_cameras(vr::vr_kit* kit_ptr);

	void start_camera();

	void stop_camera();

	/// register on device change events
	void on_device_change(void* kit_handle, bool attach);
	/// construct boxes that represent a table of dimensions tw,td,th and leg width tW
	void construct_movable_boxes(cgv::nui::nui_primitive_node_ptr node, float tw, float td, float th, float tW, size_t nr);
	/// construct labels
	void construct_labels(cgv::nui::nui_primitive_node_ptr node);
	/// construct lab and movable parts
	void construct_scene();
public:
	vr_stream_gui();
	
	std::string get_type_name() { return "vr_stream_gui"; }

	void stream_help(std::ostream& os);

	void on_set(void* member_ptr);

	bool handle(cgv::gui::event& e);
	
	bool init(cgv::render::context& ctx);

	void clear(cgv::render::context& ctx);

	void init_frame(cgv::render::context& ctx);

	void draw(cgv::render::context& ctx);

	void finish_draw(cgv::render::context& ctx);

	void create_gui();
};

///@}
