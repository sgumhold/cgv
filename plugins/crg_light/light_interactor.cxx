#include "light_interactor.h"
#include <cgv/utils/convert.h>
#include <cgv/signal/rebind.h>
#include <cgv/math/ftransform.h>
#include <cgv/gui/file_dialog.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <stdio.h>

using namespace cgv::gui;
using namespace cgv::utils;
using namespace cgv::render;
using namespace cgv::signal;

light_interactor::light_interactor()
{
	light_scale = 0.2f;
}

/// hide all lights
void light_interactor::hide_all()
{
	std::fill(show.begin(), show.end(), false);
	post_redraw();
}

/// show all lights
void light_interactor::show_all()
{
	std::fill(show.begin(), show.end(), true);
	post_redraw();
}

std::string light_interactor::get_type_name() const
{
	return "light_interactor";
}

void light_interactor::on_set(void* member_ptr)
{
	size_t n = lights.size();
	if (get_context()) {
		cgv::render::context& ctx = *get_context();
		for (size_t i = 0; i < n; ++i) {
			if (member_ptr >= &lights[i] && member_ptr < &lights[i] + 1 || member_ptr == &intensities[i]) {
				light_source L = lights[i];
				L.set_emission(intensities[i] * L.get_emission());
				ctx.set_light_source(new_handles[i], L);
			}
			if (member_ptr == &enabled[i]) {
				if (enabled[i])
					ctx.enable_light_source(new_handles[i]);
				else
					ctx.disable_light_source(new_handles[i]);
			}
		}
	}
	if (member_ptr == &file_name) {
		load(file_name);
		on_load();
	}

	post_redraw();
}

bool light_interactor::self_reflect(reflection_handler& rh)
{
	if (! (rh.reflect_member("file_name", file_name) &&
		   rh.reflect_member("light_scale", light_scale) ) )
		   return false;

	size_t n = lights.size();
	for (size_t i = 0; i < n; ++i) {
		if (! (rh.reflect_member(std::string("enabled_")+to_string(i), (bool&)(enabled[i])) &&
			   rh.reflect_member(std::string("show_")+to_string(i), (bool&)(show[i])) &&
			   rh.reflect_member(std::string("toggle_")+to_string(i), (bool&)(toggles[7*i])) &&
			   rh.reflect_member(std::string("intensity_")+to_string(i), intensities[i]) ) )
			   return false;
	}
	return true;
}

bool light_interactor::init(context& ctx)
{
	unsigned n = ctx.get_max_nr_lights();
	lights.resize(n);
	handles.resize(n);
	new_handles.resize(n);
	intensities.resize(n);
	toggles.resize(7*n);
	enabled.resize(n);
	show.resize(n);
	for (unsigned i = 0; i < n; ++i) {
		lights[i].set_position(light_source::vec_type(2*(float)((i+6)&1)-1,(float)((i+6)&2)-1,(float)((i+6)&4)/2-1));
		enabled[i] = i < 2 ? 1 : 0;
		handles[i] = 0;
		intensities[i] = 1;
		for (unsigned j=0; j<7; ++j)
			toggles[7*i+j] = j!=0;
		enabled[i];
		new_handles[i] = ctx.add_light_source(lights[i], enabled[i]);
	}
    ctx.set_default_render_pass_flags(
		(RenderPassFlags)(ctx.get_default_render_pass_flags() & ~(RPF_SET_LIGHTS|RPF_SET_LIGHTS_ON))
	);
	return true;
}

void light_interactor::init_frame(context& ctx)
{
	size_t i, n = lights.size();
	ctx.push_modelview_matrix();
	ctx.set_modelview_matrix(cgv::math::identity4<double>());
	for (i=0; i<n; ++i) {
		if (enabled[i] && lights[i].is_local_to_eye()) {
			ctx.place_light_source(new_handles[i]);
		}		
	}
	ctx.pop_modelview_matrix();
	for (i=0; i<n; ++i) {
		if (enabled[i] && !lights[i].is_local_to_eye()) {
			ctx.place_light_source(new_handles[i]);
		}		
	}
}


void light_interactor::draw(context& ctx)
{
	static surface_material default_mat;
	ctx.ref_surface_shader_program().enable(ctx);
	ctx.ref_surface_shader_program().set_uniform(ctx, "map_color_to_material", int(3));
	ctx.set_material(default_mat);
		size_t i, n = lights.size();
		ctx.push_modelview_matrix();
		ctx.set_modelview_matrix(cgv::math::identity4<double>());
			for (i = 0; i < n; ++i) {
				if (show[i] && lights[i].is_local_to_eye())
					ctx.draw_light_source(lights[i], enabled[i] != 0 ? intensities[i] : 0.0f, light_scale);
			}
		ctx.pop_modelview_matrix();
		for (i = 0; i < n; ++i) {
			if (show[i] && !lights[i].is_local_to_eye())
				ctx.draw_light_source(lights[i], enabled[i] != 0 ? intensities[i] : 0.0f, light_scale);
		}
	ctx.ref_surface_shader_program().disable(ctx);
}

