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
#include <cgv/gui/provider.h>

#include "lib_begin.h"

///@ingroup VR
///@{

///
namespace vr {
	class CGV_API vr_wall : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider
	{
		std::string kit_enum_definition;
	protected:
		cgv::render::context* fst_context;
		cgv::gui::window_ptr window;
		int vr_wall_kit_index;
		int vr_wall_hmd_index;
		int window_width;
		int window_height;
		int window_x;
		int window_y;
		vr_wall_kit* wall_kit_ptr;
		quat screen_orientation;
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
		bool init(cgv::render::context& ctx);
		///
		void clear(cgv::render::context& ctx);
		///
		void draw(cgv::render::context& ctx);
	};
}

#include <cgv/config/lib_end.h>
