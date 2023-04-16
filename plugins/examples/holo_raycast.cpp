#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/math/ftransform.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_library.h>
#include <cgv/render/stereo_view.h>
#include <cgv_gl/gl/gl_context.h>


class holo_raycast :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler {
protected:
	cgv::render::view* view_ptr = nullptr;
	cgv::render::stereo_view* stereo_view_ptr = nullptr;

	cgv::render::shader_program raycast_prog;
	
	struct Calibration {
		float pitch;
		float slope;
		float center;
	} calibration;

	bool interpolate_view_matrix = true;
	float eye_separation_factor = 1.0f;
	vec3 light_direction;
	bool enable_shadows = true;

	bool debug_matrices = false;
	float view_test = 0.0f;

public:
	holo_raycast() : cgv::base::node("Holographic Raycast Demo")
	{
		light_direction = normalize(vec3(1.0f, 1.5f, 1.0f));

		/* Example calibration file (visual.json)
		{
			"configVersion": "1.0",
			"serial": "LKG-********",
			"pitch": {
				"value": 50.0693473815918
			},
			"slope": {
				"value": -7.521984100341797
			},
			"center": {
				"value": 0.07635253667831421
			},
			"fringe": {
				"value": 0.0
			},
			"viewCone": {
				"value": 40.0
			},
			"invView": {
				"value": 1.0
			},
			"verticalAngle": {
				"value": 0.0
			},
			"DPI": {
				"value": 283.0
			},
			"screenW": {
				"value": 3840.0
			},
			"screenH": {
				"value": 2160.0
			},
			"flipImageX": {
				"value": 0.0
			},
			"flipImageY": {
				"value": 0.0
			},
			"flipSubp": {
				"value": 0.0
			}
		}
		*/

		// Extracted values from calibration file
		const double screenW = 3840.0;
		const double screenH = 2160.0;
		const double DPI = 283.0;
		const double pitch = 50.0693473815918;
		const double slope = -7.521984100341797;
		const double center = 0.07635253667831421;
		const double flipImageX = 0.0;
		const double flipImageY = 0.0;

		double screenInches = screenW / DPI;
		double newPitch = pitch * screenInches;
		newPitch *= cos(atan(1.0 / slope));

		double newTilt = screenH / (screenW * slope);

		if(flipImageX == 1.0)
			newTilt *= -1.0;

		std::cout << "Calibration values:" << std::endl;
		std::cout << "  pitch: " << pitch << std::endl;
		std::cout << "  slope: " << slope << std::endl;
		std::cout << "  center: " << center << std::endl;
		std::cout << "Computed values:" << std::endl;
		std::cout << "  pitch: " << newPitch << std::endl;
		std::cout << "  slope: " << newTilt << std::endl;
		std::cout << "  center: " << center << std::endl;
		std::cout << std::endl;

		calibration.pitch = static_cast<float>(newPitch);
		calibration.slope = static_cast<float>(newTilt);
		calibration.center = static_cast<float>(center);

		// Raw computed calibration variables for dev display
		// new pitch: 673.46088569750157
		// new slope : -0.074780801514116493
		// new center : 0.076352536678314209

		/*
		 * Example for quilt rendering using the calibration values from above
		 *
		// mpv glsl shader hook for looking glass
		// Usage sample:
		//  mpv --screen=1 --fs-screen=1 --fs --glsl-shader=quiltshader.glsl --no-keepaspect *.mp4
		// Decent sample frame: Holo Reality at 26 seconds, -ss 26 Holo*.mp4

		//!HOOK MAINPRESUB
		//!DESC Looking Glass Quilt renderer
		//!BIND HOOKED
		//!WIDTH 2560
		//!HEIGHT 1600

		// TODO: Fill these in from HID calibration data.
		const float tilt = -0.12039111681976107; //{tilt};
		const float pitch = 370.66407267416486; //{pitch};
		const float center = 0.13695651292800903 + tilt*pitch; //{center};
		const float subp = 1.0 / (3*2560) * pitch; //{subp};

		// not all the streams are 5x9 quilts.
		// For instance Baby* is 4x8

		const vec2 tiles = vec2(5,9);

		vec2 quilt_map(vec2 pos, float a) {
		  // Y major positive direction, X minor negative direction
		  vec2 tile = vec2(tiles.x-1,0), dir=vec2(-1,1);
		  a = fract(a)*tiles.y;
		  tile.y += dir.y*floor(a);
		  a = fract(a)*tiles.x;
		  tile.x += dir.x*floor(a);
		  return (tile+pos)/tiles;
		}

		vec4 hook() {
		  vec4 res;
		  float a;
		  a = (HOOKED_pos.x + HOOKED_pos.y*tilt)*pitch - center;
		  res.x = HOOKED_tex(quilt_map(HOOKED_pos, a)).x;
		  res.y = HOOKED_tex(quilt_map(HOOKED_pos, a+subp)).y;
		  res.z = HOOKED_tex(quilt_map(HOOKED_pos, a+2*subp)).z;
		  res.w = 1.0;
		  return res;
		}
		*/
	}

