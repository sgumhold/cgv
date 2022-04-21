#include "debug_visualization_helper.h"

void cgv::nui::ref_debug_visualization_helper(cgv::nui::debug_visualization_helper** instance, int** reference_count)
{
	static int ref_count = 0;
	static cgv::nui::debug_visualization_helper r;
	*instance = &r;
	*reference_count = &ref_count;
}

cgv::nui::debug_visualization_helper& cgv::nui::ref_debug_visualization_helper()
{
	cgv::nui::debug_visualization_helper* r;
	int* ref_count;
	ref_debug_visualization_helper(&r, &ref_count);
	return *r;
}

cgv::nui::debug_visualization_helper& cgv::nui::ref_debug_visualization_helper(cgv::render::context& ctx, int ref_count_change)
{
	cgv::nui::debug_visualization_helper* r;
	int* ref_count;
	ref_debug_visualization_helper(&r, &ref_count);
	r->manage_singleton(ctx, "debug_visualization_helper", *ref_count, ref_count_change);
	return *r;
}

void cgv::nui::debug_visualization_helper::manage_singleton(cgv::render::context& ctx, const std::string& renderer_name, int& ref_count, int ref_count_change)
{
	switch (ref_count_change) {
	case 1:
		if (ref_count == 0) {
			if (!init(ctx))
				ctx.error(std::string("unable to initialize ") + renderer_name + " singleton");
		}
		++ref_count;
		break;
	case 0:
		break;
	case -1:
		if (ref_count == 0)
			ctx.error(std::string("attempt to decrease reference count of ") + renderer_name + " singleton below 0");
		else {
			if (--ref_count == 0)
				clear(ctx);
		}
		break;
	default:
		ctx.error(std::string("invalid change reference count outside {-1,0,1} for ") + renderer_name + " singleton");
	}
}

bool cgv::nui::debug_visualization_helper::init(render::context& ctx)
{
	render::ref_sphere_renderer(ctx, 1);
	render::ref_box_renderer(ctx, 1);
	render::ref_arrow_renderer(ctx, 1);
	render::ref_spline_tube_renderer(ctx, 1);

	return true;
}

void cgv::nui::debug_visualization_helper::clear(render::context& ctx)
{
	render::ref_sphere_renderer(ctx, -1);
	render::ref_box_renderer(ctx, -1);
	render::ref_arrow_renderer(ctx, -1);
	render::ref_spline_tube_renderer(ctx, -1);
}

void cgv::nui::debug_visualization_helper::draw(render::context& ctx)
{
	// Arrows
	auto& ar = render::ref_arrow_renderer(ctx);
	// TODO: Group together arrows with the same radius
	for (int i = 0; i < arrow_positions.size(); ++i) {
		auto ars = render::arrow_render_style();
		ars.radius_relative_to_length = 1.0f / (arrow_directions[i].length() / arrow_shaft_radii[i]);
		ar.set_render_style(ars);
		std::vector<vec3> pos = { arrow_positions[i] };
		ar.set_position_array(ctx, pos);
		std::vector<vec3> dir = { arrow_directions[i] };
		ar.set_direction_array(ctx, dir);
		std::vector<rgb> col = { arrow_colors[i] };
		ar.set_color_array(ctx, col);
		ar.render(ctx, 0, 1);
	}

	// Spheres
	auto& sr = render::ref_sphere_renderer(ctx);
	// TODO: Group together spheres with the same radius
	for (int i = 0; i < sphere_positions.size(); ++i) {
		auto srs = render::sphere_render_style();
		srs.radius = sphere_radii[i];
		sr.set_render_style(srs);
		std::vector<vec3> pos = { sphere_positions[i] };
		sr.set_position_array(ctx, pos);
		std::vector<rgb> col = { sphere_colors[i] };
		sr.set_color_array(ctx, col);
		sr.render(ctx, 0, 1);
	}
}

int cgv::nui::debug_visualization_helper::register_debug_value(debug_value* value)
{
	int handle = debug_values.size();
	if (!unused_handles.empty()) {
		handle = unused_handles.back();
		unused_handles.pop_back();
	}
	debug_values[handle] = value;
	return handle;
}

void cgv::nui::debug_visualization_helper::construct_arrow(int& idx, vec3 position, vec3 direction, float shaft_radius,
	rgb color)
{
	if (idx == -1) {
		idx = arrow_positions.size();
		arrow_positions.push_back(position);
		arrow_directions.push_back(direction);
		arrow_shaft_radii.push_back(shaft_radius);
		arrow_colors.push_back(color);
	}
	else {
		arrow_positions[idx] = position;
		arrow_directions[idx] = direction;
		arrow_shaft_radii[idx] = shaft_radius;
		arrow_colors[idx] = color;
	}
}

