#pragma once

#include <cgv/gui/property_string.h>
#include <cgv/gui/provider.h>

namespace cgv {
namespace gui {

/// @brief A convenience class that provides a directory input gui control
///
/// Usage:
/// Add as a member to your provider class:
///		cgv::gui::directory_helper input;
/// 
/// Set the provider pointer and mode in the constructor of your class:
///		input = cgv::gui::file_helper(this, "Title", cgv::gui::file_helper::Mode::kOpenAndSave);
/// 
/// Create the gui in your create_gui method:
///		input.create_gui("Label");
/// 
/// Testing for value changes in on_set(void* member_ptr):
///		if(member_ptr == &input.file_name) {
///			if(input.is_save_action()) {
///				...handle save
///			} else {
///				...handle open
///			}
///		}
class directory_helper {
public:
	/// @brief Operation mode enum
	enum class Mode {
		kOpen,
		kSave
	};
protected:
	/// The pointer to the gui provider for the control
	provider* provider_ptr_ = nullptr;
	/// Operating mode of the control that controls the visibility of open and save buttons. Defaults to Mode::kOpen.
	Mode mode_ = Mode::kOpen;
	/// The title shown in the file system dialog
	std::string title_ = "";
	/// The default path used for the file system dialog
	std::string path_ = "";

public:
	/// The current directory name
	/// This member is public to make it accessible to the GUI control flow. It should never be set directly
	/// and set_directory_name should be used instead.
	std::string directory_name = "";

	/// @brief Construct with default parameters
	///
	/// Default constructor is given to enable using this class as a stack variable member in other classes.
	/// Before usage it must be re-assigned with a valid provider pointer.
	directory_helper() {}

	/// @brief Construct with arguments
	/// @param p The gui provider pointer.
	/// @param title The title shown in the directory dialog.
	/// @param mode The operating mode.
	directory_helper(provider* p, const std::string& title, Mode mode) : provider_ptr_(p), title_(title), mode_(mode) {}

	/// @brief Return true if this directory_helper supports opening files, false otherwise.
	bool can_open() const {
		return mode_ == Mode::kOpen;
	}

	/// @brief Return true if this directory_helper supports saving files, false otherwise.
	bool can_save() const {
		return mode_ == Mode::kSave;
	}

	/// @brief Get the title
	/// 
	/// @return The title string.
	const std::string& get_title() const {
		return title_;
	}

	/// @brief Set the title
	/// 
	/// @param title The new title.
	void set_title(const std::string& title) {
		title_ = title;

		if(provider_ptr_)
			provider_ptr_->set_control_property(directory_name, "title", title_);
	}

	/// @brief Get the default path
	/// 
	/// @return The default path string.
	const std::string& get_default_path() const {
		return path_;
	}

	/// @brief Set the default path
	/// 
	/// @param path The new defautl path.
	void set_default_path(const std::string& path) {
		path_ = path;

		if(provider_ptr_)
			provider_ptr_->set_control_property(directory_name, "path", path_);
	}

	/// @brief Set the current directory name
	/// 
	/// @param directory_name The directory name.
	void set_directory_name(const std::string& directory_name) {
		this->directory_name = directory_name;

		if(provider_ptr_)
			provider_ptr_->update_member(&this->directory_name);
	}

	/// @brief Create the gui control for the directoy input
	/// @param label The control label
	/// @param extra_options Additional options applied to the string input control.
	void create_gui(const std::string& label, const std::string& extra_options = "") {
		property_string options;

		options.add("open", can_open());
		options.add("save", can_save());

		options.add_bracketed("path", path_);

		options.add("", extra_options);

		if(provider_ptr_)
			provider_ptr_->add_gui(label, directory_name, "directory", options);
	}

	/// @brief Check whether a save action was performed
	/// @return True if a save action occured, false otherwise.
	bool is_save_action() const {
		return provider_ptr_ ? provider_ptr_->ref_tree_node_visible_flag(directory_name) : false;
	}
};

}
}
