#include "gui_driver.h"
#include "file_dialog.h"

namespace cgv {
	namespace gui {

std::string file_open_dialog(const std::string& title, const std::string& filter, const std::string& path)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return "";
	return d->file_open_dialog(title,filter,path);
}

/// ask user for an open dialog that can select multiple files, return common path prefix and fill field of filenames
std::string files_open_dialog(std::vector<std::string>& file_names, const std::string& title, const std::string& filter, const std::string& path)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return "";
	return d->files_open_dialog(file_names,title,filter,path);
}


std::string file_save_dialog(const std::string& title, const std::string& filter, const std::string& path)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return "";
	return d->file_save_dialog(title,filter,path);
}

	}
}