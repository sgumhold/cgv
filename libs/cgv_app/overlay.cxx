#include "overlay.h"

namespace cgv {
namespace app {

overlay::overlay() {
	last_viewport_size = ivec2(-1);
	horizontal_alignment = AO_START;
	vertical_alignment = AO_START;
	stretch = SO_NONE;

	margin = ivec2(0);
	container.position = ivec2(0);
	container.size = ivec2(0);

	last_size = ivec2(-1);

	show = true;
	block_events = true;
	draw_in_finish_frame = false;
}

void overlay::on_visibility_change() 
{
	update_member(&show);
	post_redraw();
}

void overlay::on_layout_change() {

	update_overlay_layout();
	post_redraw();
}

void overlay::on_set(void* member_ptr) {

	handle_member_change(cgv::utils::pointer_test(member_ptr));
	update_member(member_ptr);
	post_redraw();
}

void overlay::set_overlay_alignment(AlignmentOption horizontal, AlignmentOption vertical, vec2 percentual_offset)
{
	horizontal_alignment = horizontal;
	vertical_alignment = vertical;
	for(int i = 0; i < 2; ++i)
		if(percentual_offset[i] != -1.0f)
			this->percentual_offset[i] = percentual_offset[i];
	update_overlay_layout();
}

/// sets the stretch option
void overlay::set_overlay_stretch(StretchOption stretch, vec2 percentual_size)
{
	this->stretch = stretch;
	for(int i = 0; i < 2; ++i)
		if(percentual_size[i] != -1.0f)
			this->percentual_size[i] = percentual_size[i];
	update_overlay_layout();
}

void overlay::update_overlay_layout() {

	ivec2 pos = margin;
	ivec2 size = container.size;

	ivec2 max_size = last_viewport_size - 2 * margin;

	switch(stretch) {
	case SO_HORIZONTAL:
		size.x() = max_size.x();
		break;
	case SO_VERTICAL:
		size.y() = max_size.y();
		break;
	case SO_BOTH:
		size = max_size;
		break;
	case SO_PERCENTUAL:
		size = percentual_size * max_size;
		break;
	case SO_NONE:
	default:
		break;
	}

	switch(horizontal_alignment) {
	case AO_CENTER:
		pos.x() = (last_viewport_size.x() - size.x()) / 2;
		break;
	case AO_END:
		pos.x() = last_viewport_size.x() - size.x() - margin.x();
		break;
	case AO_PERCENTUAL:
		pos.x() = int32_t(margin.x() + percentual_offset.x() * max_size.x());
		break;
	case AO_START:
	case AO_FREE:
	default:
		pos.x() = margin.x();
		break;
	}

	switch(vertical_alignment) {
	case AO_CENTER:
		pos.y() = (last_viewport_size.y() - size.y()) / 2;
		break;
	case AO_END:
		pos.y() = last_viewport_size.y() - size.y() - margin.y();
		break;
	case AO_PERCENTUAL:
		pos.y() = int32_t(margin.y() + percentual_offset.y() * max_size.y());
		break;
	case AO_START:
	case AO_FREE:
	default:
		pos.y() = margin.y();
		break;
	}

	container.position = pos;
	container.size = size;
}

bool overlay::ensure_viewport(cgv::render::context& ctx) {

	ivec2 viewport_size(ctx.get_width(), ctx.get_height());
	if(last_viewport_size != viewport_size) {
		last_viewport_size = viewport_size;
		update_overlay_layout();
		return true;
	}
	return false;
}

bool overlay::is_hit(const ivec2& mouse_pos) const {

	ivec2 test_pos = cgv::g2d::get_transformed_mouse_pos(mouse_pos, last_viewport_size);
	return container.contains(test_pos);
};

bool overlay::begin_overlay_gui() {
	bool node_is_open = begin_tree_node_void(gui_options.heading != "" ? gui_options.heading : name, &name, -1, false, "level=2;options='w=160';align=''");

	cgv::signal::connect_copy(
		add_control("Show", show, "toggle", "w=40")->value_change,
		cgv::signal::rebind(this, &overlay::on_layout_change)
	);

	if(node_is_open) {
		align("\a");
		return true;
	}

	return false;
}

void overlay::end_overlay_gui() {

	align("\b");
	end_tree_node(name);
}

void overlay::create_layout_gui() {

	if(!gui_options.show_layout_options)
		return;

	if(!gui_options.allow_alignment && !gui_options.allow_stretch && !gui_options.allow_margin)
		return;

	if(begin_tree_node("Layout", gui_options, false)) {
		align("\a");

		if(gui_options.allow_alignment) {
			add_decorator("Alignment", "heading", "level=4", "%y-=8\n");

			add_decorator("Horizontal", "heading", "level=4;font_style=regular;w=94", " ");
			add_decorator("Vertical", "heading", "level=4;font_style=regular;w=94");

			cgv::signal::connect_copy(
				add_control("", horizontal_alignment, "dropdown", "enums='Free,Left,Center,Right';w=94", " ")->value_change,
				cgv::signal::rebind(this, &overlay::on_layout_change)
			);
			cgv::signal::connect_copy(
				add_control("", vertical_alignment, "dropdown", "enums='Free=0,Top=3,Center=2,Bottom=1';w=94")->value_change,
				cgv::signal::rebind(this, &overlay::on_layout_change)
			);
		}

		if(gui_options.allow_stretch) {
			cgv::signal::connect_copy(
				add_control("Stretch", stretch, "dropdown", "enums='None,Horizontal,Vertical,Both'")->value_change,
				cgv::signal::rebind(this, &overlay::on_layout_change)
			);
		}

		if(gui_options.allow_margin) {
			cgv::signal::connect_copy(
				add_control("Margin", margin[0], "value", "w=94;min=-10000;max=10000;step=1;", " ")->value_change,
				cgv::signal::rebind(this, &overlay::on_layout_change)
			);
			cgv::signal::connect_copy(
				add_control("", margin[1], "value", "w=94;min=-10000;max=10000;step=1;")->value_change,
				cgv::signal::rebind(this, &overlay::on_layout_change)
			);
		}

		align("\b");
		end_tree_node(gui_options);
	}
}

void overlay::create_gui() {

	if(gui_options.create_default_tree_node) {
		if(begin_overlay_gui()) {
			create_layout_gui();
			create_gui_impl();
			end_overlay_gui();
		}
	} else {
		create_layout_gui();
		create_gui_impl();
	}
}

}
}
