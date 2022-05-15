#include "overlay.h"

namespace cgv {
namespace glutil {

overlay::overlay() {
	last_viewport_size = ivec2(-1);
	horizontal_alignment = AO_START;
	vertical_alignment = AO_START;
	stretch = SO_NONE;

	margin = ivec2(0);
	container.set_pos(ivec2(0));
	container.set_size(ivec2(0));

	last_size = ivec2(-1);

	show = true;
	block_events = true;
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

overlay::ivec2 overlay::get_transformed_mouse_pos(ivec2 mouse_pos) {

	mouse_pos.y() = last_viewport_size.y() - mouse_pos.y();
	return mouse_pos;
}

overlay::ivec2 overlay::get_local_mouse_pos(ivec2 mouse_pos) {

	return get_transformed_mouse_pos(mouse_pos) - container.pos();
}

void overlay::set_overlay_alignment(AlignmentOption horizontal, AlignmentOption vertical, vec2 _percentual_offset)
{
	horizontal_alignment = horizontal;
	vertical_alignment = vertical;
	for (int i = 0; i < 2; ++i)
		if (_percentual_offset[i] != -1.0f)
			percentual_offset[i] = _percentual_offset[i];
	update_overlay_layout();
}

/// sets the stretch option
void overlay::set_overlay_stretch(StretchOption _stretch, vec2 _percentual_size)
{
	stretch = _stretch;
	for (int i = 0; i < 2; ++i)
		if (_percentual_size[i] != -1.0f)
			percentual_size[i] = _percentual_size[i];
	update_overlay_layout();
}

void overlay::update_overlay_layout() {

	ivec2 pos = margin;
	ivec2 size = this->size;

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
		pos.x() = margin.x() + percentual_offset.x() * max_size.x();
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
		pos.y() = margin.y() + percentual_offset.y() * max_size.y();
		break;
	case AO_START:
	case AO_FREE:
	default:
		pos.y() = margin.y();
		break;
	}

	container.set_pos(pos);
	container.set_size(size);
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

bool overlay::is_hit(const ivec2& mouse_pos) {

	ivec2 test_pos = get_transformed_mouse_pos(mouse_pos);
	return container.is_inside(test_pos);
};

void overlay::create_overlay_gui() {

	if(gui_options.show_heading)
		add_decorator(name, "heading", "level=2");

	bool use_tree_node = gui_options.show_layout_options;

	if(!gui_options.allow_alignment && !gui_options.allow_stretch && !gui_options.allow_margin)
		use_tree_node = false;


	if(use_tree_node) {
		bool node_is_open = begin_tree_node_void("Overlay", &gui_options, -1, false, "level=2;options='w=160';align=''");
		
		cgv::signal::connect_copy(
			add_control("Show", show, "toggle", "w=40")->value_change,
			cgv::signal::rebind(this, &overlay::on_layout_change)
		);

		if(node_is_open) {
			align("\a");

			if(gui_options.allow_alignment) {
				cgv::signal::connect_copy(
					add_control("Horizontal Alignment", horizontal_alignment, "dropdown", "enums='Free,Left,Center,Right'")->value_change,
					cgv::signal::rebind(this, &overlay::on_layout_change)
				);
				cgv::signal::connect_copy(
					add_control("Vertical Alignment", vertical_alignment, "dropdown", "enums='Free=0,Top=3,Center=2,Bottom=1'")->value_change,
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
			end_tree_node(horizontal_alignment);
		}
	} else {
		cgv::signal::connect_copy(
			add_control("Show", show, "toggle")->value_change,
			cgv::signal::rebind(this, &overlay::on_layout_change)
		);
	}
}

}
}
