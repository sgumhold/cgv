#pragma once

#include <string>

#include "lib_begin.h" // @<

namespace test { // @<
	namespace cgv { // @<

/// some base class @>
class my_base_class : public std::string
{
protected:
	/// @>
	enum DummyEnum { UPS, APS, SIDE };
	/// @>
	string name;
public:
};// @<

	} // @<
} // @<

#include <cgv/config/lib_end.h> // @<