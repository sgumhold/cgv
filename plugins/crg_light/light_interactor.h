#pragma once

#include <cgv/render/drawable.h>
#include <cgv/media/illum/light_source.hh>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <random>

#include "lib_begin.h"

using namespace cgv::reflect;
using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::media::illum;

/// interaction class for light sources
class CGV_API light_interactor : public node, public event_handler, public provider, public drawable
{
public:
	typedef cgv::media::color<float> color_type;
	typedef std::pair<vec3, vec3> ray;
private:
	int current_light_index;
	std::default_random_engine RE;
protected:
	std::vector<light_source> lights;
	std::vector<void*> handles;
	std::vector<void*> new_handles;
	std::vector<float> intensities;
	std::vector<int> enabled;
	std::vector<int> show;
	std::vector<int> toggles;
	float light_scale;
	std::string file_name;
	void save_cb();
	void load_cb();
	void on_load();
	vec3 delta_pos;
	float speed;
	unsigned nr_light_rays;
	cgv::render::view* view_ptr;
	std::vector<vec3> light_rays;
	std::vector<int> current_ray_indices;
	float color_lambda;
	rgb default_color;
	dmat4 last_modelview_matrix;
	float ray_width;
	float opacity;
	void sample_light_rays(context& ctx, unsigned decrease_count = 0);
	void draw_light_rays(context& ctx, size_t i);
	void timer_event(double t, double dt);
public:
	/// construct default light interactor
	light_interactor();
	/// hide all lights
	void hide_all();
	/// show all lights
	void show_all();
	/**@name self reflection*/
	//@{
	/// return type name
	std::string get_type_name() const;
	/// generate callbacks
	void on_set(void* member_ptr);
	/// do self reflection
	bool self_reflect(reflection_handler& rh);
	/// save light_interactor to file
	bool save(const std::string& file_name)  const;
	/// read light_interactor from file
	bool load(const std::string& file_name);
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
	/// correct default render flags
	void clear(context&);
	//@}

	/// gui 
	void create_gui();
	bool handle(event& e);
	void stream_help(std::ostream& os);
};

typedef cgv::data::ref_ptr<light_interactor> light_interactor_ptr;

#include <cgv/config/lib_end.h>