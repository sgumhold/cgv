#include "streaming_time_series.h"

namespace stream_vis {
	/// return name
	const std::string& streaming_time_series::get_name() const
	{
		return name;
	}
	/// 
	void streaming_time_series::set_name(const std::string& new_name)
	{
		name = new_name;
		series().set_name(new_name);
	}

	streaming_time_series::streaming_time_series(cgv::type::info::TypeId _type_id) : type_id(_type_id)
	{
		outofdate = true;
		first_visible_sample_index = 0;
		// mark default values as undefined
		default_color = rgb(-1,-1,-1);
		default_opacity = -1;
		default_size = -1;
		lower_bound_index = uint16_t(-1);
		upper_bound_index = uint16_t(-1);
		aabb_mode = AM_BLOCKED_16;
	}
	/// return type id of value
	cgv::type::info::TypeId streaming_time_series::get_value_type_id() const
	{
		return type_id;
	}
	//
	std::string streaming_time_series::get_value_type_name() const
	{
		return cgv::type::info::get_type_name(type_id);
	}

	/// 
	//void streaming_time_series::append_cached_samples(std::vector<cgv::math::fvec<float, 2>>& samples, unsigned component_index) const
	//{
	//	lock.lock();
	//	const auto& s = series();
	//	for (size_t index = s.get_front_index(); index < s.get_nr_samples(); ++index) {
	//		cgv::math::fvec<float, 2> sample;
	//		if (s.put_sample_as_float(index, sample[0], sample[1]))
	//			samples.push_back(sample);
	//	}
	//	outofdate = false;
	//	lock.unlock();
	//}
	bool streaming_time_series::is_outofdate() const
	{
		return outofdate;
	}

	float_time_series::float_time_series(uint16_t i) : streaming_time_series(cgv::type::info::TI_FLT64), index(i)
	{
	}
	bool float_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp)
	{
		for (uint16_t i = 0; i < num_values; ++i) {
			if (values[i].index == index) {
				lock.lock();
				append_sample(timestamp, reinterpret_cast<const double&>(values[i].value[0]));
				outofdate = true;
				lock.unlock();
				return true;
			}
		}
		return false;
	}

	int_time_series::int_time_series(uint16_t i) : streaming_time_series(cgv::type::info::TI_INT64), index(i)
	{
		set_value_offset(0);
	}

	bool int_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp)
	{
		for (uint16_t i = 0; i < num_values; ++i) {
			if (values[i].index == index) {
				lock.lock();
				append_sample(timestamp, reinterpret_cast<const int64_t&>(values[i].value[0]));
				outofdate = true;
				lock.unlock();
				return true;
			}
		}
		return false;
	}

	uint_time_series::uint_time_series(uint16_t i) : streaming_time_series(cgv::type::info::TI_UINT64), index(i)
	{
		set_value_offset(0);
	}
	bool uint_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp)
	{
		for (uint16_t i = 0; i < num_values; ++i) {
			if (values[i].index == index) {
				lock.lock();
				append_sample(timestamp, reinterpret_cast<const uint64_t&>(values[i].value[0]));
				outofdate = true;
				lock.unlock();
				return true;
			}
		}
		return false;
	}
	bool_time_series::bool_time_series(uint16_t i) : streaming_time_series(cgv::type::info::TI_BOOL), index(i)
	{
	}
	bool bool_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp)
	{
		for (uint16_t i = 0; i < num_values; ++i) {
			if (values[i].index == index) {
				lock.lock();
				append_sample(timestamp, reinterpret_cast<const bool&>(values[i].value[0]));
				outofdate = true;
				lock.unlock();
	//			std::cout << streaming_time_series::get_name() << "(" << construct_stored_time(timestamp)  << ")" << reinterpret_cast<const uint64_t&>(values[i].value[0]) << " = " << (reinterpret_cast<const bool&>(values[i].value[0]) ? "true" : "false") << std::endl;
				return true;
			}
		}
		return false;
	}
	quat_time_series::quat_time_series(uint16_t i0, uint16_t i1, uint16_t i2, uint16_t i3) : streaming_time_series(cgv::type::info::TI_ARRAY)
	{
		indices[0] = i0;
		indices[1] = i1;
		indices[2] = i2;
		indices[3] = i3;
	}
	/// return name of value type
	std::string quat_time_series::get_value_type_name() const
	{
		return "quat";
	}
	bool quat_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp)
	{
		cgv::math::quaternion<double> q;
		int cnt = 0;
		for (uint16_t i = 0; i < num_values; ++i) {
			for (int c = 0; c < 3; ++c)
				if (values[i].index == indices[c]) {
					q[c] = reinterpret_cast<const double&>(values[i].value[0]);
					++cnt;
				}
		}
		if (cnt != 4)
			return false;
		lock.lock();
		append_sample(timestamp, q);
		outofdate = true;
		lock.unlock();
		return true;
	}
}



