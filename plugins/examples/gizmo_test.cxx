#include <cgv/base/group.h>
#include <cgv/math/quaternion.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/box_renderer.h>
#include <cg_gizmo/transformation_gizmo.h>


class gizmo_test :
	public cgv::base::group,			// derive from group to support child nodes (needed for overlays)
	public cgv::gui::event_handler,		// derive from event handler to receive input events
	public cgv::gui::provider,			// derive from gui provider to have gui controls
	public cgv::render::drawable		// derive from drawable to allow drawing in the GL context
{
protected:
	cgv::gui::transformation_gizmo_ptr tg_ptr;
	cgv::render::box_render_style brs;
	std::vector<cgv::box3> boxes;
	std::vector<cgv::rgb> colors;
	std::vector<cgv::vec3> translations;
	std::vector<cgv::quat> rotations;

	cgv::vec3 on_scale_start_center;
	cgv::vec3 on_scale_start_half_extent;

	int current_primitive = -1;

	void callback(cgv::gui::GizmoAction action, cgv::gui::transformation_gizmo::Mode mode)
	{
		switch (mode) {
		case cgv::gui::transformation_gizmo::Mode::kTranslation:
			translations[current_primitive] = tg_ptr->get_position() - on_scale_start_center;
			post_redraw();
			break;
		case cgv::gui::transformation_gizmo::Mode::kRotation:
			rotations[current_primitive] = tg_ptr->get_rotation();
			post_redraw();
			break;
		case cgv::gui::transformation_gizmo::Mode::kScale:
			boxes[current_primitive] = cgv::box3(
				on_scale_start_center - tg_ptr->get_scale() * on_scale_start_half_extent,
				on_scale_start_center + tg_ptr->get_scale() * on_scale_start_half_extent);
			post_redraw();
			break;
		}
	}
	void begin_gizmo()
	{
		if (current_primitive == -1)
			tg_ptr->hide();
		else {
			if (!tg_ptr->is_visible())
				tg_ptr->show();
			tg_ptr->set_position(boxes[current_primitive].get_center()+translations[current_primitive]);
			tg_ptr->set_rotation(rotations[current_primitive]);
			on_scale_start_center = boxes[current_primitive].get_center();
			if (tg_ptr->get_mode() == cgv::gui::transformation_gizmo::Mode::kScale) {
				on_scale_start_center = boxes[current_primitive].get_center();
				on_scale_start_half_extent = 0.5f * boxes[current_primitive].get_extent();
			}
		}
	}
	void set_mode(cgv::gui::transformation_gizmo::Mode mode)
	{
		if (tg_ptr->get_mode() == mode)
			return;
		tg_ptr->set_mode(mode);
		begin_gizmo();
		post_redraw();
	}
	void select_primitive(int i)
	{
		if (i == current_primitive)
			return;
		current_primitive = i;
		begin_gizmo();
		post_redraw();
	}
public:
	gizmo_test() : cgv::base::group("gizmo_test")
	{
		tg_ptr = create_and_append_child<cgv::gui::transformation_gizmo>("Gizmo");
		tg_ptr->on_change = [this](cgv::gui::GizmoAction action, cgv::gui::transformation_gizmo::Mode mode) {
			this->callback(action, mode); };
		tg_ptr->set_mode(cgv::gui::transformation_gizmo::Mode::kRotation);
		for (int i = 0; i < 5; ++i) {
			boxes.push_back(cgv::box3(cgv::vec3(float(i - 2), -0.5f, -0.3f), cgv::vec3(i - 1.5f, 0.5f, 0.3f)));
			colors.push_back(cgv::rgb(0.2f * i, 1.0f - 0.2f * i, 0.0f));
			translations.push_back(cgv::vec3(0.0f));
			rotations.push_back(cgv::quat(1.0f, cgv::vec3(0.0f)));
		}
		select_primitive(2);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &current_primitive) {
			begin_gizmo();
		}
		update_member(member_ptr);
		post_redraw();
	}
	bool handle(cgv::gui::event& e)
	{
		if (e.get_kind() == cgv::gui::EID_KEY) {
			auto& ke = reinterpret_cast<cgv::gui::key_event&>(e);
			if (ke.get_action() != cgv::gui::KA_RELEASE) {
				switch (ke.get_key()) {
				case cgv::gui::KEY_Right:
					if (++current_primitive == boxes.size())
						current_primitive = -1;
					on_set(&current_primitive);
					return true;
				case cgv::gui::KEY_Left:
					if (current_primitive == -1)
						current_primitive = boxes.size() - 1;
					else
						--current_primitive;
					on_set(&current_primitive);
					return true;
				case 'T': 
					set_mode(cgv::gui::transformation_gizmo::Mode::kTranslation); 
					return true;
				case 'R': 
					set_mode(cgv::gui::transformation_gizmo::Mode::kRotation); 
					return true;
				case 'S': 
					set_mode(cgv::gui::transformation_gizmo::Mode::kScale); 
					return true;
				}
			}
		}
		return false;
	}
	void stream_help(std::ostream& os)
	{
		return;
	}
	void create_gui()
	{
		add_decorator("Gizmo Test", "heading", "level=2");
		add_member_control(this, "Selected", current_primitive, "value_slider", "min=-1;ticks=true")->set("max", boxes.size() - 1);
		if (begin_tree_node("Box Style", brs)) {
			align("\a");
			add_gui("Box Style", brs);
			align("\b");
			end_tree_node(brs);
		}
		inline_object_gui(tg_ptr);
	}
	bool init(cgv::render::context& ctx)
	{
		cgv::render::ref_box_renderer(ctx, 1);
		return true;
	}
	void clear(cgv::render::context& ctx)
	{
		cgv::render::ref_box_renderer(ctx, -1);
	}
	void draw(cgv::render::context& ctx)
	{
		auto& br = cgv::render::ref_box_renderer(ctx, 1);
		br.set_render_style(brs);
		br.set_box_array(ctx, boxes);
		br.set_color_array(ctx, colors);
		br.set_translation_array(ctx, translations);
		br.set_rotation_array(ctx, rotations);
		br.render(ctx, 0, boxes.size());
	}
};

#include <cgv/base/register.h>

cgv::base::factory_registration<gizmo_test> gizmo_test_fac("Gizmo Test", "shortcut='Ctrl-G';menu_text='New/Render/Gizmo Test'", true);
