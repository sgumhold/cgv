#pragma once

#include "texture_algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

enum class DifferentiationOperator {
	kForwardDifference,
	kBackwardDifference,
	kCentralDifference,
	kSobel
};

enum class DifferentiationOutput {
	kDerivative = 0,			// output the unchanged derivative
	kScaledDerivative,			// output the derivative with components scaled to range [-1,1]
	kScaledDerivativeUNorm,		// output the derivative with components scaled to range [-1,1] and then mapped to range [0,1]
	kNormalizedDerivative,		// output the normalized derivative
	kNormalizedDerivativeUNorm,	// output the normalized derivative with components mapped from [-1,1] to [0,1]
	kMagnitude					// output the magnitude (L2 norm) of the derivative
};

enum class TextureChannel : int32_t {
	kR = 0,
	kG,
	kB,
	kA
};

extern CGV_API std::string to_string(DifferentiationOperator differentiation_operator);

class CGV_API texture_differentiate_base : public texture_algorithm {
public:
	using texture_algorithm::texture_algorithm;

protected:
	static texture_algorithm_create_info get_create_info(cgv::render::TextureType texture_type,
														 sl::ImageFormatLayoutQualifier image_format,
														 DifferentiationOperator differentiation_operator,
														 DifferentiationOutput differentiation_output);
private:
	static std::string get_scaling_func(DifferentiationOperator differentiation_operator, DifferentiationOutput differentiation_output, uint32_t dimensions);
};

/// GPU compute shader implementation for computing derivatives of a texture.
class CGV_API texture_differentiate : public texture_differentiate_base {
public:
	texture_differentiate();

	bool init(cgv::render::context& ctx,
			  cgv::render::TextureType texture_type,
			  sl::ImageFormatLayoutQualifier image_format,
			  DifferentiationOperator differentiation_operator = cgv::gpgpu::DifferentiationOperator::kCentralDifference,
			  DifferentiationOutput differentiation_output = cgv::gpgpu::DifferentiationOutput::kDerivative);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, cgv::render::texture& input_texture, cgv::render::texture& output_texture, TextureChannel texture_channel = TextureChannel::kR);

private:
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
