#pragma once

// This file contains implementation of helper methods and classes to perform print operations on a tinyxml2::XMLPrinter.

#include <cgv/media/color.h>
#include <cgv/math/fvec.h>
#include <cgv/utils/scan.h>

#include "../tinyxml2/tinyxml2.h"

namespace cgv {
namespace xml {

void PushAttribute(tinyxml2::XMLPrinter& printer, const std::string& name, const std::string& value) {

	printer.PushAttribute(name.c_str(), value.c_str());
};

void PushAttribute(tinyxml2::XMLPrinter& printer, const std::string& name, bool value) {

	std::string str = value ? "true" : "false";
	PushAttribute(printer, name, str);
};

template<typename T, cgv::type::uint32_type N>
void PushAttribute(tinyxml2::XMLPrinter& printer, const std::string& name, const cgv::math::fvec<T, N>& value) {

	PushAttribute(printer, name, cgv::math::to_string(value));
};

void PushAttribute(tinyxml2::XMLPrinter& printer, const std::string& name, const cgv::media::color<float, cgv::media::RGB>& value) {

	cgv::math::fvec<float, 3u> vec(value.R(), value.G(), value.B());
	PushAttribute(printer, name, cgv::math::to_string(vec));
};

}
}