#include "streaming_time_series.h"

#include <cgv/media/color.h>

namespace stream_vis {
	const std::string& streaming_time_series::get_name() const
	{
		return name;
	}
	void streaming_time_series::set_name(const std::string& new_name)
	{
		name = new_name;
		series().set_name(new_name);
	}
	bool streaming_time_series::has_new_value() const
	{
		return last_had_new_values;
	}
	void streaming_time_series::set_new_value(bool has_new)
	{
		last_had_new_values = has_new;
	}
	streaming_time_series::streaming_time_series(cgv::type::info::TypeId _type_id) : type_id(_type_id)
	{
		last_had_new_values = false;
		have_new_value = false;
		nan_value = std::numeric_limits<float>::quiet_NaN();
		uses_nan = false;
		nan_mapping_mode = NMM_DEFAULT;
		outofdate = true;
		first_visible_sample_index = 0;
		// mark default values as undefined
		default_color = cgv::rgb(-1,-1,-1);
		default_opacity = -1;
		default_size = -1;
		lower_bound_index = uint16_t(-1);
		upper_bound_index = uint16_t(-1);
		aabb_mode = AM_BLOCKED_16;
		transform = TT_NONE;
		transform_origin = cgv::dvec3(0.0);
		//series().set_ringbuffer_size(1024);
	}
	cgv::type::info::TypeId streaming_time_series::get_value_type_id() const
	{
		return type_id;
	}
	std::string streaming_time_series::get_value_type_name() const
	{
		return cgv::type::info::get_type_name(type_id);
	}
	bool streaming_time_series::is_outofdate() const
	{
		return outofdate;
	}
	bool streaming_time_series::is_resample() const
	{
		return false;
	}
	void streaming_time_series::copy_from(const streaming_time_series& sts)
	{
		type_id = sts.type_id;
		nan_value = sts.nan_value;
		nan_mapping_mode = sts.nan_mapping_mode;
		aabb_mode = sts.aabb_mode;
		default_color = sts.default_color;
		default_opacity = sts.default_opacity;
		default_size = sts.default_size;
		lower_bound_index = sts.lower_bound_index;
		upper_bound_index = sts.upper_bound_index;
	}
	streaming_time_series* streaming_time_series::construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const
	{
		return 0;
	}

