#pragma once

#include "write_if.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for copying values based on a boolean predicate.
class copy_if : public detail::write_if {
public:
	copy_if() : detail::write_if("copy_if", false) {}
};

} // namespace gpgpu
} // namespace cgv
