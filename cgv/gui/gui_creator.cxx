#include "gui_creator.h"
#include "provider.h"
#include "file_dialog.h"
#include <cgv/base/base.h>
#include <cgv/base/base_generator.h>
#include <vector>

using namespace cgv::signal;

namespace cgv {
	namespace gui {

std::vector<gui_creator*>& ref_gui_creators()
{
	static std::vector<gui_creator*> creators;
	return creators;
}

/// register a gui creator
void register_gui_creator(gui_creator* gc)
{
	ref_gui_creators().push_back(gc);
}

/// create the gui for a composed structure
bool create_gui(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool* toggles)
{
	for (unsigned i=0; i<ref_gui_creators().size(); ++i)
		if (ref_gui_creators()[i]->create(p,label,value_ptr,value_type,gui_type,options,toggles))
			return true;
	return false;
}

struct file_name_gui_creator : public cgv::gui::gui_creator
{
	void open(std::string* value_ptr, cgv::base::base* b, const std::string& options)
	{
		std::string title, filter, path;
		cgv::base::has_property(options, "title", title);
		cgv::base::has_property(options, "filter", filter);
		cgv::base::has_property(options, "path", path);
		std::string fn;
		bool save = false;
		cgv::base::has_property(options, "save", save);
		if (save)
			fn = file_save_dialog(title, filter, path);
		else
			fn = file_open_dialog(title, filter, path);
		if (fn.empty())
			return;
		*value_ptr = fn;
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
		if (!gui_type.empty() && gui_type != "file_name")
			return false;
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
		std::string& v = *((std::string*)value_ptr);
		bool save = false;
		cgv::base::has_property(options, "save", save);
		std::string opts = "image='res://open32.png';w=32;h=32;label=''";
		if (save)
			opts = "image='res://save32.png';w=32;h=32;label=''";
		if (b)
			p->add_member_control(b, label, v, "", options, " ");
		else
			p->add_control(label, v, "", options, " ");
		connect_copy(p->add_button(label, opts)->click,
			rebind(this, &file_name_gui_creator::open, &v, b, options));
		return true;
	}
};

gui_creator_registration<file_name_gui_creator> file_name_gui_creator_reg;

	}
}