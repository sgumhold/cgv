#pragma once

// This file contains implementation of helper methods and classes to perform query operations on a tinyxml2::XMLDocument.

#include <cgv/media/color.h>
#include <cgv/math/fvec.h>
#include <cgv/utils/scan.h>

#include "../tinyxml2/tinyxml2.h"

namespace cgv {
namespace xml {

/// @brief Finds a tinyxml2::XMLElement by name.
///
/// Implements a tinyxml2::XMLVisitor for tinyxml2::XMLNode::Accept.
/// 
/// Example usage:
/// @code
/// tinyxml2::XMLDocument document;
/// cgv::xml::FindElementByNameVisitor findElementByName("Name");
/// document.Accept(&findElementByName);
/// if(auto color_maps_elem = findElementByName.Result()) {
///		// element with name was found
/// }
/// @endcode
class FindElementByNameVisitor : public tinyxml2::XMLVisitor {
private:
	std::string name = "";
	const tinyxml2::XMLElement* result = nullptr;

public:
	FindElementByNameVisitor() {}

	FindElementByNameVisitor(const std::string& name) : name(name) {}

	/// @brief Changes the name used to find a matching element.
	/// @param name the query name string.
	void SetQueryName(const std::string& name) {

		this->name = name;
	}

	/// @brief Get the query result.
	/// @return the pointer to the matching tinyxml::XMLElement; if no match was found return nullptr
	const tinyxml2::XMLElement* Result() const {

		return result;
	}

	bool VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* attribute) override {

		if(strcmp(element.Name(), name.c_str()) == 0) {
			result = &element;
			return false;
		}

		return true;
	}
};

/// @brief Query a string attribute from a tinyxml::XMLElement using std::string directly instead of a c-style string.
/// @param [in] elem the tinyxml::XMLElement to query the attribute from.
/// @param [in] name the name string of the query attribute.
/// @param [out] value of the query attribute.
/// @return the tinyxml2::XMLError state indicating success or failure of the query.
static tinyxml2::XMLError QueryStringAttribute(const tinyxml2::XMLElement& elem, const std::string& name, std::string& value) {

	const char* c = "";
	tinyxml2::XMLError result = elem.QueryStringAttribute(name.c_str(), &c);
	if (result == tinyxml2::XML_SUCCESS)
		value = std::string(c);

	return result;
};

/// @brief Query a boolean attribute from a tinyxml::XMLElement.
/// 
/// On successful read, the attribute value is interpreted as true if the string reads
/// 'true' or '1' and false if the string reads 'false', '0' or is empty. Case is ignored. All
/// other values return a tinyxml2::XMLError::XML_WRONG_ATTRIBUTE_TYPE error.
/// 
/// @param [in] elem the tinyxml::XMLElement to query the attribute from.
/// @param [in] name the name string of the query attribute.
/// @param [out] value of the query attribute.
/// @return the tinyxml2::XMLError state indicating success or failure of the query.
static tinyxml2::XMLError QueryBoolAttribute(const tinyxml2::XMLElement& elem, const std::string& name, bool& value) {

	std::string str = "";
	tinyxml2::XMLError result = QueryStringAttribute(elem, name, str);
	if(result == tinyxml2::XML_SUCCESS) {
		if(str.length() > 0) {
			str = cgv::utils::to_lower(str);

			if(str[0] == '0' || str == "false") {
				value = false;
				return result;
			} else if(str[0] == '1' || str == "true") {
				value = true;
				return result;
			}

			return tinyxml2::XMLError::XML_WRONG_ATTRIBUTE_TYPE;
		} else {
			value = false;
			return result;
		}
	}

	return result;
};

/// @brief Query a cgv::math::fvec<T, N> attribute from a tinyxml::XMLElement.
/// 
/// Converts a given string attribute value into a cgv::math::fvec<T, N> with component
/// type T and N-dimensions. Components have to be separated by exactly one whitespace
/// (and optionally a comma). The entire string can optionally be enclosed in parentheses.
/// 
/// Supported formats for, e.g. a 3-dimensional vector, are:
/// value="x y z"; value="x, y, z"; value="(x y z)" and value="(x, y, z)".
/// 
/// @param [in] elem the tinyxml::XMLElement to query the attribute from.
/// @param [in] name the name string of the query attribute.
/// @param [out] value of the query attribute.
/// @return the tinyxml2::XMLError state indicating success or failure of the query.
template <typename T, cgv::type::uint32_type N>
static tinyxml2::XMLError QueryVecAttribute(const tinyxml2::XMLElement& elem, const std::string& name, cgv::math::fvec<T, N>& value) {

	std::string str = "";
	tinyxml2::XMLError result = QueryStringAttribute(elem, name, str);
	if(result == tinyxml2::XML_SUCCESS) {
		cgv::utils::remove(cgv::utils::trim(str, "()"), ',');
		
		if(!cgv::math::from_string(str, value))
			return tinyxml2::XMLError::XML_WRONG_ATTRIBUTE_TYPE;
	}

	return result;
};

/// @brief Query a cgv::media::color<float, cgv::media::RGB> attribute from a tinyxml::XMLElement.
///
/// Converts a given string attribute value into a cgv::media::color<float, cgv::media::RGB> using
/// QueryVecAttribute() with component type float and dimensionality 3.
///
/// @param [in] elem the tinyxml::XMLElement to query the attribute from.
/// @param [in] name the name string of the query attribute.
/// @param [out] value of the query attribute.
/// @return the tinyxml2::XMLError state indicating success or failure of the query.
static tinyxml2::XMLError QueryRGBAttribute(const tinyxml2::XMLElement& elem, const std::string& name, cgv::media::color<float, cgv::media::RGB>& value) {
	
	cgv::math::fvec<float, 3u> vec(0.0f);
	tinyxml2::XMLError result = QueryVecAttribute(elem, name, vec);
	if(result == tinyxml2::XML_SUCCESS)
		value = cgv::media::color<float, cgv::media::RGB>(vec[0], vec[1], vec[2]);

	return result;
};

}
}