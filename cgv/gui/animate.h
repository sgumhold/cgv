#pragma once

// make sure this is the first thing the compiler sees, while preventing warnings if
// it happened to already be defined by something else including this header
#ifndef _USE_MATH_DEFINES
	#define _USE_MATH_DEFINES 1
#endif
#include <cmath>
#include <limits>
#include <cgv/data/ref_ptr.h>
#include <cgv/base/base.h>
#include <cgv/math/geom.h>
#include <cgv/gui/trigger.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {
		
		class CGV_API animation;

		typedef cgv::data::ref_ptr<animation, true> animation_ptr;
		
		enum AnimationParameterMapping
		{
			APM_LINEAR = -3,
			APM_SIN_SQUARED = -2,
			APM_CUBIC = -1,
		};

		class CGV_API animation : public cgv::data::ref_counted
		{
		protected:
			AnimationParameterMapping parameter_mapping;
			double start_time;
			double end_time;
			cgv::base::base_ptr bp;
			virtual void set_value(double time) = 0;
		public:
			animation(double _start_time, double _end_time, AnimationParameterMapping _parameter_mapping = APM_LINEAR);
			virtual ~animation() {}
			void set_base_ptr(cgv::base::base_ptr _bp);
			void configure(AnimationParameterMapping _parameter_mapping, cgv::base::base_ptr _bp);
			void set_parameter_mapping(AnimationParameterMapping _parameter_mapping);
			double get_start_time() const;
			bool has_started(double time) const;
			bool is_over(double time) const;
			double get_parameter(double time) const;
			bool animates(const void* ptr) const;
			bool overlaps(const char* value_ptr, size_t value_size) const;
			virtual char* get_ptr() const = 0;
			virtual size_t get_value_size() const = 0;
			bool set_time(double time);
		};

		extern CGV_API void add_animation(animation_ptr a_ptr, bool terminate_other_animations = true);

		template <typename T>
		class value_animation : public animation
		{
		protected:
			T start_value;
		public:
			T end_value;
		protected:
			T* value_ptr;
			char* get_ptr() const { return reinterpret_cast<char*>(value_ptr); }
			size_t get_value_size() const { return sizeof(T); }
		public:
			value_animation(T& value, const T& _end_value, double _start_time, double _end_time, AnimationParameterMapping _parameter_mapping = APM_SIN_SQUARED) :
				animation(_start_time, _end_time, _parameter_mapping), value_ptr(&value), start_value(value), end_value(_end_value)
			{}
		};

		template <typename T>
		class linear_blend_animation : public value_animation<T>
		{
		protected:
			void set_value(double time) {
				double lambda = this->get_parameter(time);
				*this->value_ptr = (T)(1.0 - lambda)*this->start_value + (T)lambda*this->end_value;
			}
		public:
			linear_blend_animation(T& value, const T& _end_value, double _start_time, double _end_time, AnimationParameterMapping _parameter_mapping = APM_SIN_SQUARED) :
				value_animation<T>(value, _end_value, _start_time, _end_time, _parameter_mapping)
			{}
		};

		template <typename T>
		class geometric_blend_animation : public value_animation<T>
		{
		protected:
			void set_value(double time) {
				double lambda = this->get_parameter(time);
				*this->value_ptr = pow(this->start_value,(1.0 - lambda))*pow(this->end_value, lambda);
			}
		public:
			geometric_blend_animation(T& value, const T& _end_value, double _start_time, double _end_time, AnimationParameterMapping _parameter_mapping = APM_SIN_SQUARED) :
				value_animation<T>(value, _end_value, _start_time, _end_time, _parameter_mapping)
			{}
		};

		template <typename T>
		class slerp_animation : public value_animation<T>
		{
			double Omega;
		protected:
			void set_value(double time) {
				double lambda = this->get_parameter(time);
				*this->value_ptr = pow(this->start_value, (1.0 - lambda))*pow(this->end_value, lambda);
			}
		public:
			slerp_animation(T& value, const T& _end_value, double _start_time, double _end_time, AnimationParameterMapping _parameter_mapping = APM_LINEAR) :
				value_animation<T>(value, _end_value, _start_time, _end_time, _parameter_mapping)
			{
				Omega = acos(dot(this->start_value, this->end_value) / sqrt(dot(this->start_value,this->start_value)*dot(this->end_value,this->end_value)));
			}
		};


		template <typename T>
		class rotation_animation : public value_animation<cgv::math::fvec<T,3> >
		{
		public:
			cgv::math::fvec<T, 3> axis;
			T angle;
		protected:
			void set_value(double time) {
				double lambda = this->get_parameter(time);
				*this->value_ptr = cgv::math::rotate(this->start_value, axis, lambda * angle);
			}
		public:
			rotation_animation(cgv::math::fvec<T, 3>& value, const cgv::math::fvec<T, 3>& _axis, double _angle, double _start_time, double _end_time, AnimationParameterMapping _parameter_mapping = APM_SIN_SQUARED) :
				value_animation<cgv::math::fvec<T, 3> >(value, rotate(value, _axis, _angle), _start_time, _end_time, _parameter_mapping), axis(_axis), angle(_angle)
			{}
			rotation_animation(cgv::math::fvec<T, 3>& value, const cgv::math::fvec<T, 3>& _end_value, double _start_time, double _end_time, AnimationParameterMapping _parameter_mapping = APM_SIN_SQUARED) :
				value_animation<cgv::math::fvec<T, 3> >(value, _end_value, _start_time, _end_time, _parameter_mapping)
			{
				compute_rotation_axis_and_angle_from_vector_pair(this->start_value, this->end_value, axis, angle);
			}
		};


		template <typename T>
		animation_ptr animate_with_linear_blend(T& value, const T& target_value, double duration_sec, double delay_sec = 0.0f, bool terminate_other_animations = true)
		{
			double time = trigger::get_current_time();
			animation_ptr a_ptr(new linear_blend_animation<T>(value, target_value, time + delay_sec, time + duration_sec + delay_sec));
			add_animation(a_ptr, terminate_other_animations);
			return a_ptr;
		}

		template <typename T>
		animation_ptr animate_with_geometric_blend(T& value, const T& target_value, double duration_sec, double delay_sec = 0.0f, bool terminate_other_animations = true)
		{
			double time = trigger::get_current_time();
			animation_ptr a_ptr(new geometric_blend_animation<T>(value, target_value, time + delay_sec, time + duration_sec + delay_sec));
			add_animation(a_ptr, terminate_other_animations);
			return a_ptr;
		}

		template <typename T>
		animation_ptr animate_with_slerp(T& value, const T& target_value, double duration_sec, double delay_sec = 0.0f, bool terminate_other_animations = true)
		{
			double time = trigger::get_current_time();
			animation_ptr a_ptr(new slerp_animation<T>(value, target_value, time + delay_sec, time + duration_sec + delay_sec));
			add_animation(a_ptr, terminate_other_animations);
			return a_ptr;
		}
		template <typename T>
		animation_ptr animate_with_rotation(cgv::math::fvec<T, 3>& value, const cgv::math::fvec<T, 3>& target_value, double duration_sec, double delay_sec = 0.0f, bool terminate_other_animations = true)
		{
			double time = trigger::get_current_time();
			animation_ptr a_ptr(new rotation_animation<T>(value, target_value, time + delay_sec, time + duration_sec + delay_sec));
			add_animation(a_ptr, terminate_other_animations);
			return a_ptr;
		}
		template <typename T>
		animation_ptr animate_with_axis_rotation(cgv::math::fvec<T, 3>& value, const cgv::math::fvec<T, 3>& axis, T angle, double duration_sec, double delay_sec = 0.0f, bool terminate_other_animations = true)
		{
			double time = trigger::get_current_time();
			animation_ptr a_ptr(new rotation_animation<T>(value, axis, angle, time + delay_sec, time + duration_sec + delay_sec));
			add_animation(a_ptr, terminate_other_animations);
			return a_ptr;
		}
	}
}

#include <cgv/config/lib_end.h>
