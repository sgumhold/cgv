#pragma once

#include <cstdint>

#include <cgv/media/color_scale.h>
#include <cgv/media/transfer_function.h>
#include <cgv/type/bool32_t.h>

#include "lib_begin.h"

namespace cgv {
namespace render {

/// @brief Sample mode of the device color scale.
enum class DeviceColorScaleSampleMode : uint8_t {
	kContinuous = 0,
	kDiscrete = 1
};

/// @brief Flags that represent boolean mapping options of the color scales.
enum class DeviceColorScaleMappingOptions : uint8_t {
	kNone = 0,
	kClamped = 1,
	kDiverging = 2
};

static DeviceColorScaleMappingOptions operator|(DeviceColorScaleMappingOptions lhs, DeviceColorScaleMappingOptions rhs) {
	using T = std::underlying_type_t<DeviceColorScaleMappingOptions>;
	return static_cast<DeviceColorScaleMappingOptions>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

static DeviceColorScaleMappingOptions& operator|=(DeviceColorScaleMappingOptions& lhs, DeviceColorScaleMappingOptions rhs) {
	lhs = lhs | rhs;
	return lhs;
}

/// @brief Color scale arguments as used by the graphics device or shader program.
/// Generalizes the mapping options of different color scale implementations to make
/// them usable within a shader.
/// Members must follow the alignment rules of the device and the size of the struct
/// must be a multiple of 16 bytes, which may necessitate the use of padding.
struct device_color_scale_arguments {
	static constexpr size_t k_max_indexed_color_count = 255;
	vec2 domain = { 0.0f, 1.0f };
	rgba8 unknown_color = { 0, 0, 0, 255 };
	float midpoint = 0.5f;
	float exponent = 1.0f;
	float log_base = 1.0f;
	float log_midpoint = 0.5f;
	float log_lower_bound = 0.0f;
	float log_upper_bound = 1.0f;
	float log_sign = 1.0f;
	/// The next three fields "transform, mapping_options and sample_mode" take up 4 bytes in total.
	/// They have to be specified in reverse order of how they are used in the shader to account for different endianness.
	uint16_t transform = 0;
	DeviceColorScaleMappingOptions mapping_options = DeviceColorScaleMappingOptions::kNone;
	DeviceColorScaleSampleMode sample_mode = DeviceColorScaleSampleMode::kContinuous;
	int32_t indexed_color_count = 0;
};

/// @brief Base class defining an interface for using color scales on the graphics device, e.g. in a shader.
class CGV_API device_color_scale {
public:
	/// @brief Get the color scale arguments of the underlying color scale implementation.
	/// 
	/// @return The arguments struct.
	device_color_scale_arguments get_arguments() const;

	/// @brief Get the time point of the last modification of this object.
	/// 
	/// @return The time point.
	cgv::data::time_point get_modified_time() const {
		return get_color_scale()->get_modified_time();
	}

	/// @brief Get the texture data of the underlying color scales's color ramp or indexed colors.
	/// The size of the returned sequence may only exactly match requested_resolution if the underlying
	/// color scale is continuous. Discrete color scales may only return as much samples as governed by
	/// their indexed color count.
	/// If the underlying color scale does not provide opacity mapping, the opacity of the returned samples
	/// is expected to be 1.
	/// 
	/// @param texture_resolution The requested resolution.
	/// @return The sequence of color RGBA samples.
	std::vector<cgv::rgba> get_texture_data(size_t texture_resolution) const;

private:
	/// @brief Get the underlying color scale.
	/// 
	/// @return The color scale.
	virtual const cgv::media::color_scale* get_color_scale() const = 0;

	/// @brief Update part of the given color scale arguments with the mapping options specific to the
	/// underlying color scale implementation.
	/// Arguments that are global to all color scale implementations as defined in the cgv::media::color_scale
	/// interface will be set by this base class.
	/// 
	/// @param out_uniforms The arguments to update.
	virtual void update_color_scale_specific_arguments(device_color_scale_arguments& out_uniforms) const = 0;
};

/// @brief Base class template that provides a pointer to a concrete color scale implementation.
/// 
/// @tparam T The color scale type.
template<typename T>
class CGV_API device_color_scale_storage : public device_color_scale {
public:
	/// @brief Construct using an default-constructed color scale of type T.
	device_color_scale_storage() : color_scale(std::make_shared<T>()) {}

	/// @brief Construct using the given color scale pointer.
	/// 
	/// @param color_scale The color scale.
	device_color_scale_storage(std::shared_ptr<T> color_scale) : color_scale(color_scale) {}

	/// The underlying host-side color scale.
	std::shared_ptr<T> color_scale;

private:
	/// See device_color_scale::get_color_scale().
	const cgv::media::color_scale* get_color_scale() const override {
		return color_scale.get();
	};
};

/// @brief Implementation of a device_color_scale that wraps cgv::media::continuous_color_scale.
class CGV_API device_continuous_color_scale : public device_color_scale_storage<cgv::media::continuous_color_scale> {
	using base = device_color_scale_storage<cgv::media::continuous_color_scale>;

public:
	using base::base;

private:
	/// See device_color_scale::update_color_scale_specific_arguments().
	void update_color_scale_specific_arguments(device_color_scale_arguments& out_arguments) const override;
};

/// @brief Implementation of a device_color_scale that wraps cgv::media::discrete_color_scale.
class CGV_API device_discrete_color_scale : public device_color_scale_storage<cgv::media::discrete_color_scale> {
	using base = device_color_scale_storage<cgv::media::discrete_color_scale>;

public:
	using base::base;

private:
	/// See device_color_scale::update_color_scale_specific_arguments().
	void update_color_scale_specific_arguments(device_color_scale_arguments& out_arguments) const override;
};

/// @brief Implementation of a device_color_scale that wraps cgv::media::transfer_function.
class CGV_API device_transfer_function : public device_color_scale_storage<cgv::media::transfer_function> {
	using base = device_color_scale_storage<cgv::media::transfer_function>;

public:
	using base::base;

private:
	/// See device_color_scale::update_color_scale_specific_arguments().
	void update_color_scale_specific_arguments(device_color_scale_arguments& out_arguments) const override {
		// Transfer function uses the default arguments.
	}
};

} // namespace render
} // namespace cgv

#include <cgv/config/lib_end.h>
