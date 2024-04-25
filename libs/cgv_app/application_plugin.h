#pragma once

#ifdef CGV_FORCE_STATIC
	#define APP_PATH cgv::base::ref_prog_path_prefix();
#else
	#define APP_PATH QUOTE_SYMBOL_VALUE(INPUT_DIR) "/";
#endif

#include <cgv/base/node.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/utils/pointer_test.h>

#include "overlay.h"

#include "lib_begin.h"

namespace cgv {
namespace app {

/// @brief Abstract base class for application plugins that provides capability to handle overlays, GUI interaction and event handling.
///
/// For concrete class implementations see generic_application_plugin and application_plugin.
class CGV_API application_plugin_base :
	public cgv::base::group,		// derive from group to integrate into global tree structure and allow registration of child nodes
	public cgv::gui::provider,		// derive from provider to obtain a GUI tab
	public cgv::gui::event_handler	// derive from event handler to be able to directly react to user interaction
{
protected:
	/// Stores a pointer to the base view. If a specific view is required in an implementation,
	/// it is advised to declare a new member with the specific view name in the actual implementation of this class.
	cgv::render::view* view_ptr;

	/// Stores a list of all overlays registered as children of this application plugin.
	std::vector<overlay_ptr> overlays;

	/// Stores a pointer to the overlay that handled the previous event.
	overlay_ptr last_blocking_overlay_ptr;

	/// Stores a pointer to the overlay that handles the current event.
	overlay_ptr blocking_overlay_ptr;

public:
	application_plugin_base() = delete;

	/// @brief Provides a base constructor that calls the base class constructor of group.
	/// @param name the name of this node.
	application_plugin_base(const std::string& name);

	/// @brief Registers an overlay instance as a child of this group.
	/// 
	/// @tparam T the overlay class type; must inherit from overlay.
	/// @param name the name of the overlay instance.
	/// @return a ref_ptr to the registered overlay instance.
	template<class T>
	cgv::data::ref_ptr<T> register_overlay(const std::string& name) {
		static_assert(std::is_base_of<overlay, T>::value, "T must inherit from overlay");
		cgv::data::ref_ptr<T> ptr(new T());
		ptr->set_name(name);
		cgv::base::group::append_child(ptr);
		overlays.push_back(ptr);
		return ptr;
	}

	/// @brief Unregisters an overlay instance that is a child of this group.
	/// 
	/// @param ptr the overlay pointer.
	/// @param name the name of the overlay instance.
	/// @return the number of removed children.
	unsigned unregister_overlay(overlay_ptr ptr) {
		unsigned count = cgv::base::group::remove_child(ptr);
		auto it = std::find(overlays.begin(), overlays.end(), ptr);
		if(it != overlays.end())
			overlays.erase(it);
		return count;
	}

	/// @brief Final implementation of event_handler::handle() to support event handling of registered overlays.
	/// 
	/// Implement handle_event to handle any events that are passed to the instance of this class.
	/// 
	/// @param e the event to handle.
	/// @return true if the event was handled and stopped here, false otherwise.
	virtual bool handle(cgv::gui::event& e) final;

	/// @brief Implement this method to handle any events not blocked by an registered overlay.
	/// 
	/// @param e the event to handle.
	/// @return true if the event was handled and stopped here, false otherwise.
	virtual bool handle_event(cgv::gui::event& e) = 0;

	/// @brief Implement this method to handle member changes from GUI interactions or manual calls to on_set().
	/// @param member_ptr the pointer to the affected member.
	virtual void on_set(void* member_ptr) = 0;

	/// @brief Implement this method to handle member changes from GUI interactions or manual calls to on_set()
	/// provided through a convenient evaluation interface.
	/// 
	/// @param m the pointer_test interface used to conveniently evaluate the affected member.
	virtual void handle_member_change(const cgv::utils::pointer_test& m) = 0;

	/// @brief Implement this method to initialize the view pointer to a valid value and return whether this was successfull.
	virtual bool initialize_view_ptr() = 0;
};

/// @brief Extension of application_plugin_base that allows to define a used drawable base class.
/// @tparam base_drawable the used drawable type.
template <class base_drawable>
class generic_application_plugin : public application_plugin_base, public base_drawable
{
public:
	/// @brief Constructs an instance of the class as a group node with the given name.
	/// @param name the name of this node.
	generic_application_plugin(const std::string& name) : application_plugin_base(name) {}

	/// @brief Default implementation of the pure virtual base class method that delegates the member change evaluation to
	/// handle_member_change and performs the default actions of updating all member GUI views and requesting a redraw.
	/// 
	/// @param member_ptr the pointer to the affected member.
	virtual void on_set(void* member_ptr) {
		handle_member_change(cgv::utils::pointer_test(member_ptr));
		update_member(member_ptr);
		base_drawable::post_redraw();
	}

	/// @brief Empty default implementation to handle member changes from GUI interactions or manual calls to on_set()
	/// provided through a convenient evaluation interface.
	/// 
	/// @param m the pointer_test interface used to conveniently evaluate the affected member.
	virtual void handle_member_change(const cgv::utils::pointer_test& m) {}

	/// @brief Initializes the view pointer using drawable::find_view_as_node().
	///
	/// Can be used to check when the view pointer is available and, e.g., perform one-time initializations of dependent
	/// members. Due to the non-guaranteed registration order of plugins it is advisable to call this method in
	/// drawable::init_frame.
	///
	/// @return true when the view pointer is set initially (this will only fire once), false otherwise.
	virtual bool initialize_view_ptr() final {
		return !application_plugin_base::view_ptr && (application_plugin_base::view_ptr = base_drawable::find_view_as_node());
	}
};

/// @brief Specialization of generic_application_plugin that uses the default drawable as the template argument.
class CGV_API application_plugin : public generic_application_plugin<cgv::render::drawable>
{
public:
	/// @brief Constructs an instance of the class as a group node with the given name.
	/// @param name the name of this node.
	application_plugin(const std::string& name);
};

} // namespace app
} // namespace cgv

#include <cgv/config/lib_end.h>
