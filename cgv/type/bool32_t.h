#pragma once

#include <cstdint>
#include <utility>

namespace cgv {
namespace type {

/// @brief Class type that emulates the primitive bool data type by storing its value as a 32-bit integer.
/// Allows representing boolean values with a size and alignment of 4 bytes. Is implicitly convertible to bool
/// to enable default operator behaviour.
/// Typical uses include using this class instead of the raw bool type to avoiding the specialization and packing
/// of std::vector and to allow alternative struct packing.
class bool32_t {
public:
	bool32_t() = default;
	bool32_t(const bool32_t&) = default;
	bool32_t& operator=(const bool32_t&) = default;
	bool32_t(bool32_t&&) noexcept = default;
	bool32_t& operator=(bool32_t&&) noexcept = default;

	// construct from primitive bool type
	bool32_t(const bool& other) : v_(other) {};

	// assign from primitive bool type
	bool32_t& operator=(const bool& other) {
		v_ = other;
		return *this;
	};

	// move construct from primitive bool type
	bool32_t(bool&& other) noexcept : v_(std::move(other)) {};

	// move assign from primitive bool type
	bool32_t& operator=(bool&& other) noexcept {
		v_ = std::move(other);
		return *this;
	};

	// convert to primitive bool type
	operator bool() const {
		return static_cast<bool>(v_);
	}

private:
	// store the value as an int32_t to occupy 4 bytes of memory
	int32_t v_;
};

} // namespace type
} // namespace cgv
