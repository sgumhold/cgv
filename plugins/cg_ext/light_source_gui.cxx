#include <cgv/gui/provider.h>
#include <cgv/media/illum/light_source.hh>

using namespace cgv::media::illum;
using namespace cgv::signal;

namespace cgv { 
	namespace gui {

void type_change_cb(provider* p, light_source* light)
{
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
		p->add_member_control(b, "local_to_eye", light.ref_local_to_eye(), "toggle");
		p->add_member_control(b, "emission", light.ref_emission());
		p->add_member_control(b, "ambient_scale", light.ref_ambient_scale(), "value_slider", "min=0;max=1;ticks=true");
		if (light.get_type() == LT_DIRECTIONAL) {
			p->add_gui("direction", light.ref_position(), "direction");
		}
		else {
			p->add_gui("position", light.ref_position(), "vector", "min=-10;max=10;ticks=true;log=true");
		}
		if(p->begin_tree_node("Attenuation", light.ref_constant_attenuation())){
			p->align("\a");
				p->add_member_control(b, "constant", light.ref_constant_attenuation(), "value_slider", "step=0.01;min=0;max=1;ticks=true");
				p->add_member_control(b, "linear", light.ref_linear_attenuation(), "value_slider", "step=0.01;min=0;max=1;ticks=true");
				p->add_member_control(b, "quadratic", light.ref_quadratic_attenuation(), "value_slider", "step=0.01;min=0;max=1;ticks=true");
			p->align("\b");
			p->end_tree_node(light.ref_constant_attenuation());
		}
		if(p->begin_tree_node("Spot Parameters", light.ref_spot_exponent())){
			p->align("\a");
				p->add_member_control(b, "exponent", light.ref_spot_exponent(), "value_slider", "step=0.01;min=0;max=128;ticks=true;log=true");
				p->add_member_control(b, "cutoff angle", light.ref_spot_cutoff(), "value_slider", "step=0.01;min=0;max=90;ticks=true");
				p->add_gui("direction", light.ref_spot_direction(), "direction");
			p->align("\b");
			p->end_tree_node(light.ref_spot_exponent());
		}
		return true;
	}
};

#include "lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<light_source_gui_creator> light_source_gc_reg("light_source_gui_creator");


	}
}