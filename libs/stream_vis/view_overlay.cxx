#include "view_overlay.h"
#include <cgv/gui/theme_info.h>
#include <cgv/math/ftransform.h>
#include <cgv/gui/key_event.h>

namespace stream_vis {

	void view_overlay::on_visibility_change()
	{
		overlay::on_visibility_change();
		if (handler) {
			for (auto& pl : plots)
				handler->handle_plot_visibility_update(pl.first, is_visible());
		}
	}

	void view_overlay::set_update_handler(view_update_handler* _handler)
	{
		handler = _handler;
	}

	view_overlay::view_overlay()
	{
		set_alignment(AO_START, AO_START);
		set_stretch(SO_BOTH);
		set_margin(cgv::ivec2(-3));
		set_size(cgv::ivec2(600u, 600u));
	}
	void view_overlay::add_plot(int pi, cgv::plot::plot_base* plot_ptr)
	{
		plots.push_back({ pi, plot_ptr });
	}
	void view_overlay::init_frame(cgv::render::context& ctx)
	{
		if (ensure_layout(ctx))
			update_views();
	}
	void view_overlay::draw(cgv::render::context& ctx)
	{
		// TODO: Due to recent changes this check should not be necessary anymore. Test this and remove the guard if applicable.
		if (!is_visible())
			return;
		cgv::ivec4 vp;
		(cgv::ivec2&)vp = get_rectangle().position;
		((cgv::ivec2*)&vp)[1] = get_rectangle().size;
		float aspect = (float)vp[2]/ vp[3];
		ctx.push_window_transformation_array();
		ctx.push_projection_matrix();
		ctx.push_modelview_matrix();

			ctx.set_viewport(vp);

			ctx.set_projection_matrix(cgv::math::identity4<float>());
			ctx.set_modelview_matrix(cgv::math::identity4<float>());
			glDepthMask(GL_FALSE);
			ctx.ref_default_shader_program().enable(ctx);
			auto& ti = cgv::gui::theme_info::instance();
			ctx.set_color(mouse_is_on_overlay ? ti.control() : ti.group());
			ctx.tesselate_unit_square();
			ctx.ref_default_shader_program().disable(ctx);
			glDepthMask(GL_TRUE);

			set_modelview_projection(ctx);

			for (auto p : plots)
				p.second->draw(ctx);

		ctx.pop_modelview_matrix();
		ctx.pop_projection_matrix();
		ctx.pop_window_transformation_array();
	}
	bool view_overlay::handle_event(cgv::gui::event& e)
	{
		if (e.get_kind() == cgv::gui::EID_KEY) {
			auto& ke = reinterpret_cast<cgv::gui::key_event&>(e);
			if (ke.get_action() == cgv::gui::KA_RELEASE)
				return false;
			if (!mouse_is_on_overlay)
				return false;
			if (current_pi != -1) {
				if (ke.get_key() >= (int)'1' && ke.get_key() <= (int)'9') {
					int si = ke.get_key() - (int)'1';
					auto& pl = plots[current_pi].second;
					if (si < pl->get_nr_sub_plots()) {
						pl->ref_sub_plot_config(si).show_plot = !pl->ref_sub_plot_config(si).show_plot;
						handler->handle_subplot_visibility_update(plots[current_pi].first, si, pl->ref_sub_plot_config(si).show_plot);
//						std::cout << "plot " << plots[current_pi].first << "[" << si << "] : " << (pl->ref_sub_plot_config(si).show_plot ? "show" : "hide") << std::endl;
						return true;
					}
				}
				if (ke.get_key() >= (int)'X' && ke.get_key() < (int)'X' + plots[current_pi].second->get_dim()) {
					int ai = ke.get_key() - (int)'X';
					auto& pl = plots[current_pi].second;
					bool log_scale = !pl->get_domain_config_ptr()->axis_configs[ai].get_log_scale();
					pl->get_domain_config_ptr()->axis_configs[ai].set_log_scale(log_scale);
					handler->handle_plot_axis_update(plots[current_pi].first, ai, log_scale);
				}
			}
			switch (ke.get_key()) {
			case cgv::gui::KEY_Space:
				if (ke.get_modifiers() == cgv::gui::EM_CTRL) {
					toggle_default_view(true);
					return true;
				}
				else if (ke.get_modifiers() == cgv::gui::EM_SHIFT) {
					toggle_default_view(false);
					return true;
				}
				break;
			}
			return false;
		}
		else if (e.get_kind() == cgv::gui::EID_MOUSE) {
			auto& me = reinterpret_cast<cgv::gui::mouse_event&>(e);
			switch (me.get_action()) {
			case cgv::gui::MA_ENTER:
				mouse_is_on_overlay = true;
				post_redraw();
				return true;
			case cgv::gui::MA_LEAVE:
				mouse_is_on_overlay = false;
				post_redraw();
				return true;
			case cgv::gui::MA_PRESS:
			case cgv::gui::MA_RELEASE:
			case cgv::gui::MA_DRAG:
			case cgv::gui::MA_MOVE:
			case cgv::gui::MA_WHEEL:
				return handle_mouse_event(me);
			}
		}
		return false;
	}
}