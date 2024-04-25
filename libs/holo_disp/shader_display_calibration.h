#pragma once

#include "holographic_display_calibration.h"
#include <cgv/render/context.h>
#include <cgv/render/stereo_view.h>
#include <cgv/render/shader_program.h>

#include "lib_begin.h"

namespace holo_disp {
	//! class to manage the calibration of holographic display for a shader program that uses the shader "holo_disp.glfs" in its fragment shader
	struct CGV_API shader_display_calibration
	{
		float pitch = 673.46088569750157f;
		float slope = -0.074780801514116493f;
		float center = 0.076352536678314209f;
		bool interpolate_view_matrix = true;
		float eye_separation_factor = 10.0f;
		shader_display_calibration();
		void compute(const holographic_display_calibration& hdc);
		void stereo_translate_modelview_matrix(float eye, float eye_separation, float screen_width, cgv::mat4& M);
		void set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, const cgv::render::stereo_view& sv);
	};
}

#include <cgv/config/lib_end.h>