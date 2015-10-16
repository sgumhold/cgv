#include <cgv/gui/provider.h>
#include <cgv/media/illum/light_source.hh>

using namespace cgv::media::illum;
using namespace cgv::signal;

namespace cgv { 
	namespace gui {

void type_change_cb(provider* p, light_source* light)
{
	if (light->get_type() == LT_DIRECTIONAL)
		light->ref_location()(3) = 0;
	else if (light->ref_location()(3) == 0)
		light->ref_location()(3) = 1;
	p->post_recreate_gui();
	cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
	if (b)
		b->on_set(&light->ref_type());
}

struct light_source_gui_creator : public gui_creator
{
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool*)
	{
		if (value_type != cgv::type::info::type_name<light_source>::get_name())
			return false;
		light_source& light = *((light_source*) value_ptr);
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

		connect_copy(p->add_control("type", light.ref_type(), "dropdown", "enums='directional,point,spot'")->value_change,
			rebind(type_change_cb, p, &light));
		connect_copy(p->add_control("eye", light.ref_local_to_eye(), "toggle")->value_change,
			rebind(b, &cgv::base::base::on_set, &light.ref_local_to_eye()));

		if(p->begin_tree_node("Colors", light.ref_ambient())){
			p->align("\a");
				connect_copy(p->add_control("ambient",light.ref_ambient())->value_change,rebind(b, &cgv::base::base::on_set, &light.ref_ambient()));
				connect_copy(p->add_control("diffuse",light.ref_diffuse())->value_change,rebind(b, &cgv::base::base::on_set, &light.ref_diffuse()));
				connect_copy(p->add_control("specular",light.ref_specular())->value_change,rebind(b, &cgv::base::base::on_set, &light.ref_specular()));
			p->align("\b");
			p->end_tree_node(light.ref_ambient());
		}
		if (light.get_type() == LT_DIRECTIONAL) {
			if(p->begin_tree_node("Direction", light.ref_location())){
				p->align("\a");
					p->add_gui("direction", (light_source::vec_type&)light.ref_location(), "direction");
				p->align("\b");
				p->end_tree_node(light.ref_location());
			}
		}
		else {
			if(p->begin_tree_node("Position", light.ref_location())){
				p->align("\a");
					p->add_gui("position", light.ref_location(), "vector", "min=-1;max=1;ticks=true;step=0.001");
					p->find_control(light.ref_location()(3))->multi_set("min=0;step=0.0001;log=true", false);
					connect_copy(p->find_control(light.ref_location()(3))->value_change,
						rebind(b, &cgv::base::base::on_set, &light.ref_location()(3)));
				p->align("\b");
				p->end_tree_node(light.ref_location());
			}
		}
		if(p->begin_tree_node("Attenuation", light.ref_attenuation())){
			p->align("\a");
				connect_copy(p->add_control("constant", light.ref_attenuation()(0), "value_slider", "step=0.01;min=0;max=1;ticks=true")->value_change,
					rebind(b, &cgv::base::base::on_set, &light.ref_attenuation()(0)));
				connect_copy(p->add_control("linear", light.ref_attenuation()(1), "value_slider", "step=0.01;min=0;max=1;ticks=true")->value_change,
					rebind(b, &cgv::base::base::on_set, &light.ref_attenuation()(1)));
				connect_copy(p->add_control("quadratic", light.ref_attenuation()(2), "value_slider", "step=0.01;min=0;max=1;ticks=true")->value_change,
					rebind(b, &cgv::base::base::on_set, &light.ref_attenuation()(2)));
			p->align("\b");
			p->end_tree_node(light.ref_attenuation());
		}
		if(p->begin_tree_node("Spot Parameters", light.ref_spot_exponent())){
			p->align("\a");
				connect_copy(p->add_control("exponent", light.ref_spot_exponent(), "value_slider", "step=0.01;min=0;max=128;ticks=true;log=true")->value_change,
					rebind(b, &cgv::base::base::on_set, &light.ref_spot_exponent()));
				connect_copy(p->add_control("cutoff angle", light.ref_spot_cutoff(), "value_slider", "step=0.01;min=0;max=90;ticks=true")->value_change,
					rebind(b, &cgv::base::base::on_set, &light.ref_spot_cutoff()));
				p->add_gui("direction", light.ref_spot_direction(), "direction");
			p->align("\b");
			p->end_tree_node(light.ref_spot_exponent());
		}
		return true;
	}
};

#include "lib_begin.h"

extern CGV_API cgv::gui::gui_creator_registration<light_source_gui_creator> light_source_gc_reg("light_source_gui_creator");


	}
}