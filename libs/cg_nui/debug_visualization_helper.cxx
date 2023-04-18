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
		srs.radius = sphere_radii[i];
		sr.set_render_style(srs);
		std::vector<vec3> pos = { sphere_positions[i] };
		sr.set_position_array(ctx, pos);
		std::vector<rgb> col = { sphere_colors[i] };
		sr.set_color_array(ctx, col);
		sr.render(ctx, 0, 1);
	}

	// Boxes
	auto& br = render::ref_box_renderer(ctx);
	br.set_render_style(brs);
	br.set_position_array(ctx, box_positions);
	br.set_extent_array(ctx, box_extents);
	br.set_color_array(ctx, box_colors);
	br.render(ctx, 0, 1);

	// Splines
	auto& str = cgv::render::ref_spline_tube_renderer(ctx);
	str.set_render_style(strs);
	for (int i = 0; i < splines.size(); ++i) {
		strs.radius = spline_radii[i];
		if (splines[i].first.empty())
			continue;
		str.set_position_array(ctx, splines[i].first);
		str.set_tangent_array(ctx, splines[i].second);
		std::vector<rgb> col;
		for (int j = 0; j < splines[i].first.size(); ++j) {
			col.push_back(spline_colors[i]);
		}
		str.set_color_array(ctx, col);
		str.render(ctx, 0, splines[i].first.size(), true);
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

void cgv::nui::debug_visualization_helper::construct_box(int& idx, vec3 position, vec3 extent, rgb color)
{
	if (idx == -1) {
		idx = box_positions.size();
		box_positions.push_back(position);
		box_extents.push_back(extent);
		box_colors.push_back(color);
	}
	else {
		box_positions[idx] = position;
		box_extents[idx] = extent;
		box_colors[idx] = color;
	}
}

void cgv::nui::debug_visualization_helper::construct_spline(int& idx, std::vector<vec3> positions,
                                                            std::vector<vec4> tangents, float radius, rgb color)
{
	if (idx == -1) {
		idx = splines.size();
		splines.push_back(spline_data_t(positions, tangents));
		spline_radii.push_back(radius);
		spline_colors.push_back(color);
	}
	else {
		splines[idx] = spline_data_t(positions, tangents);
		spline_radii[idx] = radius;
		spline_colors[idx] = color;
	}
}

void cgv::nui::debug_visualization_helper::construct_position_geometry(debug_value_position* debug_value)
{
	if (!debug_value->is_enabled)
		return;
	construct_sphere(debug_value->sphere_geometry_idx, debug_value->value, debug_value->config.sphere_radius, debug_value->config.color);
}

void cgv::nui::debug_visualization_helper::construct_vector_geometry(debug_value_vector* debug_value)
{
	if (!debug_value->is_enabled)
		return;
	construct_arrow(debug_value->arrow_geometry_idx, debug_value->config.position, debug_value->value, debug_value->config.shaft_radius, debug_value->config.color);
}

void cgv::nui::debug_visualization_helper::construct_coordinate_system_geometry(
	debug_value_coordinate_system* debug_value)
{
	if (!debug_value->is_enabled)
		return;
	// calculation from https://math.stackexchange.com/q/1463487 (lookup 28.06.2022)
			// assuming no perspective transformation, no shear, no negative scaling factors
	vec3 translation = debug_value->value.col(3);
	vec3 scale = vec3(vec3(debug_value->value.col(0)).length(), vec3(debug_value->value.col(1)).length(), vec3(debug_value->value.col(2)).length());
	quat rotation = quat(mat3({ normalize(vec3(debug_value->value.col(0))), normalize(vec3(debug_value->value.col(1))), normalize(vec3(debug_value->value.col(2))) }));
	rotation.normalize();
	vec3 cs_pos = debug_value->config.position;
	if (debug_value->config.show_translation)
		cs_pos = translation;
	vec3 cs_axis0 = vec3(1.0, 0.0, 0.0) * debug_value->config.base_axis_length;
	vec3 cs_axis1 = vec3(0.0, 1.0, 0.0) * debug_value->config.base_axis_length;
	vec3 cs_axis2 = vec3(0.0, 0.0, 1.0) * debug_value->config.base_axis_length;
	if (debug_value->config.show_rotation) {
		cs_axis0 = rotation.apply(cs_axis0);
		cs_axis1 = rotation.apply(cs_axis1);
		cs_axis2 = rotation.apply(cs_axis2);
	}
	if (debug_value->config.show_scale) {
		cs_axis0 = cs_axis0 * scale.x();
		cs_axis1 = cs_axis1 * scale.y();
		cs_axis2 = cs_axis2 * scale.z();
	}
	construct_sphere(debug_value->sphere_geometry_idx, cs_pos, debug_value->config.center_radius, debug_value->config.center_color);
	construct_arrow(debug_value->arrow0_geometry_idx, cs_pos, cs_axis0, debug_value->config.axis_radius, coordinate_system_arrow_colors[0]);
	construct_arrow(debug_value->arrow1_geometry_idx, cs_pos, cs_axis1, debug_value->config.axis_radius, coordinate_system_arrow_colors[1]);
	construct_arrow(debug_value->arrow2_geometry_idx, cs_pos, cs_axis2, debug_value->config.axis_radius, coordinate_system_arrow_colors[2]);
}

void cgv::nui::debug_visualization_helper::construct_ray_geometry(debug_value_ray* debug_value)
{
	if (!debug_value->is_enabled)
		return;
	if (debug_value->config.show_origin)
		construct_sphere(debug_value->sphere_geometry_idx, debug_value->origin, debug_value->config.origin_radius, debug_value->config.origin_color);
	vec3 start_point = debug_value->origin + debug_value->direction * debug_value->config.start_offset;
	vec3 end_point = start_point + debug_value->direction * debug_value->config.length;
	construct_spline(debug_value->spline_geometry_idx, { start_point, end_point },
		{ vec4(debug_value->direction,0.0f), vec4(debug_value->direction, 0.0f) }, debug_value->config.ray_radius, debug_value->config.ray_color);
}

void cgv::nui::debug_visualization_helper::construct_cylinder_geometry(debug_value_cylinder* debug_value)
{
	if (!debug_value->is_enabled)
		return;
	construct_spline(debug_value->spline_geometry_idx, { debug_value->origin, debug_value->origin + debug_value->direction },
		{ vec4(debug_value->direction, 0.0f), vec4(debug_value->direction, 0.0f) }, debug_value->radius,
		debug_value->config.color);
}

void cgv::nui::debug_visualization_helper::construct_box_geometry(debug_value_box* debug_value)
{
	if (!debug_value->is_enabled)
		return;
	construct_box(debug_value->box_geometry_idx, debug_value->origin, debug_value->extent, debug_value->config.color);
}


void cgv::nui::debug_visualization_helper::reconstruct_geometry()
{
	// clear all geometry data
	sphere_positions.clear();
	sphere_radii.clear();
	sphere_colors.clear();
	box_positions.clear();
	box_extents.clear();
	box_colors.clear();
	arrow_positions.clear();
	arrow_directions.clear();
	arrow_shaft_radii.clear();
	arrow_colors.clear();
	splines.clear();
	spline_radii.clear();
	spline_colors.clear();

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
		debug_value_coordinate_system* dv_cs = retrieve_debug_value<debug_value_coordinate_system>(it.first);
		if (dv_cs != nullptr) {
			dv_cs->arrow0_geometry_idx = -1;
			dv_cs->arrow1_geometry_idx = -1;
			dv_cs->arrow2_geometry_idx = -1;
			dv_cs->sphere_geometry_idx = -1;
			construct_coordinate_system_geometry(dv_cs);
			continue;
		}
		debug_value_ray* dv_ray = retrieve_debug_value<debug_value_ray>(it.first);
		if (dv_ray != nullptr) {
			dv_ray->spline_geometry_idx = -1;
			dv_ray->sphere_geometry_idx = -1;
			construct_ray_geometry(dv_ray);
			continue;
		}
		debug_value_cylinder* dv_cylinder = retrieve_debug_value<debug_value_cylinder>(it.first);
		if (dv_cylinder != nullptr) {
			dv_cylinder->spline_geometry_idx = -1;
			construct_cylinder_geometry(dv_cylinder);
			continue;
		}
		debug_value_box* dv_box = retrieve_debug_value<debug_value_box>(it.first);
		if (dv_box != nullptr) {
			dv_box->box_geometry_idx = -1;
			construct_box_geometry(dv_box);
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

cgv::nui::debug_value_config_position cgv::nui::debug_visualization_helper::
get_config_debug_value_position(int handle)
{
	auto* debug_value = retrieve_debug_value<debug_value_position>(handle);
	if (debug_value)
		return debug_value->config;
	return debug_value_config_position();
}

void cgv::nui::debug_visualization_helper::set_config_debug_value_position(int handle,
	debug_value_config_position config)
{
	auto* debug_value = retrieve_debug_value<debug_value_position>(handle);
	if (debug_value)
		debug_value->config = config;
}

cgv::nui::debug_value_config_vector cgv::nui::debug_visualization_helper::
get_config_debug_value_vector(int handle)
{
	auto* debug_value = retrieve_debug_value<debug_value_vector>(handle);
	if (debug_value)
		return debug_value->config;
	return debug_value_config_vector();
}

void cgv::nui::debug_visualization_helper::
set_config_debug_value_vector(int handle, debug_value_config_vector config)
{
	auto* debug_value = retrieve_debug_value<debug_value_vector>(handle);
	if (debug_value)
		debug_value->config = config;
}

cgv::nui::debug_value_config_coordinate_system cgv::nui::debug_visualization_helper::
get_config_debug_value_coordinate_system(int handle)
{
	auto* debug_value = retrieve_debug_value<debug_value_coordinate_system>(handle);
	if (debug_value)
		return debug_value->config;
	return debug_value_config_coordinate_system();
}

void cgv::nui::debug_visualization_helper::set_config_debug_value_coordinate_system(int handle,
	debug_value_config_coordinate_system config)
{
	auto* debug_value = retrieve_debug_value<debug_value_coordinate_system>(handle);
	if (debug_value)
		debug_value->config = config;
}

cgv::nui::debug_value_config_ray cgv::nui::debug_visualization_helper::
get_config_debug_value_ray(int handle)
{
	auto* debug_value = retrieve_debug_value<debug_value_ray>(handle);
	if (debug_value)
		return debug_value->config;
	return debug_value_config_ray();
}

void cgv::nui::debug_visualization_helper::set_config_debug_value_ray(int handle, debug_value_config_ray config)
{
	auto* debug_value = retrieve_debug_value<debug_value_ray>(handle);
	if (debug_value)
		debug_value->config = config;
}

cgv::nui::debug_value_config_cylinder cgv::nui::debug_visualization_helper::
get_config_debug_value_cylinder(int handle)
{
	auto* debug_value = retrieve_debug_value<debug_value_cylinder>(handle);
	if (debug_value)
		return debug_value->config;
	return debug_value_config_cylinder();
}

void cgv::nui::debug_visualization_helper::set_config_debug_value_cylinder(int handle,
	debug_value_config_cylinder config)
{
	auto* debug_value = retrieve_debug_value<debug_value_cylinder>(handle);
	if (debug_value)
		debug_value->config = config;
}

cgv::nui::debug_value_config_box cgv::nui::debug_visualization_helper::get_config_debug_value_box(int handle)
{
	auto* debug_value = retrieve_debug_value<debug_value_box>(handle);
	if (debug_value)
		return debug_value->config;
	return debug_value_config_box();
}

void cgv::nui::debug_visualization_helper::set_config_debug_value_box(int handle, debug_value_config_box config)
{
	auto* debug_value = retrieve_debug_value<debug_value_box>(handle);
	if (debug_value)
		debug_value->config = config;
}


int cgv::nui::debug_visualization_helper::register_debug_value_position(vec3 value)
{
	auto debug_value = new debug_value_position(value);
	int handle = register_debug_value(debug_value);
	construct_position_geometry(debug_value);
	return handle;
}

int cgv::nui::debug_visualization_helper::register_debug_value_position()
{
	return register_debug_value_position(vec3(0.0f));
}

int cgv::nui::debug_visualization_helper::register_debug_value_vector(vec3 value)
{
	auto debug_value = new debug_value_vector(value);
	int handle = register_debug_value(debug_value);
	construct_vector_geometry(debug_value);
	return handle;
}

int cgv::nui::debug_visualization_helper::register_debug_value_vector()
{
	return register_debug_value_vector(vec3(1.0f, 0.0f, 0.0f));
}

int cgv::nui::debug_visualization_helper::register_debug_value_coordinate_system(mat4 value)
{
	auto debug_value = new debug_value_coordinate_system(value);
	int handle = register_debug_value(debug_value);
	construct_coordinate_system_geometry(debug_value);
	return handle;
}

int cgv::nui::debug_visualization_helper::register_debug_value_coordinate_system()
{
	mat4 id;
	id.identity();
	return register_debug_value_coordinate_system(id);
}

int cgv::nui::debug_visualization_helper::register_debug_value_ray(vec3 origin, vec3 direction)
{
	auto debug_value = new debug_value_ray(origin, normalize(direction));
	int handle = register_debug_value(debug_value);
	construct_ray_geometry(debug_value);
	return handle;
}

int cgv::nui::debug_visualization_helper::register_debug_value_ray()
{
	return register_debug_value_ray(vec3(0.0f), vec3(1.0f, 0.0f, 0.0f));
}

int cgv::nui::debug_visualization_helper::register_debug_value_cylinder(vec3 origin, vec3 direction, float radius)
{
	auto debug_value = new debug_value_cylinder(origin, direction, radius);
	int handle = register_debug_value(debug_value);
	construct_cylinder_geometry(debug_value);
	return handle;
}

int cgv::nui::debug_visualization_helper::register_debug_value_cylinder()
{
	return register_debug_value_cylinder(vec3(0.0f), vec3(1.0f, 0.0f, 0.0f), 0.2f);
}

int cgv::nui::debug_visualization_helper::register_debug_value_box(vec3 origin, vec3 extent)
{
	auto debug_value = new debug_value_box(origin, extent);
	int handle = register_debug_value(debug_value);
	construct_box_geometry(debug_value);
	return handle;
}

int cgv::nui::debug_visualization_helper::register_debug_value_box()
{
	return register_debug_value_box(vec3(0.0f), vec3(0.1f));
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
	dv->config.position = value;
	construct_vector_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_coordinate_system(int handle, mat4 value)
{
	debug_value_coordinate_system* dv = retrieve_debug_value<debug_value_coordinate_system>(handle);
	if (dv == nullptr)
		return;
	dv->value = value;
	construct_coordinate_system_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_ray(int handle, vec3 origin, vec3 direction)
{
	debug_value_ray* dv = retrieve_debug_value<debug_value_ray>(handle);
	if (dv == nullptr)
		return;
	dv->origin = origin;
	dv->direction = direction;
	construct_ray_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_ray_origin(int handle, vec3 origin)
{
	debug_value_ray* dv = retrieve_debug_value<debug_value_ray>(handle);
	if (dv == nullptr)
		return;
	dv->origin = origin;
	construct_ray_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_ray_direction(int handle, vec3 direction)
{
	debug_value_ray* dv = retrieve_debug_value<debug_value_ray>(handle);
	if (dv == nullptr)
		return;
	dv->direction = direction;
	construct_ray_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_cylinder(int handle, vec3 origin, vec3 direction,
	float radius)
{
	debug_value_cylinder* dv = retrieve_debug_value<debug_value_cylinder>(handle);
	if (dv == nullptr)
		return;
	dv->origin = origin;
	dv->direction = direction;
	dv->radius = radius;
	construct_cylinder_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_cylinder_origin(int handle, vec3 origin)
{
	debug_value_cylinder* dv = retrieve_debug_value<debug_value_cylinder>(handle);
	if (dv == nullptr)
		return;
	dv->origin = origin;
	construct_cylinder_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_cylinder_direction(int handle, vec3 direction)
{
	debug_value_cylinder* dv = retrieve_debug_value<debug_value_cylinder>(handle);
	if (dv == nullptr)
		return;
	dv->direction = direction;
	construct_cylinder_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_cylinder_radius(int handle, float radius)
{
	debug_value_cylinder* dv = retrieve_debug_value<debug_value_cylinder>(handle);
	if (dv == nullptr)
		return;
	dv->radius = radius;
	construct_cylinder_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_box(int handle, vec3 origin, vec3 extent)
{
	debug_value_box* dv = retrieve_debug_value<debug_value_box>(handle);
	if (dv == nullptr)
		return;
	dv->origin = origin;
	dv->extent = extent;
	construct_box_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_box_origin(int handle, vec3 origin)
{
	debug_value_box* dv = retrieve_debug_value<debug_value_box>(handle);
	if (dv == nullptr)
		return;
	dv->origin = origin;
	construct_box_geometry(dv);
}

void cgv::nui::debug_visualization_helper::update_debug_value_box_extent(int handle, vec3 extent)
{
	debug_value_box* dv = retrieve_debug_value<debug_value_box>(handle);
	if (dv == nullptr)
		return;
	dv->extent = extent;
	construct_box_geometry(dv);
}
