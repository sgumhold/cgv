#pragma once

// implementation of to_json and from_json helper functions for cgv math types
// please add missing implementations as you need them

#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/math/mat.h>
#include <cgv/math/camera.h>
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
		template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
		void to_json(nlohmann::json& j, const cgv::math::fmat<T, N, M>& A) {
			j["rows"] = N;
			j["columns"] = M;
			j["column_major"] = true;
			auto& jv = j["values"];
			const cgv::math::fvec<T, N* M>& v = A;
			for (cgv::type::uint32_type i = 0; i < N*M; ++i)
				jv.push_back(v[i]);
		}
		template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
		void from_json(const nlohmann::json& j, cgv::math::fmat<T, N, M>& A) {
			assert(j.at("rows") == N);
			assert(j.at("columns") == M);
			assert(j.at("column_major") == true);
			const auto& jv = j["values"];
			cgv::math::fvec<T, N* M>& v = A;
			for (cgv::type::uint32_type i = 0; i < N * M; ++i)
				jv[i].get_to(v[i]);
		}
		template <typename T>
		void to_json(nlohmann::json& j, const cgv::math::pinhole<T>& p) {
			j["w"] = p.w;
			j["h"] = p.h;
			j["s"] = p.s;
			j["c"] = p.c;
			j["skew"] = p.skew;
		}
		template <typename T>
		void from_json(const nlohmann::json& j, cgv::math::pinhole<T>& p) {
			j.at("w").get_to(p.w);
			j.at("h").get_to(p.h);
			j.at("s").get_to(p.s);
			j.at("c").get_to(p.c);
			j.at("skew").get_to(p.skew);
		}
		template <typename T>
		void to_json(nlohmann::json& j, const cgv::math::distorted_pinhole<T>& dp) {
			to_json(j, static_cast<const pinhole<T>&>(dp));
			j["distortion_center"] = dp.dc;
			j["k"] = dp.k;
			j["p"] = dp.p;
			j["max_radius_for_projection"] = dp.max_radius_for_projection;
		}
		template <typename T>
		void from_json(const nlohmann::json& j, cgv::math::distorted_pinhole<T>& dp) {
			from_json(j, static_cast<pinhole<T>&>(dp));
			j.at("distortion_center").get_to(dp.dc);
			j.at("k").get_to(dp.k);
			j.at("p").get_to(dp.p);
			j.at("max_radius_for_projection").get_to(dp.max_radius_for_projection);
		}
		template <typename T>
		void to_json(nlohmann::json& j, const cgv::math::camera<T>& c) {
			to_json(j, static_cast<const distorted_pinhole<T>&>(c));
			j["pose"] = c.pose;
		}
		template <typename T>
		void from_json(const nlohmann::json& j, cgv::math::camera<T>& c) {
			from_json(j, static_cast<distorted_pinhole<T>&>(c));
			j.at("pose").get_to(c.pose);
		}
	}
}