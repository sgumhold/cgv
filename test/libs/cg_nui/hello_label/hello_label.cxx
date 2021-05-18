#include <cgv/base/node.h>
#include <cgv/render/context.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cg_nui/label_manager.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/trigger.h>
#include <sstream>
#include <iomanip>

class hello_label :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::gui::event_handler,
	public cgv::gui::provider
{
protected:
	// use label manager to organize labels in texture
	cgv::nui::label_manager lm;

	// store label placements for rectangle renderer
	std::vector<vec3> label_positions;
	std::vector<quat> label_orientations;
	std::vector<vec2> label_extents;
	std::vector<vec4> label_texture_ranges;

	// scale used to translate texture coordinate extents to world extents
	float label_scale;

	// indices of labels that will be changed during execution
	uint32_t time_label_index;
	uint32_t mouse_position_label_index;

	// whether dynamic labels need to be updated
	bool dynamic_labels_out_of_date;

	// remember time when app started
	double start_time;

	// for rectangle renderer a plane_render_style is needed
	cgv::render::rectangle_render_style rrs;

	// keep pointer to other objects
	cgv::render::view* view_ptr;
	
	// after packing has changed, we need to update extends (actually only from size changing labels) and texture ranges again
	void update_texture_ranges_and_extents()
	{
		label_extents.clear();
		label_texture_ranges.clear();
		for (uint32_t li = 0; li < lm.get_nr_labels(); ++li) {
			const auto& l = lm.get_label(li);
			label_extents.push_back(vec2(label_scale * l.get_width(), label_scale * l.get_height()));
			label_texture_ranges.push_back(lm.get_texcoord_range(li));
		}
	}
	/// construct labels in label manager together with rectangle geometry
	void construct_hello_label(cgv::render::context& ctx)
	{
		// configure label manager
		cgv::media::font::font_ptr f = cgv::media::font::find_font("Arial");
		lm.set_font_face(f->get_font_face(cgv::media::font::FFA_BOLD));
		lm.set_font_size(36);
		lm.set_text_color(rgba(0, 0, 0, 1));
		
		// add labels to label manager
		lm.add_label("Hello", rgba(1, 0, 0, 1));
		lm.add_label("Label", rgba(0, 1, 0, 1));
		lm.add_label("!", rgba(0, 0, 1, 1));
		time_label_index = lm.add_label("00:00:00,00", rgba(0.75f, 0.75f, 0.5f, 1));
		mouse_position_label_index = lm.add_label("0000,0000", rgba(0.75f, 0.75f, 0.75f, 1));
		// fix size of dynamic labels to size computed from initial text
		lm.fix_label_size(time_label_index);
		lm.fix_label_size(mouse_position_label_index);
		// pack labels to compute texture ranges
		lm.pack_labels();
		update_texture_ranges_and_extents();
		// place labels in world space
		for (uint32_t li = 0; li < lm.get_nr_labels(); ++li) {
			const auto& l = lm.get_label(li);
			label_positions.push_back(vec3(0.4f * (li - 2.0f), 1, 0.2f * std::abs(li - 2.0f)));
			// rotate labels around y-axis
			label_orientations.push_back(quat(vec3(0, 1, 0), -0.2f * (li - 2.0f) ));
		}
	}
public:
	hello_label() : cgv::base::node("hello_label")
	{
		label_scale = 0.002f;
		dynamic_labels_out_of_date = true;
		view_ptr = 0;
		start_time = cgv::gui::get_animation_trigger().get_current_time();
		rrs.illumination_mode = cgv::render::IM_OFF;
		connect(cgv::gui::get_animation_trigger().shoot, this, &hello_label::timer_event);
	}
	void timer_event(double t, double dt)
	{
		double time_since_start = t - start_time;
		int h = int(floor(time_since_start / 3600));
		time_since_start -= 3600 * h;
		int m = int(floor(time_since_start / 60));
		time_since_start -= 60 * m;
		int s = int(floor(time_since_start));
		time_since_start -= s;
		int cs = int(floor(100*time_since_start));
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(2) << h << ':'
		   << std::setfill('0') << std::setw(2) << m << ':'
		   << std::setfill('0') << std::setw(2) << s << ','
		   << std::setfill('0') << std::setw(2) << cs;
		ss.flush();
		lm.update_label_text(time_label_index, ss.str());
		dynamic_labels_out_of_date = true;
		post_redraw();
	}
	std::string get_type_name() 
	{ 
		return "vr_draw"; 
	}
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		construct_hello_label(ctx);
		view_ptr = find_view_as_node();
		cgv::render::ref_rectangle_renderer(ctx, 1);
		return true;
	}
	void clear(cgv::render::context& ctx)
	{
		cgv::render::ref_rectangle_renderer(ctx, -1);
	}
	void init_frame(cgv::render::context& ctx)
	{
		if (dynamic_labels_out_of_date) {
			bool repack = lm.is_packing_outofdate();
			lm.ensure_texture_uptodate(ctx);
			if (repack)
				update_texture_ranges_and_extents();
			dynamic_labels_out_of_date = false;
		}
	}
	void draw(cgv::render::context& ctx)
	{
		auto& rr = cgv::render::ref_rectangle_renderer(ctx);
		rr.set_position_array(ctx, label_positions);
		rr.set_rotation_array(ctx, label_orientations);
		rr.set_extent_array(ctx, label_extents);
		rr.set_texcoord_array(ctx, label_texture_ranges);
		if (rr.validate_and_enable(ctx)) {
			lm.get_texture()->enable(ctx);
			rr.draw(ctx, 0, lm.get_nr_labels());
			lm.get_texture()->disable(ctx);
			rr.disable(ctx);
		}
	}
	void stream_help(std::ostream& os)
	{
		os << "lables: just tracks mouse position" << std::endl;
	}
	bool handle(cgv::gui::event& e)
	{
		if (e.get_kind() != cgv::gui::EID_MOUSE)
			return false;
		auto& me = reinterpret_cast<cgv::gui::mouse_event&>(e);
		std::stringstream ss;
		ss << std::setfill(' ') << std::setw(4) << me.get_x() << ',' << me.get_y();
		ss.flush();
		lm.update_label_text(mouse_position_label_index, ss.str());
		dynamic_labels_out_of_date = true;
		post_redraw();
		return false;
	}
	void create_gui()
	{
		add_decorator("hello_label", "heading");
		if (begin_tree_node("rectangle rendering", rrs)) {
			align("\a");
			add_gui("rectangles", rrs);
			align("\b");
			end_tree_node(rrs);
		}
	}
};

#include <cgv/base/register.h>
cgv::base::object_registration<hello_label> hello_label_reg("hello_label");
