#pragma once

#include <memory>
#include <string>

#include <torch/script.h>
#include <torch/torch.h>

#include "lib_begin.h"

namespace cgv {
	namespace nn {
		using tn_ptr = std::shared_ptr<torch::jit::Module>;
		using module_map = std::map<std::string, torch::jit::Module>;

		tn_ptr CGV_API load_net_from_file(std::string const& filepath, bool* const success = nullptr);

		module_map CGV_API get_named_modules(tn_ptr nn);

		int64_t typesize(c10::ScalarType scalar_type);

		int64_t CGV_API element_count(at::Tensor t);

		template<typename T>
		std::vector<T> to_vector(at::Tensor t);

		template<>
		std::vector<float> CGV_API to_vector(at::Tensor t);
		
		template<>
		std::vector<double> CGV_API to_vector(at::Tensor t);
	}
}
