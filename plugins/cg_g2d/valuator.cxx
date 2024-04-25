#include "valuator.h"

namespace cg {
namespace g2d {

bool valuator::set_value(double v) {
	if(abs(v - value) < std::numeric_limits<double>::epsilon())
		return false;

	value = v;
	update();
	return true;
}

void valuator::set_range(cgv::dvec2 range) {
	this->range = range;

	// TODO: udpate value to be in range
	update();
}

void valuator::handle_value_change(double v) {
	// round to nearest multiple of step
	if(step >= 1.0) {
		v = std::round(v / step) * step;
	} else if(step > 0.0) {
		// Try to detect fractions like .1 which are actually stored as
		// .9999999 and thus would round to unexpected values. This is done
		// by seeing if 1/N is close to an integer:
		double is = std::round(1.0 / step);
		if(std::abs(is * step - 1.0) < 0.001)
			v = std::round(v * is) / is;
		else
			v = std::round(v / step) * step;
	} else {
		// check for them incrementing toward zero and don't produce tiny
		// numbers:
		//if(previous_value_ && fabs(v / previous_value_) < 1e-5) v = 0;
	}

	// If original value was in-range, clamp the new value:
	double min = range[0];
	double max = range[1];

	if(min > max)
		std::swap(min, max);

	//if(v < A && previous_value_ >= A)
	//	v = A;
	//else if(v > B && previous_value_ <= B)
	//	v = B;
	// 
	// store the value, redraw the widget, and do callback:

	if(set_value(v))
		do_callback();
}

}
}
