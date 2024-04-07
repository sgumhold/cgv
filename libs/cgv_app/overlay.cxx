#include "overlay.h"

namespace cgv {
namespace app {

void overlay::on_visibility_change() 
{
	update_member(&(cgv::render::drawable::active));
	post_redraw();
}

void overlay::on_layout_change() {

	update_layout();
	post_redraw();
}

void overlay::on_set(void* member_ptr) {

	handle_member_change(cgv::utils::pointer_test(member_ptr));
	update_member(member_ptr);
	post_redraw();
}

ivec2 overlay::get_local_mouse_pos(ivec2 mouse_pos) const {
	return cgv::g2d::get_transformed_mouse_pos(mouse_pos, last_viewport_size_) - container_.position;
}

void overlay::set_alignment(AlignmentOption horizontal, AlignmentOption vertical, vec2 percentual_offset)
{
	horizontal_alignment_ = horizontal;
	vertical_alignment_ = vertical;
	
	for(int i = 0; i < 2; ++i) {
		if(percentual_offset[i] != -1.0f)
			percentual_offset_[i] = percentual_offset[i];
	}

	update_layout();
}

void overlay::set_stretch(StretchOption stretch, vec2 percentual_size)
{
	stretch_ = stretch;

	for(int i = 0; i < 2; ++i) {
		if(percentual_size[i] != -1.0f)
			percentual_size_[i] = percentual_size[i];
	}

	update_layout();
}

void overlay::set_margin(const ivec2& margin) {
	margin_ = margin;
	update_layout();
}

void overlay::set_size(const ivec2& size) {
	container_.size = size;
	update_layout();
}

void overlay::set_visibility(bool visible) {
	if(visible)
		show();
	else
		hide();
	on_visibility_change();
}

void overlay::toggle_visibility() {
	set_visibility(!is_visible());
}

bool overlay::ensure_viewport(cgv::render::context& ctx) {

	ivec2 viewport_size(ctx.get_width(), ctx.get_height());
	if(last_viewport_size_ != viewport_size) {
		last_viewport_size_ = viewport_size;
		update_layout();
		return true;
	}
	return false;
}

bool overlay::ensure_layout(cgv::render::context& ctx) {
	bool ret = ensure_viewport(ctx);

	if(last_size_ != container_.size) {
		last_size_ = container_.size;
		ret = true;
	}

	return ret;
}

bool overlay::is_hit(const ivec2& mouse_pos) const {

	ivec2 test_pos = cgv::g2d::get_transformed_mouse_pos(mouse_pos, last_viewport_size_);
	return container_.contains(test_pos);
};

bool overlay::begin_overlay_gui() {
	bool node_is_open = begin_tree_node_void(gui_options.heading != "" ? gui_options.heading : name, &name, -1, false, "level=2;options='w=160';align=''");

	add_layout_member_control("Show", cgv::render::drawable::active, "toggle", "w=40");

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

			add_layout_member_control("", horizontal_alignment_, "dropdown", "enums='Free,Left,Center,Right';w=94", " ");
			add_layout_member_control("", vertical_alignment_, "dropdown", "enums='Free=0,Top=3,Center=2,Bottom=1';w=94");
		}

		if(gui_options.allow_stretch)
			add_layout_member_control("Stretch", stretch_, "dropdown", "enums='None,Horizontal,Vertical,Both'");

		if(gui_options.allow_margin) {
			add_layout_member_control("Margin", margin_[0], "value", "w=94;min=-10000;max=10000;step=1;", " ");
			add_layout_member_control("", margin_[1], "value", "w=94;min=-10000;max=10000;step=1;");
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

void overlay::update_layout() {

	container_.position = margin_;
	ivec2 max_size = last_viewport_size_ - 2 * margin_;

	switch(stretch_) {
	case SO_HORIZONTAL:
		container_.w() = max_size.x();
		break;
	case SO_VERTICAL:
		container_.h() = max_size.y();
		break;
	case SO_BOTH:
		container_.size = max_size;
		break;
	case SO_PERCENTUAL:
		container_.size = percentual_size_ * max_size;
		break;
	default:
		break;
	}

	const auto compute_aligned_position = [this, &max_size](AlignmentOption alignment, int axis) {
		switch(alignment) {
		case AO_CENTER: return (last_viewport_size_[axis] - container_.size[axis]) / 2;
		case AO_END: return last_viewport_size_[axis] - container_.size[axis] - margin_[axis];
		case AO_PERCENTUAL: return static_cast<int32_t>(margin_[axis] + percentual_offset_[axis] * max_size[axis]);
		case AO_START:
		case AO_FREE:
		default: return margin_[axis];
		}
	};

	container_.x() = compute_aligned_position(horizontal_alignment_, 0);
	container_.y() = compute_aligned_position(vertical_alignment_, 1);
}

}
}
