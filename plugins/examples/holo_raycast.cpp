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

	vec2 mouse_pos = vec2(0.0f);

public:
	holo_raycast() : cgv::base::node("Holographic Raycast Demo") {
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
		std::cout << std::endl;
		std::cout << "Computed values:" << std::endl;
		std::cout << "  pitch: " << newPitch << std::endl;
		std::cout << "  slope: " << newTilt << std::endl;
		std::cout << "  center: " << center << std::endl;

		calibration.pitch = static_cast<float>(newPitch);
		calibration.slope = static_cast<float>(newTilt);
		calibration.center = static_cast<float>(center);

		// Raw computed calibration variables
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

	void stream_help(std::ostream& os) {
		os << "";
	}

	void on_set(void* member_ptr) {
		post_redraw();
		update_member(member_ptr);
	}

	bool handle(cgv::gui::event& e) {
		if(e.get_kind() == cgv::gui::EID_MOUSE) {
			auto& me = static_cast<cgv::gui::mouse_event&>(e);
			auto ma = me.get_action();
		
			if(me.get_button_state() & cgv::gui::MouseButton::MB_LEFT_BUTTON) {
				switch(ma) {
				case cgv::gui::MA_PRESS:
					return true;
				case cgv::gui::MA_DRAG:
					mouse_pos.x() = me.get_x();
					mouse_pos.y() = me.get_y();
					post_redraw();
					return true;
				case cgv::gui::MA_RELEASE:
					return true;
				default: break;
				}
			}
		}

		return false;
	}

	std::string get_type_name() const {
		return "holo_raycast";
	}

	void clear(cgv::render::context& ctx) {
		raycast_prog.destruct(ctx);
	}

	bool init(cgv::render::context& ctx) {
		return load_shader(ctx);
	}

	void draw(cgv::render::context& ctx) {
		raycast_prog.enable(ctx);
		
		vec2 viewport_dims = vec2(static_cast<float>(ctx.get_width()), static_cast<float>(ctx.get_height()));
		
		raycast_prog.set_uniform(ctx, "viewport_dims", viewport_dims);
		raycast_prog.set_uniform(ctx, "mouse_pos", mouse_pos);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		raycast_prog.disable(ctx);
	}

	void create_gui() {
		add_decorator(name, "heading");
	}

	bool load_shader(cgv::render::context& ctx) {
		bool success = cgv::render::shader_library::load(ctx, raycast_prog, "holo_raycast.glpr", true, "holo_raycast::init()");

		if(success) {
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
cgv::base::factory_registration<holo_raycast> holo_raycast_fac("New/Demo/Holographic Raycast Demo");