	float_time_series::float_time_series(uint16_t i) : streaming_time_series(cgv::type::info::TI_FLT64), index(i)
	{
	}
	bool float_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx)
	{
		uint16_t vi = val_idx_from_ts_idx[index];
		if (vi == uint16_t(-1))
			return false;
		lock.lock();
		append_sample(timestamp, reinterpret_cast<const double&>(values[vi].value[0]));
		outofdate = true;
		lock.unlock();
		return true;
	}
	streaming_time_series* float_time_series::construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const
	{
		resampled_float_time_series* rts = new resampled_float_time_series(name, sampling_ts);
		rts->copy_from(*this);
		rts->index = index;
		return rts;
	}

	resampled_float_time_series::resampled_float_time_series(const std::string& _name, streaming_time_series* _sampling_ts_ptr)
		: float_time_series(0)
	{
		streaming_time_series::name = _name;
		time_series<float, float, double>::name = _name;
		sampling_ts_ptr = _sampling_ts_ptr;
	}
	bool resampled_float_time_series::is_resample() const
	{
		return true;
	}
	bool resampled_float_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx)
	{
		// if sampling time series did not sample a value
		if (!sampling_ts_ptr->has_new_value()) {
			// cache potentially new value for next sample of sampling time series
			uint16_t vi = val_idx_from_ts_idx[index];
			if (vi != uint16_t(-1)) {
				have_new_value = true;
				new_value = float(reinterpret_cast<const double&>(values[vi].value[0]));
				new_timestamp = timestamp;
			}
			// announce that we did not store a new value yet
			return false;
		}
		// if no value is sampled
		if (!float_time_series::extract_from_values(num_values, values, timestamp, val_idx_from_ts_idx)) {
			// either store cached new value or nan value 
			lock.lock();
			append_sample(timestamp, have_new_value ? new_value : nan_value);
			outofdate = true;
			lock.unlock();
		}
		// always clear cache
		have_new_value = false;
		return true;
	}

	int_time_series::int_time_series(uint16_t i) : streaming_time_series(cgv::type::info::TI_INT64), index(i)
	{
		set_value_offset(0);
		nan_value = float(std::numeric_limits<int32_t>::max());
	}
	bool int_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx)
	{
		uint16_t vi = val_idx_from_ts_idx[index];
		if (vi == uint16_t(-1))
			return false;
		lock.lock();
		append_sample(timestamp, reinterpret_cast<const int64_t&>(values[vi].value[0]));
		outofdate = true;
		lock.unlock();
		return true;
	}
	streaming_time_series* int_time_series::construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const
	{
		resampled_int_time_series* rts = new resampled_int_time_series(name, sampling_ts);
		rts->copy_from(*this);
		rts->index = index;
		return rts;
	}

	bool resampled_int_time_series::is_resample() const
	{
		return true;
	}
	resampled_int_time_series::resampled_int_time_series(const std::string& _name, streaming_time_series* _sampling_ts_ptr)
		: int_time_series(0)
	{
		streaming_time_series::name = _name;
		time_series<float, int32_t, int64_t>::name = _name;
		sampling_ts_ptr = _sampling_ts_ptr;
	}
	bool resampled_int_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx)
	{
		// if sampling time series did not sample a value
		if (!sampling_ts_ptr->has_new_value()) {
			// cache potentially new value for next sample of sampling time series
			uint16_t vi = val_idx_from_ts_idx[index];
			if (vi != uint16_t(-1)) {
				have_new_value = true;
				new_value = int32_t(reinterpret_cast<const int64_t&>(values[vi].value[0]));
				new_timestamp = timestamp;
			}
			// announce that we did not store a new value yet
			return false;
		}
		// if no value is sampled
		if (!int_time_series::extract_from_values(num_values, values, timestamp, val_idx_from_ts_idx)) {
			// either store cached new value or nan value 
			lock.lock();
			append_sample(timestamp, have_new_value ? new_value : int32_t(nan_value));
			outofdate = true;
			lock.unlock();
		}
		// always clear cache
		have_new_value = false;
		return true;
	}

	uint_time_series::uint_time_series(uint16_t i) : streaming_time_series(cgv::type::info::TI_UINT64), index(i)
	{
		set_value_offset(0);
		nan_value = float(std::numeric_limits<uint32_t>::max());
	}
	bool uint_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx)
	{
		uint16_t vi = val_idx_from_ts_idx[index];
		if (vi == uint16_t(-1))
			return false;
		lock.lock();
		append_sample(timestamp, reinterpret_cast<const uint64_t&>(values[vi].value[0]));
		outofdate = true;
		lock.unlock();
		return true;
	}
	streaming_time_series* uint_time_series::construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const
	{
		resampled_uint_time_series* rts = new resampled_uint_time_series(name, sampling_ts);
		rts->copy_from(*this);
		rts->index = index;
		return rts;
	}

	bool resampled_uint_time_series::is_resample() const
	{
		return true;
	}
	resampled_uint_time_series::resampled_uint_time_series(const std::string& _name, streaming_time_series* _sampling_ts_ptr)
		: uint_time_series(0)
	{
		streaming_time_series::name = _name;
		time_series<float, uint32_t, uint64_t>::name = _name;
		sampling_ts_ptr = _sampling_ts_ptr;
	}
	bool resampled_uint_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx)
	{
		// if sampling time series did not sample a value
		if (!sampling_ts_ptr->has_new_value()) {
			// cache potentially new value for next sample of sampling time series
			uint16_t vi = val_idx_from_ts_idx[index];
			if (vi != uint16_t(-1)) {
				have_new_value = true;
				new_value = uint32_t(reinterpret_cast<const uint64_t&>(values[vi].value[0]));
				new_timestamp = timestamp;
			}
			// announce that we did not store a new value yet
			return false;
		}
		// if no value is sampled
		if (!uint_time_series::extract_from_values(num_values, values, timestamp, val_idx_from_ts_idx)) {
			// either store cached new value or nan value 
			lock.lock();
			append_sample(timestamp, have_new_value ? new_value : uint32_t(nan_value));
			outofdate = true;
			lock.unlock();
		}
		// always clear cache
		have_new_value = false;
		return true;
	}

	bool_time_series::bool_time_series(uint16_t i) : streaming_time_series(cgv::type::info::TI_BOOL), index(i)
	{
	}
	bool bool_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx)
	{
		uint16_t vi = val_idx_from_ts_idx[index];
		if (vi == uint16_t(-1))
			return false;
		lock.lock();
		append_sample(timestamp, reinterpret_cast<const bool&>(values[vi].value[0]));
		outofdate = true;
		lock.unlock();
		return true;
	}
	streaming_time_series* bool_time_series::construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const
	{
		resampled_bool_time_series* rts = new resampled_bool_time_series(name, sampling_ts);
		rts->copy_from(*this);
		rts->index = index;
		return rts;
	}

	bool resampled_bool_time_series::is_resample() const
	{
		return true;
	}
	resampled_bool_time_series::resampled_bool_time_series(const std::string& _name, streaming_time_series* _sampling_ts_ptr)
		: bool_time_series(0)
	{
		streaming_time_series::name = _name;
		time_series<float, uint8_t, bool, false>::name = _name;
		sampling_ts_ptr = _sampling_ts_ptr;
		new_value = false;
	}
	bool resampled_bool_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx)
	{
		// is sampling time series did not sample a value, in resample we do not need to sample either
		if (!sampling_ts_ptr->has_new_value()) {
			// cache potentially new value for next sample of sampling time series
			uint16_t vi = val_idx_from_ts_idx[index];
			if (vi != uint16_t(-1)) {
				have_new_value = true;
				new_value = reinterpret_cast<const bool&>(values[vi].value[0]);
				new_timestamp = timestamp;
			}
			// announce that we did not store a new value yet
			return false;
		}
		// if value is sampled, store this
		if (bool_time_series::extract_from_values(num_values, values, timestamp, val_idx_from_ts_idx))
			return true;
		// otherwise store cached or nan value
		lock.lock();
		if (!this->initialized) {
			this->time_offset = timestamp;
			this->initialized = true;
			if (this->has_ringbuffer())
				sample_cache.reserve(this->get_ringbuffer_size());
		}
		stored_sample_type s(this->construct_stored_time(timestamp), uint8_t(have_new_value ? new_value : nan_value));
		if (this->has_ringbuffer() && this->get_nr_samples() >= this->ringbuffer_size)
			sample_cache[this->get_cached_sample_index(this->get_nr_samples())] = s;
		else
			sample_cache.push_back(s);
		++this->nr_samples;
		outofdate = true;
		lock.unlock();
		have_new_value = false;
		return true;
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
	bool quat_time_series::extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx)
	{
		cgv::math::quaternion<double> q;
		for (int ci = 0; ci < 4; ++ci) {
			uint16_t vi = val_idx_from_ts_idx[indices[ci]];
			if (vi == uint16_t(-1))
				return false;
			q[ci] = reinterpret_cast<const double&>(values[vi].value[0]);
		}
		lock.lock();
		append_sample(timestamp, q);
		outofdate = true;
		lock.unlock();
		return true;
	}
}



