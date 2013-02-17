#pragma once

#include <cgv/render/drawable.h>
#include <cgv/media/illum/light_source.hh>
#include <cgv/gui/provider.h>

#include "lib_begin.h"

using namespace cgv::reflect;
using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::media::illum;

/// interaction class for light sources
class CGV_API light_interactor : public base, public provider, public drawable
{
public:
	typedef cgv::media::color<float> color_type;
protected:
	std::vector<light_source> lights;
	std::vector<void*> handles;
	std::vector<float> intensities;
	std::vector<int> enabled;
	std::vector<int> show;
	std::vector<int> toggles;
	float light_scale;
	std::string file_name;
	void save_cb();
	void load();
	void load_cb();
public:
	/// construct default light interactor
	light_interactor();
	/**@name self reflection*/
	//@{
	/// return type name
	std::string get_type_name() const;
	/// generate callbacks
	void on_set(void* member_ptr);
	/// do self reflection
	bool self_reflect(reflection_handler& rh);
	//@}

	/**@name rendering*/
	//@{
	///
	bool init(context&);
	/// activate light sources
	void init_frame(context&);
	/// draw light sources
	void draw(context&);
	/// deactivate light sources
	void finish_frame(context&);
	//@}

	/// gui creation
	void create_gui();
};

typedef cgv::data::ref_ptr<light_interactor> light_interactor_ptr;

#include <cgv/config/lib_end.h>