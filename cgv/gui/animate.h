#pragma once
#include <cgv/data/ref_ptr.h>
#include <cgv/base/base.h>
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
			void set_base_ptr(cgv::base::base_ptr _bp);
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
			T end_value;
			T* value_ptr;
			char* get_ptr() const { return reinterpret_cast<char*>(value_ptr); }
			size_t get_value_size() const { return sizeof(T); }
			void set_value(double time) {
				double lambda = get_parameter(time);
				*value_ptr = (1.0 - lambda)*start_value + lambda*end_value;
			}
		public:
			value_animation(T& value, const T& _end_value, double _start_time, double _end_time, AnimationParameterMapping _parameter_mapping = APM_LINEAR) :
				animation(_start_time,_end_time,_parameter_mapping), value_ptr(&value), start_value(value), end_value(_end_value)
			{}
		};

		template <typename T>
		animation_ptr animate(T& value, const T& target_value, double duration_sec, double delay_sec = 0.0f, bool terminate_other_animations = true)
		{
			double time = trigger::get_current_time();
			animation_ptr a_ptr(new value_animation<T>(value, target_value, time + delay_sec, time + duration_sec + delay_sec));
			add_animation(a_ptr, terminate_other_animations);
			return a_ptr;
		}
	}
}

#include <cgv/config/lib_end.h>