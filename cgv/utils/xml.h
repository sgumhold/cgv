#pragma once

/** \file xml.h
 * Helper functions to process XML files.
 */

#include <string>
#include <map>

#include "advanced_scan.h"

#include "lib_begin.h"

namespace cgv {
	namespace utils {

typedef std::pair<std::string, std::string> xml_attribute;
typedef std::map<xml_attribute::first_type, xml_attribute::second_type> xml_attribute_map;

enum XMLTagType {
	XTT_UNDEF, // undefined
	XTT_OPEN,  // opening/starting tag
	XTT_CLOSE, // closing tag
	XTT_SINGLE // singleton or self closing tag
};

struct xml_tag {
	XMLTagType type = XTT_UNDEF;
	std::string name = "";
	xml_attribute_map attributes;
};

extern CGV_API xml_attribute xml_read_attribute(const std::string& attribute);

extern CGV_API xml_tag xml_read_tag(const std::string& str);

	}
}

// maybe useful (save parsing of strings to numbers
/*
static bool xml_attribute_to_int(const std::string& attribute, int& value) {

	std::string value_str = xml_attribute_value(attribute);

	if(!value_str.empty()) {
		int value_i = 0;

		try {
			value_i = stoi(value_str);
		} catch(const std::invalid_argument&) {
			return false;
		} catch(const std::out_of_range&) {
			return false;
		}

		value = value_i;
		return true;
	}

	return false;
}

static bool xml_attribute_to_float(const std::string& attribute, float& value) {

	std::string value_str = xml_attribute_value(attribute);

	if(!value_str.empty()) {
		float value_f = 0.0f;

		try {
			value_f = stof(value_str);
		} catch(const std::invalid_argument&) {
			return false;
		} catch(const std::out_of_range&) {
			return false;
		}

		value = value_f;
		return true;
	}

	return false;
}
*/

#include <cgv/config/lib_end.h>