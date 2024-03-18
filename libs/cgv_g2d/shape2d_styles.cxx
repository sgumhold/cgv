#include "shape2d_styles.h"

#include <cgv/gui/provider.h>

namespace cgv {
namespace gui {

/// define a gui creator for the shape2d style struct
struct shape2d_style_gui_creator : public gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, bool*) {
		if(value_type != cgv::type::info::type_name<cgv::g2d::shape2d_style>::get_name())
			return false;

		cgv::g2d::shape2d_style* s_ptr = reinterpret_cast<cgv::g2d::shape2d_style*>(value_ptr);
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

		p->add_decorator("Render Options", "heading", "level=3");
		p->add_member_control(b, "Use Fill Color", s_ptr->use_fill_color, "check");
		p->add_member_control(b, "Use Texture", s_ptr->use_texture, "check");
		p->add_member_control(b, "Use Texture Alpha", s_ptr->use_texture_alpha, "check");
		p->add_member_control(b, "Use Blending", s_ptr->use_blending, "check");
		p->add_member_control(b, "Use Smooth Feather", s_ptr->use_smooth_feather, "check");

		p->add_decorator("Appearance", "heading", "level=3");
		p->add_member_control(b, "Fill Color", s_ptr->fill_color, "");
		p->add_member_control(b, "Border Color", s_ptr->border_color, "");
		p->add_member_control(b, "Border Width", s_ptr->border_width, "value_slider", "min=0;max=20;step=0.5;ticks=true");
		p->add_member_control(b, "Border Radius", s_ptr->border_radius, "value_slider", "min=0;max=20;step=0.5;ticks=true");
		p->add_member_control(b, "Ring Width", s_ptr->ring_width, "value_slider", "min=0;max=20;step=0.5;ticks=true");
		p->add_member_control(b, "Feather Width", s_ptr->feather_width, "value_slider", "min=0;max=20;step=0.5;ticks=true");
		p->add_member_control(b, "Feather Origin", s_ptr->feather_origin, "value_slider", "min=0;max=1;step=0.01;ticks=true");

		p->add_member_control(b, "Texcoord Offset", s_ptr->texcoord_offset[0], "value", "min=-2;max=2;step=0.01;ticks=true;w=95", " ");
		p->add_member_control(b, "", s_ptr->texcoord_offset[1], "value", "min=-2;max=2;step=0.01;ticks=true;w=95");
		p->add_member_control(b, "Texcoord Scaling", s_ptr->texcoord_scaling[0], "value", "min=-2;max=2;step=0.01;ticks=true;w=95", " ");
		p->add_member_control(b, "", s_ptr->texcoord_scaling[1], "value", "min=-2;max=2;step=0.01;ticks=true;w=95");

		return true;
	}
};

/// define a gui creator for the circle2d style struct
struct circle2d_style_gui_creator : public gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, bool*) {
		if(value_type != cgv::type::info::type_name<cgv::g2d::circle2d_style>::get_name())
			return false;

		cgv::g2d::circle2d_style* s_ptr = reinterpret_cast<cgv::g2d::circle2d_style*>(value_ptr);
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

		p->add_gui("shape2d_style", *static_cast<cgv::g2d::shape2d_style*>(s_ptr));
		p->add_member_control(b, "Polar Texture Coordinates", s_ptr->use_polar_texcoords, "check");

		return true;
	}
};

/// define a gui creator for the ring2d style struct
struct ring2d_style_gui_creator : public gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, bool*) {
		if(value_type != cgv::type::info::type_name<cgv::g2d::ring2d_style>::get_name())
			return false;

		cgv::g2d::ring2d_style* s_ptr = reinterpret_cast<cgv::g2d::ring2d_style*>(value_ptr);
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

		p->add_gui("shape2d_style", *static_cast<cgv::g2d::shape2d_style*>(s_ptr));
		p->add_member_control(b, "Thickness", s_ptr->thickness, "value_slider", "min=0;max=1;step=0.01;ticks=true");

		return true;
	}
};

/// define a gui creator for the line2d style struct
struct line2d_style_gui_creator : public gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, bool*) {
		if(value_type != cgv::type::info::type_name<cgv::g2d::line2d_style>::get_name())
			return false;

		cgv::g2d::line2d_style* s_ptr = reinterpret_cast<cgv::g2d::line2d_style*>(value_ptr);
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

