#pragma once

#include "focusable.h"
#include <cgv/render/context.h>
#include <cgv/math/ftransform.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
		using namespace cgv::render;
		/// interface for objects that provide a model transformation with default implementation under the assumption that get_model_transform() is implemented.
		class CGV_API transforming
		{
		public:
			/// init to identity matrix
			transforming();
			/// read access to model transform
			virtual mat4 get_model_transform() const = 0;
			/// read access to inverse model transform
			virtual mat4 get_inverse_model_transform() const;
			/// set model transform and compute inverse model transform
			virtual void set_model_transform(const mat4& _M) = 0;
			/// set model transform and inverse model transform
			virtual void set_model_transform(const mat4& _M, const mat4& _iM) = 0;
			/// transform a point
			virtual vec3 transform_point(const vec3& p);
			/// inverse transform a point
			virtual vec3 inverse_transform_point(const vec3& p);
			/// transform a vector
			virtual vec3 transform_vector(const vec3& v);
			/// inverse transform a vector
			virtual vec3 inverse_transform_vector(const vec3& v);
			/// transform a normal
			virtual vec3 transform_normal(const vec3& n);
			/// inverse transform a normal
			virtual vec3 inverse_transform_normal(const vec3& n);
		};
		/// implement transforming interface by storing model matrix and its inverse
		class CGV_API matrix_cached_transforming : public transforming
		{
		protected:
			/// store model and inverse model matrix
			mat4 M, iM;
		public:
			/// construct from given matrix or as identity
			matrix_cached_transforming(const mat4& _M = cgv::math::identity4<float>());
			/// read access to model transform
			mat4 get_model_transform() const;
			/// read access to inverse model transform
			mat4 get_inverse_model_transform() const;
			/// set model transform and compute inverse model transform
			void set_model_transform(const mat4& _M);
			/// set model transform and inverse model transform
			void set_model_transform(const mat4& _M, const mat4& _iM);
		};
		template <typename ...Transformables> class concatenating_transforming;
		//! lifts implementation of transformation_matrix_provider (TMP template argument).
		/*! Implements access to pre/post transformation for TMPs that are concatenated 
		    through the variadic template concatenating_transforming<Transformables...>. */
		template <typename TMP, typename ...Transformables>
		struct transformation_matrix_provider_lifter : public TMP
		{
			struct detail {
				/// pre_transformer implements transformation_matrix_provider::has_pre_transformation(), transformation_matrix_provider::get_pre_transformation() and transformation_matrix_provider::get_inverse_pre_transformation()
				template <bool first, typename Head, typename ...Tail> 
				struct pre_transformer // default handles cases where Head as well as head of Tail are both different from TMP independent of first
				{
					static bool has() { return true; }
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) { // concatenate pre transforms
						return static_cast<const Head*>(
							static_cast<const concatenating_transforming<Transformables...>*>(
								This))->get_transformation_matrix() *
							pre_transformer<false, Tail...>::get(This);
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) { // concatenate inverse pre transforms
						return pre_transformer<false, Tail...>::inv(This) * 
							static_cast<const Head*>(
								static_cast<const concatenating_transforming<Transformables...>*>(
									This))->get_inverse_transformation_matrix();
					}
				};
				template <typename ...Tail> struct pre_transformer<false, TMP, Tail...> // case where TMP is head of sublist but not of Transformables
				{
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return cgv::math::identity4<float>(); 
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return cgv::math::identity4<float>(); 
					}
				};
				template <typename ...Tail> struct pre_transformer<true, TMP, Tail...> // case where TMP is head of Transformables
				{
					static bool has() { return false; }
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) { // this is never called but necessary to compile
						return cgv::math::identity4<float>();
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) { // this is never called but necessary to compile
						return cgv::math::identity4<float>();
					}
				};
				/// post_transformer implements transformation_matrix_provider::has_post_transformation(), transformation_matrix_provider::get_post_transformation() and transformation_matrix_provider::get_inverse_post_transformation()
				template <bool found, typename Head, typename ...Tail>
				struct post_transformer // default handles found=false cases where TMP is different from Head
				{
					static bool has() { return post_transformer<found, Tail...>::has(); }
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return post_transformer<found, Tail...>::get(This); 
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return post_transformer<found, Tail...>::inv(This); 
					}
				};
				template <typename ...Tail> struct post_transformer<false, TMP, Tail...> // here TMP is head of sublist
				{
					static bool has() { return true; }
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return post_transformer<true, Tail...>::get(This);
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return post_transformer<true, Tail...>::inv(This);
					}
				};
				template <> struct post_transformer<false, TMP> // here TMP is last transformable and no post transformation available
				{
					static bool has() { return false; }
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return cgv::math::identity4<float>(); 
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return cgv::math::identity4<float>(); 
					}
				};
				template <typename Head, typename ...Tail> struct post_transformer<true, Head, Tail...> // here we need to concatenate post
				{
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return static_cast<const Head*>(
							static_cast<const concatenating_transforming<Transformables...>*>(This))->get_transformation_matrix() *
							post_transformer<true, Tail...>::get(This);
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return post_transformer<true, Tail...>::inv(This)*
							static_cast<const Head*>(
								static_cast<const concatenating_transforming<Transformables...>*>(This))->get_inverse_transformation_matrix();
					}
				};
				template <typename Head> struct post_transformer<true, Head> // here we are at last post transform
				{
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return static_cast<const Head*>(
							static_cast<const concatenating_transforming<Transformables...>*>(
								This))->get_transformation_matrix();
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return static_cast<const Head*>(
							static_cast<const concatenating_transforming<Transformables...>*>(
								This))->get_inverse_transformation_matrix();
					}
				};

				/// partial_transformer implements transformation_matrix_provider::get_partial_transformation() and transformation_matrix_provider::get_inverse_partial_transformation()
				template <bool found, typename Head, typename ...Tail>
				struct partial_transformer // default handles found=false cases where TMP is different from Head
				{
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return partial_transformer<found, Tail...>::get(This);
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return partial_transformer<found, Tail...>::inv(This);
					}
				};
				template <typename ...Tail> struct partial_transformer<false, TMP, Tail...> // here TMP is head of sublist
				{
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return static_cast<const TMP*>(
							static_cast<const concatenating_transforming<Transformables...>*>(
								This))->get_transformation_matrix() *
							partial_transformer<true, Tail...>::get(This);
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return partial_transformer<true, Tail...>::inv(This) *
							static_cast<const TMP*>(
								static_cast<const concatenating_transforming<Transformables...>*>(
									This))->get_inverse_transformation_matrix();
					}
				};
				template <> struct partial_transformer<false, TMP> // here TMP is last transformable and no post transformation available
				{
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return static_cast<const TMP*>(
							static_cast<const concatenating_transforming<Transformables...>*>(
								This))->get_transformation_matrix();
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return static_cast<const TMP*>(
							static_cast<const concatenating_transforming<Transformables...>*>(
								This))->get_inverse_transformation_matrix();
					}
				};
				template <typename Head, typename ...Tail> struct partial_transformer<true, Head, Tail...> // here we need to concatenate post
				{
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return static_cast<const Head*>(
							static_cast<const concatenating_transforming<Transformables...>*>(This))->get_transformation_matrix() *
							partial_transformer<true, Tail...>::get(This);
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return partial_transformer<true, Tail...>::inv(This) *
							static_cast<const Head*>(
								static_cast<const concatenating_transforming<Transformables...>*>(This))->get_inverse_transformation_matrix();
					}
				};
				template <typename Head> struct partial_transformer<true, Head> // here we are at last post transform
				{
					static mat4 get(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return static_cast<const Head*>(
							static_cast<const concatenating_transforming<Transformables...>*>(
								This))->get_transformation_matrix();
					}
					static mat4 inv(const transformation_matrix_provider_lifter<TMP, Transformables...>* This) {
						return static_cast<const Head*>(
							static_cast<const concatenating_transforming<Transformables...>*>(
								This))->get_inverse_transformation_matrix();
					}
				};
			};
			/// perfect forwarding of constructor
			transformation_matrix_provider_lifter(TMP&& tmp) : TMP(std::forward<TMP>(tmp)) {}
			/// use detail::pre_transformer for implementation
			bool has_pre_transformation() const { return detail::pre_transformer<true, Transformables...>::has(); }
			mat4 get_pre_transformation_matrix() const { return detail::pre_transformer<true, Transformables...>::get(this); }
			mat4 get_inverse_pre_transformation_matrix() const { return detail::pre_transformer<true, Transformables...>::inv(this); }
			/// use detail::post_transformer for implementation
			bool has_post_transformation() const { return detail::post_transformer<false, Transformables...>::has(); }
			mat4 get_post_transformation_matrix() const { return detail::post_transformer<false, Transformables...>::get(this); }
			mat4 get_inverse_post_transformation_matrix() const { return detail::post_transformer<false, Transformables...>::inv(this); }
			/// use detail::partial_transformer for implementation
			mat4 get_partial_transformation_matrix() const { return detail::partial_transformer<false, Transformables...>::get(this); }
			mat4 get_inverse_partial_transformation_matrix() const { return detail::partial_transformer<false, Transformables...>::inv(this); }
		};
		/// implement transforming interface through concatenation of transformable interfaces
		template <typename ...Transformables>
		class concatenating_transforming : public transforming, 
			public transformation_matrix_provider_lifter<Transformables, Transformables...>...
		{
		protected:
			struct detail { // template helpers for implementation of transforming::get_model_transform() and transforming::get_inverse_model_transform() methods
				template <typename Head, typename ...Tail> struct model_transform_getter {
					static mat4 matrix(const concatenating_transforming<Transformables...>* This) {
						return ((const Head*)This)->get_transformation_matrix() *
							model_transform_getter<Tail...>::matrix(This);
					}
				};
				template <typename T> struct model_transform_getter<T> {
					static mat4 matrix(const concatenating_transforming<Transformables...>* This) {
						return ((const T*)This)->get_transformation_matrix();
					}
				};
				template <typename Head, typename ...Tail> struct inverse_model_transform_getter {
					static mat4 matrix(const concatenating_transforming<Transformables...>* This) {
						return inverse_model_transform_getter<Tail...>::matrix(This) *
							((const Head*)This)->get_inverse_transformation_matrix();
					}
				};
				template <typename T> struct inverse_model_transform_getter<T> {
					static mat4 matrix(const concatenating_transforming<Transformables...>* This) {
						return ((const T*)This)->get_inverse_transformation_matrix();
					}
				};
			};
		public:
			/// constructor concatenates transformables' contructors via perfect forwarding
			concatenating_transforming(Transformables&&... transformables) : 
				transformation_matrix_provider_lifter<Transformables, Transformables...>(
					std::forward<Transformables>(transformables))... {}
			/// read access to model transform
			mat4 get_model_transform() const { return detail::model_transform_getter<Transformables...>::matrix(this); }
			/// read access to inverse model transform
			mat4 get_inverse_model_transform() const { return detail::inverse_model_transform_getter<Transformables...>::matrix(this); }
			/// set model transform and compute inverse model transform
			void set_model_transform(const mat4& _M) {
				std::cerr << "concatenating_transforming::set_model_transform(M) not implemented" << std::endl;
			}
			/// set model transform and inverse model transform
			void set_model_transform(const mat4& _M, const mat4& _iM) {
				std::cerr << "concatenating_transforming::set_model_transform(M, iM) not implemented" << std::endl;
			}
		};
	}
}
#include <cgv/config/lib_end.h>