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
	/// check for new value
	bool streaming_time_series::has_new_value() const
	{
		return last_had_new_values;
	}
	/// set status of new value
	void streaming_time_series::set_new_value(bool has_new)
	{
		last_had_new_values = has_new;
	}
	streaming_time_series::streaming_time_series(cgv::type::info::TypeId _type_id) : type_id(_type_id)
	{
		last_had_new_values = false;
		nan_value = std::numeric_limits<float>::quiet_NaN();
		uses_nan = false;
		nan_mapping_mode = NMM_DEFAULT;
		outofdate = true;
		first_visible_sample_index = 0;
		// mark default values as undefined
		default_color = rgb(-1,-1,-1);
		default_opacity = -1;
		default_size = -1;
		lower_bound_index = uint16_t(-1);
		upper_bound_index = uint16_t(-1);
		aabb_mode = AM_BLOCKED_16;
		//series().set_ringbuffer_size(1024);
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
	bool streaming_time_series::is_resample() const
	{
		return false;
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
	streaming_time_series* streaming_time_series::construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const
	{
		return 0;
	}
	streaming_time_series* float_time_series::construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const
	{
		resampled_float_time_series* rts = new resampled_float_time_series(name, sampling_ts);
		rts->aabb_mode = aabb_mode;
		rts->default_color = default_color;
		rts->default_opacity = default_opacity;
		rts->default_size = default_size;
		rts->lower_bound_index = lower_bound_index;
		rts->upper_bound_index = upper_bound_index;
		rts->index = index;
		return rts;
	}
	resampled_float_time_series::resampled_float_time_series(const std::string& _name, streaming_time_series* _sampling_ts_ptr)
		: float_time_series(0)
	{
		streaming_time_series::name = _name;
		time_series<float, float, double>::name = _name;
		sampling_ts_ptr = _sampling_ts_ptr;
		have_new_value = false;
	}
	bool resampled_float_time_series::is_resample() const
	{
		return true;
	}

	bool resampled_bool_time_series::is_resample() const
	{
		return true;
	}

	bool resampled_float_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp)
	{
		// is sampling time series did not sample a value, in resample we do not need to sample either
		if (!sampling_ts_ptr->has_new_value()) {
			for (uint16_t i = 0; i < num_values; ++i) {
				if (values[i].index == index) {
					have_new_value = true;
					new_value = float(reinterpret_cast<const double&>(values[i].value[0]));
					new_timestamp = timestamp;
					break;
				}
			}
			return false;
		}
		// if value is sampled, store this
		if (float_time_series::extract_from_values(num_values, values, timestamp)) {
			have_new_value = false;
			return true;
		}
		if (have_new_value) {
			have_new_value = false;
			append_sample(timestamp, new_value);
			return true;
		}
		// otherwise store nan
		lock.lock();
		append_sample(timestamp, nan_value);
		outofdate = true;
		lock.unlock();
		return true;
	}

	streaming_time_series* bool_time_series::construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const
	{
		resampled_bool_time_series* rts = new resampled_bool_time_series(name, sampling_ts);
		rts->aabb_mode = aabb_mode;
		rts->default_color = default_color;
		rts->default_opacity = default_opacity;
		rts->default_size = default_size;
		rts->lower_bound_index = lower_bound_index;
		rts->upper_bound_index = upper_bound_index;
		rts->index = index;
		return rts;
	}

	resampled_bool_time_series::resampled_bool_time_series(const std::string& _name, streaming_time_series* _sampling_ts_ptr)
		: bool_time_series(0)
	{
		streaming_time_series::name = _name;
		time_series<float, uint8_t, bool, false>::name = _name;
		sampling_ts_ptr = _sampling_ts_ptr;
	}
	
	bool resampled_bool_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp)
	{
		// is sampling time series did not sample a value, in resample we do not need to sample either
		if (!sampling_ts_ptr->has_new_value())
			return false;
		// if value is sampled, store this
		if (bool_time_series::extract_from_values(num_values, values, timestamp))
			return true;
		// otherwise store nan
		lock.lock();
		if (!this->initialized) {
			this->time_offset = timestamp;
			this->initialized = true;
			if (this->has_ringbuffer())
				sample_cache.reserve(this->get_ringbuffer_size());
		}
		stored_sample_type s(this->construct_stored_time(timestamp), uint8_t(nan_value));
		if (this->has_ringbuffer() && this->get_nr_samples() >= this->ringbuffer_size)
			sample_cache[this->get_cached_sample_index(this->get_nr_samples())] = s;
		else
			sample_cache.push_back(s);
		++this->nr_samples;
		outofdate = true;
		lock.unlock();
		return true;
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



