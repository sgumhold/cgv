#include "bounding_box_cache.h"

namespace cgv {
	namespace nui {

bounding_box_cache::bounding_box_cache()
{
	box_outofdate = true;
}

void bounding_box_cache::check_box_update(const box3& old_box, const box3& new_box) const
{
	if (box_outofdate)
		return;

	// check whether we cannot update the box
	for (uint32_t i = 0; i < 3; ++i) {
		if (old_box.get_min_pnt()(i) == box.get_min_pnt()(i)) {
			if (new_box.get_min_pnt()(i) <= old_box.get_min_pnt()(i))
				box.ref_min_pnt()(i) = new_box.get_min_pnt()(i);
			else {
				box_outofdate = true;
				return;
			}
		}
		if (old_box.get_max_pnt()(i) == box.get_max_pnt()(i)) {
			if (new_box.get_max_pnt()(i) >= old_box.get_max_pnt()(i))
				box.ref_max_pnt()(i) = new_box.get_max_pnt()(i);
			else {
				box_outofdate = true;
				return;
			}
		}
	}
	// finally extend box by new box
	box.add_axis_aligned_box(new_box);
}

const bounding_box_cache::box3& bounding_box_cache::compute_bounding_box() const
{
	if (box_outofdate) {
		box.invalidate();
		for (uint32_t i = 0; i < get_nr_primitives(); ++i)
			box.add_axis_aligned_box(get_bounding_box(i));
		box_outofdate = false;
	}
	return box;
}

	}
}