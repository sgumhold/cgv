#include <cgv/gui/gui_creator.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/gui/provider.h>
#include <cgv/base/base.h>
#include <cgv/base/base_generator.h>
#include <cgv/utils/file.h>

using namespace cgv::signal;

namespace cgv {
	namespace gui {

struct file_name_gui_creator : public cgv::gui::gui_creator
{
	void button_cb(std::string* value_ptr, cgv::base::base* b, const std::string& options, bool is_save)
	{
		std::string title, filter, path;
		if (!cgv::base::has_property(options, is_save ? "save_title" : "open_title", title, true))
			cgv::base::has_property(options, "title", title, true);

		if (!cgv::base::has_property(options, is_save ? "save_filter" : "open_filter", filter, true))
			cgv::base::has_property(options, "filter", filter, true);

		if (!cgv::base::has_property(options, is_save ? "save_path" : "open_path", path, true))
			cgv::base::has_property(options, "path", path, true);

		std::string fn;
		if (is_save)
			fn = file_save_dialog(title, filter, path);
		else
			fn = file_open_dialog(title, filter, path);
		if (fn.empty())
			return;
		*value_ptr = fn;
		if (!b)
			return;
		provider* p = b->get_interface<provider>();
		if (p) {
			p->ref_tree_node_visible_flag(*value_ptr) = is_save;
			p->update_member(value_ptr);
		}
		b->on_set(value_ptr);
		if (p)
			p->ref_tree_node_visible_flag(*value_ptr) = false;
	}
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool*)
	{
		if (value_type != cgv::type::info::type_name<std::string>::get_name())
			return false;
		if (!gui_type.empty() && gui_type != "file_name")
			return false;
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
		std::string& v = *((std::string*)value_ptr);

		bool save = false;
		bool open = false;
		int button_count = 0;
		if (cgv::base::has_property(options, "save", save, true) && save)
			++button_count;
		if (cgv::base::has_property(options, "open", open, true) && open)
			++button_count;
		if (button_count == 0) {
			open = true;
			++button_count;
		}

		bool control = true;
		cgv::base::has_property(options, "control", control, true);

		std::string align_gui = "\n";
		cgv::base::has_property(options, "align_gui", align_gui, true);

		if (control) {
			if (b)
				p->add_member_control(b, label, v, "", options, " ");
			else
				p->add_control(label, v, "", options, " ");
		}
		if (open)
			connect_copy(
				p->add_button(label, "image='res://open32.png';w=32;h=32;label=''", save ? std::string(" ") : align_gui)->click,
				rebind(this, &file_name_gui_creator::button_cb, &v, b, options, false)
			);
		if (save)
			connect_copy(
				p->add_button(label, "image='res://save32.png';w=32;h=32;label=''", align_gui)->click,
				rebind(this, &file_name_gui_creator::button_cb, &v, b, options, true)
			);
		if (!open && !save)
			p->align(align_gui);

		return true;
	}
};


struct directory_gui_creator : public cgv::gui::gui_creator
{
	void open(std::string* value_ptr, cgv::base::base* b, const std::string& options)
	{
		std::string title, path;
		cgv::base::has_property(options, "title", title, true);
		cgv::base::has_property(options, "path", path, true);
		if (path.empty()) {
			if (value_ptr && !value_ptr->empty())
				path = *value_ptr;
		}
		std::string fn;
		bool save = false;
		cgv::base::has_property(options, "save", save, true);
		if (save)
			fn = directory_save_dialog(title, path);
		else
			fn = directory_open_dialog(title, path);
		if (fn.empty())
			return;
		*value_ptr = fn;
		if (b->get_interface<provider>())
			b->get_interface<provider>()->update_member(value_ptr);
		if (b)
			b->on_set(value_ptr);
	}
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool*)
	{
		if (value_type != cgv::type::info::type_name<std::string>::get_name())
			return false;
		if (!gui_type.empty() && gui_type != "directory")
			return false;
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
		std::string& v = *((std::string*)value_ptr);
		bool save = false;
		cgv::base::has_property(options, "save", save, true);
		std::string opts = "image='res://open32.png';w=32;h=32;label=''";
		if (save)
			opts = "image='res://save32.png';w=32;h=32;label=''";
		if (b)
			p->add_member_control(b, label, v, "", options, " ");
		else
			p->add_control(label, v, "", options, " ");
		connect_copy(p->add_button(label, opts)->click,
			rebind(this, &directory_gui_creator::open, &v, b, options));
		return true;
	}
};

#include "lib_begin.h"

CGV_API gui_creator_registration<file_name_gui_creator> file_name_gui_creator_reg("file_name_gui_creator");
CGV_API gui_creator_registration<directory_gui_creator> directory_gui_creator_reg("directory_gui_creator");

	}
}