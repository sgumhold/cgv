#pragma once

#include <string>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

class provider;

/// interface for gui creators
struct gui_creator
{
	/// attempt to create a gui and return whether this was successful
	virtual bool create(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool* toggles) = 0;
};

/// register a gui creator
extern CGV_API void register_gui_creator(gui_creator* gc, const char* creator_name);

/// create the gui for a composed structure
extern CGV_API bool create_gui(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type = "", const std::string& options = "", bool* toggles = 0);

/// helper template for registration of gui creators
template <class T>
class gui_creator_registration 
{
public:
	gui_creator_registration(const char* creator_name) {
		register_gui_creator(new T(), creator_name);
	}
};


	}
}

#include <cgv/config/lib_end.h>