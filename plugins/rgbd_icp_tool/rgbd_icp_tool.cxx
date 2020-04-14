#include "rgbd_icp_tool.h"
#include <ICP.h>
#include <GoICP.h>

using namespace std;
using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::type;
using namespace cgv::gui;
using namespace cgv::data;
using namespace cgv::utils;
using namespace cgv::render;

rgbd_icp_tool::rgbd_icp_tool() {
	set_name("rgbd_icp_tool");
}

bool rgbd_icp_tool::self_reflect(cgv::reflect::reflection_handler & rh)
{
	return
		rh.reflect_member("ply_path", ply_path);
}

void rgbd_icp_tool::on_set(void * member_ptr)
{
}

void rgbd_icp_tool::on_register()
{
}

void rgbd_icp_tool::unregister()
{
}

bool rgbd_icp_tool::init(cgv::render::context & ctx)
{
	return false;
}

void rgbd_icp_tool::draw(cgv::render::context & ctx)
{
}

void rgbd_icp_tool::clear(cgv::render::context & ctx)
{
}

bool rgbd_icp_tool::handle(cgv::gui::event & e)
{
	return false;
}

void rgbd_icp_tool::stream_help(std::ostream & os)
{
}

void rgbd_icp_tool::create_gui()
{
}

void rgbd_icp_tool::timer_event(double t, double dt)
{
}

#include "lib_begin.h"
#include "rgbd_icp_tool.h"

extern CGV_API object_registration<rgbd_icp_tool> kc_or("");