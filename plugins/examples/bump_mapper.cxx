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
	bump_scale = 1;
	wire_frame = false;
	use_phong = false;
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

bool bump_mapper::init(context& ctx)
{
	if (!prog.build_program(ctx, "bump_mapper.glpr")) {
		std::cout << "link error\n" << prog.last_error.c_str() << std::endl;
		return false;
	}
	return true;
}

void bump_mapper::init_frame(context& ctx)
{
	if (bump_map.is_created())
		return;

	if (texture_selection == ALHAMBRA) {
		std::cout << "before read bump texture" << std::endl;		
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
	// apply surface material
	glDisable(GL_COLOR_MATERIAL);
	ctx.enable_material(surface_material);

	// apply texture transformation
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glTranslated(texture_u_offset, texture_v_offset,0);
	glRotated(texture_rotation, 0, 0, 1);
	glScaled(texture_scale,texture_scale/texture_aspect,texture_scale);

	// setup bump mapping
	bump_map.enable(ctx,0);
	prog.enable(ctx);
	prog.set_uniform(ctx, "bump_map", 0);
	prog.set_uniform(ctx, "use_bump_map", use_bump_map);
	prog.set_uniform(ctx, "use_phong", use_phong);
	prog.set_uniform(ctx, "use_diffuse_map", use_diffuse_map);
	prog.set_uniform(ctx, "bump_map_res", (int)texture_resolution);
	prog.set_uniform(ctx, "bump_scale", bump_scale);
	gl::set_lighting_parameters(ctx, prog);
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
	prog.disable(ctx);
	bump_map.disable(ctx);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	ctx.disable_material(surface_material);
}

void bump_mapper::clear(context& ctx)
{
	prog.destruct(ctx);
	bump_map.destruct(ctx);
}

void bump_mapper::on_texture_change()
{
	if (!get_context())
		return;
	get_context()->make_current();
	bump_map.destruct(*get_context());
	post_redraw();
}

/// you must overload this for gui creation
void bump_mapper::create_gui() 
{	
	add_decorator("Surface Properties", "heading");
	connect_copy(add_control("primitive", surface_primitive, "square,cube,sphere,torus")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("minor radius", minor_radius, "value_slider", "min=0.05;max=1;log=true;ticks=true")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("resolution", surface_resolution, "value_slider", "min=4;max=100;log=true;ticks=true")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("ambient", surface_material.ref_ambient())
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("diffuse", surface_material.ref_diffuse())
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("specular", surface_material.ref_specular())
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("emission", surface_material.ref_emission())
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("shininess", surface_material.ref_shininess(), "value_slider", "min=0;max=128;ticks=true;log=true")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
//	connect_copy(add_control("color", surface_color, "color<float,rgb>")
//		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));

	add_decorator("Bump Map Properties", "heading");
	connect_copy(add_control("wire_frame", wire_frame, "check")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("use_diffuse_map", use_diffuse_map, "check")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("use_bump_map", use_bump_map, "check")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("bump_scale", bump_scale, "value_slider", "min=0.02;max=50;log=true;ticks=true")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("use_phong", use_phong, "check")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));

	add_decorator("Texture Properties", "heading");
	connect_copy(add_control("texture", texture_selection, "checker,waves,alhambra,cartuja")
		->value_change,rebind(this,&bump_mapper::on_texture_change));
	connect_copy(add_control("frequency", texture_frequency, "value_slider", "min=0;max=200;log=true;ticks=true")
		->value_change,rebind(this,&bump_mapper::on_texture_change));
	connect_copy(add_control("frequency aspect", texture_frequency_aspect, "value_slider", "min=0.1;max=10;log=true;ticks=true")
		->value_change,rebind(this,&bump_mapper::on_texture_change));
	connect_copy(add_control("texture resolution", texture_resolution, "value_slider", "min=4;max=1024;log=true;ticks=true")
		->value_change,rebind(this,&bump_mapper::on_texture_change));

	add_decorator("Transformation", "heading", "level=2");
	connect_copy(add_control("texture u offset", texture_u_offset, "value_slider", "min=0;max=1;ticks=true")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("texture v offset", texture_v_offset, "value_slider", "min=0;max=1;ticks=true")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("texture rotation", texture_rotation, "value_slider", "min=-180;max=180;ticks=true")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("texture scale", texture_scale, "value_slider", "min=0.01;max=100;log=true;ticks=true")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	connect_copy(add_control("texture aspect", texture_aspect, "value_slider", "min=0.1;max=10;log=true;ticks=true")
		->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
}


factory_registration<bump_mapper> bump_mapper_fac("new/bump mapper", 'B', true);

