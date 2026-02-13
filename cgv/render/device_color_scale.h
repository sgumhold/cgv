#pragma once

#include <cstdint>

#include <cgv/media/color_scale.h>
#include <cgv/media/transfer_function.h>
#include <cgv/type/bool32_t.h>

#include "lib_begin.h"

namespace cgv {
namespace render {

enum class DeviceColorScaleSampleMode : uint8_t {
	kContinuous = 0,
	kDiscrete = 1
};

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
/// Members must follow the alignment rules of the device and the size of the struct
/// must be a multiple of 16 bytes, which may necessitate the use of padding.
struct device_color_scale_arguments {
	static const size_t k_max_indexed_color_count = 255;
	vec2 domain = { 0.0f, 1.0f };
	rgba8 unknown_color = { 0, 0, 0, 255 };
	float midpoint = 0.5f;
	float exponent = 1.0f;
	float log_base = 1.0f;
	float log_midpoint = 0.5f;
	float log_lower_bound = 0.0f;
	float log_upper_bound = 1.0f;
	float log_sign = 1.0f;
	// The next three fields "transform, mapping_options and sample_mode" take up 4 bytes total. They have to be specified in reverse order of how they are used in the shader.
	uint16_t transform = 0;
	DeviceColorScaleMappingOptions mapping_options = DeviceColorScaleMappingOptions::kNone;
	DeviceColorScaleSampleMode sample_mode = DeviceColorScaleSampleMode::kContinuous;
	int32_t indexed_color_count = 0;
};

class CGV_API device_color_scale {
public:
	device_color_scale_arguments get_arguments() const;

	cgv::data::time_point get_modified_time() const {
		return get_color_scale()->get_modified_time();
	}

	std::vector<cgv::rgba> get_texture_data(size_t texture_resolution) const;

private:
	virtual const cgv::media::color_scale* get_color_scale() const = 0;

	virtual void update_color_scale_specific_arguments(device_color_scale_arguments& out_uniforms) const = 0;
};

template<typename T>
class CGV_API device_color_scale_storage : public device_color_scale {
public:
	device_color_scale_storage() : color_scale(std::make_shared<T>()) {}

	device_color_scale_storage(std::shared_ptr<T> color_scale) : color_scale(color_scale) {}

	std::shared_ptr<T> color_scale;

private:
	const cgv::media::color_scale* get_color_scale() const override {
		return color_scale.get();
	};
};

class CGV_API device_continuous_color_scale : public device_color_scale_storage<cgv::media::continuous_color_scale> {
	using base = device_color_scale_storage<cgv::media::continuous_color_scale>;

public:
	using base::base;

private:
	void update_color_scale_specific_arguments(device_color_scale_arguments& out_arguments) const override;
};

class CGV_API device_discrete_color_scale : public device_color_scale_storage<cgv::media::discrete_color_scale> {
	using base = device_color_scale_storage<cgv::media::discrete_color_scale>;

public:
	using base::base;

private:
	void update_color_scale_specific_arguments(device_color_scale_arguments& out_arguments) const override;
};

class CGV_API device_transfer_function : public device_color_scale_storage<cgv::media::transfer_function> {
	using base = device_color_scale_storage<cgv::media::transfer_function>;

public:
	using base::base;

private:
	void update_color_scale_specific_arguments(device_color_scale_arguments& out_arguments) const override {
		// Transfer function uses the default arguments.
	}
};

} // namespace render
} // namespace cgv

#include <cgv/config/lib_end.h>
