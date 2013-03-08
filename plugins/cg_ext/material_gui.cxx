#include <cgv/gui/provider.h>

#include <cgv/media/illum/obj_material.hh>

using namespace cgv::media::illum;
using namespace cgv::signal;

namespace cgv { 
	namespace gui {

struct phong_material_gui_creator : public gui_creator
{
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool* toggles)
	{
		bool is_phong   = value_type == cgv::type::info::type_name<phong_material>::get_name();
		bool is_obj_mat = value_type == cgv::type::info::type_name<obj_material>::get_name();
		if (!is_phong && !is_obj_mat)
			return false;
		obj_material* obj_mat = 0;
		phong_material* phong_mat = 0;
		if (is_obj_mat) {
			obj_mat = reinterpret_cast<obj_material*>(value_ptr);
			phong_mat = static_cast<phong_material*>(obj_mat);
		}
		else
			phong_mat = reinterpret_cast<phong_material*>(value_ptr);

		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
		if (obj_mat)
			p->add_member_control(b, "opacity",   obj_mat->ref_opacity(), "value_slider", "min=0;max=1;ticks=true");
		p->add_member_control(b, "ambient",   phong_mat->ref_ambient(), "color<float,RGBA>");
		p->add_member_control(b, "diffuse",   phong_mat->ref_diffuse(), "color<float,RGBA>");
		p->add_member_control(b, "specular",  phong_mat->ref_specular(), "color<float,RGBA>");
		p->add_member_control(b, "emission",  phong_mat->ref_emission(), "color<float,RGBA>");
		p->add_member_control(b, "shininess", phong_mat->ref_shininess(), "value_slider", "min=0;max=128;ticks=true");
		if (obj_mat) {
			p->add_view("diffuse map", obj_mat->get_diffuse_texture_name());
			p->add_view("bump map", obj_mat->get_bump_texture_name());
			p->add_member_control(b, "bump_scale", obj_mat->ref_bump_scale(), "value_slider", "min=0;max=1;log=true;ticks=true");
		}
		return true;
	}
};
#include "lib_begin.h"

extern CGV_API cgv::gui::gui_creator_registration<phong_material_gui_creator> material_gc_reg("material_gui_creator");


	}
}