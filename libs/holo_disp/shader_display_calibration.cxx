#include "shader_display_calibration.h"
#include <cgv/math/ftransform.h>

namespace holo_disp {

shader_display_calibration::shader_display_calibration()
{
}
void shader_display_calibration::compute(const holographic_display_calibration& hdc)
{
	double screenInches = hdc.screen_width / hdc.DPI;
	pitch = float(hdc.pitch * screenInches * cos(atan(1.0 / hdc.slope)));
	slope = float(hdc.screen_height / (hdc.screen_width * hdc.slope));
	if (hdc.flip_x)
		slope *= -1.0;
	center = float(hdc.center);
}
void shader_display_calibration::stereo_translate_modelview_matrix(float eye, float eye_separation, float screen_width, cgv::mat4& M) {
	M(0, 3) = float(M(0, 3) - 0.5 * eye_separation * eye * screen_width);
}
void shader_display_calibration::set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, const cgv::render::stereo_view& sv)
{
	prog.set_uniform(ctx, "pitch", pitch);
	prog.set_uniform(ctx, "slope", slope);
	prog.set_uniform(ctx, "center", center);

	cgv::vec2 viewport_dims = cgv::vec2(static_cast<float>(ctx.get_width()), static_cast<float>(ctx.get_height()));
	cgv::vec3 eye_pos = sv.get_eye();
	const cgv::ivec4& current_vp = ctx.get_window_transformation_array().front().viewport;
	float aspect = static_cast<float>(current_vp[2]) / static_cast<float>(current_vp[3]);
	float y_extent_at_focus = static_cast<float>(sv.get_y_extent_at_focus());
	float eye_separation = static_cast<float>(sv.get_eye_distance());
	float parallax_zero_depth = static_cast<float>(sv.get_parallax_zero_depth());
	float z_near = static_cast<float>(sv.get_z_near());
	float z_far = static_cast<float>(sv.get_z_far());
	float screen_width = y_extent_at_focus * aspect;
	eye_separation *= eye_separation_factor;

	// general parameters needed to perform raycasting
	prog.set_uniform(ctx, "viewport_dims", viewport_dims);
	// variables needed for constructing matrices
	prog.set_uniform(ctx, "eye_pos", eye_pos);
	prog.set_uniform(ctx, "eye_separation", eye_separation);
	prog.set_uniform(ctx, "screen_width", screen_width);
	prog.set_uniform(ctx, "screen_height", y_extent_at_focus);
	prog.set_uniform(ctx, "parallax_zero_depth", parallax_zero_depth);
	prog.set_uniform(ctx, "z_near", z_near);
	prog.set_uniform(ctx, "z_far", z_far);

	// projection matrices for left and right view
	cgv::mat4 P_left = cgv::math::stereo_frustum_screen4(-1.0f, eye_separation, y_extent_at_focus * aspect, y_extent_at_focus, parallax_zero_depth, z_near, z_far);
	cgv::mat4 P_right = cgv::math::stereo_frustum_screen4(1.0f, eye_separation, y_extent_at_focus * aspect, y_extent_at_focus, parallax_zero_depth, z_near, z_far);

	// modelview matrices for left and right view
	cgv::mat4 MV_left = ctx.get_modelview_matrix();
	cgv::mat4 MV_right = MV_left;
	stereo_translate_modelview_matrix(-1.0f, eye_separation, screen_width, MV_left);
	stereo_translate_modelview_matrix(1.0f, eye_separation, screen_width, MV_right);

	// get world-space eye position from inverse modelview matrix per view
	cgv::vec3 eye_left = cgv::vec3(inv(MV_left).col(3));
	cgv::vec3 eye_right = cgv::vec3(inv(MV_right).col(3));

	// compute inverse modelview projection matrices for left and right view
	cgv::mat4 iMVP_left = inv(P_left * MV_left);
	cgv::mat4 iMVP_right = inv(P_right * MV_right);
	// acquire view-dependent varying elements (last two columns)
	cgv::mat4 stereo_view_params;
	stereo_view_params.set_col(0, iMVP_left.col(2));
	stereo_view_params.set_col(1, iMVP_right.col(2));
	stereo_view_params.set_col(2, iMVP_left.col(3));
	stereo_view_params.set_col(3, iMVP_right.col(3));

	// variables needed for interpolating left and right view matrices
	prog.set_uniform(ctx, "stereo_view_params", stereo_view_params);
	prog.set_uniform(ctx, "eye_pos_left", eye_left);
	prog.set_uniform(ctx, "eye_pos_right", eye_right);
	prog.set_uniform(ctx, "interpolate_view_matrix", interpolate_view_matrix);
}

}