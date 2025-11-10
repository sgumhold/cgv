#pragma once

#include "write_if.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation that outputs indices of elements in a range based on a boolean predicate.
class select_if : public detail::write_if {
public:
	select_if() : detail::write_if("select_if", true) {}
};

} // namespace gpgpu
} // namespace cgv
