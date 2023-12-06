#include "value_input.h"

namespace cg {
namespace g2d {

// TODO: make static member of value_input_control?
void value_input_cb(widget*, void* v) {
	value_input& value_input_widget = *(value_input*)v;

	double next_value;
	if(value_input_widget.get_step() >= 1.0) {
		try {
			next_value = static_cast<double>(std::stol(value_input_widget.input_widget.get_value()));
		} catch(std::exception& e) {
			return;
		}
	} else {
		try {
			next_value = std::stod(value_input_widget.input_widget.get_value());
		} catch(std::exception& e) {
			return;
		}
	}

	if(next_value != value_input_widget.get_value()) {
		value_input_widget.set_value(next_value);
		value_input_widget.do_callback();
	}
}

value_input::value_input(const std::string& label, cgv::g2d::irect rectangle) : valuator(label, rectangle), input_widget(label, rectangle) {
	input_widget.set_callback(value_input_cb, this);
	input_widget.type = input::Type::kFloat;

	update_input();
}

bool value_input::set_value(double v) {
	if(valuator::set_value(v)) {
		update_input();
		return true;
	}

	return false;
}

void value_input::update_input() {
	// Only redraw the text if the numeric value is different..
	if(!input_widget.get_value().empty()) {
		if(get_step() >= 1.0) {
			int v = 0;

			try {
				v = std::stol(input_widget.get_value());
			} catch(std::exception& e) {
				return;
			}

			if(v == static_cast<int>(get_value()))
				return;
		} else {
			// parse the existing text and see if it is close enough to number
			double v = 0.0;
			try {
				v = std::stod(input_widget.get_value());
			} catch(std::exception& e) {
				return;
			}

			if(std::round(get_value()) != std::round(v) ||
			   std::signbit(get_value()) != std::signbit(v) ||
#ifdef _WIN32
			   (v ?
				(std::abs(std::abs(get_value() / v) - 1.0) > 1.2e-7) :
				(get_value() != 0.0)
			   )
#else
			   float(get_value()) != float(v)
#endif
			   ) {
				;
			} else {
				return; // it is close enough, leave it alone
			}
		}
	}

	char buf[32];
	if(std::floor(get_value()) == get_value()) {
		snprintf(buf, 32, "%d", static_cast<int>(get_value()));
	} else {
		// this is a very brute force way to allow 6 digits to the right instead
		// of the %g default of 4:
		int n = static_cast<int>(std::ceil(std::log10(std::abs(get_value()))));
		if(n > 0 || n < -6) {
			snprintf(buf, 32, "%g", get_value());
		} else {
			snprintf(buf, 32, "%.7f", get_value());
			// strip trailing 0's and the period:
			char* s = buf;
			while(*s)
				s++;
			s--;
			while(s > buf && *s == '0')
				s--;
			if(*s == '.')
				s--;
			s[1] = 0;
		}
	}

	input_widget.set_value(std::string(buf));
}

}
}
