#include <plugins/vr_lab/vr_tool.h>
#include <cgv/defines/quote.h>
#include <cgv/base/node.h>
#include <cgv/render/context.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv/gui/event_handler.h>
#include <vr/vr_state.h>
#include <vr/vr_kit.h>
#include <vr/vr_driver.h>
#include <cg_vr/vr_events.h>
#include <vr_view_interactor.h>
#include <cgv/utils/dir.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/file.h>
#include <fstream>
#include <libs/plot/plot2d.h>

class vr_lab_test : 
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider,
	public vr::vr_tool
{
	cgv::render::sphere_render_style srs;
	cgv::render::cone_render_style rcrs;
	/// label index to show statistics
	uint32_t li_stats;
	/// background color of statistics label
	rgba stats_bgclr;
	/// labels to show help on controllers
	uint32_t li_help[2];

	// whether to show plot
	bool show_plot;
	// plot that can manage several 2d sub plots
	cgv::plot::plot2d plot;
	// persistent vector with plot data
	std::vector<vec4> P;
public:
	std::string get_type_name() const
	{
		return "vr_lab_test";
	}
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return false;
	}
	/// transform point with pose to lab coordinate system 
	vec3 compute_lab_draw_position(const float* pose, const vec3& p)
	{
		return mat34(3, 4, pose) * vec4(p, 1.0f);
	}