		p->add_gui("shape2d_style", *static_cast<cgv::g2d::shape2d_style*>(s_ptr));
		p->add_member_control(b, "Width", s_ptr->width, "value_slider", "min=0;max=40;step=0.5;ticks=true");
		p->add_member_control(b, "Dash Length", s_ptr->dash_length, "value_slider", "min=0;max=100;step=0.5;ticks=true");
		p->add_member_control(b, "Dash Ratio", s_ptr->dash_ratio, "value_slider", "min=0;max=1;step=0.01;ticks=true");

		return true;
	}
};

/// define a gui creator for the arrow2d style struct
struct arrow2d_style_gui_creator : public gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, bool*) {
		if(value_type != cgv::type::info::type_name<cgv::g2d::arrow2d_style>::get_name())
			return false;

		cgv::g2d::arrow2d_style* s_ptr = reinterpret_cast<cgv::g2d::arrow2d_style*>(value_ptr);
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

		p->add_gui("shape2d_style", *static_cast<cgv::g2d::shape2d_style*>(s_ptr));
		p->add_member_control(b, "Stem Width", s_ptr->stem_width, "value_slider", "min=0;max=100;step=0.5;ticks=true");
		p->add_member_control(b, "Head Width", s_ptr->head_width, "value_slider", "min=0;max=100;step=0.5;ticks=true");
		p->add_member_control(b, "Absolute Head Length", s_ptr->absolute_head_length, "value_slider", "min=0;max=200;step=0.5;ticks=true");
		p->add_member_control(b, "Relative Head Length", s_ptr->relative_head_length, "value_slider", "min=0;max=1;step=0.01;ticks=true");
		p->add_member_control(b, "Head Length is Relative", s_ptr->head_length_is_relative, "check");

		return true;
	}
};

/// define a gui creator for the grid2d style struct
struct grid2d_style_gui_creator : public gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, bool*) {
		if(value_type != cgv::type::info::type_name<cgv::g2d::grid2d_style>::get_name())
			return false;

		cgv::g2d::grid2d_style* s_ptr = reinterpret_cast<cgv::g2d::grid2d_style*>(value_ptr);
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

		p->add_gui("shape2d_style", *static_cast<cgv::g2d::shape2d_style*>(s_ptr));
		p->add_member_control(b, "Pattern", s_ptr->pattern, "dropdown", "enums='Grid,Squares,Checker'");
		p->add_member_control(b, "Scale", s_ptr->scale, "value_slider", "min=0;max=1;step=0.001;ticks=true");

		return true;
	}
};

/// define a gui creator for the text2d style struct
struct text2d_style_gui_creator : public gui_creator {
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, bool*) {
		if(value_type != cgv::type::info::type_name<cgv::g2d::text2d_style>::get_name())
			return false;

		cgv::g2d::text2d_style* s_ptr = reinterpret_cast<cgv::g2d::text2d_style*>(value_ptr);
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

		p->add_gui("shape2d_style", *static_cast<cgv::g2d::shape2d_style*>(s_ptr));
		p->add_member_control(b, "Font Size", s_ptr->font_size, "value_slider", "min=1;max=256;step=0.5;ticks=true");

		return true;
	}
};

#include "lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<shape2d_style_gui_creator> shape2d_s_gc_reg("shape2d_style_gui_creator");
CGV_API cgv::gui::gui_creator_registration<circle2d_style_gui_creator> circle2d_s_gc_reg("circle2d_style_gui_creator");
CGV_API cgv::gui::gui_creator_registration<ring2d_style_gui_creator> ring2d_s_gc_reg("ring2d_style_gui_creator");
CGV_API cgv::gui::gui_creator_registration<line2d_style_gui_creator> line2d_s_gc_reg("line2d_style_gui_creator");
CGV_API cgv::gui::gui_creator_registration<arrow2d_style_gui_creator> arrow2d_s_gc_reg("arrow2d_style_gui_creator");
CGV_API cgv::gui::gui_creator_registration<grid2d_style_gui_creator> grid2d_s_gc_reg("grid2d_style_gui_creator");
CGV_API cgv::gui::gui_creator_registration<text2d_style_gui_creator> text2d_s_gc_reg("text2d_style_gui_creator");

}
}
