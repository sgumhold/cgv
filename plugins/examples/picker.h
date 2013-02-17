#pragma once

#include <cgv/base/node.h>
#include <cgv/math/vec.h>
#include <cgv/gui/event_handler.h>
#include <cgv/render/drawable.h>

/// the picker allows to place points on a square and to remove them again
class picker : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::event_handler,  /// derive from handler to receive events and to be asked for a help string
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
public:
	/// define point type
	typedef cgv::math::vec<double> Pnt;
protected:
	/// store list of picked points
	std::vector<Pnt> pnts;
	/// check if a world point is close enough to the drawing square
	bool is_inside(const Pnt& p3d) const;
	/// transform from 3d world to 2d parametric space
	Pnt transform_2_local(const Pnt& p3d) const;
	/// find closest point and return index or -1 if we do not have any points yet
	int find_closest(const Pnt& p2d) const;
private:
	/// store index of to be tracked point
	int drag_pnt_idx;
	///
	bool is_drag_action;
	/// store the last transformation matrix
	cgv::render::context::mat_type DPV;
public:
	/// construct from name which is necessary construction argument to node
	picker(const char* name);
	/// necessary method of event_handler
	bool handle(cgv::gui::event& e);
	/// show internal values
	void stream_stats(std::ostream& os);
	/// necessary method of event_handler
	void stream_help(std::ostream& os);
	/// optional method of drawable
	void draw(cgv::render::context&);
};
