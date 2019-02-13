#pragma once

#include "vr_kit.h"

#include <vector>

#include "lib_begin.h"

namespace vr {
	/// information provided per vr device
	class CGV_API gl_vr_display : public vr_kit
	{
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
		/// initialize render targets and framebuffer objects in current opengl context
		bool init_fbos();
		/// enable the framebuffer object of given eye (0..left, 1..right) 
		void enable_fbo(int eye);
		/// disable the framebuffer object of given eye
		void disable_fbo(int eye);
	};
}

#include <cgv/config/lib_end.h>
