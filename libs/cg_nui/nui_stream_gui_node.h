#pragma once

#include <cgv/base/group.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <3rd/screen_capture_lite/include/ScreenCapture.h>
#include "nui_node.h"
#include "rectangle_container.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class nui_stream_gui_node : public nui_node
		{
		protected:
			rectangle_container rc;
			std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> frame_grabber;
			std::string capture_config;
		public:
			nui_stream_gui_node(const std::string& name);
			void stream_help(std::ostream& os);
			bool handle(cgv::gui::event& e);
			void add_window_capture(const std::string& title_search);
			void add_monitor_capture(int i);
			bool start_grabbing();
			bool stop_grabbing();
		};
	}
}

#include <cgv/config/lib_end.h>