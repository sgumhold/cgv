#define _USE_MATH_DEFINES
#include "bump_mapper.h"
#include <cgv/signal/rebind.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv/base/register.h>
#include <cgv/data/data_view.h>

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::data;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;

bump_mapper::bump_mapper(unsigned _texture_resolution) : 
	node("bump mapper"), 
	texture_resolution(_texture_resolution),
	bump_map("[L]", TF_LINEAR, TF_LINEAR_MIPMAP_LINEAR, TW_REPEAT, TW_REPEAT)
{
	surface_primitive = TORUS;
	minor_radius = 0.5f;
	surface_resolution = 50;
	bump_scale = 0.1f;
	wire_frame = false;
	use_bump_map = true;
	use_diffuse_map = true;
	texture_frequency = 50;
	texture_frequency_aspect = 1;
	texture_scale = 1;
	texture_aspect = 1;
	texture_rotation = 0;
	texture_u_offset = 0;
	texture_v_offset = 0;
	texture_selection = ALHAMBRA;
}
/// add custom texture to material
bool bump_mapper::init(cgv::render::context& ctx)
{
	tex_index = material.add_texture_reference(bump_map);
	material.set_diffuse_index(tex_index);
	material.set_bump_index(tex_index);
	return true;
}

void bump_mapper::init_frame(context& ctx)
{
	if (bump_map.is_created())
		return;

	if (texture_selection == ALHAMBRA) {
		if (!bump_map.create_from_image(ctx,"res://alhambra.png", 
			(int*)&texture_resolution)) {
				std::cout << "could not read" << std::endl ;
				exit(0);
		}
		update_member(&texture_resolution);
		return;
	}
	
	if (texture_selection == CARTUJA) {
		bump_map.create_from_image(ctx,"res://cartuja.png", 
			                           (int*)&texture_resolution);
		update_member(&texture_resolution);
		return;
	}
	
	data_format df(texture_resolution,texture_resolution,TI_FLT32,CF_L);
	data_view dv(&df);
	unsigned i,j;
	float* ptr = (float*)dv.get_ptr<unsigned char>();
	for (i=0; i<texture_resolution; ++i)
		for (j=0; j<texture_resolution; ++j)
			if (texture_selection == CHECKER)
				ptr[i*texture_resolution+j] = (float)(((i/8)&1) ^((j/8)&1));
			else
				ptr[i*texture_resolution+j] = 
				   (float)(0.5*(pow(cos(M_PI*texture_frequency/texture_frequency_aspect*i/(texture_resolution-1)),3)*
				               sin(M_PI*texture_frequency*j/(texture_resolution-1))+1));
	bump_map.create(ctx, dv);
}

void bump_mapper::draw(context& ctx)
{
	cgv::render::shader_program& prog = ctx.ref_surface_shader_program(true);
	prog.enable(ctx);
		ctx.enable_material(material);
			if (!use_bump_map)
				prog.set_uniform(ctx, "bump_index", -1);
			if (!use_diffuse_map)
				prog.set_uniform(ctx, "diffuse_index", -1);
			prog.set_uniform(ctx, "bump_scale", bump_scale);

			if (wire_frame) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glDisable(GL_CULL_FACE);
			}
			switch (surface_primitive) {
			case SQUARE : ctx.tesselate_unit_square(); break;
			case CUBE : ctx.tesselate_unit_cube(); break;
			case SPHERE : ctx.tesselate_unit_sphere(surface_resolution); break;
			case TORUS : ctx.tesselate_unit_torus(minor_radius,surface_resolution); break;
			}
			if (wire_frame) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_CULL_FACE);
			}
		ctx.disable_material(material);
	prog.disable(ctx);
}

void bump_mapper::clear(context& ctx)
{
	bump_map.destruct(ctx);
}

///
void bump_mapper::on_set(void* member_ptr)
{
	if ((member_ptr == &texture_selection) ||
		(member_ptr == &texture_frequency) ||
		(member_ptr == &texture_frequency_aspect) ||
		(member_ptr == &texture_resolution)) {

		cgv::render::context* ctx_ptr = get_context();
		if (ctx_ptr) {
			ctx_ptr->make_current();
			bump_map.destruct(*ctx_ptr);
		}
	}
	update_member(member_ptr);
	post_redraw();
}

/// you must overload this for gui creation
void bump_mapper::create_gui() 
{	
	add_decorator("Surface Properties", "heading");
	add_member_control(this, "primitive", surface_primitive, "dropdown", "enums='square,cube,sphere,torus'");
	add_member_control(this, "minor radius", minor_radius, "value_slider", "min=0.05;max=1;log=true;ticks=true");
	add_member_control(this, "resolution", surface_resolution, "value_slider", "min=4;max=100;log=true;ticks=true");

	add_decorator("Bump Map Properties", "heading");
	add_member_control(this, "wire_frame", wire_frame, "check");
	add_member_control(this, "use_diffuse_map", use_diffuse_map, "check");
	add_member_control(this, "use_bump_map", use_bump_map, "check");
	add_member_control(this, "bump_scale", bump_scale, "value_slider", "min=0.0001;max=1;log=true;ticks=true");

	add_decorator("Texture Properties", "heading");
	add_member_control(this, "texture", texture_selection, "dropdown", "enums='checker,waves,alhambra,cartuja'");
	add_member_control(this, "frequency", texture_frequency, "value_slider", "min=0;max=200;log=true;ticks=true");
	add_member_control(this, "frequency aspect", texture_frequency_aspect, "value_slider", "min=0.1;max=10;log=true;ticks=true");
	add_member_control(this, "texture resolution", texture_resolution, "value_slider", "min=4;max=1024;log=true;ticks=true");

	add_decorator("Texture Transformation", "heading", "level=2");
	add_member_control(this, "texture u offset", texture_u_offset, "value_slider", "min=0;max=1;ticks=true");
	add_member_control(this, "texture v offset", texture_v_offset, "value_slider", "min=0;max=1;ticks=true");
	add_member_control(this, "texture rotation", texture_rotation, "value_slider", "min=-180;max=180;ticks=true");
	add_member_control(this, "texture scale", texture_scale, "value_slider", "min=0.01;max=100;log=true;ticks=true");
	add_member_control(this, "texture aspect", texture_aspect, "value_slider", "min=0.1;max=10;log=true;ticks=true");

	add_decorator("Surface Material", "heading", "level=2");
	add_gui("material", static_cast<cgv::media::illum::textured_surface_material&>(material));
}


factory_registration<bump_mapper> bump_mapper_fac("New/Render/Bump Mapper", 'B', true);

