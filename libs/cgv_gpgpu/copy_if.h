#pragma once

#include "write_if.h"

namespace cgv {
namespace gpgpu {

namespace generic {

/// GPU compute shader implementation for copying values based on a boolean predicate.
class copy_if : public detail::write_if {
public:
	copy_if(GroupSize group_size = k_default_group_size) : write_if("copy_if", OutputMode::Values, group_size) {}
};

} // namespace generic

/// GPU compute shader implementation for copying values based on a boolean predicate.
template<class T>
class copy_if : public detail::write_if<T> {
public:
	copy_if(GroupSize group_size = k_default_group_size) : write_if<T>("copy_if", OutputMode::Values, group_size) {}
};

} // namespace gpgpu
} // namespace cgv
