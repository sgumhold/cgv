#include "bounding_box_cache.h"

namespace cgv {
	namespace nui {

bounding_box_cache::bounding_box_cache()
{
	box_outofdate = true;
}

bool bounding_box_cache::check_box_update(const box3& old_box, const box3& new_box) const
{
	if (box_outofdate)
		return false;

	// flag that tells whether box has been updated
	bool result = false;
	for (uint32_t i = 0; i < 3; ++i) {
		// check for update of min pnt
		if (new_box.get_min_pnt()(i) < box.get_min_pnt()(i)) {
			box.ref_min_pnt()(i) = new_box.get_min_pnt()(i);
			result = true;
		}
		// check for potential shrinkage of box, where recomputation is necessary
		else if ((new_box.get_min_pnt()(i) > box.get_min_pnt()(i)) && (old_box.get_min_pnt()(i) == box.get_min_pnt()(i))) {
			box_outofdate = true;
			return true;
		}
		// check for update of max pnt
		if (new_box.get_max_pnt()(i) > box.get_max_pnt()(i)) {
			box.ref_max_pnt()(i) = new_box.get_max_pnt()(i);
			result = true;
		}
		// check for potential shrinkage of box, where recomputation is necessary
		else if ((new_box.get_max_pnt()(i) < box.get_max_pnt()(i)) && (old_box.get_max_pnt()(i) == box.get_max_pnt()(i))) {
			box_outofdate = true;
			return true;
		}
	}
	return result;
}

void bounding_box_cache::set_box_outofdate()
{
	box_outofdate = true;
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