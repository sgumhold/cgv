#pragma once

// implementation of to_json and from_json helper functions for cgv math types
// please add missing implementations as you need them

#include <cgv/math/fvec.h>
#include "../nlohmann/json.hpp"

namespace cgv {
	namespace math {
		template <typename T, cgv::type::uint32_type N>
		void to_json(nlohmann::json& j, const cgv::math::fvec<T, N>& v) {
			for (cgv::type::uint32_type i = 0; i < N; ++i)
				j.push_back(v[i]);
		}
		template <typename T, cgv::type::uint32_type N>
		void from_json(const nlohmann::json& j, cgv::math::fvec<T, N>& v) {
			for (cgv::type::uint32_type i = 0; i < N; ++i)
				j[i].get_to(v[i]);
		}
	}
}