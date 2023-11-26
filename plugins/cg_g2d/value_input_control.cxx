#include "value_input_control.h"

namespace cg {
namespace g2d {

// TODO: make static member of value_input_control
void value_input_cb(control_base*, void* v) {
	value_input_control& t = *(value_input_control*)v;
	double next_value;
	if(t.get_step() >= 1.0)
		next_value = strtol(t.input.get_value().c_str(), 0, 0);
	else
		next_value = strtod(t.input.get_value().c_str(), 0);

	if(next_value != t.get_value()) {
		t.set_value(next_value);
		t.do_callback();
	}
}

value_input_control::value_input_control(const std::string& label, cgv::g2d::irect rectangle) : value_control(label, rectangle), input(label, rectangle) {
	input.callback(value_input_cb, this);
	// TODO: set from ouside and allow integer
	input.type = input_control::input_type::kFloat;
}

bool value_input_control::set_value(double v) {
	// TODO: set the buffer here or in update?
	char buf[32];
	if(std::floor(v) == v) {
		snprintf(buf, 32, "%d", static_cast<int>(v));
	} else {
		// this is a very brute force way to allow 6 digits to the right instead
		// of the %g default of 4:
		int n = (int)ceil(log10(fabs(v)));
		if(n > 0 || n < -6) {
			snprintf(buf, 32, "%g", v);
		} else {
			snprintf(buf, 32, "%.7f", v);
			// strip trailing 0's and the period:
			char* s = buf; while(*s) s++; s--;
			while(s > buf && *s == '0') s--;
			if(*s == '.') s--;
			s[1] = 0;
		}
	}

	std::string str(buf);

	input.set_value(str);

	return value_control::set_value(v);
}

}
}
