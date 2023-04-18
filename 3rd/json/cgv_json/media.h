#pragma once

// implementation of to_json and from_json helper functions for cgv media types
// please add missing implementations as you need them here
// it is important that the implemented functions live in the same namespace as the relfected classes

#include <cgv/media/color.h>
#include "../nlohmann/json.hpp"

namespace cgv {
	namespace media {
		template <typename T, ColorModel cm, AlphaModel am>
		void to_json(nlohmann::json& j, const cgv::media::color<T, cm, am>& c) {
			for (cgv::type::uint32_type i = 0; i < cgv::media::color<T, cm, am>::nr_components; ++i)
				j.push_back(c[i]);
		}
		template <typename T, ColorModel cm, AlphaModel am>
		void from_json(const nlohmann::json& j, cgv::media::color<T, cm, am>& c) {
			for (cgv::type::uint32_type i = 0; i < cgv::media::color<T, cm, am>::nr_components; ++i)
				j[i].get_to(c[i]);
		}
	}
}
