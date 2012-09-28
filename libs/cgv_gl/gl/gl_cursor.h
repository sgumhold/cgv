#pragma once

#include <string>
#include <vector>

#include "lib_begin.h"

namespace cgv {
	namespace render {
		namespace gl {

/** windows only implementation of functionality to create and draw an RGBA texture
    representing standard mouse cursor icons. This class is completely standalone
	and does not depend on any other file in the cgv framework. */
class CGV_API gl_cursor
{
public:
	int hot_x, hot_y;
	unsigned w, h;
	std::vector<unsigned> tex_ids;
	std::vector<unsigned> frames;
	std::vector<unsigned> periods;
	unsigned duration;
	void update_duration();
	void clear();
	void add_frame(unsigned tex_id);
public:
	/// construct empty gl_cursor instance
	gl_cursor();
	/// check whether cursor has been created
	bool is_created() const;
	//! Create cursor from textual name of default cursor.
	/*! This can only be called when the opengl context is active
	    in which this cursor is used. The return value tells whether 
		the creation was successful. Possible values for the cursor
		id are:
		- "startup" ... arrow with wait icon
		- "arrow" ... default arrow
		- "cross" ... simple cross
		- "wait" ... icon used when applications are stalled
		- "hand" ... hand with pointing finger
		- "help" ... help icon
		- "no"   ... circle with diagonal dash
		- "ns"   ... double arrow from north to south
		- "we"   ... double arrow from west to east
		- "move" ... quadrupel arrow pointing north, east, south, west
		- "nesw" ... double arrow pointing from north-east to south-west
		- "nwse" ... double arrow pointing from north-west to south-east. */
	bool create(const std::string& id = "arrow");
	/// Create cursor from a file of type .cur or .ani
	bool create_from_file(const std::string& file_name);
	/// return the width of the cursor
	unsigned get_width() const { return w; }
	/// return the height of the cursor
	unsigned get_height() const { return h; }
	/// return the x-coordinate of the hot spot of the cursor in local pixel coordinates
	int get_hot_x() const { return hot_x; }
	/// return the y-coordinate of the hot spot of the cursor in local pixel coordinates
	int get_hot_y() const { return hot_y; }
	/// set a new hot spot of the cursor in local pixel coordinates
	void set_hot_spot(int _x, int _y) { hot_x = _x; hot_y = _x; }
	/// return the number of animation steps
	unsigned get_nr_steps() const;
	/// return the number of animation frames
	unsigned get_nr_frames() const;
	/// return the gl texture id of the given frame
	unsigned get_texture_id(unsigned frame_idx = 0) const;
	/// return the frame index of given step
	unsigned get_step_frame(unsigned step_idx) const;
	/// set the frame index of given step
	void set_step_frame(unsigned step_idx, unsigned frame_idx);
	/// return the period of given step in 1/60th of a second
	unsigned get_step_period(unsigned step_idx) const;
	/// return the period of given step in 1/60th of a second
	void set_step_period(unsigned step_idx, unsigned period);
	/// append an animation step given by frame index and step period
	void append_step(unsigned frame_idx, unsigned period);
	/// find the step index of a given elapsed time in seconds
	unsigned find_step_index(double elapsed_seconds) const;
	/// return the gl texture id of the given frame
	unsigned get_frame_texture_id(int frame_idx) const;
	//! draw the cursor at the given location
	/*! The current opengl transformation must result in a scaling proportional to
	    pixel coordinates. By default the texture color and alpha values are directly
		drawn to screen. If \c use_color is set to true, color and alpha are multiplied
		with the currently set color. */
	void draw(int x, int y, bool use_color = false, unsigned frame_idx = 0);
};

		}
	}
}

#include <cgv/config/lib_end.h>