public:
	vr_lab_test() : cgv::base::node("vr lab test"), plot("trigonometry", 2)
	{
		li_help[0] = li_help[1] = -1;
		li_stats = -1;
		stats_bgclr = rgba(0.8f, 0.6f, 0.0f, 0.6f);

		show_plot = false;
		// compute vector of vec3 with x coordinates and function values of cos and sin
		unsigned i;
		for (i = 1; i < 50; ++i) {
			float x = 0.1f * i;
			P.push_back(vec4(x, cos(x), sin(x), cos(x) * cos(x)));
		}
		// create two sub plots and configure their colors
		unsigned p1 = plot.add_sub_plot("cos");
		unsigned p2 = plot.add_sub_plot("sin");
		unsigned p3 = plot.add_sub_plot("cos²");
		//plot.set_sub_plot_colors(p1, rgb(1.0f, 0.0f, 0.1f));	// will be set later to the attribute with index 2
		plot.set_sub_plot_colors(p2, rgb(0.1f, 0.0f, 1.0f));
		plot.set_sub_plot_colors(p3, rgb(0.0f, 1.0f, 0.1f));

		// attach sub plot attributes to previously created vector
		// CAREFUL: this creates references to P and P is not allowed to be deleted thereafter
		plot.set_sub_plot_attribute(p1, 0, &P[0][0], P.size(), sizeof(vec4));
		plot.set_sub_plot_attribute(p1, 1, &P[0][1], P.size(), sizeof(vec4));
		plot.set_sub_plot_attribute(p1, 2, &P[0][2], P.size(), sizeof(vec4));
		plot.set_sub_plot_attribute(p1, 3, &P[0][3], P.size(), sizeof(vec4));
		plot.set_sub_plot_attribute(p2, 0, &P[0][0], P.size(), sizeof(vec4));
		plot.set_sub_plot_attribute(p2, 1, &P[0][2], P.size(), sizeof(vec4));
		plot.set_sub_plot_attribute(p2, 2, &P[0][0], P.size(), sizeof(vec4));
		plot.set_sub_plot_attribute(p2, 3, &P[0][3], P.size(), sizeof(vec4));
		plot.set_sub_plot_attribute(p3, 0, &P[0][0], P.size(), sizeof(vec4));
		plot.set_sub_plot_attribute(p3, 1, &P[0][3], P.size(), sizeof(vec4));
		plot.set_sub_plot_attribute(p3, 2, &P[0][1], P.size(), sizeof(vec4));
		plot.set_sub_plot_attribute(p3, 3, &P[0][2], P.size(), sizeof(vec4));

		plot.legend_components = cgv::plot::LegendComponent(cgv::plot::LC_PRIMARY_COLOR + cgv::plot::LC_PRIMARY_OPACITY);
		plot.color_mapping[0] = 2;
		plot.color_scale_index[0] = cgv::media::CS_HUE;
		plot.opacity_mapping[0] = 3;

		plot.ref_sub_plot2d_config(0).set_color_indices(0);
		plot.ref_sub_plot2d_config(0).line_halo_color.color_idx = 0;

		// adjust domain, tick marks and extent in world space (of offline rendering process)
		plot.adjust_domain_to_data();
		plot.adjust_tick_marks();
		plot.adjust_extent_to_domain_aspect_ratio();

		plot.place_center(vec3(0, 0.5f * plot.get_extent()(1), -0.5f));
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &stats_bgclr && li_stats != -1)
			get_scene_ptr()->update_label_background_color(li_stats, stats_bgclr);

		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		cgv::render::ref_sphere_renderer(ctx, 1);
		cgv::render::ref_cone_renderer(ctx, 1);

		plot.set_view_ptr(find_view_as_node());
		// and init plot
		return plot.init(ctx);
	}
	void init_frame(cgv::render::context& ctx)
	{
		plot.init_frame(ctx);

		vr::vr_scene* scene_ptr = get_scene_ptr();
		if (!scene_ptr)
			return;
		// if not done before, create labels
		if (li_help[0] == -1) {
			li_stats = scene_ptr->add_label(
				"drawing index: 000000\n"
				"nr vertices:   000000\n"
				"nr edges:      000000", stats_bgclr);
			scene_ptr->fix_label_size(li_stats);
			scene_ptr->place_label(li_stats, vec3(0.0f, 0.01f, 0.0f), quat(vec3(1, 0, 0), -1.5f), vr::vr_scene::CS_TABLE);
			for (int ci = 0; ci < 2; ++ci) {
				li_help[ci] = scene_ptr->add_label("DPAD_Right .. next/new drawing\nDPAD_Left  .. prev drawing\nDPAD_Down  .. save drawing\nDPAD_Up .. toggle draw mode\nTPAD_Touch&Up/Dn .. change radius\nTPAD_Touch&Move .. change color\ncolorize (0.000)\nRGB(0.00,0.00,0.00)\nHLS(0.00,0.00,0.00)",
					rgba(ci == 0 ? 0.8f : 0.4f, 0.4f, ci == 1 ? 0.8f : 0.4f, 0.6f));
				scene_ptr->fix_label_size(li_help[ci]);
				scene_ptr->place_label(li_help[ci], vec3(ci == 1 ? -0.05f : 0.05f, 0.0f, 0.0f), quat(vec3(1, 0, 0), -1.5f),
					ci == 0 ? vr::vr_scene::CS_LEFT_CONTROLLER : vr::vr_scene::CS_RIGHT_CONTROLLER, ci == 1 ? vr::vr_scene::LA_RIGHT : vr::vr_scene::LA_LEFT, 0.2f);
				scene_ptr->hide_label(li_help[ci]);
			}
		}
		// always update visibility of visibility changing labels
		vr_view_interactor* vr_view_ptr = get_view_ptr();
		if (!vr_view_ptr)
			return;
		const vr::vr_kit_state* state_ptr = vr_view_ptr->get_current_vr_state();
		if (!state_ptr)
			return;
		vec3 view_dir = -reinterpret_cast<const vec3&>(state_ptr->hmd.pose[6]);
		vec3 view_pos = reinterpret_cast<const vec3&>(state_ptr->hmd.pose[9]);
		for (int ci = 0; ci < 2; ++ci) {
			vec3 controller_pos = reinterpret_cast<const vec3&>(state_ptr->controller[ci].pose[9]);
			float controller_depth = dot(view_dir, controller_pos - view_pos);
			float controller_dist = (view_pos + controller_depth * view_dir - controller_pos).length();
			if (view_dir.y() < -0.25f && controller_depth / controller_dist > 1.0f)
				scene_ptr->show_label(li_help[ci]);
			else
				scene_ptr->hide_label(li_help[ci]);
		}
		/*
		static const char* draw_mode_str[] = { "point","line","colorize" };
		for (int ci = 0; ci < 2; ++ci) {
			if (li_help[ci] == -1)
				continue;
			// update help text
			cgv::media::color<float, cgv::media::HLS> hls = draw_color[ci];
			std::stringstream ss;
			ss << "DPAD_Right .. next/new drawing\nDPAD_Left  .. prev drawing\nDPAD_Down  .. save drawing\nDPAD_Up .. toggle draw mode\nTPAD_Touch&Up/Dn .. change radius\nTPAD_Touch&Move .. change color\n"
				<< draw_mode_str[draw_mode[ci]] << " (" << std::setw(4) << std::setprecision(2) << draw_radius[ci] << ")"
				<< "\nRGB(" << std::setw(4) << std::setprecision(2) << draw_color[ci][0] << "," << std::setw(4) << std::setprecision(2) << draw_color[ci][1] << "," << std::setw(4) << std::setprecision(2) << draw_color[ci][2] << ")"
				<< "\nHLS(" << std::setw(4) << std::setprecision(2) << hls[0] << "," << std::setw(4) << std::setprecision(2) << hls[1] << "," << std::setw(4) << std::setprecision(2) << hls[2] << ")";
			ss.flush();
			scene_ptr->update_label_text(li_help[ci], ss.str());
			// update visibility of labels
			if (in_color_selection[ci])
				scene_ptr->show_label(li_help[ci]);
			else
				scene_ptr->hide_label(li_help[ci]);
		}*/
	}
	void clear(cgv::render::context& ctx)
	{
		cgv::render::ref_sphere_renderer(ctx, -1);
		cgv::render::ref_cone_renderer(ctx, -1);
	}
	void draw(cgv::render::context& ctx)
	{
		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(mat4(3, 4, &get_scene_ptr()->get_coordsystem(vr::vr_scene::CS_TABLE)(0, 0)));
		
		if (show_plot)
			plot.draw(ctx);

		ctx.mul_modelview_matrix(cgv::math::scale4<float>(vec3(0.25f)));
		// draw tool in case it is active and we have access to state 
		vr::vr_kit* kit_ptr = get_kit_ptr();
		vr_view_interactor* vr_view_ptr = get_view_ptr();
		if (tool_is_active && kit_ptr && vr_view_ptr) {
			const vr::vr_kit_state* state_ptr = vr_view_ptr->get_current_vr_state();
			if (state_ptr) {

				auto& sr = cgv::render::ref_sphere_renderer(ctx);
				sr.set_render_style(srs);
				auto& rcr = cgv::render::ref_cone_renderer(ctx);
				rcr.set_render_style(rcrs);
				// draw spheres that represent the pen
				std::vector<vec3> P;
				std::vector<float> R;
				std::vector<rgb> C;
				P.push_back(vec3(0.0f));
				R.push_back(0.3f);
				C.push_back(rgb(0.5f, 0.8f, 0.3f));
				box3 B(vec3(-1.0f), vec3(1.0f));
				for (int ci = 0; ci < 8; ++ci) {
					P.push_back(B.get_corner(ci));
					R.push_back(0.1f);
					vec3 col = 0.5f * (B.get_corner(ci) - B.get_min_pnt());
					C.push_back(reinterpret_cast<const rgb&>(col));
				}
				sr.set_position_array(ctx, P);
				sr.set_radius_array(ctx, R);
				sr.set_color_array(ctx, C);
				sr.render(ctx, 0, (GLsizei)P.size());
			}
		}
		ctx.pop_modelview_matrix();
	}
	void stream_help(std::ostream& os)
	{
		os << "vr_lab_test: select draw <M>ode, press vr pad or trigger to draw, grip to change color" << std::endl;
	}
	bool handle(cgv::gui::event& e)
	{
		if ((e.get_flags() & cgv::gui::EF_VR) == 0) {
			if (e.get_kind() != cgv::gui::EID_KEY)
				return false;
			auto& ke = static_cast<cgv::gui::key_event&>(e);
			if (ke.get_action() == cgv::gui::KA_RELEASE)
				return false;
			switch (ke.get_key()) {
			case 'P': 
				show_plot = !show_plot;
				on_set(&show_plot);
				return true;
			}
			return false;
		}
		if (e.get_kind() == cgv::gui::EID_KEY) {
			auto& vrke = static_cast<cgv::gui::vr_key_event&>(e);
			int ci = vrke.get_controller_index();
			switch (vrke.get_key()) {
			case vr::VR_DPAD_RIGHT:
				if (vrke.get_action() == cgv::gui::KA_PRESS) {
				}
				return true;
			case vr::VR_DPAD_LEFT:
				if (vrke.get_action() == cgv::gui::KA_PRESS) {
				}
				return true;
			case vr::VR_GRIP:
				return false;
			case vr::VR_INPUT0:
				return true;
			}
			return false;
		}
		else if (e.get_kind() == cgv::gui::EID_THROTTLE) {
			auto& te = static_cast<cgv::gui::vr_throttle_event&>(e);
			int ci = te.get_controller_index();
			float v = te.get_value();
			return true;
		}
		else if (e.get_kind() == cgv::gui::EID_STICK) {
			auto& se = static_cast<cgv::gui::stick_event&>(e);
			int ci = se.get_controller_index();
			switch (se.get_action()) {
			case cgv::gui::SA_TOUCH :
				return true;
			case cgv::gui::SA_RELEASE:
				return true;
			case cgv::gui::SA_MOVE:
				return true;
			}
			return false;
		}
		else if (e.get_kind() == cgv::gui::EID_POSE) {
			auto& pe = static_cast<cgv::gui::vr_pose_event&>(e);
			int ci = pe.get_trackable_index();
			if (ci >= 0 && ci < 2) {
			}
		}
		return false;
	}
	void create_gui()
	{
		add_decorator("vr_lab_test", "heading");
		add_member_control(this, "stats_bgclr", stats_bgclr);
		if (begin_tree_node("rendering", srs.material)) {
			align("\a");
			if (begin_tree_node("spheres", srs)) {
				align("\a");
				add_gui("spheres", srs);
				align("\b");
				end_tree_node(srs);
			}
			if (begin_tree_node("cones", rcrs)) {
				align("\a");
				add_gui("cones", rcrs);
				align("\b");
				end_tree_node(rcrs);
			}
			align("\b");
			end_tree_node(srs.material);
		}
		add_member_control(this, "show_plot", show_plot, "check");
		if (begin_tree_node("plot", plot)) {
			align("\a");
			plot.create_gui(this, *this);
			align("\b");
			end_tree_node(plot);
		}
	}
};


#include <cgv/base/register.h>
cgv::base::object_registration<vr_lab_test> vr_draw_reg("vr_lab_test");
#ifdef CGV_FORCE_STATIC
cgv::base::registration_order_definition ro_def("vr_view_interactor;vr_emulator;vr_scene;vr_lab_test");
#endif
