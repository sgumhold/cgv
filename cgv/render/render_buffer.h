#pragma once

#include <cgv/render/context.h>
#include <cgv/data/component_format.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {

/** this class encapsulate render buffers that live on the GPU
    which must support frame buffer objects for this to work. */
class CGV_API render_buffer 
	: public render_buffer_base, 
	  public cgv::data::component_format
{
	int width, height;
public:
	using render_component::last_error;

	/// construct from description of component format, where the default format specifies a color buffer with alpha channel
	render_buffer(const std::string& description = "[R,G,B,A]");
	/// set the number of multi samples, which should be done before create
	void set_nr_multi_samples(unsigned _nr_samples);
	/// destruct the render buffer
	void destruct(const context& ctx);
	/** create a render buffer. 
	    If no extent is specified it is copied from the current viewport. */
	bool create(const context& ctx, int width = -1, int height = -1);
	/// check whether the buffer has been created
	bool is_created() const override;
	/// calls the destruct method if necessary
	~render_buffer();
	/// return the width in pixels of the buffer
	int get_width() const { return width; }
	/// return the height in pixels of the buffer
	int get_height() const { return height; }
	/// whether this buffer is a multi sample buffer
	bool is_multi_buffer() const { return nr_multi_samples > 0; }
	/// return number of multi samples
	unsigned get_nr_multi_samples() const { return nr_multi_samples; }
	/// return whether the component format corresponds to a depth buffer format
	bool is_depth_buffer() const;
	/// return whether the component format corresponds to a color buffer format
	bool is_color_buffer() const;
	// bool is_stencil_buffer() const;
};

	}
}

#include <cgv/config/lib_end.h>

