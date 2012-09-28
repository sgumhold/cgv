#include "fltk_driver_registry.h"

std::vector<abst_decorator_factory*>& ref_decorator_factories()
{
	static std::vector<abst_decorator_factory*> factories;
	return factories;
}

void register_decorator_factory(abst_decorator_factory* fac)
{
	ref_decorator_factories().push_back(fac);
}

base_ptr create_decorator(const std::string& label,
						  const std::string& gui_type, int x, int y, int w, int h)
{
	for (unsigned int i=0; i<ref_decorator_factories().size(); ++i) {
		base_ptr d = ref_decorator_factories()[i]->create(label, 
									gui_type, x, y, w, h);
		if (!d.empty())
			return d;
	}
	return base_ptr();
}


std::vector<abst_view_factory*>& ref_view_factories()
{
	static std::vector<abst_view_factory*> factories;
	return factories;
}

void register_view_factory(abst_view_factory* fac)
{
	ref_view_factories().push_back(fac);
}

view_ptr create_view(const std::string& label,
							const void* value_ptr, const std::string& value_type, 
							const std::string& gui_type, int x, int y, int w, int h)
{
	for (unsigned int i=0; i<ref_view_factories().size(); ++i) {
		view_ptr v = ref_view_factories()[i]->create(label, 
									value_ptr, value_type, gui_type, x, y, w, h);
		if (!v.empty())
			return v;
	}
	return view_ptr();
}

std::vector<abst_control_factory*>& ref_control_factories()
{
	static std::vector<abst_control_factory*> factories;
	return factories;
}

void register_control_factory(abst_control_factory* fac)
{
	ref_control_factories().push_back(fac);
}

control_ptr create_control(const std::string& label, 
									void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
									const std::string& gui_type, int x, int y, int w, int h)
{
	for (unsigned int i=0; i<ref_control_factories().size(); ++i) {
		control_ptr c = ref_control_factories()[i]->create(label, 
			value_ptr, acp, value_type, gui_type, x, y, w, h);
		if (!c.empty())
			return c;
	}
	return control_ptr();
}
