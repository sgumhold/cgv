#pragma once

#include <vr/vr_kit.h>
#include <vr/vr_state.h>
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

#include "lib_begin.h"

///@ingroup VR
///@{

///
namespace vr {

	enum WallState
	{
		WS_SCREEN_CALIB,
		WS_EYES_CALIB,
		WS_HMD
	};

	class CGV_API vr_wall : public cgv::base::node, public cgv::render::drawable, public cgv::gui::event_handler, public cgv::gui::provider
	{
	public:
		typedef cgv::math::fmat<float, 3, 4> mat34;
	private:
		std::string kit_enum_definition;
	protected:
		WallState wall_state;
		int calib_point_index;
		bool eye_calibrated[2];
		void generate_screen_calib_points();

		cgv::render::context* main_context;
		cgv::gui::window_ptr window;
		int vr_wall_kit_index;
		int vr_wall_hmd_index;
		int window_width;
		int window_height;
		int window_x;
		int window_y;
		vr_wall_kit* wall_kit_ptr;
		quat screen_orientation;

		cgv::render::point_render_style prs;
		cgv::render::point_renderer pr;
		
		cgv::render::arrow_render_style ars;
		cgv::render::box_render_style brs;

		vec3 test_screen_center;
		vec3 test_screen_x;
		vec3 test_screen_y;

		void add_screen_box(const vec3& center, const vec3& x, const vec3& y, const rgb& color)
		{
			boxes.push_back(box3(vec3(-x.length(),-y.length(),-0.01f), vec3(x.length(),y.length(),0)));
			box_colors.push_back(color);
			box_translations.push_back(center);
			mat3 R;
			vec3 x_dir = x; x_dir.normalize();
			vec3 y_dir = y; y_dir.normalize();
			vec3 z_dir = cross(x_dir, y_dir);
			R.set_col(0, x_dir);
			R.set_col(1, y_dir);
			R.set_col(2, z_dir);
			box_rotations.push_back(quat(R));
		}

		std::vector<box3> boxes;
		std::vector<rgb> box_colors;
		std::vector<vec3> box_translations;
		std::vector<quat> box_rotations;

		void add_arrows(const vec3& center, const vec3& x, const vec3& y, float lum)
		{
			arrow_positions.push_back(center);
			arrow_directions.push_back(x);
			arrow_colors.push_back(rgb(1, lum, lum));
			arrow_positions.push_back(center);
			arrow_directions.push_back(y);
			arrow_colors.push_back(rgb(lum, 1, lum));
		}

		std::vector<vec3> arrow_positions;
		std::vector<vec3> arrow_directions;
		std::vector<rgb> arrow_colors;




		std::vector<vec3> points;
		std::vector<rgb> colors[2];
		void generate_points(int n);
		bool generate_points_from_image(const std::string& file_name, float angle);
		void create_wall_window();
	public:
		/// construct vr wall kit by attaching to another vr kit
		vr_wall();
		/// destruct window here
		~vr_wall();
		///
		void on_device_change(void* device_handle, bool attach);
		///
		std::string get_type_name() const;
		///
		bool self_reflect(cgv::reflect::reflection_handler& srh);
		/// you must overload this for gui creation
		void create_gui();
		///
		void on_set(void* member_ptr);
		///
		void init_frame(cgv::render::context& ctx);
		///
		bool init(cgv::render::context& ctx);
		///
		void clear(cgv::render::context& ctx);
		///
		void draw(cgv::render::context& ctx);
		///
		bool handle(cgv::gui::event&);
		///
		void stream_help(std::ostream& os);
	};
}

#include <cgv/config/lib_end.h>
