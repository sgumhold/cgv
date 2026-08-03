#pragma once

#include <cstdint>

#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>

#include "sl.h"
#include "binding.h"
#include "function_definition_registry.h"

namespace cgv {
namespace gpgpu {

struct representable : std::true_type {
	// This function must be implemented in derived classes.
	static sl::data_type get_type();
	// This function must be implemented in derived classes using the derived class' template parameter.
	template<class T>
	static void create_uniform_binding(uniform_binding_list& bindings, const std::string& name, const T& value);
	// This function can optionally be implemented in derived classes.
	static void register_functions(const type_scoped_function_registrator& registrator, sl::data_type self_type) {}
};

template<class T>
struct type_representation : std::false_type {};

template<class T, sl::Type sl_type>
struct scalar_type_representation : representable {
	static sl::data_type get_type() {
		return sl_type;
	}

	static void create_uniform_binding(uniform_binding_list& bindings, const std::string& name, const T& value) {
		bindings.emplace_back(name, value);
	}
};

template<> struct type_representation<bool> : scalar_type_representation<bool, sl::Type::Bool> {};
template<> struct type_representation<int32_t> : scalar_type_representation<int32_t, sl::Type::Int> {};
template<> struct type_representation<uint32_t> : scalar_type_representation<uint32_t, sl::Type::UInt> {};
template<> struct type_representation<float> : scalar_type_representation<float, sl::Type::Float> {};
template<> struct type_representation<double> : scalar_type_representation<double, sl::Type::Double> {};

template<class T, cgv::type::uint32_type N, sl::Type sl_type>
struct fvec_type_representation : representable {
	static sl::data_type get_type() {
		return sl_type;
	}

	static void create_uniform_binding(uniform_binding_list& bindings, const std::string& name, const cgv::math::fvec<T, N>& value) {
		bindings.emplace_back(name, &value);
	}
};

template<class T, sl::Type sl_base_type>
struct fvec3_proxy_type_representation : representable {
	static sl::data_type get_type() {
		std::string name = "_vec3_t";
		switch(sl_base_type) {
		case sl::Type::Bool: name[0] = 'b'; break;
		case sl::Type::Int: name[0] = 'i'; break;
		case sl::Type::UInt: name[0] = 'u'; break;
		case sl::Type::Float: name = name.substr(1); break;
		case sl::Type::Double: name[0] = 'd'; break;
		default: break;
		}
		return { name, {
			{ sl_base_type, "x" },
			{ sl_base_type, "y" },
			{ sl_base_type, "z" }
		} };
	}
	 
	static void create_uniform_binding(uniform_binding_list& bindings, const std::string& name, const cgv::math::fvec<T, 3u>& value) {
		bindings.emplace_back(name + ".x", value.x());
		bindings.emplace_back(name + ".y", value.y());
		bindings.emplace_back(name + ".z", value.z());
	}

	static void register_functions(const type_scoped_function_registrator& registrator, sl::data_type self_type) {
		registrator.add({sl::Type::Vec3, "to_vec3", { { self_type, "v" } },
			"return vec3(v.x, v.y, v.z);"
		});
		registrator.add({ self_type, "to_xyz", { { sl::Type::Vec3, "v" } },
			"return " + self_type.type_name() + "(v.x, v.y, v.z);"
		});
	}
};

template<> struct type_representation<cgv::bvec2> : fvec_type_representation<bool, 2u, sl::Type::BVec2> {};
template<> struct type_representation<cgv::bvec3> : fvec3_proxy_type_representation<bool, sl::Type::Bool> {};
template<> struct type_representation<cgv::bvec4> : fvec_type_representation<bool, 4u, sl::Type::BVec4> {};

template<> struct type_representation<cgv::ivec2> : fvec_type_representation<int32_t, 2u, sl::Type::IVec2> {};
template<> struct type_representation<cgv::ivec3> : fvec3_proxy_type_representation<int32_t, sl::Type::Int> {};
template<> struct type_representation<cgv::ivec4> : fvec_type_representation<int32_t, 4u, sl::Type::IVec4> {};

template<> struct type_representation<cgv::uvec2> : fvec_type_representation<uint32_t, 2u, sl::Type::UVec2> {};
template<> struct type_representation<cgv::uvec3> : fvec3_proxy_type_representation<uint32_t, sl::Type::UInt> {};
template<> struct type_representation<cgv::uvec4> : fvec_type_representation<uint32_t, 4u, sl::Type::UVec4> {};

template<> struct type_representation<cgv::vec2> : fvec_type_representation<float, 2u, sl::Type::Vec2> {};
template<> struct type_representation<cgv::vec3> : fvec3_proxy_type_representation<float, sl::Type::Float> {};
template<> struct type_representation<cgv::vec4> : fvec_type_representation<float, 4u, sl::Type::Vec4> {};

template<> struct type_representation<cgv::dvec2> : fvec_type_representation<double, 2u, sl::Type::DVec2> {};
template<> struct type_representation<cgv::dvec3> : fvec3_proxy_type_representation<double, sl::Type::Double> {};
template<> struct type_representation<cgv::dvec4> : fvec_type_representation<double, 4u, sl::Type::DVec4> {};

template<class T, cgv::type::uint32_type N, cgv::type::uint32_type M, sl::Type sl_type>
struct fmat_type_representation : representable {
	static sl::data_type get_type() {
		return sl_type;
	}

	static void create_uniform_binding(uniform_binding_list& bindings, const std::string& name, const cgv::math::fmat<T, N, M>& value) {
		bindings.emplace_back(name, &value);
	}
};

template<> struct type_representation<cgv::mat2> : fmat_type_representation<float, 2u, 2u, sl::Type::Mat2> {};
template<> struct type_representation<cgv::mat4> : fmat_type_representation<float, 4u, 4u, sl::Type::Mat4> {};

template<> struct type_representation<cgv::dmat2> : fmat_type_representation<float, 2u, 2u, sl::Type::DMat2> {};
template<> struct type_representation<cgv::dmat4> : fmat_type_representation<double, 4u, 4u, sl::Type::DMat4> {};

template<class T>
static sl::data_type register_type_representation() {
	static_assert(type_representation<T>::value, "T must be representable as sl::data_type");
	static sl::data_type type = type_representation<T>::get_type();

	static bool registered_functions = false;
	if(!registered_functions) {
		type_scoped_function_registrator registrator(type);
		type_representation<T>::register_functions(registrator, type);
		registered_functions = true;
	}

	return type;
}

} // namespace gpgpu
} // namespace cgv
