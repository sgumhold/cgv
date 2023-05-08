#include "caching_multi_transforming.h"
#include <cgv/math/inv.h>

void cgv::nui::caching_multi_transforming::set_active_object(int primitive_index)
{
	if (primitive_index >= -1 && primitive_index < sub_matrices.size())
		active_object = primitive_index;
}

int cgv::nui::caching_multi_transforming::get_active_object() const
{
	return active_object;
}

int cgv::nui::caching_multi_transforming::add_sub_object_transform(const mat4& transform)
{
	sub_matrices.push_back(transform);
	inverse_sub_matrices.push_back(inv(transform));
	return sub_matrices.size() - 1;
}

int cgv::nui::caching_multi_transforming::add_sub_object_transform(const mat4& transform, int index)
{
	sub_matrices.insert(next(sub_matrices.begin(), index), transform);
	inverse_sub_matrices.insert(next(inverse_sub_matrices.begin(), index), inv(transform));
	return index;
}

void cgv::nui::caching_multi_transforming::remove_sub_object_transform()
{
	sub_matrices.pop_back();
	inverse_sub_matrices.pop_back();
	if (active_object == sub_matrices.size())
		active_object -= 1;
}

void cgv::nui::caching_multi_transforming::remove_sub_object_transform(int index)
{
	sub_matrices.erase(next(sub_matrices.begin(), index));
	inverse_sub_matrices.erase(next(inverse_sub_matrices.begin(), index));
	if (active_object == sub_matrices.size())
		active_object -= 1;
}

cgv::render::render_types::mat4 cgv::nui::caching_multi_transforming::get_model_transform() const
{
	if (active_object == -1)
		return M;
	return sub_matrices[active_object];
}

cgv::render::render_types::mat4 cgv::nui::caching_multi_transforming::get_inverse_model_transform() const
{
	if (active_object == -1)
		return iM;
	return inverse_sub_matrices[active_object];
}

void cgv::nui::caching_multi_transforming::set_model_transform(const mat4& _M)
{
	if (active_object == -1) {
		M = _M;
		iM = inv(_M);
	}
	else {
		sub_matrices[active_object] = _M;
		inverse_sub_matrices[active_object] = inv(_M);
	}
}

void cgv::nui::caching_multi_transforming::set_model_transform(const mat4& _M, const mat4& _iM)
{
	if (active_object == -1) {
		M = _M;
		iM = _iM;
	} else {
		sub_matrices[active_object] = _M;
		inverse_sub_matrices[active_object] = _iM;
	}
}
