#pragma once

#include <vector>
#include <string>

#include "lib_begin.h"

///@ingroup VR
///@{


///
namespace vr {
	/// enum to support restriction of eyes to a single eye
	enum EyeSelection {
		ES_BOTH,
		ES_LEFT,
		ES_RIGHT
	};
	//! implements offscreen rendering
	/*! vr_kit derives from gl_vr_display to provide offscreen rendering vor openvr driver as well as 
		for the vr_emulated_kit of the vr_emulator. Implements the init_fbos(), enable_fbos(), 
		disable_fbos(), destruct_fbos() and blit_fbo() functions with opengl and glew only. */
	class CGV_API gl_vr_display
	{
	private:
		int vp[4];
		int old_msaa;
	protected:
		/// pixel dimensions of render targets
		unsigned width, height, nr_multi_samples;
		/// ids of gl render objects
		unsigned multi_depth_buffer_id[2];
		unsigned multi_tex_id[2];
		unsigned multi_fbo_id[2];
		unsigned tex_id[2];
		unsigned fbo_id[2];

		friend class vr_wall_kit;
		/// store last error message
		std::string last_error;
	public:
		/// construct
		gl_vr_display(unsigned _width, unsigned _height, unsigned _nr_multi_samples);
		/// declare virtual destructor
		virtual ~gl_vr_display();
		/// return last error of vr_kit
		const std::string& get_last_error() const;
		/// return width in pixel of view
		int get_width() const;
		/// return height in pixel of view
		int get_height() const;
		/// allow to set a different size
		virtual void set_size(int new_width, int new_height);
		/// initialize render targets and framebuffer objects in current opengl context
		virtual bool init_fbos(EyeSelection es = ES_BOTH);
		/// initialize render targets and framebuffer objects in current opengl context
		virtual bool blit_fbo(int eye, int x, int y, int w, int h);
		/// check whether fbos have been initialized
		virtual bool fbos_initialized(EyeSelection es = ES_BOTH) const;
		/// destruct render targets and framebuffer objects in current opengl context
		virtual void destruct_fbos(EyeSelection es = ES_BOTH);
		/// enable the framebuffer object of given eye (0..left, 1..right) 
		virtual void enable_fbo(int eye);
		/// disable the framebuffer object of given eye
		virtual void disable_fbo(int eye);
		/// bind texture of given eye to current texture unit
		virtual void bind_texture(int eye);
	};
}

///@}

#include <cgv/config/lib_end.h>
