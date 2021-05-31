#include "time_series.h"

namespace stream_vis {

	std::string get_accessor_string(TimeSeriesAccessor tsa)
	{
		std::string result;
		if ((tsa & TSA_TIME) != 0)
			result += 't';
		if ((tsa & TSA_X) != 0)
			result += 'x';
		if ((tsa & TSA_Y) != 0)
			result += 'y';
		if ((tsa & TSA_Z) != 0)
			result += 'z';
		if ((tsa & TSA_W) != 0)
			result += 'w';
		if ((tsa & TSA_LENGTH) != 0)
			result += 'l';
		if ((tsa & TSA_DIRECTION_X) != 0)
			result += 'X';
		if ((tsa & TSA_DIRECTION_Y) != 0)
			result += 'Y';
		if ((tsa & TSA_DIRECTION_Z) != 0)
			result += 'Z';
		if ((tsa & TSA_DIRECTION_W) != 0)
			result += 'W';
		return result;
	}

	/// return total number of seen samples
	size_t time_series_base::get_nr_samples() const
	{
		return nr_samples;
	}
	/// return index of first sample stored in buffer
	size_t time_series_base::get_sample_index_of_first_cached_sample() const
	{
		if (!has_ringbuffer() || get_nr_samples() <= get_ringbuffer_size())
			return 0;
		return get_nr_samples() - get_ringbuffer_size();
	}
	/// return time offset
	double time_series_base::get_time_offset() const
	{
		return time_offset;
	}
	size_t time_series_base::get_nr_consecutive_samples(size_t si) const
	{
		if (si > get_nr_samples())
			return 0;
		size_t cnt = get_nr_samples() < si;
		if (!has_ringbuffer())
			return cnt;
		if (!convert_to_cached_sample_index(si))
			return 0;
		return std::min(cnt, get_ringbuffer_size() - si);
	}
	bool time_series_base::convert_to_cached_sample_index(size_t& in_si_out_csi, bool ignore_bounds) const
	{
		if (has_ringbuffer()) {
			if (!ignore_bounds) {
				if (in_si_out_csi >= get_nr_samples())
					return false;
				if (get_nr_samples() - in_si_out_csi >= get_ringbuffer_size())
					return false;
			}
			in_si_out_csi = in_si_out_csi % get_ringbuffer_size();
			return true;
		}
		else
			return ignore_bounds || (in_si_out_csi < get_nr_samples());
	}
	/// try to convert cached sample index to sample index and return false if cached sample index is invalid
	bool time_series_base::convert_to_sample_index(size_t& in_csi_out_si) const
	{
		if (in_csi_out_si > get_nr_cached_samples())
			return false;
		if (!has_ringbuffer())
			return true;
		in_csi_out_si += (get_nr_cached_samples() / get_ringbuffer_size())* get_ringbuffer_size();
		return true;
	}
	///
	void time_series_base::set_ringbuffer_size(size_t rbs)
	{
		ringbuffer_size = rbs;
	}

	/// construct time series - ring buffering is turned of if size parameter is 0
	time_series_base::time_series_base(size_t _ringbuffer_size)
	{
		initialized = false;
		ringbuffer_size = _ringbuffer_size;
		nr_samples = 0;
		time_offset = 0;
	}
	/// return name of time series
	const std::string& time_series_base::get_name() const
	{
		return name;
	}
	/// set a new name for time series
	void time_series_base::set_name(const std::string& new_name)
	{
		name = new_name;
	}

	template class time_series<float, bool, bool>;
	template class time_series<float, int32_t, int64_t>;
	template class time_series<float, uint32_t, uint64_t>;
	template class time_series<float, cgv::math::fvec<float, 3>, cgv::math::fvec<double, 3>>;
	template class time_series<float, cgv::math::quaternion<float>, cgv::math::quaternion<double>>;
}
