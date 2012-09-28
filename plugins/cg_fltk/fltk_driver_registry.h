#pragma once

#include <cgv/gui/gui_driver.h>

using namespace cgv::gui;

#include "lib_begin.h"

/// factory used to register decorators
struct CGV_API abst_decorator_factory
{
	virtual base_ptr create(const std::string& label, 
		const std::string& gui_type, int x, int y, int w, int h) = 0;
};

/// factory used to register views
struct CGV_API abst_view_factory
{
	virtual view_ptr create(const std::string& label, 
		const void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, int x, int y, int w, int h) = 0;
};

/// factory used to register controls
struct CGV_API abst_control_factory
{
	virtual control_ptr create(const std::string& label, 
		void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
		const std::string& gui_type, int x, int y, int w, int h) = 0;
};

extern CGV_API void register_decorator_factory(abst_decorator_factory* fac);
extern CGV_API void register_view_factory(abst_view_factory* fac);
extern CGV_API void register_control_factory(abst_control_factory* fac);

extern CGV_API base_ptr create_decorator(const std::string& label, 
						const std::string& gui_type, int x, int y, int w, int h);

extern CGV_API view_ptr create_view(const std::string& label, 
						const void* value_ptr, const std::string& value_type, 
						const std::string& gui_type, int x, int y, int w, int h);

extern CGV_API control_ptr create_control(const std::string& label, 
						void* value_ptr, abst_control_provider* acp, const std::string& value_type, 
						const std::string& gui_type, int x, int y, int w, int h);

template <class T>
class decorator_factory_registration 
{
public:
	decorator_factory_registration() {
		register_decorator_factory(new T());
	}
};

template <class T>
class view_factory_registration 
{
public:
	view_factory_registration() {
		register_view_factory(new T());
	}
};

template <class T>
class control_factory_registration 
{
public:
	control_factory_registration() {
		register_control_factory(new T());
	}
};

#include <cgv/config/lib_end.h>