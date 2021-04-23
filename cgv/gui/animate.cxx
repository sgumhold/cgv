#include "animate.h"
#include <algorithm>
#include <set>

namespace cgv {
	namespace gui {

		animation::animation(double _start_time, double _end_time, AnimationParameterMapping _parameter_mapping)
		{
			start_time = _start_time;
			end_time = _end_time;
			parameter_mapping = _parameter_mapping;
		}
		void animation::set_base_ptr(cgv::base::base_ptr _bp)
		{
			bp = _bp;
		}
		void animation::configure(AnimationParameterMapping _parameter_mapping, cgv::base::base_ptr _bp)
		{
			parameter_mapping = _parameter_mapping;
			bp = _bp;
		}

		void animation::set_parameter_mapping(AnimationParameterMapping _parameter_mapping)
		{
			parameter_mapping = _parameter_mapping;
		}
		bool animation::has_started(double time) const
		{
			return time >= start_time;
		}
		double animation::get_start_time() const
		{
			return start_time;
		}

		bool animation::is_over(double time) const
		{
			return time >= end_time;
		}
		double animation::get_parameter(double time) const
		{
			double lambda = std::min(1.0, std::max(0.0, (time - start_time) / (end_time - start_time)));
			switch (parameter_mapping) {
			case APM_LINEAR: return lambda;
			case APM_SIN_SQUARED: return pow(sin(1.570796327*lambda), 2.0);
			case APM_CUBIC: return lambda*lambda*(3 - 2 * lambda);
			default:
			{
				unsigned nr = (int&)parameter_mapping;
				return floor(lambda*nr + 0.5) / nr;
			}
			}
		}
		bool animation::animates(const void* ptr) const
		{
			const char* char_ptr = reinterpret_cast<const char*>(ptr);
			return char_ptr >= get_ptr() && char_ptr < get_ptr() + get_value_size();
		}
		bool animation::overlaps(const char* value_ptr, size_t value_size) const
		{
			if (value_ptr + value_size <= get_ptr())
				return false;
			if (get_ptr() + get_value_size() <= value_ptr)
				return false;
			return true;
		}

		bool animation::set_time(double time)
		{
			set_value(time);
			if (bp)
				bp->on_set(get_ptr());
			return is_over(time);
		}

		struct priority
		{
			bool operator()(const animation_ptr& _Left, const animation_ptr& _Right) const
			{
				return _Left->get_start_time() < _Right->get_start_time();
			}
		};
		class animation_manager : virtual public cgv::signal::tacker
		{
			std::multiset<animation_ptr, priority> queue;
			bool is_connected;
		public:
			void try_to_connect()
			{
				if (!is_connected) {
					if (get_trigger_server()) {
						is_connected = true;
						cgv::signal::connect(get_animation_trigger().shoot, this, &animation_manager::timer_event);
					}
				}
			}
		public:
			animation_manager()
			{
				is_connected = false;
				try_to_connect();
			}
			void terminate_animations(const char* value_ptr, size_t value_size)
			{
				for (auto i = queue.begin(); i != queue.end(); ) {
					if ((*i)->overlaps(value_ptr, value_size)) {
						auto j = i;
						++i;
						queue.erase(j);
					}
					else
						++i;
				}
			}
			void add_animation(animation_ptr a_ptr)
			{
				queue.insert(a_ptr);
			}
			void set_time(double time)
			{
				for (auto i = queue.begin(); i != queue.end(); ) {
					if (!(*i)->has_started(time))
						break;
					if ((*i)->set_time(time)) {
						auto j = i;
						++i;
						queue.erase(j);
					}
					else
						++i;
				}
			}
			void timer_event(double time, double dt)
			{
				set_time(time);
			}
		};
		animation_manager& ref_animation_manager()
		{
			static animation_manager anim_manager;
			anim_manager.try_to_connect();
			return anim_manager;
		}
		void add_animation(animation_ptr a_ptr, bool terminate_other_animations)
		{
			if (terminate_other_animations)
				ref_animation_manager().terminate_animations(a_ptr->get_ptr(), a_ptr->get_value_size());
			ref_animation_manager().add_animation(a_ptr);
		}

	}
}