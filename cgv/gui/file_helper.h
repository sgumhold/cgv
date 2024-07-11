#pragma once

#include <cgv/gui/property_string.h>
#include <cgv/gui/provider.h>
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>

namespace cgv {
namespace gui {

/// @brief A convenience class that provides a file input gui control
///
/// Usage:
/// Add as a member to your provider class:
///		cgv::gui::file_helper input;
/// 
/// Set the provider pointer and mode in the constructor of your class:
///		input = cgv::gui::file_helper(this, "Title", cgv::gui::file_helper::Mode::kOpenAndSave);
/// 
/// Set filters to match file types as needed. The first filter that is added will be selected
/// by default when openign the dialog.
///		input.add_filter("Text Files", "txt");
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
class file_helper {
public:
	/// @brief Operation mode enum
	enum class Mode {
		kOpen,
		kSave,
		kOpenAndSave
	};

private:
	/// The pointer to the gui provider for the control
	provider* provider_ptr_ = nullptr;

	/// Operating mode of the control that controls the visibility of open and save buttons. Defaults to Mode::kOpen.
	Mode mode_ = Mode::kOpen;
	/// The title shown in the file system dialog in open mode
	std::string open_title_ = "";
	/// The title shown in the file system dialog in save mode
	std::string save_title_ = "";
	/// The filter string used to control the allowed file types in open mode
	std::string open_filter_ = "";
	/// The filter string used to control the allowed file types in save mode
	std::string save_filter_ = "";
	/// The default path used for the file system dialog in open mode
	std::string open_path_ = "";
	/// The default path used for the file system dialog in save mode
	std::string save_path_ = "";

	/// @brief Return true if the given mode supports opening files, false otherwise.
	bool mode_allows_open(Mode mode) const {
		return mode == Mode::kOpenAndSave || mode == Mode::kOpen;
	}

	/// @brief Return true if the given mode supports saving files, false otherwise.
	bool mode_allows_save(Mode mode) const {
		return mode == Mode::kOpenAndSave || mode == Mode::kSave;
	}

	/// @brief Extend the given filter string with entry
	///
	/// Add the given entry to the back of the given filter string using '|' as separator if
	/// the filter string is not empty.
	///
	/// @param entry The entry to add.
	/// @param to_filter The filter string to extend.
	void extend_filter_string(const std::string& entry, std::string& to_filter) {
		if(!to_filter.empty())
			to_filter += "|";

		to_filter += entry;
	}

	/// @brief Add entry to the filter string/s based on to the value of the operation mode member.
	void add_filter_entry(const std::string& entry, Mode mode) {
		if(mode_allows_open(mode))
			extend_filter_string(entry, open_filter_);

		if(mode_allows_save(mode))
			extend_filter_string(entry, save_filter_);
	}

public:
	/// The current file name.
	/// This member is public to make it accessible to the GUI control flow. It should never be set directly
	/// and set_file_name should be used instead.
	std::string file_name = "";

	/// @brief Construct with default parameters
	///
	/// Default constructor is given to enable using this class as a stack variable member in other classes.
	/// Before usage it must be re-assigned with a valid provider pointer.
	file_helper() {}

	/// @brief Construct with arguments
	/// @param p The gui provider pointer.
	/// @param title The title shown in the file dialog.
	/// @param mode The operating mode.
	file_helper(provider* p, const std::string& title, Mode mode) : provider_ptr_(p), mode_(mode) {
		if(mode_allows_open(mode_))
			open_title_ = title;

		if(mode_allows_save(mode_))
			save_title_ = title;
	}

	/// @brief Return true if this file_helper supports opening files, false otherwise.
	bool can_open() const {
		return mode_allows_open(mode_);
	}

	/// @brief Return true if this file_helper supports saving files, false otherwise.
	bool can_save() const {
		return mode_allows_save(mode_);
	}

	/// @brief Get the title of the specified operation Mode
	/// 
	/// @param mode The optional operation Mode.
	/// @return The title string.
	const std::string& get_title(Mode mode = Mode::kOpenAndSave) const {
		if(mode_allows_open(mode))
			return open_title_;

		return save_title_;
	}

	/// @brief Set the title of the specified operation Mode
	/// 
	/// @param title The new title.
	/// @param mode The operation Mode.
	void set_title(const std::string& title, Mode mode = Mode::kOpenAndSave) {
		if(mode_allows_open(mode))
			open_title_ = title;

		if(mode_allows_save(mode))
			save_title_ = title;

		if(provider_ptr_) {
			provider_ptr_->set_control_property(file_name, "open_title", open_title_);
			provider_ptr_->set_control_property(file_name, "save_title", save_title_);
		}
	}

	/// @brief Get the default path of the specified operation Mode
	/// 
	/// @param mode The operation Mode.
	/// @return The default path string.
	const std::string& get_default_path(Mode mode = Mode::kOpenAndSave) const {
		if(mode_allows_open(mode))
			return open_path_;

		return save_path_;
	}

