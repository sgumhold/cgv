#pragma once

#include <cgv/base/group.h>
#include <cgv/os/thread.h>
#include <cgv/os/mutex.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include "stream_vis_context.h"

#include "lib_begin.h"

namespace stream_vis {

	class CGV_API stream_vis : 
		public cgv::os::thread,
		public cgv::base::group,
		public cgv::render::drawable,
		public cgv::gui::provider
	{
	public:
		typedef cgv::data::ref_ptr<stream_vis_context,true> context_ptr;
		cgv::os::mutex lock;
		std::vector<context_ptr> new_contexts;
		std::vector<context_ptr> old_contexts;
	protected:
		std::vector<context_ptr> context_pool;
		void timer_event(double t, double dt);
	public:
		stream_vis();
		~stream_vis();
		void run();
		void register_context(context_ptr ctx_ptr);
		void unregister_context(context_ptr ctx_ptr);
		void process_context_changes();
		void create_gui();
	};

}

#include <cgv/config/lib_end.h>