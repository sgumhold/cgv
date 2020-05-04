#include "primitive_container.h"
#include "nui_node.h"

namespace cgv {
	namespace nui {

primitive_container::primitive_container(nui_node* _parent, PrimitiveType _type, bool _use_colors, bool _use_orientations, ScalingMode _scaling_mode, InteractionCapabilities ic)
	: parent(_parent), type(_type), use_colors(_use_colors), use_orientations(_use_orientations), nui_interactable(_scaling_mode, ic)
{
}

nui_node* primitive_container::get_parent() const
{
	return parent;
}

primitive_container::~primitive_container()
{
}

primitive_container::box3 primitive_container::get_bounding_box(uint32_t i) const
{
	box3 b = get_oriented_bounding_box(i);
	if (!use_orientations)
		return b;
	const quat& q = orientations[i];
	box3 B;
	for (int i = 0; i < 8; ++i)
		B.add_point(q.get_rotated(b.get_corner(i) - b.get_center())+b.get_center());
	return B;
}

/// last parameter is weight for trading between position and normal distances for closest oriented point query; default implementation defers call to computer_closest_point
bool primitive_container::compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight)
{
	return compute_closest_point(info, pos);
}

void primitive_container::prepare_render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr) const
{
	r.set_render_style(rs);
	r.set_position_array(ctx, center_positions);
	if (use_colors)
		r.set_color_array(ctx, colors);
	if (indices_ptr)
		r.set_indices(ctx, *indices_ptr);
}

bool primitive_container::render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr) const
{
	prepare_render(ctx, r, rs);
	if (!r.validate_and_enable(ctx))
		return false;
	if (indices_ptr)
		r.draw(ctx, 0, indices_ptr->size(), true);
	else
		r.draw(ctx, 0, center_positions.size());
	r.disable(ctx);
	return false;
}

primitive_container::quat primitive_container::get_orientation(uint32_t i)
{
	if (!use_orientations)
		return quat(1.0f, 0.0f, 0.0f, 0.0f);
	return orientations[i];
}

bool primitive_container::set_orientation(uint32_t i, const quat& q)
{
	if (!use_orientations)
		return false;
	orientations[i] = q;
	return true;
}
primitive_container::vec3 primitive_container::get_position(uint32_t i) const
{
	return center_positions[i];
}
void primitive_container::set_position(uint32_t i, const vec3& p)
{
	center_positions[i] = p;
}

float primitive_container::get_uniform_scale(uint32_t i) const
{
	if (scaling_mode != SM_UNIFORM)
		return 1.0f;
	return uniform_scales[i];
}

bool primitive_container::set_uniform_scale(uint32_t i, float u)
{
	if (scaling_mode != SM_UNIFORM)
		return false;
	uniform_scales[i] = u;
	return true;
}
primitive_container::vec3 primitive_container::get_scale(uint32_t i) const
{
	if (scaling_mode != SM_NON_UNIFORM)
		return vec3(1.0f);
	return scales[i];
}

bool primitive_container::set_scale(uint32_t i, const vec3& s)
{
	if (scaling_mode != SM_UNIFORM)
		return false;
	scales[i] = s;
	return true;
}

	}
}