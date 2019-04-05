#include "light_interactor.h"
#include <cgv/utils/convert.h>
#include <cgv/signal/rebind.h>
#include <cgv/math/ftransform.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/trigger.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/attribute_array_binding.h>
#include <stdio.h>

using namespace cgv::gui;
using namespace cgv::utils;
using namespace cgv::render;
using namespace cgv::signal;

light_interactor::light_interactor() : node("light_interactor")
{
	last_modelview_matrix.identity();
	min_opacity = 0.2f;
	max_opacity = 1.0f;
	ray_width = 4.0f;
	color_lambda = 0.2f;
	default_color = rgb(1, 1, 0);
	nr_light_rays = 100;
	view_ptr = 0;
	delta_pos.zeros();
	light_scale = 0.2f;
	speed = 0.05f;
	current_light_index = -1;
	connect(get_animation_trigger().shoot, this, &light_interactor::timer_event);
}

void light_interactor::timer_event(double t, double dt)
{
	if (current_light_index == -1)
		return;
	if (delta_pos.length() < 0.1f)
		return;
	lights[current_light_index].ref_position() += speed * delta_pos;
	for (unsigned i = 0; i < 3; ++i)
		if (abs(delta_pos(i)) > 0.1f)
			on_set(&lights[current_light_index].ref_position()(i));
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
			if (member_ptr == &lights[i].ref_local_to_eye()) {
				bool is_dir = lights[i].get_type() == cgv::media::illum::LT_DIRECTIONAL;
				dvec4 hpos(lights[i].get_position(), is_dir ? 0.0 : 1.0);
				dvec4 hdir(lights[i].get_spot_direction(), 0.0);
				if (lights[i].is_local_to_eye()) {
					hpos = last_modelview_matrix * hpos;
					dvec3 pos = reinterpret_cast<dvec3&>(hpos);
					if (is_dir)
						pos /= 1.0 / hpos(3);
					lights[i].set_position(pos);
					dvec4 hspot_dir = last_modelview_matrix * hdir;
					lights[i].set_spot_direction(reinterpret_cast<const dvec3&>(hspot_dir));
				}
				else {
					dmat4 inv_mv = inv(last_modelview_matrix);
					hpos = inv_mv * hpos;
					dvec3 pos = reinterpret_cast<dvec3&>(hpos);
					if (is_dir)
						pos /= 1.0 / hpos(3);
					lights[i].set_position(pos);
					dvec4 hspot_dir = inv_mv * hdir;
					lights[i].set_spot_direction(reinterpret_cast<const dvec3&>(hspot_dir));
				}
				for (unsigned c = 0; c < 3; ++c) {
					update_member(&lights[i].ref_position()[c]);
					update_member(&lights[i].ref_spot_direction()[c]);
				}
			}
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
	update_member(member_ptr);
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
	view_ptr = find_view_as_node();
	unsigned n = ctx.get_max_nr_enabled_light_sources();
	lights.resize(n);
	handles.resize(n);
	new_handles.resize(n);
	intensities.resize(n);
	toggles.resize(7*n);
	enabled.resize(n);
	show.resize(n);
	light_rays.resize(2*n*nr_light_rays);
	current_ray_indices.resize(n, -1);
	for (unsigned i = 0; i < n; ++i) {
		lights[i].set_position(vec3(2*(float)((i+6)&1)-1,(float)((i+6)&2)-1,(float)((i+6)&4)/2-1));
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
	if (!prog.build_program(ctx, "light_ray.glpr", true)) {
		std::cerr << "could not build light_ray.glpr" << std::endl;
		return false;
	}
	return true;
}

void light_interactor::init_frame(context& ctx)
{
	last_modelview_matrix = ctx.get_modelview_matrix();

	sample_light_rays(ctx, 1);

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

void light_interactor::sample_light_rays(context& ctx, unsigned decrease_count)
{
	if (!view_ptr)
		return;

	float y_extent = float(view_ptr->get_y_extent_at_focus());
	float x_extent = y_extent * ctx.get_width() / ctx.get_height();

	std::uniform_real_distribution<float> D(-1.0f,1.0f);

	size_t i, n = lights.size();
	for (i = 0; i < n; ++i) {
		// only sample rays for visible lights
		if (!show[i])
			continue;

		// determine number of to be sampled rays
		size_t sample_count = decrease_count;
		if (current_ray_indices[i] == -1) {
			sample_count = nr_light_rays;
			current_ray_indices[i] = 0;
		}

		// sample rays
		for (size_t j = 0; j < nr_light_rays; ++j) {
			size_t idx = 2 * (nr_light_rays*i + current_ray_indices[i]);
			vec3 p;
			if (j < sample_count) {
				// sample random position relative to current view in world coords
				p = vec3(D(RE), D(RE), D(RE));
				p[0] *= x_extent * ctx.get_width() / ctx.get_height();
				p[1] *= y_extent;
				p[2] *= 0.5f*y_extent;

				// transform p to eye space
				vec4 hP = ctx.get_modelview_matrix() * dvec4(p, 1.0f);
				p = (1.0f / hP[3]) * reinterpret_cast<vec3&>(hP);
			}
			else {
				p = 0.5f*(light_rays[idx]+ light_rays[idx+1]);
			}
			// build ray in eye space
			vec3 dir = ctx.get_light_eye_position(lights[i], !lights[i].is_local_to_eye());;
			if (lights[i].get_type() != cgv::media::illum::LT_DIRECTIONAL)
				dir -= p;

			// store light ray
			light_rays[idx]   = p - 5.0f*y_extent*dir;
			light_rays[idx+1] = p + 5.0f*y_extent*dir;

			// move on current ray index
			current_ray_indices[i] = (current_ray_indices[i] + 1) % nr_light_rays;
		}
	}
}

void light_interactor::draw_light_rays(context& ctx, size_t i)
{
	int light_index = int(i);
	prog.set_uniform(ctx, "default_color", default_color);
	prog.set_uniform(ctx, "color_lambda", color_lambda);
	prog.set_uniform(ctx, "min_opacity", min_opacity);
	prog.set_uniform(ctx, "max_opacity", max_opacity);
	prog.set_uniform(ctx, "light_index", light_index);
//	rgb clr = color_lambda * lights[i].get_emission() + (1.0f - color_lambda)*default_color;
//	ctx.set_color(rgba(clr[0],clr[1],clr[2],opacity));
	glDrawArrays(GL_LINES, 2 * i*nr_light_rays, 2 * nr_light_rays);
}

void light_interactor::finish_frame(context& ctx)
{
	GLboolean old_line_smooth = glIsEnabled(GL_LINE_SMOOTH);
	GLboolean old_blend = glIsEnabled(GL_BLEND);
	GLfloat old_line_width;
	glGetFloatv(GL_LINE_WIDTH, &old_line_width);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(ray_width);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	size_t i, n = lights.size();
	prog.enable(ctx);
	attribute_array_binding::set_global_attribute_array(ctx, prog.get_position_index(), light_rays);
	attribute_array_binding::enable_global_array(ctx, prog.get_position_index());
	ctx.push_modelview_matrix();
	ctx.set_modelview_matrix(cgv::math::identity4<double>());
	for (i = 0; i < n; ++i)
		if (show[i])
			draw_light_rays(ctx, i);
	ctx.pop_modelview_matrix();
	attribute_array_binding::disable_global_array(ctx, prog.get_position_index());
	prog.disable(ctx);
	glLineWidth(old_line_width);
	if (!old_line_smooth)
		glDisable(GL_LINE_SMOOTH);
	if (!old_blend)
		glDisable(GL_BLEND);
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
	add_member_control(this, "ray_width", ray_width, "value_slider", "min=1;max=10;ticks=true");
	add_member_control(this, "default_color", default_color);
	add_member_control(this, "color_lambda", color_lambda, "value_slider", "min=0;max=1;ticks=true");
	add_member_control(this, "min_opacity", min_opacity, "value_slider", "min=0;max=1;ticks=true");
	add_member_control(this, "max_opacity", max_opacity, "value_slider", "min=0;max=1;ticks=true");
	add_member_control(this, "nr_light_rays", nr_light_rays, "value_slider", "min=1;max=100;ticks=true;log=true");
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
	add_member_control(this, "interaction speed", speed, "value_slider", "min=0.001;max=10;ticks=true;log=true");
}

bool light_interactor::handle(event& e)
{
	if (e.get_kind() != EID_KEY)
		return false;

	key_event& ke = static_cast<key_event&>(e);
	switch (ke.get_key()) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
		if (ke.get_action() == KA_RELEASE) {
			if (current_light_index == ke.get_key() - '0')
				current_light_index = -1;
		}
		else {
			current_light_index = ke.get_key() - '0';
		}
		return true;
	case 'V':
		if (current_light_index != -1) {
			if (ke.get_action() != KA_RELEASE) {
				show[current_light_index] = 1 - show[current_light_index];
				on_set(&show[current_light_index]);
			}
			return true;
		}
		break;
	case 'D':
	case 'P':
	case 'S':
		if (current_light_index != -1) {
			if (ke.get_action() != KA_RELEASE) {
				lights[current_light_index].set_type(
					ke.get_key() == 'D' ? cgv::media::illum::LT_DIRECTIONAL : 
					(ke.get_key() == 'P' ? cgv::media::illum::LT_POINT : cgv::media::illum::LT_SPOT)
				);
				on_set(&lights[current_light_index].ref_type());
			}
			return true;
		}
		break;
	case 'E':
		if (current_light_index != -1) {
			if (ke.get_action() != KA_RELEASE) {
				enabled[current_light_index] = 1 - enabled[current_light_index];
				on_set(&enabled[current_light_index]);
			}
			return true;
		}
		break;
	case 'L':
		if (current_light_index != -1) {
			if (ke.get_action() != KA_RELEASE) {
				lights[current_light_index].ref_local_to_eye() = 1 - lights[current_light_index].ref_local_to_eye();
				on_set(&lights[current_light_index].ref_local_to_eye());
			}
			return true;
		}
		break;
	case 'H':
		if (current_light_index != -1) {
			if (ke.get_action() != KA_RELEASE) {
				if (lights[current_light_index].get_type() != cgv::media::illum::LT_SPOT) {
					lights[current_light_index].set_type(cgv::media::illum::LT_SPOT);
					on_set(&lights[current_light_index].ref_type());
				}
				lights[current_light_index].set_position(vec3(0, 0, 0));
				lights[current_light_index].set_spot_direction(vec3(0, 0, -1));
				for (unsigned c = 0; c < 3; ++c) {
					on_set(&lights[current_light_index].ref_position()[c]);
					on_set(&lights[current_light_index].ref_spot_direction()[c]);
				}
				if (!lights[current_light_index].is_local_to_eye()) {
					lights[current_light_index].set_local_to_eye(true);
					on_set(&lights[current_light_index].ref_local_to_eye());
				}
			}
			return true;
		}
		break;
	case KEY_Left:
		if (current_light_index != -1) {
			delta_pos(0) = (ke.get_action() == KA_RELEASE) ? 0.0f : -1.0f;
			return true;
		}
		return false;
	case KEY_Right:
		if (current_light_index != -1) {
			delta_pos(0) = (ke.get_action() == KA_RELEASE) ? 0.0f : 1.0f;
			return true;
		}
		return false;
	case KEY_Down:
		if (current_light_index != -1) {
			delta_pos(1) = (ke.get_action() == KA_RELEASE) ? 0.0f : -1.0f;
			return true;
		}
		return false;
	case KEY_Up:
		if (current_light_index != -1) {
			delta_pos(1) = (ke.get_action() == KA_RELEASE) ? 0.0f : 1.0f;
			return true;
		}
		return false;
	case KEY_Page_Down:
		if (current_light_index != -1) {
			delta_pos(2) = (ke.get_action() == KA_RELEASE) ? 0.0f : -1.0f;
			return true;
		}
		return false;
	case KEY_Page_Up:
		if (current_light_index != -1) {
			delta_pos(2) = (ke.get_action() == KA_RELEASE) ? 0.0f : 1.0f;
			return true;
		}
		return false;
	}
	return false;
}

void light_interactor:: stream_help(std::ostream& os)
{
	os << "light_interactor: hold <0..7=i> to change light i: <v>isible, <e>nable, <l>ocal\n"
	   << "                  <d>irectional|<p>oint|<s>pot, direction keys to change position\n";
}

#include <cgv/base/register.h>

#include "lib_begin.h"

/// register a light interactor factory
extern cgv::base::factory_registration<light_interactor> li_reg("light_interactor", "menu_text='new/light interactor';shortcut='Ctrl-Alt-L'", true, 
														                "menu_text='&view/light interactor';shortcut='Ctrl-L'");
