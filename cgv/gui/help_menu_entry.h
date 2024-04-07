#pragma once

#include <cgv/gui/gui_group.h>
#include <cgv/gui/window.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/token.h>
#include <cgv/utils/tokenizer.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// derive from this class to provide a menu entry that can open a seperate window showing help information
class CGV_API help_menu_entry : public cgv::base::node
{
private:
	enum class SectionType {
		kHeading1,
		kHeading2,
		kHeading3,
		kHeading4,
		kText,
		kItems,
		kKeyBindings
	};

	struct section {
		SectionType type = SectionType::kText;
		std::string content;
	};

	std::vector<section> sections;

	void add_section(SectionType type, const std::string& content) {
		sections.push_back({ type, content });
	}

public:
	window_ptr wnd;
	std::vector<gui_group_ptr> groups;

	/// Construct a help menu entry with the given name
	help_menu_entry(const std::string& name = "") : cgv::base::node(name) {}

	// TODO: Do we need a destructor to clean up?
	//~help_menu_entry() {}

	/**@name window handling*/
	//@{
	/// Register the node and create a separate (non-blocking) window to show the stored information.
	void on_register() override;

	/// Unregister the node
	void unregister() override;
	//@}

	/**@name creation of help entries*/
	//@{
	/// Add a heading with level=0.
	void add_heading1(const std::string& text) {
		add_section(SectionType::kHeading1, text);
	}

	/// Add a heading with level=1.
	void add_heading2(const std::string& text) {
		add_section(SectionType::kHeading2, text);
	}

	/// Add a heading with level=2.
	void add_heading3(const std::string& text) {
		add_section(SectionType::kHeading3, text);
	}

	/// Add a heading with level=3.
	void add_heading4(const std::string& text) {
		add_section(SectionType::kHeading4, text);
	}

	/// Add a text paragraph that supports word wrapping.
	void add_text(const std::string& text) {
		add_section(SectionType::kText, text);
	}

	/// Add a list of text items that will be displayed as bullet points.
	/// Items are given as a semicolon-separated list.
	/// Example: "First item;Second item"
	void add_items(const std::string& items) {
		add_section(SectionType::kItems, items);
	}

	/// Add a list of key bindings that will be displayed as a table.
	/// Key bindings are given as a semicolon-separated list of key-action pairs (Key=Action).
	/// Example: "A='Toggle animation;B='Toggle bounding box'"
	void add_key_bindings(const std::string& key_bindings) {
		add_section(SectionType::kKeyBindings, key_bindings);
	}
	//@}
};

/// convenience class to register a factory of the given class type that must be derived from cgv::gui::help_menu_entry
template <class T>
struct help_menu_entry_registration {
	//! this registers an instance of a factory implementation that only takes classes derived from cgv::gui::help_menu_entry
	//! the instance will be automatically registered as a singleton under the 'Help' section in the menu bar
	/*! parameters:
		- \c _created_type_name ... name of the type of the instances created by the factory
		- \c _menu_entry_name ... name to show in the menu bar dropdown
	*/
	help_menu_entry_registration(const std::string& _created_type_name, const std::string& _menu_entry_name = "") {
		static_assert(std::is_base_of<help_menu_entry, T>::value, "T must inherit from cgv::gui::help_menu_entry");
		cgv::base::register_object(new cgv::base::factory_impl<T>(_created_type_name, true, ""), "menu_text='Help/" + _menu_entry_name + "'");
	}
};

	}
}

#include <cgv/config/lib_end.h>