	void stream_help (std::ostream &os) {
		os << "holo_raycast:" << std::endl
		   << "\teye_separation_mult[left|right], frustum_shear_factor[,|.], reload shader[R]" << std::endl;
	}

	void on_set (void *member_ptr) {

		if(member_ptr == &light_direction[0] || member_ptr == &light_direction[1] || member_ptr == &light_direction[2]) {
			light_direction.normalize();
			update_member(&light_direction);
		}

		update_member(member_ptr);
		post_redraw();
	}

	bool handle (cgv::gui::event& e)
	{
		if (e.get_kind() == cgv::gui::EID_MOUSE)
		{
			const auto &me = static_cast<cgv::gui::mouse_event&>(e);
			const auto ma = me.get_action();

			/*if (   me.get_button_state() & cgv::gui::MouseButton::MB_LEFT_BUTTON
			    && me.get_modifiers() & cgv::gui::EventModifier::EM_CTRL)
			{
				switch(ma)
				{
					case cgv::gui::MA_DRAG:
					{
						const auto &ctx = *get_context();
						float height = static_cast<float>(ctx.get_height());
						test.y() = (height - static_cast<float>(me.get_y()) - 1) / height;
						post_redraw();
						return true;
					}
					default:
						/* DoNothing() *;
				}
			}*/
		}
		else if (e.get_kind() == cgv::gui::EID_KEY)
		{
			const auto &ke = static_cast<cgv::gui::key_event&>(e);
			const auto ka = ke.get_action();

			switch(ke.get_key())
			{
				case 'R': if (ka == cgv::gui::KeyAction::KA_PRESS) {
					std::cout << "reload" << std::endl;
					reload_shaders();
					post_redraw();
					return true;
				}
				case cgv::gui::KEY_Left: if (ka != cgv::gui::KeyAction::KA_RELEASE) {
					eye_separation_factor -= 0.0625f;
					std::cout << "eye_separation_factor: "<< eye_separation_factor << std::endl;
					on_set(&eye_separation_factor);
					return true;
				}
				case cgv::gui::KEY_Right: if (ka != cgv::gui::KeyAction::KA_RELEASE) {
					eye_separation_factor += 0.0625f;
					std::cout << "eye_separation_factor: " << eye_separation_factor << std::endl;
					on_set(&eye_separation_factor);
					return true;
				}
				default:
					/* DoNothing() */;
			}
		}

		return false;
	}

	std::string get_type_name() const {
		return "holo_raycast";
	}

	void clear (cgv::render::context &ctx) {
		raycast_prog.destruct(ctx);
	}

	bool init (cgv::render::context &ctx) {
		return cgv::render::shader_library::load(ctx, raycast_prog, "holo_raycast.glpr", false, "holo_raycast::init()");
	}

	void init_frame(cgv::render::context &ctx) {
		if(!view_ptr) {
			view_ptr = find_view_as_node();
			stereo_view_ptr = reinterpret_cast<cgv::render::stereo_view*>(view_ptr);
		}
	}

