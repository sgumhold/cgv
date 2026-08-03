#pragma once

#include "write_if.h"

namespace cgv {
namespace gpgpu {

namespace generic {

/// GPU compute shader implementation that outputs indices of elements in a range based on a boolean predicate.
class select_if : public detail::write_if {
public:
	select_if(GroupSize group_size = k_default_group_size) : write_if("select_if", OutputMode::Indices, group_size) {}
};

} // namespace generic

/// GPU compute shader implementation that outputs indices of elements in a range based on a boolean predicate.
template<class T>
class select_if : public detail::write_if<T> {
public:
	select_if(GroupSize group_size = k_default_group_size) : write_if<T>("select_if", OutputMode::Indices, group_size) {}
};

} // namespace gpgpu
} // namespace cgv
