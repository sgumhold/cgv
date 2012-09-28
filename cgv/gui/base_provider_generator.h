#pragma once

#include <cgv/base/register.h>
#include <string>
#include "base_provider.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {

///
		class CGV_API base_provider_generator : public cgv::base::base, public cgv::signal::tacker, public cgv::base::registration_listener
{
protected:
	/// each gui definition consists of the textual definition as well as an options string
	struct gui_definition {
		std::string definition;
		std::string options;
		gui_definition(const std::string& _def="", const std::string& _opt="") : definition(_def), options(_opt) {}
	};
	/// type of mapping from strings to gui definitions
	typedef std::map<std::string,gui_definition> def_map_type;
	/// iterator type for map
	typedef def_map_type::const_iterator def_map_iter;
	/// type of map from objects to base_providers
	typedef std::map<cgv::base::base_ptr,base_provider_ptr> pvd_map_type;
	/// iterator type of base_provider map
	typedef pvd_map_type::iterator pvd_map_iter;
	/// mappings from type to gui definitions
	def_map_type defs_by_type;
	/// mappings from name to gui definitions
	def_map_type defs_by_name;
	/// keep track of unmatched objects
	std::vector<cgv::base::base_ptr> unmatched_objects;
	/// store map to base_providers
	pvd_map_type providers;
	/// check whether gui description is available for object and if yes generate a base_provider
	bool generate_object_gui(base_ptr object);
	/// store read gui files with last write times
	std::map<std::string,long long> gui_files;
	/// whether to check files
	bool check_file_update;
	///
	void timer_event(double,double);
public:
	/// construct from instance and gui definition
	base_provider_generator();
	///
	std::string get_type_name() const;
	/// 
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	/// 
	void on_set(void* member_ptr);
	/// parse file and extract gui definitions
	bool parse_gui_file(const std::string& file_name);
	/// if object is registered that does not provide its own gui but matches type or name of a parsed gui definition, register a newly created base_provider for the object
	void register_object(cgv::base::base_ptr object, const std::string& options);
	/// remove also the base_provider of an object if created
	void unregister_object(cgv::base::base_ptr object, const std::string& options);
};

	}
}

#include <cgv/config/lib_end.h>