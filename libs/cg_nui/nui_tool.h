#pragma once

#include <cgv/base/named.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/render/drawable.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class nui_tool;

		typedef cgv::data::ref_ptr<nui_tool, true> nui_tool_ptr;

		/// the tool type gives information, which device controls the tool
		enum ToolType
		{
			TT_MOUSE,
			TT_KEYBOARD,
			TT_VR_CONTROLLER,
			TT_GAMEPAD,
			TT_HAND,
			TT_BODY
		};

		/// convert an tool type into a readable string
		extern CGV_API std::string get_tool_type_string(ToolType type);

		class CGV_API nui_tool :
			public cgv::base::named, 
			public cgv::render::drawable, 
			public cgv::gui::event_handler,
			public cgv::gui::provider
		{
		protected:
			/// the number of supported modi
			uint32_t nr_modi;
			/// the currently selected mode
			uint32_t current_mode;
			/// 
			ToolType type;
			/// function to draw a mode, where the 3d model should fit into the box [-1,1]³
			virtual void draw_mode(cgv::render::context& ctx, uint32_t mode_index) { draw(ctx); }
		public:
			nui_tool(const std::string& _name, ToolType _type, uint32_t _nr_modi = 1);
			uint32_t get_nr_modi() const { return nr_modi; }
			uint32_t get_current_mode() const { return current_mode;  }
			void set_current_mode(uint32_t _mode) { current_mode = _mode; }
		};
	}
}

#include <cgv/config/lib_end.h>