#include "vr_view_interactor.h"
#include <cg_vr/vr_server.h>
#include <cgv/signal/rebind.h>
#include <iostream>
///
vr_view_interactor::vr_view_interactor(const char* name) : stereo_view_interactor(name)
{
	cgv::gui::connect_vr_server();
	cgv::signal::connect(cgv::gui::ref_vr_server().on_device_change, this, &vr_view_interactor::on_device_change);
	cgv::signal::connect(cgv::gui::ref_vr_server().on_status_change, this, &vr_view_interactor::on_status_change);
}

///
void vr_view_interactor::on_status_change(void* device_handle, int controller_index, vr::VRStatus old_status, vr::VRStatus new_status)
{
	std::cout << "on status change(" << device_handle << "," 
		<< controller_index << "," << vr::get_status_string(old_status) 
		<< "," << vr::get_status_string(new_status) << ")" << std::endl;
}

///
void vr_view_interactor::on_device_change(void* device_handle, bool attach)
{
	std::cout << "on device change(" << device_handle << ","
		<< (attach?"attach":"detach") << ")" << std::endl;
}

/// return the type name 
std::string vr_view_interactor::get_type_name() const
{
	return "vr_view_interactor";
}

/// overload to stream help information to the given output stream
void vr_view_interactor::stream_help(std::ostream& os)
{
	stereo_view_interactor::stream_help(os);
}

/// overload to show the content of this object
void vr_view_interactor::stream_stats(std::ostream& os)
{
	stereo_view_interactor::stream_stats(os);
}

bool vr_view_interactor::init(cgv::render::context& ctx)
{
	return stereo_view_interactor::init(ctx);
}

/// overload and implement this method to handle events
bool vr_view_interactor::handle(cgv::gui::event& e)
{
	if (e.get_kind() == cgv::gui::EID_VR || ((e.get_flags() & cgv::gui::EF_VR) != 0)) {
		e.stream_out(std::cout);
		std::cout << std::endl;
	}
	return stereo_view_interactor::handle(e);
}


/// this method is called in one pass over all drawables after drawing
void vr_view_interactor::finish_frame(cgv::render::context& ctx)
{
	stereo_view_interactor::finish_frame(ctx);
}


/// this method is called in one pass over all drawables before the draw method
void vr_view_interactor::init_frame(cgv::render::context& ctx)
{
	stereo_view_interactor::init_frame(ctx);
}

/// 
void vr_view_interactor::draw(cgv::render::context& ctx)
{
	stereo_view_interactor::draw(ctx);
}

/// you must overload this for gui creation
void vr_view_interactor::create_gui()
{
	stereo_view_interactor::create_gui();
}

/// you must overload this for gui creation
bool vr_view_interactor::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return stereo_view_interactor::self_reflect(srh);
}


#include <cgv/base/register.h>

/// register a newly created cube with the name "cube1" as constructor argument
extern cgv::base::object_registration_1<vr_view_interactor,const char*> 
 obj1("vr interactor", "registration of vr interactor");

