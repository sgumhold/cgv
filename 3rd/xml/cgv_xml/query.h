#pragma once

// This file contains implementation of helper methods and classes to perform query operations on tinyxml2 documents.

#include <string>

#include "../tinyxml2/tinyxml2.h"

namespace cgv {
namespace xml {

class FindElementByNameVisitor : public tinyxml2::XMLVisitor {
private:
	std::string name = "";
	const tinyxml2::XMLElement* result = nullptr;

public:
	FindElementByNameVisitor() {}

	FindElementByNameVisitor(const std::string& name) : name(name) {}

	void SetQueryName(const std::string& name) {

		this->name = name;
	}

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

}
}