void light_interactor::finish_frame(context& ctx)
{
}

/// correct default render flags
void light_interactor::clear(context& ctx)
{
	ctx.set_default_render_pass_flags(
		(RenderPassFlags)(ctx.get_default_render_pass_flags() |  (RPF_SET_LIGHTS | RPF_SET_LIGHTS_ON))
	);
}

/// save light_interactor to file
bool light_interactor::save(const std::string& file_name)  const
{
	FILE* fp = fopen(file_name.c_str(), "wb");
	if (!fp)
		return false;
	size_t n = lights.size();
	bool success = 
		fwrite(&n, sizeof(unsigned), 1, fp)             == 1 &&
		fwrite(&lights[0], sizeof(light_source), n, fp) == n &&
		fwrite(&intensities[0], sizeof(float), n, fp)   == n &&
		fwrite(&enabled[0], sizeof(int), n, fp)         == n;
	fclose(fp);
	return success;
}

void light_interactor::save_cb()
{
	std::string fn = file_save_dialog("Choose light settings", "(bin):*.bin");
	if (fn.empty())
		return;
	save(fn);
}

void light_interactor::on_load()
{
	size_t n = lights.size();
	toggles.resize(n);
	handles.resize(n);
	for (size_t i = 0; i<n; ++i) {
		handles[i] = 0;
		toggles[i] = 0;
	}
	post_recreate_gui();
	post_redraw();
}

bool light_interactor::load(const std::string& fn)
{
	file_name = fn;
	FILE* fp = fopen(file_name.c_str(), "rb");
	if (!fp)
		return false;
	unsigned n;
	bool success = fread(&n, sizeof(unsigned), 1, fp) == 1;
	if (success) {
		lights.resize(n);
		intensities.resize(n);
		enabled.resize(n);
		success = 
			fread(&lights[0], sizeof(light_source), n, fp) == n &&
			fread(&intensities[0], sizeof(float), n, fp) == n &&
			fread(&enabled[0], sizeof(int), n, fp) == n;
	}
	fclose(fp);
	return success;
}

void light_interactor::load_cb()
{
	std::string fn = file_open_dialog("Choose light settings", "(bin):*.bin");
	if (fn.empty())
		return;
	load(fn);
	on_load();
}

void light_interactor::create_gui()
{
	size_t i, n = lights.size();
	add_decorator("Light Interactor", "heading", "level=1");
	connect_copy(add_button("save", "w=90"," ")->click, rebind(this, &light_interactor::save_cb));
	connect_copy(add_button("load", "w=90")->click, rebind(this, &light_interactor::load_cb));
	add_decorator("Rendering", "heading");
	add_member_control(this, "light_scale", light_scale, "value_slider", "min=0;max=10;ticks=true;log=true");
	for (i = 0; i < n; ++i)
		add_member_control(this, i == 0 ? "show" : "", (bool&)show[i], "check", i == 0 ? "w=20;align=\"L\"" : "w=20", i == n - 1 ? "\n" : " ");

	add_decorator("Switches", "heading");
	for (i = 0; i < n; ++i)
		add_member_control(this, i==0?"enable":"", (bool&)enabled[i], "check", i==0?"w=20;align=\"L\"":"w=20", i==n-1?"\n":" ");
	for (i = 0; i < n; ++i)
		add_member_control(this, i==0?"local":"", lights[i].ref_local_to_eye(), "check", i==0?"w=20;align=\"L\"":"w=20", i==n-1?"\n":" ");
			
	add_decorator("Light Parameters", "heading");
	for (i = 0; i < n; ++i) {
		if (!add_tree_node(std::string("light ")+to_string(i), (bool&)(toggles[7*i]),2))
			continue;
		align("\a");
		add_member_control(this, "on", (bool&)enabled[i], "check");
		add_member_control(this, "intensity", intensities[i], "value_slider", "min=0;max=1");

		add_gui(std::string("light ")+to_string(i), lights[i], "", "");
		align("\b");
	}
}

#include <cgv/base/register.h>

#include "lib_begin.h"

/// register a light interactor factory
extern cgv::base::factory_registration<light_interactor> li_reg("light_interactor", "menu_text='new/light interactor';shortcut='Ctrl-Alt-L'", true, 
														                "menu_text='&view/light interactor';shortcut='Ctrl-L'");