void cgv::nui::debug_visualization_helper::construct_sphere(int& idx, vec3 position, float radius, rgb color)
{
	if (idx == -1) {
		idx = sphere_positions.size();
		sphere_positions.push_back(position);
		sphere_radii.push_back(radius);
		sphere_colors.push_back(color);
	}
	else {
		sphere_positions[idx] = position;
		sphere_radii[idx] = radius;
		sphere_colors[idx] = color;
	}
}

void cgv::nui::debug_visualization_helper::construct_position_geometry(debug_value_position* debug_value)
{
	if (!debug_value->is_enabled)
		return;
	construct_sphere(debug_value->sphere_geometry_idx, debug_value->value, debug_value->sphere_radius, debug_value->color);
}

void cgv::nui::debug_visualization_helper::construct_vector_geometry(debug_value_vector* debug_value)
{
	if (!debug_value->is_enabled)
		return;
	construct_arrow(debug_value->arrow_geometry_idx, debug_value->position, debug_value->value, debug_value->shaft_radius, debug_value->color);
}

void cgv::nui::debug_visualization_helper::reconstruct_geometry()
{
	// clear all geometry data
	sphere_positions.clear();
	sphere_radii.clear();
	sphere_colors.clear();
	arrow_positions.clear();
	arrow_directions.clear();
	arrow_shaft_radii.clear();
	arrow_colors.clear();

	// construct all geometry data
	for (auto it : debug_values) {
		debug_value_position* dv_pos = retrieve_debug_value<debug_value_position>(it.first);
		if (dv_pos != nullptr) {
			dv_pos->sphere_geometry_idx = -1;
			construct_position_geometry(dv_pos);
			continue;
		}
		debug_value_vector* dv_vec = retrieve_debug_value<debug_value_vector>(it.first);
		if (dv_vec != nullptr) {
			dv_vec->arrow_geometry_idx = -1;
			construct_vector_geometry(dv_vec);
			continue;
		}
	}
}

void cgv::nui::debug_visualization_helper::deregister_debug_value(int handle)
{
	if (debug_values.count(handle) != 0)
	{
		delete debug_values[handle];
		debug_values.erase(handle);
		unused_handles.push_back(handle);
		reconstruct_geometry();
	}
}

int cgv::nui::debug_visualization_helper::register_debug_value_position(vec3 value)
{
	auto dv_pos = new debug_value_position(value);
	int handle = register_debug_value(dv_pos);
	construct_position_geometry(dv_pos);
	return handle;
}

int cgv::nui::debug_visualization_helper::register_debug_value_vector(vec3 value)
{
	auto dv_vec = new debug_value_vector(value);
	int handle = register_debug_value(dv_vec);
	construct_vector_geometry(dv_vec);
	return handle;
}

void cgv::nui::debug_visualization_helper::configure_debug_value_position(int handle, float sphere_radius, rgb color)
{
	debug_value_position* value = retrieve_debug_value<debug_value_position>(handle);
	if (value != nullptr) {
		value->sphere_radius = sphere_radius;
		value->color = color;
		construct_position_geometry(value);
	}
}

void cgv::nui::debug_visualization_helper::configure_debug_value_vector(int handle, float shaft_radius, rgb color)
{
	debug_value_vector* value = retrieve_debug_value<debug_value_vector>(handle);
	if (value != nullptr) {
		value->shaft_radius = shaft_radius;
		value->color = color;
		construct_vector_geometry(value);
	}
}

void cgv::nui::debug_visualization_helper::enable_debug_value_visualization(int handle)
{
	if (debug_values.count(handle) != 0) {
		debug_values[handle]->is_enabled = true;
		reconstruct_geometry();
	}
}

void cgv::nui::debug_visualization_helper::disable_debug_value_visualization(int handle)
{
	if (debug_values.count(handle) != 0) {
		debug_values[handle]->is_enabled = false;
		reconstruct_geometry();
	}
}

void cgv::nui::debug_visualization_helper::update_debug_value_position(int handle, vec3 value)
{
	debug_value_position* dv = retrieve_debug_value<debug_value_position>(handle);
	if (dv == nullptr)
		return;
	dv->value = value;
	construct_position_geometry(dv);

}

void cgv::nui::debug_visualization_helper::update_debug_value_vector_direction(int handle, vec3 value)
{
	debug_value_vector* dv = retrieve_debug_value<debug_value_vector>(handle);
	if (dv == nullptr)
		return;
	dv->value = value;
	construct_vector_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_vector_position(int handle, vec3 value)
{
	debug_value_vector* dv = retrieve_debug_value<debug_value_vector>(handle);
	if (dv == nullptr)
		return;
	dv->position = value;
	construct_vector_geometry(dv);
}
