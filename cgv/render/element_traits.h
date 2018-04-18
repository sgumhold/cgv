
#pragma once

#include <cgv/math/vec.h>
#include <cgv/math/mat.h>
#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/math/quaternion.h>
#include <cgv/media/color.h>
#include <cgv/type/info/type_id.h>
#include "context.h"

namespace cgv {
	namespace render {

		template <typename T>
		struct element_descriptor_traits
		{
			static type_descriptor get_type_descriptor(const T&) { return type_descriptor(cgv::type::info::type_id <T>::get_id()); }
			static const void* get_address(const T& value) { return &value; }
			static       void* get_address(T& value) { return &value; }
		};
		template <typename T, cgv::type::uint32_type N>
		struct element_descriptor_traits < cgv::math::fvec<T, N> >
		{
			static type_descriptor get_type_descriptor(const cgv::math::fvec<T, N>&) { return type_descriptor(cgv::type::info::type_id<T>::get_id(), N); }
			static const void* get_address(const cgv::math::fvec<T, N>& element) { return &element; }
			static       void* get_address(cgv::math::fvec<T, N>& element) { return &element; }
		};
		template <typename T>
		struct element_descriptor_traits < cgv::math::quaternion<T> >
		{
			static type_descriptor get_type_descriptor(const cgv::math::quaternion<T>&) { return type_descriptor(cgv::type::info::type_id<T>::get_id(), cgv::type::uint32_type(4)); }
			static const void* get_address(const cgv::math::quaternion<T>& element) { return &element; }
			static       void* get_address(cgv::math::quaternion<T>& element) { return &element; }
		};
		template <typename T, cgv::media::ColorModel cm, cgv::media::AlphaModel am>
		struct element_descriptor_traits < cgv::media::color<T, cm, am> >
		{
			static type_descriptor get_type_descriptor(const cgv::media::color<T, cm, am>&) { return type_descriptor(cgv::type::info::type_id<T>::get_id(), cgv::media::color<T, cm, am>::nr_components, true); }
			static const void* get_address(const cgv::media::color<T, cm, am>& element) { return &element; }
			static       void* get_address(cgv::media::color<T, cm, am>& element) { return &element; }
		};
		template <typename T>
		struct element_descriptor_traits < cgv::math::vec<T> >
		{
			static type_descriptor get_type_descriptor(const cgv::math::vec<T>& vec) { return type_descriptor(cgv::type::info::type_id<T>::get_id(), vec.size()); }
			static const void* get_address(const cgv::math::vec<T>& element) { return &element(0); }
			static       void* get_address(cgv::math::vec<T>& element) { return &element(0); }
		};
		template <typename T, cgv::type::uint32_type N, cgv::type::uint32_type M>
		struct element_descriptor_traits < cgv::math::fmat<T, N, M> >
		{
			static type_descriptor get_type_descriptor(const cgv::math::fmat<T, N, M>&) { return type_descriptor(cgv::type::info::type_id<T>::get_id(), N, M, true); }
			static const void* get_address(const cgv::math::fmat<T, N, M>& element) { return &element(0,0); }
			static       void* get_address(cgv::math::fmat<T, N, M>& element) { return &element(0,0); }
		};
		template <typename T>
		struct element_descriptor_traits < cgv::math::mat<T> >
		{
			static type_descriptor get_type_descriptor(const cgv::math::mat<T>& mat) { return type_descriptor(cgv::type::info::type_id<T>::get_id(), mat.nrows(), mat.ncols(), true); }
			static const void* get_address(const cgv::math::mat<T>& element) { return &element(0, 0); }
			static       void* get_address(cgv::math::mat<T>& element) { return &element(0, 0); }
		};

		template <typename T>
		struct array_descriptor_traits
		{
			//TODO: What is this supposed to do; prevents compilation under Linux
			//enum dummy { d = (sizeof(struct this_type_is_not_an_array_type_or_was_not_detected_as_an_array_type) == sizeof(T)) };
		};

		template <typename T>
		struct array_descriptor_traits < std::vector<T> >
		{
			/// return type descriptor for array
			static type_descriptor get_type_descriptor(const std::vector<T>& vec) { return type_descriptor(element_descriptor_traits<T>::get_type_descriptor(vec[0]), true); }
			/// return const start address in array
			static const T* get_address(const std::vector<T>& vec) { return &vec.front(); }
			/// return start address in array
			static       T* get_address(std::vector<T>& vec)       { return &vec.front(); }
			/// return number elements in array
			static      size_t get_nr_elements(const std::vector<T>& vec) { return vec.size(); }
			/// return size of array in bytes
			static      size_t get_size(const std::vector<T>& vec) { return vec.size() * sizeof(T); }
		};

		template <typename T>
		struct array_descriptor_traits < cgv::math::vec<T> >
		{
			/// return type descriptor for array
			static type_descriptor get_type_descriptor(const cgv::math::vec<T>& vec) { return type_descriptor(element_descriptor_traits<T>::get_type_descriptor(vec(0)), true); }
			/// return const start address in array
			static const T* get_address(const cgv::math::vec<T>& vec) { return &vec(0); }
			/// return start address in array
			static       T* get_address(cgv::math::vec<T>& vec) { return &vec(0); }
			/// return number elements in array
			static      size_t get_nr_elements(const cgv::math::vec<T>& vec) { return vec.size(); }
			/// return size of array in bytes
			static      size_t get_size(const cgv::math::vec<T>& vec) { return vec.size() * sizeof(T); }
		};
	}
}