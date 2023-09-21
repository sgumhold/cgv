#pragma once

#include <cgv/gui/provider.h>
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>

namespace cgv {
namespace gui {

class file_helper {
public:
	enum class Mode {
		kOpen,
		kSave,
		kOpenAndSave
	};
protected:
	cgv::gui::provider* provider_ptr = nullptr;
	Mode mode = Mode::kOpen;
	std::string title = "";
	std::string filter = "";

public:
	bool allow_all_files = true;
	bool default_to_all_files = false;
	std::string file_name = "";

	file_helper() {}

	file_helper(cgv::gui::provider* p, const std::string& title, Mode mode) : provider_ptr(p), title(title), mode(mode) {}

	void clear_filter() {

		filter = "";
	}

	void add_filter(const std::string& name, const std::string& extension) {

		if(!filter.empty())
			filter += "|";

		filter += name + " (*." + extension + "):*." + extension;
	}

	void add_filter(const std::string& name, const std::vector<std::string>& extensions) {

		if(!filter.empty())
			filter += "|";

		filter += name + " (*." + cgv::utils::join(extensions, ", *.") + "):*." + cgv::utils::join(extensions, ";*.");
	}

	void update() {

		if(provider_ptr)
			provider_ptr->update_member(&file_name);
	}

	bool compare_extension(const std::string& extension) const {

		return cgv::utils::to_upper(cgv::utils::file::get_extension(file_name)) == cgv::utils::to_upper(extension);
	}

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

	bool save() const {

		return provider_ptr ? provider_ptr->ref_tree_node_visible_flag(file_name) : false;
	}
};

class directory_helper {
public:
	enum class Mode {
		kOpen,
		kSave
	};
protected:
	cgv::gui::provider* provider_ptr = nullptr;
	Mode mode = Mode::kOpen;
	std::string title = "";

public:
	std::string directory_name = "";

	directory_helper() {}

	directory_helper(cgv::gui::provider* p, const std::string& title, Mode mode) : provider_ptr(p), title(title), mode(mode) {}

	void update() {

		if(provider_ptr)
			provider_ptr->update_member(&directory_name);
	}

	void create_gui(const std::string& label, const std::string& options = "") {

		std::string configuration = "";
		switch(mode) {
		case Mode::kOpen: configuration = "open=true"; break;
		case Mode::kSave: configuration = "save=true"; break;
		default: "open=false;save=false";
		}

		if(provider_ptr)
			provider_ptr->add_gui(label, directory_name, "directory", "title='" + title + "';" + configuration + ";" + options);
	}

	bool save() const {

		return provider_ptr ? provider_ptr->ref_tree_node_visible_flag(directory_name) : false;
	}
};

}
}
