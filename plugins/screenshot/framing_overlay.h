#pragma once

//#include <memory>
#include <cgv/gui/dialog.h>
#include <cgv/gui/help_message.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv_g2d/draggable_collection.h>
#include <cgv_g2d/generic_2d_renderer.h>
#include <cgv_g2d/msdf_text_geometry.h>
#include <cgv_overlay/themed_canvas_overlay.h>

class framing_overlay : public cgv::overlay::themed_canvas_overlay {
public:
	framing_overlay();
	std::string get_type_name() const override { return "framing_overlay"; }

	bool init(cgv::render::context& ctx) override;
	void clear(cgv::render::context& ctx) override;

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::ivec2 local_mouse_pos) override;

	void init_frame(cgv::render::context& ctx) override;
	void draw_content(cgv::render::context& ctx) override;

	void clear_frame();

	void set_frame(const cgv::g2d::rect& frame);

	cgv::g2d::rect get_frame() const;

	void enable_editing();

	void disable_editing();

	cgv::signal::signal<> on_change;

private:
	static float corner_handle_size;
	static float min_frame_size;

	cgv::g2d::rect frame_absolute;
	cgv::g2d::rect frame_relative;
	
	bool editing_enabled = false;

	cgv::g2d::draggable_collection<cgv::g2d::draggable> handles;
	
	std::vector<std::string> label_texts;
	cgv::g2d::msdf_text_geometry labels;
	cgv::g2d::text2d_style label_style;

	void init_styles() override;

	bool is_frame_valid() const;

	cgv::g2d::rect to_relative_frame(const cgv::g2d::rect& absolute_frame, cgv::vec2 viewport_size);

	cgv::g2d::rect to_absolute_frame(const cgv::g2d::rect& relative_frame, cgv::vec2 viewport_size);

	void create_handles();

	void update_handles();

	void handle_drag(cgv::g2d::DragAction action);
};

typedef cgv::data::ref_ptr<framing_overlay> framing_overlay_ptr;
