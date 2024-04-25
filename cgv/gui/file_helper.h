#pragma once

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
/// Create the gui in your create_gui method:
///		input.create_gui("Label");
/// 
/// Testing for value changes in on_set(void* member_ptr):
///		if(member_ptr == &input.file_name) {
///			if(input.save()) {
///				...handle save
///			} else {
///				...handle open
///			}
///		}
/// 
/// Manually updating the file_name:
///		input.file_name = "some file name";
///		input.update();
class file_helper {
public:
	/// @brief Operation mode enum
	enum class Mode {
		kOpen,
		kSave,
		kOpenAndSave
	};
protected:
	/// The pointer to the gui provider for the control
	cgv::gui::provider* provider_ptr = nullptr;
	/// Operating mode of the control that controls the visibility of open and save buttons. Defaults to Mode::kOpen.
	Mode mode = Mode::kOpen;
	/// The title shown in the file system dialog
	std::string title = "";
	/// The filter string used by the gui control, controlling the allowed file types
	std::string filter = "";

public:
	/// Whether to add a default filter matching all files
	bool allow_all_files = true;
	/// Whether to default to the all files filter when opening the dialog
	bool default_to_all_files = false;
	/// The current store file name
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
	file_helper(cgv::gui::provider* p, const std::string& title, Mode mode) : provider_ptr(p), title(title), mode(mode) {}

	/// Clear all custom filters
	void clear_filter() {

		filter = "";
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
	void add_filter(const std::string& name, const std::string& extension) {

		if(!filter.empty())
			filter += "|";

		filter += name + " (*." + extension + "):*." + extension;
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
	void add_multi_filter(const std::string& name, const std::vector<std::string>& extensions) {

		if(!filter.empty())
			filter += "|";

		filter += name + " (*." + cgv::utils::join(extensions, ", *.") + "):*." + cgv::utils::join(extensions, ";*.");
	}

	/// Update the gui control after changing file_name
	void update() {

		if(provider_ptr)
			provider_ptr->update_member(&file_name);
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
				file_name += "." + extension;
				return extension;
			}
		}

		return current_extension;
	}

	/// @brief Create the gui control for the file input
	/// @param label The control label
	/// @param options Additional options applied to the string input control.
	void create_gui(const std::string& label, const std::string& options = "") {

		std::string configuration = "";
		switch(mode) {
		case Mode::kOpen: configuration = "open=true"; break;
		case Mode::kSave: configuration = "save=true"; break;
		case Mode::kOpenAndSave: configuration = "open=true;save=true"; break;
		default: "open=false;save=false"; break;
		}

		std::string all_files_filter = allow_all_files ? "All Files (*.*):*.*" : "";
		std::string file_filter = filter;

		if(default_to_all_files)
			file_filter = all_files_filter + "|" + filter;
		else
			file_filter = filter + "|" + all_files_filter;

		if(provider_ptr)
			provider_ptr->add_gui(label, file_name, "file_name", "title='" + title + "';filter='" + file_filter + "';" + configuration + ";" + options);
	}

	/// @brief Check whether a save action was performed
	/// @return True if a save action occured, false otherwise.
	bool save() const {

		return provider_ptr ? provider_ptr->ref_tree_node_visible_flag(file_name) : false;
	}
};

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
///			if(input.save()) {
///				...handle save
///			} else {
///				...handle open
///			}
///		}
/// 
/// Manually updating the directory_name:
///		input.directory_name = "some directory name";
///		input.update();
class directory_helper {
public:
	/// @brief Operation mode enum
	enum class Mode {
		kOpen,
		kSave
	};
protected:
	/// The pointer to the gui provider for the control
	cgv::gui::provider* provider_ptr = nullptr;
	/// Operating mode of the control that controls the visibility of open and save buttons. Defaults to Mode::kOpen.
	Mode mode = Mode::kOpen;
	/// The title shown in the file system dialog
	std::string title = "";

public:
	/// The current store file name
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
	directory_helper(cgv::gui::provider* p, const std::string& title, Mode mode) : provider_ptr(p), title(title), mode(mode) {}

	/// Update the gui control after changing directory_name
	void update() {

		if(provider_ptr)
			provider_ptr->update_member(&directory_name);
	}

	/// @brief Create the gui control for the directoy input
	/// @param label The control label
	/// @param options Additional options applied to the string input control.
	void create_gui(const std::string& label, const std::string& options = "") {

		std::string configuration = "";
		switch(mode) {
		case Mode::kOpen: configuration = "open=true"; break;
		case Mode::kSave: configuration = "save=true"; break;
		default: "open=false;save=false"; break;
		}

		if(provider_ptr)
			provider_ptr->add_gui(label, directory_name, "directory", "title='" + title + "';" + configuration + ";" + options);
	}

	/// @brief Check whether a save action was performed
	/// @return True if a save action occured, false otherwise.
	bool save() const {

		return provider_ptr ? provider_ptr->ref_tree_node_visible_flag(directory_name) : false;
	}
};

}
}
