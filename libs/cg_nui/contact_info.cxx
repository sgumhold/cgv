#include "contact_info.h"
#include "primitive_container.h"
#include "ray_axis_aligned_box_intersection.h"
#include "nui_node.h"

namespace cgv {
	namespace nui {

void contact_info::contact::update_orientation(const quat& q)
{
	if (container && primitive_index != -1) {
		if (container->rotatable())
			container->set_orientation(primitive_index, q);
	}
	else if (node->rotatable())
		node->set_rotation(q);
}
void contact_info::contact::update_position(const vec3& r)
{
	if (container && primitive_index != -1) {
		if (container->translatable())
			container->set_position(primitive_index, r);
	}
	else if (node->translatable())
		node->set_translation(r);
}

bool contact_info::contact::check_box_update(const box3& old_box, const box3& new_box) const
{
	nui_node* N = 0;
	if (container && primitive_index != -1) {
		if (container->check_box_update(old_box, new_box))
			N = node;
	}
	else {
		nui_node* P = get_parent();
		if (P && P->check_box_update(old_box, new_box))
			N = P;
	}
	bool result = N != 0;
	while (N) {
		N->set_box_outofdate();
		N = &(*(N->get_parent()->cast<nui_node>()));
	}
	return result;
}

bool contact_info::contact::approachable() const
{
	if (container && primitive_index != -1)
		return container->approachable();
	return node->approachable();
}

bool contact_info::contact::translatable() const
{
	if (container && primitive_index != -1)
		return container->translatable();
	return node->translatable();
}

bool contact_info::contact::rotatable() const
{
	if (container && primitive_index != -1)
		return container->rotatable();
	return node->rotatable();
}

contact_info::box3 contact_info::contact::get_oriented_bounding_box() const
{
	if (container && primitive_index != -1)
		return container->get_oriented_bounding_box(primitive_index);
	box3 B = node->compute_bounding_box();
	switch (node->get_scaling_mode()) {
	case SM_UNIFORM: B.scale(node->get_scale()[0]); break;
	case SM_NON_UNIFORM :
		B.ref_min_pnt() *= node->get_scale();
		B.ref_max_pnt() *= node->get_scale();
		break;
	}
	B.translate(node->get_translation());
	return B;
}

contact_info::vec3 contact_info::contact::get_position() const
{
	if (container && primitive_index != -1)
		return container->get_position(primitive_index);
	return node->get_translation();
}

contact_info::quat contact_info::contact::get_orientation() const
{
	if (container && primitive_index != -1)
		return container->get_orientation(primitive_index);
	return node->get_rotation();
}


contact_info::box3 contact_info::contact::get_bounding_box() const
{
	if (container && primitive_index != -1)
		return container->get_bounding_box(primitive_index);
	return node->compute_and_transform_bounding_box();
}

nui_node* contact_info::contact::get_parent() const
{
	if (container && primitive_index != -1)
		return node;
	return &(*(node->get_parent()->cast<nui_node>()));
}

bool contact_info::consider_closest_contact(const contact& C)
{
	if (contacts.empty())
		contacts.push_back(C);
	else
		if (contacts.front().distance <= C.distance)
			return false;
	contacts.front() = C;
	return true;
}
/*
	contact_info::contact& C = contacts.front();
	C.distance = distance;
	C.position = p;
	C.normal = n;
	C.texcoord = tc;
	C.primitive_index = i;
	C.container = _container;
	C.node = (_container && _node == 0) ? _container->get_parent() : _node;
	return true;
}
*/

void compute_closest_box_point(contact_info::contact& C, const contact_info::vec3& pos,
	const contact_info::vec3& center, const contact_info::vec3& extent, const contact_info::quat* orientation_ptr)
{
	bool result = false;
	contact_info::vec3 h = 0.5f * extent;
	contact_info::vec3& p = C.position;
	p = pos-center;
	if (orientation_ptr)
		orientation_ptr->inverse_rotate(p);
	bool inside[3];
	float closest[3];
	float distances[3];
	int inside_count = 0;
	for (int j = 0; j < 3; ++j) {
		closest[j] = p[j] < 0.0f ? -1.0f : 1.0f;
		distances[j] = fabs(p[j] - closest[j] * h[j]);
		inside[j] = (p[j] >= -h[j]) && (p[j] <= h[j]);
		if (inside[j])
			++inside_count;
	}
	contact_info::vec3& n = C.normal;
	n.zeros();
	if (inside_count == 3) {
		int j_min = distances[1] < distances[0] ? 1 : 0;
		if (distances[2] < distances[j_min])
			j_min = 2;
		C.distance = distances[j_min];
		n[j_min] = closest[j_min];
		p[j_min] = closest[j_min] * h[j_min];
	}
	else {
		float sqr_dist = 0;
		for (int j = 0; j < 3; ++j) {
			if (!inside[j]) {
				sqr_dist += distances[j] * distances[j];
				n[j] = closest[j];
				p[j] = closest[j] * h[j];
			}
		}
		C.distance = sqrt(sqr_dist);
		n.normalize();
	}
	contact_info::vec3& tc = C.texcoord;
	tc = (p + h) / extent;
	if (orientation_ptr) {
		orientation_ptr->rotate(p);
		orientation_ptr->rotate(n);
	}
	p += center;
}

int compute_box_intersection(
	const contact_info::vec3& box_center, const contact_info::vec3& box_extent,
	const contact_info::vec3& ray_start,  const contact_info::vec3& ray_direction, contact_info::contact& C, contact_info::contact* C2_ptr)
{
	float t_result;
	contact_info::vec3 p_result, n_result;
	contact_info::box3 B(-0.5f * box_extent, 0.5f * box_extent);
	if (!ray_axis_aligned_box_intersection(ray_start - box_center, ray_direction, B, t_result, p_result, n_result, 0.000001f))
		return 0;
	C.distance = t_result;
	C.position = p_result + box_center;
	C.normal = n_result;
	C.texcoord = (p_result + 0.5f * box_extent) / box_extent;
	return 1;
}

int compute_box_intersection(
	const contact_info::vec3& box_center, const contact_info::vec3& box_extent, const contact_info::quat& box_rotation,
	const contact_info::vec3& ray_start,  const contact_info::vec3& ray_direction, contact_info::contact& C, contact_info::contact* C2_ptr)
{
	contact_info::vec3 ro = ray_start - box_center;
	box_rotation.inverse_rotate(ro);
	ro += box_center;
	contact_info::vec3 rd = ray_direction;
	box_rotation.inverse_rotate(rd);
	int cnt = compute_box_intersection(box_center, box_extent, ro, rd, C, C2_ptr);
	// transform result back
	if (cnt > 0) {
		C.position = box_rotation.get_rotated(C.position - box_center) + box_center;
		box_rotation.rotate(C.normal);
	}
	if (cnt > 1) {
		C2_ptr->position = box_rotation.get_rotated(C2_ptr->position - box_center) + box_center;
		box_rotation.rotate(C2_ptr->normal);
	}
	return cnt;
}

	}
}

