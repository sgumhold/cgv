#include <plugins/vr_lab/vr_tool.h>
#include <cgv/defines/quote.h>
#include <cgv/base/node.h>
#include <cgv/render/context.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/rounded_cone_renderer.h>
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

std::string to_hex(uint8_t v)
{
	static const char hex_digits[] = "0123456789abcdef";
	char res[2] = { hex_digits[v / 16], hex_digits[v & 15] };
	return std::string(res,2);
}

class vr_lab_test : 
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public vr::vr_tool
{
	cgv::render::sphere_render_style srs;
	cgv::render::rounded_cone_render_style rcrs;
	/// label index to show statistics
	uint32_t li_stats;
	/// labels to show help on controllers
	uint32_t li_help[2];
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
	vr_lab_test() : cgv::base::node("vr lab test")
	{
		li_help[0] = li_help[1] = -1;
		li_stats = -1;
	}
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		cgv::render::ref_sphere_renderer(ctx, 1);
		cgv::render::ref_rounded_cone_renderer(ctx, 1);
		return true;
	}
	struct tud_color
	{
		std::string function;
		std::string name;
		ivec4 cymk_color;
		rgb8 rgb_color;
	};
	struct kostenstelle
	{
		int nummer;
		std::string name;
		rgb8 color;
	};
	std::vector<std::pair<int,std::vector<kostenstelle>>> kostenstellen;
	std::vector<tud_color> tud_colors;
	std::vector<int> tud_color_lis;
	void read_kostenstellen()
	{
		std::string content;
		cgv::utils::file::read("D:/admin/räume/RE-FX/2021_03_04_Raumliste.txt", content, true);
		std::vector<cgv::utils::line> lines;
		cgv::utils::split_to_lines(content, lines);
		int ii = -1;
		for (unsigned li = 0; li < lines.size(); ++li) {
			std::vector<cgv::utils::token> tokens;
			cgv::utils::split_to_tokens(lines[li], tokens, "\t", false, "\"'", "\"'", "");
			if (tokens.size() != 3) {
				if (tokens.size() == 1) {
					int ci;
					if (cgv::utils::is_integer(tokens[0].begin, tokens[0].end, ci)) {
						++ii;
						kostenstellen.resize(ii + 1);
						kostenstellen.back().first = ci;
					}
				}
				continue;
			}
			kostenstelle kst;
			kst.name = cgv::utils::to_string(tokens[2]);
			if (!cgv::utils::is_integer(tokens[0].begin, tokens[0].end, kst.nummer))
				continue;
			kostenstellen[ii].second.push_back(kst);
		}
		std::cout << "read " << kostenstellen.size() << " Kostenstellen" << std::endl;
	}
	void read_tud_colors()
	{
		std::string content;
		cgv::utils::file::read("D:/admin/räume/RE-FX/TUD CD Colors.txt", content, true);
		std::vector<cgv::utils::line> lines;
		cgv::utils::split_to_lines(content, lines);
		for (unsigned li = 1; li < lines.size(); ++li) {
			std::vector<cgv::utils::token> tokens;
			cgv::utils::split_to_tokens(lines[li], tokens, ",", false, "\"'", "\"'", "");
			if (tokens.size() != 17)
				continue;
			tud_color tc;
			tc.function = cgv::utils::to_string(tokens[0]);
			tc.name = cgv::utils::to_string(tokens[2]);
			int v[7];
			for (int i = 0; i < 7; ++i) {
				if (!cgv::utils::is_integer(tokens[2 * i + 4].begin, tokens[2 * i + 4].end, v[i]))
					continue;
			}
			
			tc.cymk_color = ivec4(v[0], v[1], v[2], v[3]);
			tc.rgb_color = rgb8((uint8_t)v[4], (uint8_t)v[5], (uint8_t)v[6]);
			tud_colors.push_back(tc);
		}
		std::ofstream os("D:/admin/räume/RE-FX/hex_colors.txt");
		for (auto tc : tud_colors) {
			os << tc.function << ", " << tc.name << ": " << 
				(int)tc.rgb_color[0] << "/" << (int)tc.rgb_color[1] << "/" << (int)tc.rgb_color[2] <<
				" = #" << to_hex(tc.rgb_color[0]) << to_hex(tc.rgb_color[1]) << to_hex(tc.rgb_color[2]) << std::endl;
		}
		std::cout << "read " << tud_colors.size() << " tud colors" << std::endl;
	}
	void init_frame(cgv::render::context& ctx)
	{
		vr::vr_scene* scene_ptr = get_scene_ptr();
		if (!scene_ptr)
			return;
		if (tud_colors.empty()) {
			read_tud_colors();
			read_kostenstellen();
			float h = 3.0f;
			int gi = 0;
			for (const auto& kst_grp : kostenstellen) {
				rgb C_rgb = tud_colors[kst_grp.first].rgb_color;
				cgv::media::color<float, cgv::media::HLS> C_hls = C_rgb;
				float s = 1.0f/(kst_grp.second.size()+1);
				int n = int(kst_grp.second.size());
				int i = 0;
				for (auto kst : kst_grp.second) {
					rgb C = C_hls;
					kst.color = C;
					std::cout << kst.nummer << ": #" 
						<< to_hex(kst.color[0]) << to_hex(kst.color[1]) << to_hex(kst.color[2]) << " ("
						<< int(kst.color[0]) << "," << int(kst.color[1]) << "," << int(kst.color[2]) << 
						")" << std::endl;
					int li = scene_ptr->add_label(kst.name, C);
					scene_ptr->fix_label_size(li);
					scene_ptr->place_label(li, vec3(0.0f, h, 0.0f), quat(1.0f, vec3(0.0f)), 
										vr::vr_scene::CS_TABLE, vr::vr_scene::LA_CENTER);
					h -= 0.05f;
					++i;
					if (gi < 2) {
						C_hls[1] -= 0.3f * s;
						C_hls[2] -= 0.6f * s;
					}
					else {
						if (gi < 6)
							C_hls[2] -= (n-i)*0.2f * s;
						C_hls[1] += (0.25f) * s;
					}
				}
				++gi;
			}

//			float h = 0.1f * tud_colors.size() + 0.1f;
//			int i = 0;
//			for (const auto& tc : tud_colors) {
//				int li = scene_ptr->add_label(tc.function + "\n" + cgv::utils::to_string(i++)+" "+tc.name, tc.rgb_color);
//				tud_color_lis.push_back(li);
//				scene_ptr->fix_label_size(li);
//				scene_ptr->place_label(li, vec3(0.0f, h, 0.0f), quat(1.0f, vec3(0.0f)), 
//					vr::vr_scene::CS_TABLE, vr::vr_scene::LA_CENTER);
//				h -= 0.1f;
//			}
		}
		// if not done before, create labels
		if (li_help[0] == -1) {
			li_stats = scene_ptr->add_label(
				"drawing index: 000000\n"
				"nr vertices:   000000\n"
				"nr edges:      000000", rgba(0.8f, 0.6f, 0.0f, 0.6f));
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
		cgv::render::ref_rounded_cone_renderer(ctx, -1);
	}
	void draw(cgv::render::context& ctx)
	{
		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(
			cgv::math::translate4<float>(vec3(0, 1.5f, 0)) *
			cgv::math::scale4<float>(vec3(0.25f)));
		// draw tool in case it is active and we have access to state 
		vr::vr_kit* kit_ptr = get_kit_ptr();
		vr_view_interactor* vr_view_ptr = get_view_ptr();
		if (!(tool_is_active && kit_ptr && vr_view_ptr))
			return;
		const vr::vr_kit_state* state_ptr = vr_view_ptr->get_current_vr_state();
		if (!state_ptr)
			return;

		auto& sr = cgv::render::ref_sphere_renderer(ctx);
		sr.set_render_style(srs);
		auto& rcr = cgv::render::ref_rounded_cone_renderer(ctx);
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
			vec3 col = 0.5f*(B.get_corner(ci) - B.get_min_pnt());
			C.push_back(reinterpret_cast<const rgb&>(col));
		}
		sr.set_position_array(ctx, P);
		sr.set_radius_array(ctx, R);
		sr.set_color_array(ctx, C);
		sr.render(ctx, 0, (GLsizei)P.size());
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
			case 'M': 
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
	}
};


#include <cgv/base/register.h>
cgv::base::object_registration<vr_lab_test> vr_draw_reg("vr_lab_test");
#ifdef CGV_FORCE_STATIC
cgv::base::registration_order_definition ro_def("vr_view_interactor;vr_emulator;vr_scene;vr_lab_test");
#endif
