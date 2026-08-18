#include "error.h"

namespace cgv {
namespace gpgpu {

std::string to_string(errc e) {
	switch(e) {
	case errc::invalid_argument:
		return "invalid argument";
	case errc::kernel_not_initialized:
		return "compute shader kernel could not be initialized";
	case errc::type_not_supported:
		return "data type is not supported";
	case errc::texture_type_not_supported:
		return "texture type is not supported";
	case errc::invalid_type:
		return "invalid data type";
	case errc::size_too_large:
		return "count exceeds device capabilities";
	case errc::not_enough_shared_memory:
		return "request exceeds available shared memory";
	case errc::invalid_range:
		return "the given iterators define an invalid range";
	case errc::overlapping_range:
		return "the given iterators point to an overlapping range";
	case errc::iterators_not_compatible:
		return "the given iterators do not point to the same range";
	case errc::buffer_copy_to_device_error:
		return "could not copy buffer from host to device memory";
	case errc::buffer_copy_to_host_error:
		return "could not copy buffer from device to host memory";
	case errc::cyclic_type_dependency:
		return "at least two data types have a cyclic dependency";
	case errc::duplicate_type_name:
		return "encountered duplicate type names for different data types";
	default:
		return "(unknown error)";
	}
}

errc error::code() const {
	return _e;
}

std::string error::message() const {
	return to_string(_e) + ": " + _description;
}

bool error::operator==(const error& other) const {
	return _e == other._e;
}

bool error::operator==(const errc& e) const {
	return _e == e;
}

error::operator bool() const {
	return _e != errc{};
}

std::ostream& operator<<(std::ostream& os, const error& e) {
	os << e.message();
	return os;
}

} // namespace gpgpu
} // namespace cgv
