#pragma once

#include <cgv/base/base.h>
#include <cgv/gui/provider.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

///
class CGV_API base_provider : public cgv::base::base, public provider
{
protected:
	std::string parent_type, parent_options;
	cgv::base::base_ptr instance;
	std::string textual_gui_definition;
	bool is_named_gui_assignment_m;
	unsigned nr_toggles;
	bool* toggles;
	enum ParsingTasks { PT_NR_TOGGLES, PT_INIT_TOGGLES, PT_CREATE_GUI };
	void parse_definition(ParsingTasks pt);
	std::string error_start(const char* ptr) const;
	bool find_member(const std::string& name, void*& member_ptr, std::string& member_type);
public:
	/// construct from instance and gui definition
	base_provider(cgv::base::base_ptr _instance = cgv::base::base_ptr(), 
		          const std::string& gui_def = "", bool _is_named_gui_assignment = false);
	/// construct from gui definition file and instance
	base_provider(const std::string& file_name, 
				  cgv::base::base_ptr _instance = cgv::base::base_ptr());
	/// destruct base provider
	~base_provider();
	///
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	///
	std::string get_parent_type() const;
	/// overload to return the type name of this object. By default the type interface is queried over get_type.
	std::string get_type_name() const;
	///
	data::ref_ptr<cgv::base::named,true> get_named();
	///
	void read_gui_definition(const std::string& file_name);
	///
	void set_gui_definition(const std::string& new_def);
	///
	const std::string& get_gui_definition() const;
	///
	bool is_named_gui_assignment() const { return is_named_gui_assignment_m; }
	///
	void set_named_gui_assignment(bool value = true) { is_named_gui_assignment_m = value; }
	///
	void set_instance(cgv::base::base_ptr _instance);
	///
	cgv::base::base_ptr get_instance() const;
	///
	void create_gui();
};

typedef cgv::data::ref_ptr<base_provider> base_provider_ptr;

	}
}

#include <cgv/config/lib_end.h>