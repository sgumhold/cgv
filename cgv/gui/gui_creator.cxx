#include "gui_creator.h"
#include <vector>


namespace cgv {
	namespace gui {

std::vector<gui_creator*>& ref_gui_creators()
{
	static std::vector<gui_creator*> creators;
	return creators;
}

/// register a gui creator
void register_gui_creator(gui_creator* gc, const char* creator_name)
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

	}
}