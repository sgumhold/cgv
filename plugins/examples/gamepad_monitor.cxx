#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/render/drawable.h>
#include <cgv/utils/convert_string.h>
//#include <plot/plot2d.h>
#include <cg_gamepad/gamepad_server.h>

class gamepad_monitor : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::event_handler,
	public cgv::gui::provider,
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
protected:
	std::vector<void*> devices;
	std::vector<gamepad::gamepad_state> states;
public:
	gamepad_monitor() :	node("gamepad monitor")
	{
		connect_copy(cgv::gui::ref_gamepad_server().on_device_change, 
			cgv::signal::rebind(this, &gamepad_monitor::on_device_change, cgv::signal::_1, cgv::signal::_2));
		
		static bool gamepad_started = false;
		if (!gamepad_started) {
			cgv::gui::connect_gamepad_server();
			gamepad_started = true;
		}
	}

	void on_device_change(void* device, bool attach)
	{
		if (attach) {
			devices.push_back(device);
			states.push_back(gamepad::gamepad_state());
			gamepad::get_state(device, states.back());
		}
		else {
			auto iter = std::find(devices.begin(), devices.end(), device);
			if (iter != devices.end()) {
				states.erase(states.begin() + (iter - devices.begin()));
				devices.erase(iter);
			}
		}
		post_recreate_gui();
		post_redraw();
	}
	std::string get_type_name() const 
	{ 
		return "gamepad_monitor"; 
	}
	void stream_stats(std::ostream& os)
	{
	//	cgv::utils::oprintf(os, "min_filter: %s [edit:<F>], anisotropy: %f [edit:<A>]\n", filter_names[min_filter], anisotropy);
	}
	void stream_help(std::ostream& os)
	{
//		os << "select object: <1> ... cube, <2> ... sphere, <3> ... square" << std::endl;
	}
	void on_set(void* member_ptr)
	{
		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		return true;
	}
	void init_frame(cgv::render::context& ctx)
	{
	}
	void draw(cgv::render::context& ctx)
	{
	}
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const { return "example/gamepad"; }
	
	void create_gamepad_gui(size_t i)
	{
		if (!begin_tree_node(std::string("device ") + cgv::utils::to_string(devices[i]), devices[i], true))
			return;
		align("\a");
		auto& state = states[i];
		add_view("left_stick_x", state.left_stick_position[0]);
		add_view("left_stick_y", state.left_stick_position[1]);
		add_view("rightstick_x", state.right_stick_position[0]);
		add_view("rightstick_y", state.right_stick_position[1]);
		add_view("left_trigger", state.trigger_position[0]);
		add_view("right_trigger", state.trigger_position[1]);
		add_view("buttons", state.button_flags);
		align("\b");
		end_tree_node(devices[i]);
	}

	/// you must overload this for gui creation
	void create_gui() 
	{
		add_decorator("gamepad monitor", "heading");
		for (size_t i = 0; i < devices.size(); ++i) {
			create_gamepad_gui(i);
		}
	}

	bool handle(cgv::gui::event& e)
	{
		if (e.get_flags() | cgv::gui::EF_PAD) {

			for (size_t i = 0; i < devices.size(); ++i) {
				gamepad::get_state(devices[i], states[i]);
			}
			update_all_members();
		}
		return false;
	}
};

#include <cgv/base/register.h>

cgv::base::factory_registration<gamepad_monitor> gamepad_monitor_fac("gamepad_monitor", 
	"shortcut='Ctrl-Shift-G';menu_text='New/Gamepad Monitor'", true);


