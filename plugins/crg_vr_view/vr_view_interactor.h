#pragma once

#include <cgv/base/base.h>
#include <vr/vr_event.h>
#include <stereo_view_interactor.h>

#include "lib_begin.h"

class CGV_API vr_view_interactor : 
	public stereo_view_interactor	
{
protected:
public:
	///
	vr_view_interactor(const char* name);
	/// return the type name 
	std::string get_type_name() const;
	///
	void on_status_change(void* device_handle, int controller_index, vr::VRStatus old_status, vr::VRStatus new_status);
	///
	void on_device_change(void* device_handle, bool attach);
	/// overload to show the content of this object
	void stream_stats(std::ostream&);
	///
	bool init(cgv::render::context& ctx);
	/// overload and implement this method to handle events
	bool handle(cgv::gui::event& e);
	/// overload to stream help information to the given output stream
	void stream_help(std::ostream& os);
	/// this method is called in one pass over all drawables before the draw method
	void init_frame(cgv::render::context&);
	/// 
	void draw(cgv::render::context&);
	/// this method is called in one pass over all drawables after drawing
	void finish_frame(cgv::render::context&);
	///
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	/// you must overload this for gui creation
	void create_gui();
};

#include <cgv/config/lib_end.h>
