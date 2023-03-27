#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
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
	cgv::render::shader_program raycast_prog;

	struct Calibration {
		float pitch;
		float slope;
		float center;
	} calibration;

	vec4 test = vec4(
		/* eye_separation_factor */ 1.0f,
		/* frustum_shear_factor */  1.0f,
		0, 0 // as-of-yet unused
	);

public:
	holo_raycast() : cgv::base::node("Holographic Raycast Demo")
	{
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
		   << "\teye_separation_multiplier[left|right], frustum_shear_factor[,|.], reload shader [R]" << std::endl;
	}

	void on_set (void *member_ptr) {
		update_member(member_ptr);
		post_redraw();
	}

	bool handle (cgv::gui::event& e)
	{
		if (e.get_kind() == cgv::gui::EID_MOUSE)
		{
			const auto &me = static_cast<cgv::gui::mouse_event&>(e);
			const auto ma = me.get_action();

			if (   me.get_button_state() & cgv::gui::MouseButton::MB_LEFT_BUTTON
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
						/* DoNothing() */;
				}
			}
		}
		else if (e.get_kind() == cgv::gui::EID_KEY)
		{
			const auto &ke = static_cast<cgv::gui::key_event&>(e);
			const auto ka = ke.get_action();

			switch(ke.get_key())
			{
				case 'R': if (ka == cgv::gui::KeyAction::KA_PRESS) {
					std::cout << "reload" << std::endl;
					reload_shader();
					post_redraw();
					return true;
				}
				case cgv::gui::KEY_Left: if (ka != cgv::gui::KeyAction::KA_RELEASE) {
					test.x() -= 0.0625f;
					std::cout << "eye_separation_factor: "<<test.x() << std::endl;
					on_set(&test.x());
					return true;
				}
				case cgv::gui::KEY_Right: if (ka != cgv::gui::KeyAction::KA_RELEASE) {
					test.x() += 0.0625f;
					std::cout << "eye_separation_factor: " << test.x() << std::endl;
					on_set(&test.x());
					return true;
				}
				case '.': if (ka != cgv::gui::KeyAction::KA_RELEASE) {
					test.y() = std::min(1.f, test.y()+0.0625f);
					std::cout << "frustum_shear_factor: "<<test.y() << std::endl;
					on_set(&test.y());
					return true;
				}
				case ',': if (ka != cgv::gui::KeyAction::KA_RELEASE) {
					test.y() = std::max(0.f, test.y()-0.0625f);
					std::cout << "frustum_shear_factor: "<<test.y() << std::endl;
					on_set(&test.y());
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
		return load_shader(ctx);
	}

	void draw (cgv::render::context &ctx)
	{
		raycast_prog.enable(ctx);

		vec2 viewport_dims = vec2(static_cast<float>(ctx.get_width()), static_cast<float>(ctx.get_height()));

		cgv::render::view* view_ptr = find_view_as_node();
		cgv::render::stereo_view*  sview_ptr = reinterpret_cast<cgv::render::stereo_view*>(view_ptr);

		vec3 eye_pos = view_ptr->get_eye();

		const ivec4& current_vp = ctx.get_window_transformation_array().front().viewport;
		float aspect = static_cast<float>(current_vp[2]) / static_cast<float>(current_vp[3]);
		float y_extent_at_focus = static_cast<float>(sview_ptr->get_y_extent_at_focus());
		float eye_separation = static_cast<float>(sview_ptr->get_eye_distance());
		float parallax_zero_depth = static_cast<float>(sview_ptr->get_parallax_zero_depth());
		float z_near = static_cast<float>(sview_ptr->get_z_near());
		float z_far = static_cast<float>(sview_ptr->get_z_far());

		raycast_prog.set_uniform(ctx, "viewport_dims", viewport_dims);
		raycast_prog.set_uniform(ctx, "eye_pos", eye_pos);
		raycast_prog.set_uniform(ctx, "eye_separation", test.x() * eye_separation);
		raycast_prog.set_uniform(ctx, "screen_width", y_extent_at_focus * aspect);
		raycast_prog.set_uniform(ctx, "screen_height", y_extent_at_focus);
		raycast_prog.set_uniform(ctx, "parallax_zero_depth", parallax_zero_depth);
		raycast_prog.set_uniform(ctx, "z_near", z_near);
		raycast_prog.set_uniform(ctx, "z_far", z_far);
		raycast_prog.set_uniform(ctx, "test", test);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		raycast_prog.disable(ctx);
	}

	void create_gui() {
		add_decorator(name, "heading");
		add_member_control(this, "eye_separation_factor", test[0], "value_slider", "min=0;max=2;step=0.0625");
		add_member_control(this, "frustum_shear_factor", test[1], "value_slider", "min=0;max=1;step=0.0625");
		add_member_control(this, "C", test[2], "value_slider", "min=0;max=2;step=0.0625");
		add_member_control(this, "D", test[3], "value_slider", "min=0;max=2;step=0.0625");
	}

	bool load_shader(cgv::render::context& ctx)
	{
		bool success = cgv::render::shader_library::load(
			ctx, raycast_prog, "holo_raycast.glpr", true, "holo_raycast::init()"
		);

		if (success) {
			raycast_prog.set_uniform(ctx, "pitch", calibration.pitch);
			raycast_prog.set_uniform(ctx, "slope", calibration.slope);
			raycast_prog.set_uniform(ctx, "center", calibration.center);
		}

		return success;
	}

	void reload_shader() {
		if(auto ctx_ptr = get_context()) {
			auto& ctx = *ctx_ptr;
			ctx.disable_shader_file_cache();
			load_shader(ctx);
			ctx.enable_shader_file_cache();
		}
	}
};

#include <cgv/base/register.h>

/// register a factory to create new holographic raycast demos
cgv::base::factory_registration<holo_raycast> holo_raycast2_fac("New/Demo/Holographic Raycast Demo");