	/// @brief Set the default path of the specified operation Mode
	/// 
	/// @param path The new defautl path.
	/// @param mode The operation Mode.
	void set_default_path(const std::string& path, Mode mode = Mode::kOpenAndSave) {
		if(mode_allows_open(mode))
			open_path_ = path;

		if(mode_allows_save(mode))
			save_path_ = path;

		if(provider_ptr_) {
			provider_ptr_->set_control_property(file_name, "open_path", open_path_);
			provider_ptr_->set_control_property(file_name, "save_path", save_path_);
		}
	}

	/// @brief Set the current file name
	/// 
	/// @param file_name The file name.
	void set_file_name(const std::string& file_name) {
		this->file_name = file_name;

		if(provider_ptr_)
			provider_ptr_->update_member(&this->file_name);
	}

	/// @brief Clear all custom filters
	/// 
	/// After this method has been called new filters may be set.
	/// The gui must be redrawn manually in order for this change to take effect.
	void clear_filters() {
		open_filter_ = "";
		save_filter_ = "";
	}

	/// @brief Add a custom filter matching a single extension
	/// 
	/// Add a filter for matching files with the given extension in the file dialog.
	/// Example:
	///		add_filter("Text Files", "txt");
	/// will add a filter displaying "Text Files (*.txt)"
	/// 
	/// @param name The display name.
	/// @param extension The extensions matched.
	/// @param mode The operation mode where this filter is used (defaults to kOpenAndSave).
	void add_filter(const std::string& name, const std::string& extension, Mode mode = Mode::kOpenAndSave) {
		std::string entry = name + " (*." + extension + "):*." + extension;
		add_filter_entry(entry, mode);
	}

	/// @brief Add a custom filter matching multiple extensions
	/// 
	/// Add a filter for matching files with one of the given extensions in the file dialog.
	/// Example:
	///		add_multi_filter("Table Files", { "csv", "txt" });
	/// will add a filter displaying "Table Files (*.csv, *.txt)"
	/// 
	/// @param name The display name.
	/// @param extension The extensions matched.
	/// @param mode The operation mode where this filter is used (defaults to kOpenAndSave).
	void add_multi_filter(const std::string& name, const std::vector<std::string>& extensions, Mode mode = Mode::kOpenAndSave) {
		std::string entry = name + " (*." + cgv::utils::join(extensions, ", *.") + "):*." + cgv::utils::join(extensions, ";*.");
		add_filter_entry(entry, mode);
	}

	/// @brief Add a filter that matches all files
	void add_filter_for_all_files(Mode mode = Mode::kOpenAndSave) {
		add_filter_entry("All Files (*.*):*.*", mode);
	}

	/// @brief Compare the extension of file_name to the given string (case insensitive)
	/// @param extension The extension to compare against.
	/// @return True if the stored file_name has extension, false otherwise.
	bool compare_extension(const std::string& extension) const {
		return cgv::utils::to_upper(cgv::utils::file::get_extension(file_name)) == cgv::utils::to_upper(extension);
	}

	/// @brief Ensure the stored file_name has the given extension
	/// 
	/// When a file_name was selected through a file dialog, a user might not always input an extension.
	/// This method ensures the file_name hast the given extension and adds it if not present or in all
	/// cases when force is true.
	/// 
	/// @param extension The extension string to add (without dot).
	/// @param force Force adding the extension.
	/// @return The extension of the file_name after this operation.
	const std::string ensure_extension(const std::string& extension, bool force = false) {
		std::string current_extension = cgv::utils::file::get_extension(file_name);

		if(cgv::utils::to_upper(current_extension) != cgv::utils::to_upper(extension)) {
			if(current_extension.empty() || force) {
				set_file_name(file_name + "." + extension);
				return extension;
			}
		}

		return current_extension;
	}

	/// @brief Create the gui control for the file input
	/// @param label The control label
	/// @param extra_options Additional options applied to the string input control.
	void create_gui(const std::string& label, const std::string& extra_options = "") {
		property_string options;

		options.add("open", can_open());
		options.add("save", can_save());

		options.add_bracketed("open_title", open_title_);
		options.add_bracketed("save_title", save_title_);

		options.add_bracketed("open_filter", open_filter_);
		options.add_bracketed("save_filter", save_filter_);

		options.add_bracketed("open_path", open_path_);
		options.add_bracketed("save_path", save_path_);

		options.add("", extra_options);

		if(provider_ptr_)
			provider_ptr_->add_gui(label, file_name, "file_name", options);
	}

	/// @brief Check whether a save action was performed
	/// @return True if a save action occured, false otherwise.
	bool is_save_action() const {
		return provider_ptr_ ? provider_ptr_->ref_tree_node_visible_flag(file_name) : false;
	}
};

}
}
