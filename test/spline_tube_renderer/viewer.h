#pragma once

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/spline_tube_renderer.h>

using namespace cgv::render;

class viewer :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler
{
protected:
	view* view_ptr;

	spline_tube_render_style spline_tubes_style;

	bool use_radii;
	bool use_position_tangents;
	bool use_radius_tangents;
	bool use_colors;

public:
	class viewer();
	std::string get_type_name() const { return "viewer"; }

	void clear(context& ctx);

	bool self_reflect(cgv::reflect::reflection_handler& rh);
	void stream_help(std::ostream& os) {}
	void stream_stats(std::ostream& os) {}

	bool handle(cgv::gui::event& e);
	void on_set(void* member_ptr);

	bool init(context& ctx);
	void init_frame(context& ctx);
	void draw(context& ctx);

	void create_gui();
};