	void draw (cgv::render::context &ctx)
	{
		if(!stereo_view_ptr)
			return;

		if(debug_matrices)
			test_matrix_interpolation(ctx, stereo_view_ptr);

		raycast_prog.enable(ctx);
		// set Looking Glass calibration parameters
		raycast_prog.set_uniform(ctx, "pitch", calibration.pitch);
		raycast_prog.set_uniform(ctx, "slope", calibration.slope);
		raycast_prog.set_uniform(ctx, "center", calibration.center);

		vec2 viewport_dims = vec2(static_cast<float>(ctx.get_width()), static_cast<float>(ctx.get_height()));

		vec3 eye_pos = view_ptr->get_eye();

		const ivec4& current_vp = ctx.get_window_transformation_array().front().viewport;
		float aspect = static_cast<float>(current_vp[2]) / static_cast<float>(current_vp[3]);
		float y_extent_at_focus = static_cast<float>(stereo_view_ptr->get_y_extent_at_focus());
		float eye_separation = static_cast<float>(stereo_view_ptr->get_eye_distance());
		float parallax_zero_depth = static_cast<float>(stereo_view_ptr->get_parallax_zero_depth());
		float z_near = static_cast<float>(stereo_view_ptr->get_z_near());
		float z_far = static_cast<float>(stereo_view_ptr->get_z_far());

		float screen_width = y_extent_at_focus * aspect;

		eye_separation *= eye_separation_factor;

		// general parameters needed to perform raycasting
		raycast_prog.set_uniform(ctx, "viewport_dims", viewport_dims);
		// variables needed for constructing matrices
		raycast_prog.set_uniform(ctx, "eye_pos", eye_pos);
		raycast_prog.set_uniform(ctx, "eye_separation", eye_separation);
		raycast_prog.set_uniform(ctx, "screen_width", screen_width);
		raycast_prog.set_uniform(ctx, "screen_height", y_extent_at_focus);
		raycast_prog.set_uniform(ctx, "parallax_zero_depth", parallax_zero_depth);
		raycast_prog.set_uniform(ctx, "z_near", z_near);
		raycast_prog.set_uniform(ctx, "z_far", z_far);
		
		// projection matrices for left and right view
		mat4 P_left = cgv::math::stereo_frustum_screen4(-1.0f, eye_separation, y_extent_at_focus * aspect, y_extent_at_focus, parallax_zero_depth, z_near, z_far);
		mat4 P_right = cgv::math::stereo_frustum_screen4(1.0f, eye_separation, y_extent_at_focus * aspect, y_extent_at_focus, parallax_zero_depth, z_near, z_far);

		// modelview matrices for left and right view
		mat4 MV_left = ctx.get_modelview_matrix();
		mat4 MV_right = MV_left;
		stereo_translate_modelview_matrix(-1.0f, eye_separation, screen_width, MV_left);
		stereo_translate_modelview_matrix(1.0f, eye_separation, screen_width, MV_right);

		// get world-space eye position from inverse modelview matrix per view
		vec3 eye_left = vec3(inv(MV_left).col(3));
		vec3 eye_right = vec3(inv(MV_right).col(3));

		// compute inverse modelview projection matrices for left and right view
		mat4 iMVP_left = inv(P_left * MV_left);
		mat4 iMVP_right = inv(P_right * MV_right);
		// acquire view-dependent varying elements (last two columns)
		mat4 stereo_view_params;
		stereo_view_params.set_col(0, iMVP_left.col(2));
		stereo_view_params.set_col(1, iMVP_right.col(2));
		stereo_view_params.set_col(2, iMVP_left.col(3));
		stereo_view_params.set_col(3, iMVP_right.col(3));

		// variables needed for interpolating left and right view matrices
		raycast_prog.set_uniform(ctx, "stereo_view_params", stereo_view_params);
		raycast_prog.set_uniform(ctx, "eye_pos_left", eye_left);
		raycast_prog.set_uniform(ctx, "eye_pos_right", eye_right);

		// general appearance and test parameters
		raycast_prog.set_uniform(ctx, "light_direction", light_direction);
		raycast_prog.set_uniform(ctx, "enable_shadows", enable_shadows);
		raycast_prog.set_uniform(ctx, "interpolate_view_matrix", interpolate_view_matrix);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		raycast_prog.disable(ctx);
	}

	void stereo_translate_modelview_matrix(float eye, float eye_separation, float screen_width, mat4& M) {
		M(0, 3) += -0.5 * eye_separation * eye * screen_width;
	}

