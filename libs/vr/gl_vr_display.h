#pragma once

#include "vr_kit.h"

#include <vector>

#include "lib_begin.h"

///@ingroup VR
///@{


///
namespace vr {
	//! implements offscreen rendering
	/*! gl_vr_display is used by the openvr driver as well as by the vr_emulated_kit
	    of the vr_emulator. Implements the init_fbos(), enable_fbos(), disable_fbos(), 
		destruct_fbos() functions with opengl and glew */
	class CGV_API gl_vr_display : public vr_kit
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

		/// construct
		gl_vr_display(unsigned _width, unsigned _height, vr_driver* _driver, void* _handle, const std::string& _name, bool _ffb_support, bool _wireless);
	public:
		/// declare virtual destructor
		~gl_vr_display();
		/// return width in pixel of view
		int get_width() const;
		/// return height in pixel of view
		int get_height() const;
		/// initialize render targets and framebuffer objects in current opengl context
		bool init_fbos();
		/// initialize render targets and framebuffer objects in current opengl context
		bool blit_fbo(int eye, int x, int y, int w, int h);
		/// check whether fbos have been initialized
		bool fbos_initialized() const;
		/// destruct render targets and framebuffer objects in current opengl context
		void destruct_fbos();
		/// enable the framebuffer object of given eye (0..left, 1..right) 
		void enable_fbo(int eye);
		/// disable the framebuffer object of given eye
		void disable_fbo(int eye);
	};
}

///@}

#include <cgv/config/lib_end.h>
