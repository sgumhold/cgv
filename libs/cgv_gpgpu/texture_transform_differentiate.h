#pragma once

#include "texture_differentiate.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

/// GPU compute shader implementation for computing derivatives of scalar values derived from a texture.
class CGV_API texture_transform_differentiate : public texture_differentiate_base {
public:
	texture_transform_differentiate();

	bool init(
		cgv::render::context& ctx,
		cgv::render::TextureType texture_type,
		sl::ImageFormatLayoutQualifier image_format,
		WrapMode wrap_mode,
		const std::string& unary_operation,
		const argument_definitions& arguments = {},
		DifferentiationOperator differentiation_operator = cgv::gpgpu::DifferentiationOperator::kCentralDifference,
		DifferentiationOutput differentiation_output = cgv::gpgpu::DifferentiationOutput::kDerivative
	);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, cgv::render::texture& input_texture, cgv::render::texture& output_texture, const argument_bindings& arguments = {});

private:
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