	void test_matrix_interpolation(cgv::render::context& ctx, cgv::render::stereo_view* sview_ptr) {

		const ivec4& current_vp = ctx.get_window_transformation_array().front().viewport;
		float aspect = static_cast<float>(current_vp[2]) / static_cast<float>(current_vp[3]);
		float y_extent_at_focus = static_cast<float>(stereo_view_ptr->get_y_extent_at_focus());
		float eye_separation = static_cast<float>(stereo_view_ptr->get_eye_distance());
		float parallax_zero_depth = static_cast<float>(stereo_view_ptr->get_parallax_zero_depth());
		float z_near = static_cast<float>(stereo_view_ptr->get_z_near());
		float z_far = static_cast<float>(stereo_view_ptr->get_z_far());

		float screen_width = y_extent_at_focus * aspect;

		eye_separation *= eye_separation_factor;

		mat4 P0 = cgv::math::stereo_frustum_screen4(-1.0f, eye_separation, y_extent_at_focus * aspect, y_extent_at_focus, parallax_zero_depth, z_near, z_far);
		mat4 P1 = cgv::math::stereo_frustum_screen4(1.0f, eye_separation, y_extent_at_focus * aspect, y_extent_at_focus, parallax_zero_depth, z_near, z_far);

		mat4 MV0 = ctx.get_modelview_matrix();
		mat4 MV1 = ctx.get_modelview_matrix();
		stereo_translate_modelview_matrix(-1.0f, eye_separation, screen_width, MV0);
		stereo_translate_modelview_matrix(1.0f, eye_separation, screen_width, MV1);

		mat4 iP0 = inv(P0);
		mat4 iP1 = inv(P1);

		mat4 iMV0 = inv(MV0);
		mat4 iMV1 = inv(MV1);

		mat4 MVP0 = P0 * MV0;
		mat4 MVP1 = P1 * MV1;

		mat4 iMVP0 = inv(MVP0);
		mat4 iMVP1 = inv(MVP1);

		float eye_value = view_test - 1.0f;
		float interpolation_param = 0.5f * view_test;

		mat4 Pmid = cgv::math::stereo_frustum_screen4(eye_value, eye_separation, y_extent_at_focus * aspect, y_extent_at_focus, parallax_zero_depth, z_near, z_far);
		mat4 MVmid = ctx.get_modelview_matrix();
		stereo_translate_modelview_matrix(eye_value, eye_separation, screen_width, MVmid);

		mat4 iPmid = inv(Pmid);
		mat4 iMVmid = inv(MVmid);

		mat4 MVPmid = Pmid * MVmid;
		mat4 iMVPmid = inv(MVPmid);

		vec3 col20(iMVP0.col(2));
		vec3 col30(iMVP0.col(3));

		vec3 col21(iMVP1.col(2));
		vec3 col31(iMVP1.col(3));

		vec3 col2int = cgv::math::lerp(col20, col21, interpolation_param);
		vec3 col3int = cgv::math::lerp(col30, col31, interpolation_param);

		vec3 col2mid(iMVPmid.col(2));
		vec3 col3mid(iMVPmid.col(3));

		mat4 Pint(0.0f);
		mat4 MVint(0.0f);
		mat4 MVPint(0.0f);
		mat4 iPint(0.0f);
		mat4 iMVint(0.0f);
		mat4 iMVPint(0.0f);
		for(int i = 0; i < 4; ++i) {
			for(int j = 0; j < 4; ++j) {
				Pint(i, j) = cgv::math::lerp(P0(i, j), P1(i, j), interpolation_param);
				MVint(i, j) = cgv::math::lerp(MV0(i, j), MV1(i, j), interpolation_param);
				MVPint(i, j) = cgv::math::lerp(MVP0(i, j), MVP1(i, j), interpolation_param);
				iPint(i, j) = cgv::math::lerp(iP0(i, j), iP1(i, j), interpolation_param);
				iMVint(i, j) = cgv::math::lerp(iMV0(i, j), iMV1(i, j), interpolation_param);
				iMVPint(i, j) = cgv::math::lerp(iMVP0(i, j), iMVP1(i, j), interpolation_param);
			}
		}

		mat4 D(0.0f);
		for(int i = 0; i < 4; ++i) {
			for(int j = 0; j < 4; ++j) {
				D(i, j) = iMVPmid(i, j) - iMVPint(i, j);
			}
		}

		std::cout << "Eye: " << eye_value << ", Interpoaltion t: " << interpolation_param << std::endl;
		//std::cout << "Left Eye: " << std::endl;
		//std::cout << iMVP0 << std::endl;
		//std::cout << "Right Eye: " << std::endl;
		//std::cout << iMVP1 << std::endl;
		std::cout << "Center: " << std::endl;
		std::cout << iMVPmid << std::endl;
		std::cout << "Diff: " << std::endl;
		vec3 d2 = col2int - col2mid;
		vec3 d3 = col3int - col3mid;
		std::cout << d2 << std::endl;
		std::cout << d3 << std::endl;
		//std::cout << "Interpolated: " << std::endl;
		//std::cout << MVint << std::endl;*/
		//std::cout << "Difference Original vs Interpolated: " << std::endl;
		//std::cout << D << std::endl;
	}

	void create_gui() {
		add_decorator(name, "heading");

		add_member_control(this, "Interpolate View Matrix", interpolate_view_matrix, "check");

		add_member_control(this, "Eye Separation Factor", eye_separation_factor, "value_slider", "min=0;max=2;step=0.0625");

		add_decorator("Light Direction", "separator");
		add_member_control(this, "X", light_direction[0], "value_slider", "min=-1;max=1;step=0.0625");
		add_member_control(this, "Y", light_direction[1], "value_slider", "min=-1;max=1;step=0.0625");
		add_member_control(this, "Z", light_direction[2], "value_slider", "min=-1;max=1;step=0.0625");

		add_member_control(this, "Shadows", enable_shadows, "check");

		add_member_control(this, "Debug Matrices", debug_matrices, "check");
		add_member_control(this, "View Test", view_test, "value_slider", "min=-1;max=1;step=0.0625");
	}

	void reload_shaders() {
		if(auto ctx_ptr = get_context()) {
			auto& ctx = *ctx_ptr;
			ctx.disable_shader_file_cache();
			cgv::render::shader_library::load(ctx, raycast_prog, "holo_raycast.glpr", true, "holo_raycast::init()");
			ctx.enable_shader_file_cache();
		}
	}
};

#include <cgv/base/register.h>

/// register a factory to create new holographic raycast demos
cgv::base::factory_registration<holo_raycast> holo_raycast2_fac("New/Demo/Holographic Raycast Demo");
