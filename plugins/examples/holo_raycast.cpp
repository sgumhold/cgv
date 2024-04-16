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
#include <cgv/defines/quote.h>
#include <holo_disp/shader_display_calibration.h>

class holo_raycast :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::provider,
	public cgv::gui::event_handler {
protected:
	cgv::render::stereo_view* stereo_view_ptr = nullptr;

	cgv::render::shader_program raycast_prog;
	holo_disp::holographic_display_calibration disp_calib;
	holo_disp::shader_display_calibration shader_calib;

	cgv::vec3 light_direction;
	bool enable_shadows = true;

	bool debug_matrices = false;
	float view_test = 0.0f;
public:
	holo_raycast() : cgv::base::node("Holographic Raycast Demo")
	{
		light_direction = normalize(cgv::vec3(1.0f, 1.5f, 1.0f));
		if (!disp_calib.read(QUOTE_SYMBOL_VALUE(INPUT_DIR) "/visual.json"))
			std::cerr << "could not read holographic display calibration from <"
					  << QUOTE_SYMBOL_VALUE(INPUT_DIR) "/visual.json>" << std::endl;
		shader_calib.compute(disp_calib);
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
					default: break;
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
					shader_calib.eye_separation_factor -= 0.0625f;
					std::cout << "eye_separation_factor: "<< shader_calib.eye_separation_factor << std::endl;
					on_set(&shader_calib.eye_separation_factor);
					return true;
				}
				case cgv::gui::KEY_Right: if (ka != cgv::gui::KeyAction::KA_RELEASE) {
					shader_calib.eye_separation_factor += 0.0625f;
					std::cout << "eye_separation_factor: " << shader_calib.eye_separation_factor << std::endl;
					on_set(&shader_calib.eye_separation_factor);
					return true;
				}
				default: break;
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
		if(!stereo_view_ptr)
			stereo_view_ptr = dynamic_cast<cgv::render::stereo_view*>(find_view_as_node());
	}
	void draw (cgv::render::context &ctx)
	{
		if(!stereo_view_ptr)
			return;
		if(debug_matrices)
			test_matrix_interpolation(ctx, stereo_view_ptr);
		raycast_prog.enable(ctx);
		// set Looking Glass calibration parameters
		shader_calib.set_uniforms(ctx, raycast_prog, *stereo_view_ptr);
		// general appearance and test parameters
		raycast_prog.set_uniform(ctx, "light_direction", light_direction);
		raycast_prog.set_uniform(ctx, "enable_shadows", enable_shadows);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		raycast_prog.disable(ctx);
	}
	void test_matrix_interpolation(cgv::render::context& ctx, cgv::render::stereo_view* sview_ptr) {

		const cgv::ivec4& current_vp = ctx.get_window_transformation_array().front().viewport;
		float aspect = static_cast<float>(current_vp[2]) / static_cast<float>(current_vp[3]);
		float y_extent_at_focus = static_cast<float>(stereo_view_ptr->get_y_extent_at_focus());
		float eye_separation = static_cast<float>(stereo_view_ptr->get_eye_distance());
		float parallax_zero_depth = static_cast<float>(stereo_view_ptr->get_parallax_zero_depth());
		float z_near = static_cast<float>(stereo_view_ptr->get_z_near());
		float z_far = static_cast<float>(stereo_view_ptr->get_z_far());

		float screen_width = y_extent_at_focus * aspect;

		eye_separation *= shader_calib.eye_separation_factor;

		cgv::mat4 P0 = cgv::math::stereo_frustum_screen4(-1.0f, eye_separation, y_extent_at_focus * aspect, y_extent_at_focus, parallax_zero_depth, z_near, z_far);
		cgv::mat4 P1 = cgv::math::stereo_frustum_screen4(1.0f, eye_separation, y_extent_at_focus * aspect, y_extent_at_focus, parallax_zero_depth, z_near, z_far);

		cgv::mat4 MV0 = ctx.get_modelview_matrix();
		cgv::mat4 MV1 = ctx.get_modelview_matrix();
		shader_calib.stereo_translate_modelview_matrix(-1.0f, eye_separation, screen_width, MV0);
		shader_calib.stereo_translate_modelview_matrix(1.0f, eye_separation, screen_width, MV1);

		cgv::mat4 iP0 = inv(P0);
		cgv::mat4 iP1 = inv(P1);

		cgv::mat4 iMV0 = inv(MV0);
		cgv::mat4 iMV1 = inv(MV1);

		cgv::mat4 MVP0 = P0 * MV0;
		cgv::mat4 MVP1 = P1 * MV1;

		cgv::mat4 iMVP0 = inv(MVP0);
		cgv::mat4 iMVP1 = inv(MVP1);

		float eye_value = view_test - 1.0f;
		float interpolation_param = 0.5f * view_test;

		cgv::mat4 Pmid = cgv::math::stereo_frustum_screen4(eye_value, eye_separation, y_extent_at_focus * aspect, y_extent_at_focus, parallax_zero_depth, z_near, z_far);
		cgv::mat4 MVmid = ctx.get_modelview_matrix();
		shader_calib.stereo_translate_modelview_matrix(eye_value, eye_separation, screen_width, MVmid);

		cgv::mat4 iPmid = inv(Pmid);
		cgv::mat4 iMVmid = inv(MVmid);

		cgv::mat4 MVPmid = Pmid * MVmid;
		cgv::mat4 iMVPmid = inv(MVPmid);

		cgv::vec3 col20(iMVP0.col(2));
		cgv::vec3 col30(iMVP0.col(3));

		cgv::vec3 col21(iMVP1.col(2));
		cgv::vec3 col31(iMVP1.col(3));

		cgv::vec3 col2int = cgv::math::lerp(col20, col21, interpolation_param);
		cgv::vec3 col3int = cgv::math::lerp(col30, col31, interpolation_param);

		cgv::vec3 col2mid(iMVPmid.col(2));
		cgv::vec3 col3mid(iMVPmid.col(3));

		cgv::mat4 Pint(0.0f);
		cgv::mat4 MVint(0.0f);
		cgv::mat4 MVPint(0.0f);
		cgv::mat4 iPint(0.0f);
		cgv::mat4 iMVint(0.0f);
		cgv::mat4 iMVPint(0.0f);
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

		cgv::mat4 D(0.0f);
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
		cgv::vec3 d2 = col2int - col2mid;
		cgv::vec3 d3 = col3int - col3mid;
		std::cout << d2 << std::endl;
		std::cout << d3 << std::endl;
		//std::cout << "Interpolated: " << std::endl;
		//std::cout << MVint << std::endl;*/
		//std::cout << "Difference Original vs Interpolated: " << std::endl;
		//std::cout << D << std::endl;
	}
	void create_gui() {
		add_decorator(name, "heading");

		add_member_control(this, "Interpolate View Matrix", shader_calib.interpolate_view_matrix, "check");

		add_member_control(this, "Eye Separation Factor", shader_calib.eye_separation_factor, "value_slider", "min=0;max=20;ticks=true");

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
