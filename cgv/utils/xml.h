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

#include <cgv/config/lib_end.h>