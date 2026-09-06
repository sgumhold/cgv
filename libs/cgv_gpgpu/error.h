#pragma once

#include <sstream>
#include <string>
#include <system_error>

#include "lib_begin.h"

#define CGV_GPGPU_DISABLE_DERIVED_TYPES(TYPE) typename std::enable_if<!std::is_base_of<TYPE, T>::value, bool>::type = true

namespace cgv {
namespace gpgpu {

// Defines error conditions for algorithm implementations. 
enum class errc {
	// 0 is reserved for no error/success
	invalid_argument,
	kernel_not_initialized = 1,
	type_not_supported,
	texture_type_not_supported,
	invalid_type,
	size_too_large,
	not_enough_shared_memory,
	invalid_range,
	overlapping_range,
	iterators_not_compatible,
	buffer_copy_to_device_error,
	buffer_copy_to_host_error,
	cyclic_type_dependency,
	duplicate_type_name,
};


extern CGV_API std::string to_string(errc e);

class CGV_API error {
public:
	error() {}
	error(errc e) : _e(e) {}
	error(errc e, const std::string& description) : _e(e), _description(description) {}

	errc code() const;

	std::string message() const;

	bool operator==(const error& other) const;
	bool operator==(const errc& e) const;

	operator bool() const;

	friend CGV_API std::ostream& operator<<(std::ostream& os, const error& e);

private:
	errc _e = {};
	std::string _description;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
