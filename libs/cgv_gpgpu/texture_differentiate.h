#pragma once

#include "texture_algorithm.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

enum class DifferentiationOperator {
	ForwardDifference,
	BackwardDifference,
	CentralDifference,
	Sobel
};

enum class DifferentiationOutput {
	Derivative = 0,			// output the unchanged derivative
	ScaledDerivative,			// output the derivative with components scaled to range [-1,1]
	ScaledDerivativeUNorm,		// output the derivative with components scaled to range [-1,1] and then mapped to range [0,1]
	NormalizedDerivative,		// output the normalized derivative
	NormalizedDerivativeUNorm,	// output the normalized derivative with components mapped from [-1,1] to [0,1]
	Magnitude					// output the magnitude (L2 norm) of the derivative
};

enum class TextureChannel : int32_t {
	Red = 0,
	Greed,
	Blue,
	Alpha
};

enum class WrapMode : int32_t {
	Repeat = 0,
	MirroredRepeat,
	ClampToEdge,
	ClampToBorder
};

extern CGV_API std::string to_string(DifferentiationOperator differentiation_operator);

class CGV_API texture_differentiate_base : public texture_algorithm {
public:
	using texture_algorithm::texture_algorithm;

protected:
	static texture_algorithm_create_info get_create_info(cgv::render::TextureType texture_type,
														 sl::ImageFormatLayoutQualifier image_format,
														 WrapMode wrap_mode,
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
			  WrapMode wrap_mode,
			  DifferentiationOperator differentiation_operator = cgv::gpgpu::DifferentiationOperator::CentralDifference,
			  DifferentiationOutput differentiation_output = cgv::gpgpu::DifferentiationOutput::Derivative);

	void destruct(const cgv::render::context& ctx);

	bool dispatch(cgv::render::context& ctx, cgv::render::texture& input_texture, cgv::render::texture& output_texture, TextureChannel texture_channel = TextureChannel::Red);

private:
	compute_kernel _kernel;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
