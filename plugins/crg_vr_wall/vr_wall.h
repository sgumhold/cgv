#pragma once

#include <vr/vr_kit.h>
#include <vr/vr_state.h>
#include <cg_vr/vr_server.h>
#include <cg_vr/vr_events.h>
#include "vr_wall_kit.h"
#include <vr/gl_vr_display.h>
#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv/gui/window.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv_gl/point_renderer.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/arrow_renderer.h>
#include <cgv_gl/sphere_renderer.h>

#include "lib_begin.h"

///@ingroup VR
///@{

///
namespace vr {

	/// the different states of the wall display
	enum WallState
	{
		WS_SCREEN_CALIB, // screen calibration
		WS_EYES_CALIB,   // head to eye transformation
		WS_HMD           // hmd emulation
	};

	/// different modes for stereo shader
	enum StereoShaderMode {
		SSM_LEFT_ONLY,
		SSM_RIGHT_ONLY,
		SSM_SIDE_BY_SIDE,
		SSM_TOP_BOTTOM,
		SSM_COLUMN_INTERLEAVED,
		SSM_ROW_INTERLEAVED,
		SSM_ANAGLYPH_RED_CYAN,
		SSM_ANAGLYPH_COLOR,
		SSM_ANAGLYPH_HALF_COLOR,
		SSM_ANAGLYPH_DUBOID
	};

	enum StereoWindowMode {
		SWM_SINGLE,
		SWM_DOUBLE,
		SWM_TWO
	};

	/// the vr_wall class manages an additional window to cover the display and a wall_vr_kit that can be attached to an existing vr_kit
	class CGV_API vr_wall : public cgv::base::node, public cgv::render::drawable, public cgv::gui::event_handler, public cgv::gui::provider
	{
		/// enum definition to make vr_kit selection possible
		std::string kit_enum_definition;
	protected:
		/**@name management of window and secondary context*/
		//@{
		/// remember main context before it is overwritten by child configuration of context of owned window
		cgv::render::context* main_context;
		/// helper member that allows to configure width of the window before creation
		int window_width;
		/// helper member that allows to configure height of the window before creation
		int window_height;
		/// helper member that allows to configure x position of the window before creationint window_width;
		int window_x;
		/// helper member that allows to configure y position of the window before creationint window_width;
		int window_y;
		///
		StereoWindowMode stereo_window_mode;
		/// 0 for no fullscreen or index of fullscreen monitor (1, 2, ...) for left/main window and optionally for right window
		int fullscreen, right_fullscreen;
		/// pointers to wall display windows for left and optionally (stereo_window_mode==SWM_TWO) right eyes
		cgv::gui::window_ptr window, right_window;
		/// 
		cgv::gui::window_ptr create_wall_window(const std::string& name, int x, int y, int width, int height, int fullscr);
		/// helper function to create the window for the wall display
		void create_wall_windows();
		///
		void draw_in_main_context(cgv::render::context& ctx);
		//@}

		/**@name management of wall_vr_kit*/
		//@{
		/// index of vr_kit to which wall_vr_kit should be attached
		int vr_wall_kit_index;
		/// index of trackable (-1 ... hmd; 0,1 ... controllers; 2,3 ... trackers)
		int vr_wall_hmd_index;
		/// pointer to wall_vr_kit
		vr_wall_kit* wall_kit_ptr;
		///
		uint32_t blit_fbo, blit_tex[2];
		/// screen information during screen calibration
		vec3 screen_center;
		vec3 screen_x;
		vec3 screen_y;
		/// helper member to allow to adjust orientation of the virtual screen
		quat screen_orientation;
		/// screen calibration points
		std::vector<vec3> calib_points_screen;
		/// screen calibration points
		std::vector<vec3> calib_points_world;
		/// helper function to fill the point 
		void generate_screen_calib_points();
		/// update screen calibration
		void on_update_screen_calibration();
		// handle screen calibration specific keys
		bool handle_key_event_screen_calib(cgv::gui::vr_key_event& vrke);
		//@}

		/**@name state control and calibration*/
		//@{
		/// current state of wall display
		WallState wall_state;
		/// index of to be calibrated point or eye
		int calib_index;
		/// position of peek point in controller coordinate system that is used to define 3d calibration points
		vec3 peek_point;
		/// 
		vec3 aim_direction;
		///
		float aim_circle_radius;
		///
		float aim_width;
		///
		float aim_angle;
		///
		vec3 aim_center;
		///
		float aim_beta;
		///
		vec3 eye_position_tracker[2];
		///
		bool eye_calibrated[2];
		float eye_downset;
		float eye_backset;

		///
		cgv::render::shader_program stereo_prog;
		///
		StereoShaderMode stereo_shader_mode;
		/// current pose matrices of controllers need to render peek point
		mat34 controller_pose[2], hmd_pose;
		///
		int box_index;
		// handle eyes calibration specific keys
		bool handle_key_event_eyes_calib(cgv::gui::vr_key_event& vrke);

		//@}

		/**@name rendering in wall display context*/
		//@{
		/// point render style
		cgv::render::point_render_style prs;
		/// point renderer
		cgv::render::point_renderer pr;
		/// method to generate random dots
		void generate_points(int n);
		/// use low res image to create point sampling
		bool generate_points_from_image(const std::string& file_name, float angle);
		///
		void generate_eye_calib_points();
		//@}

		/**@name rendering in main context*/
		//@{
		/// sphere render style
		cgv::render::sphere_render_style srs;
		/// box render style
		cgv::render::box_render_style brs;
		/// arrow render style
		cgv::render::arrow_render_style ars;

		/// geometry of spheres
		std::vector<vec3> sphere_positions;
		std::vector<float> sphere_radii;
		std::vector<rgb> sphere_colors;

		/// geometry of oriented boxes
		std::vector<box3> boxes;
		std::vector<rgb> box_colors;
		std::vector<vec3> box_translations;
		std::vector<quat> box_rotations;

		/// geometry of arrows
		std::vector<vec3> arrow_positions;
		std::vector<vec3> arrow_directions;
		std::vector<rgb> arrow_colors;

		/// sample screen
		vec3 test_screen_center;
		vec3 test_screen_x;
		vec3 test_screen_y;

		/// add screen center sphere, x & y arrows and box for extruded screen rectangle
		void add_screen(const vec3& center, const vec3& x, const vec3& y, const rgb& clr, float lum);
		/// recompute the geometry based on current available  screens
		void rebuild_screens();
		//@}
	public:
		/// construct vr wall kit by attaching to another vr kit
		vr_wall();
		/// destruct window here
		~vr_wall();
		/// update vr_kit enum string
		void on_device_change(void* handle, bool attach);
		/// returns "vr_wall"
		std::string get_type_name() const;
		/// reflect members to provide access through config file
		bool self_reflect(cgv::reflect::reflection_handler& srh);
		/// callback management
		void on_set(void* member_ptr);
		/// gui creation
		void create_gui();
		///
		void init_frame(cgv::render::context& ctx);
		///
		bool init(cgv::render::context& ctx);
		///
		void clear(cgv::render::context& ctx);
		///
		void draw(cgv::render::context& ctx);
		///
		void finish_frame(cgv::render::context& ctx);
		///
		bool handle(cgv::gui::event&);
		///
		void stream_help(std::ostream& os);
	};
}

#include <cgv/config/lib_end.h>
