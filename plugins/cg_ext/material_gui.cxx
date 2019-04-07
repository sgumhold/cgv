#include <cgv/gui/provider.h>

#include <cgv/media/illum/obj_material.hh>
#include <cgv/media/illum/textured_surface_material.h>

using namespace cgv::media::illum;
using namespace cgv::signal;

namespace cgv { 
	namespace gui {

struct phong_material_gui_creator : public gui_creator
{
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool*)
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


struct surface_material_gui_creator : public gui_creator
{
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label,
		void* value_ptr, const std::string& value_type,
		const std::string& gui_type, const std::string& options, bool*)
	{
		bool is_surface  = value_type == cgv::type::info::type_name<surface_material>::get_name();
		bool is_textured = value_type == cgv::type::info::type_name<textured_surface_material>::get_name();
		if (!is_surface && !is_textured)
			return false;

		textured_surface_material* textured_mat = 0;
		surface_material* surface_mat = 0;
		if (is_textured) {
			textured_mat = reinterpret_cast<textured_surface_material*>(value_ptr);
			surface_mat  = static_cast<surface_material*>(textured_mat);
		}
		else
			surface_mat = reinterpret_cast<surface_material*>(value_ptr);

		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
		p->add_member_control(b, "brdf_type", surface_mat->ref_brdf_type(), "dropdown", "enums='Lambert,OrenNayar,Strauss,undef,\
Lambert+Phong,OrenNayar+Phong,Strauss+Phong,+Phong,\
Lambert+Blinn,OrenNayar+Blinn,Strauss+Blinn,+Blinn,\
Lambert+CookTorrance,OrenNayar+CookTorrance,Strauss+CookTorrance,+CookTorrance,\
Lambert+Strauss,OrenNayar+Strauss,Strauss+Strauss,+Strauss'");
		p->add_member_control(b, "diffuse_reflectance", surface_mat->ref_diffuse_reflectance(), "color<float,RGB>");
		p->add_member_control(b, "roughness", surface_mat->ref_roughness(), "value_slider", "min=0;max=1;ticks=true");
		p->add_member_control(b, "metalness", surface_mat->ref_metalness(), "value_slider", "min=0;max=1;ticks=true");
		p->add_member_control(b, "ambient_occlusion", surface_mat->ref_ambient_occlusion(), "value_slider", "min=0;max=1;ticks=true");
		p->add_member_control(b, "emission", surface_mat->ref_emission(), "color<float,RGB>");
		p->add_member_control(b, "transparency", surface_mat->ref_transparency(), "value_slider", "min=0;max=1;ticks=true");
		p->add_member_control(b, "propagation_slow_down_re", reinterpret_cast<float(&)[2]>(surface_mat->ref_propagation_slow_down())[0], "value_slider", "min=1;max=10;ticks=true;log=true");
		p->add_member_control(b, "propagation_slow_down_im", reinterpret_cast<float(&)[2]>(surface_mat->ref_propagation_slow_down())[1], "value_slider", "min=0;max=10;ticks=true;log=true");
		p->add_member_control(b, "roughness_anisotropy", surface_mat->ref_roughness_anisotropy(), "value_slider", "min=0;max=1;ticks=true");
		p->add_member_control(b, "roughness_orientation", surface_mat->ref_roughness_orientation(), "value_slider", "min=0;max=1;ticks=true");
		p->add_member_control(b, "specular_reflectance", surface_mat->ref_specular_reflectance(), "color<float,RGB>");
		if (textured_mat) {
			for (unsigned i=0; i<textured_mat->get_nr_image_files(); ++i)
				p->add_view("image", textured_mat->get_image_file_name(i));
			p->add_member_control(b, "sRGB textures", textured_mat->ref_sRGBA_textures(), "check");
			std::string opt = "min=-1;max=";
			opt += cgv::utils::to_string((int)textured_mat->get_nr_textures() - 1);
			opt += ";ticks=true";
			p->add_member_control(b, "diffuse_index", textured_mat->ref_diffuse_index(), "value_slider", opt);
			p->add_member_control(b, "roughness_index", textured_mat->ref_roughness_index(), "value_slider", opt);
			p->add_member_control(b, "metalness_index", textured_mat->ref_metalness_index(), "value_slider", opt);
			p->add_member_control(b, "ambient_index", textured_mat->ref_ambient_index(), "value_slider", opt);
			p->add_member_control(b, "emission_index", textured_mat->ref_emission_index(), "value_slider", opt);
			p->add_member_control(b, "transparency_index", textured_mat->ref_transparency_index(), "value_slider", opt);
//			p->add_member_control(b, "propagation_slow_down_index", textured_mat->ref_propagation_slow_down(), "value_slider", opt);
			p->add_member_control(b, "specular_index", textured_mat->ref_specular_index(), "value_slider", opt);
			p->add_member_control(b, "normal_index", textured_mat->ref_normal_index(), "value_slider", opt);
			p->add_member_control(b, "bump_index", textured_mat->ref_bump_index(), "value_slider", opt);
			p->add_member_control(b, "bump_scale", textured_mat->ref_bump_scale(), "value_slider", "min=0;max=10;log=true;ticks=true");
		}
		return true;
	}
};
#include "lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<phong_material_gui_creator> phong_material_gc_reg("phong_material_gui_creator");
CGV_API cgv::gui::gui_creator_registration<surface_material_gui_creator> surface_material_gc_reg("surface_material_gui_creator");


	}
}