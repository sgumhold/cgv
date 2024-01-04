#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <cgv/math/fvec.h>
#include <cgv/math/quaternion.h>

#include "lib_begin.h"

namespace stream_vis {
	/// different access types to time series samples
	enum TimeSeriesAccessor
	{
		TSA_ALL         = 31,
		TSA_NONE        =  0,
		TSA_TIME        =  1,
		TSA_X           =  2,
		TSA_Y           =  4,
		TSA_Z           =  8,
		TSA_W           = 16,
		TSA_LENGTH      = 32,
		TSA_DIRECTION_X = 64,
		TSA_DIRECTION_Y = 128,
		TSA_DIRECTION_Z = 256,
		TSA_DIRECTION_W = 512,
		TSA_DIRECTION_ALL = 64+128+256+512,
		TSA_FIRST = 1,
		TSA_LAST = 512
	};

	extern CGV_API std::string get_accessor_string(TimeSeriesAccessor tsa);

	/** Base class of time series that optionally organizes samples in a ring buffer and provides untyped access to stored data samples.
	    When ringbuffer is used not all samples are stored and one distinguishes between sample index (si) and cached
		sample index (csi). These can be converted into each other with convert_to_cached_sample_index() and
		get_cached_sample_index()
	*/
	class CGV_API time_series_base
	{
	protected:
		bool initialized;
		double time_offset;
		/// keep name of time series
		std::string name;
		size_t ringbuffer_size;
		std::atomic<size_t> nr_samples;
	public:
		/// construct time series - ring buffering is turned of if size parameter is 0
		time_series_base(size_t _ringbuffer_size = 0);
		/// return name of time series
		const std::string& get_name() const;
		/// set a new name for time series
		void set_name(const std::string& new_name);
		/// return number of components per sample
		virtual unsigned get_nr_components() const = 0;
		/// return current number of cached samples
		virtual size_t get_nr_cached_samples() const = 0;
		/// return void pointer to stored sample
		virtual void* get_void_sample_ptr(size_t index) = 0;
		/// return const void pointer to stored sample
		virtual const void* get_void_sample_ptr(size_t index) const = 0;
		/// return total number of seen samples
		size_t get_nr_samples() const;
		/// return sample index of first cached sample
		size_t get_sample_index_of_first_cached_sample() const;
		/// return time offset
		double get_time_offset() const;
		/// set new time offset and update time values of already stored samples
		virtual void set_time_offset(double offset) = 0;
		/// return the number of consecutively stored samples for given sample index si, which is 0 if si is outside of cached samples
		size_t get_nr_consecutive_samples(size_t si) const;
		/// try to convert sample index to cached sample index and return false if sample index points outside of cache
		bool convert_to_cached_sample_index(size_t& in_si_out_csi, bool ignore_bounds = false) const;
		/// convert to cached sample index by ignoring bounds
		size_t get_cached_sample_index(size_t si) const { convert_to_cached_sample_index(si, true); return si; }
		/// try to convert cached sample index to sample index and return false if cached sample index is invalid
		bool convert_to_sample_index(size_t& in_csi_out_si) const;
		/// check if time series is organized as ringbuffer
		inline bool has_ringbuffer() const { return get_ringbuffer_size() > 0; }
		/// query ringbuffer size
		inline size_t get_ringbuffer_size() const { return ringbuffer_size; }
		/// set the ringbuffer size (currently it is assumed that this is called only directly after construction!!)
		virtual void set_ringbuffer_size(size_t rbs);
		/// access sample of given sample index and store components specified in tsa in passed float array that must have sufficient space
		virtual bool put_sample_as_float(size_t sample_index, float* output, TimeSeriesAccessor tsa = TSA_ALL) const = 0;
	};

	/// template class that optionally allows to subtract value offset from stored values
	template <typename Time, typename Store, typename Value, bool use_value_offset = true>
	class time_series_value_offset : public time_series_base
	{
	protected:
		mutable bool value_offset_initialized;
		mutable Value value_offset;
	public:
		time_series_value_offset(size_t _ringbuffer_size = 0) : time_series_base(_ringbuffer_size), value_offset_initialized(false) {}
		/// return value offset
		Value get_value_offset() const { return value_offset; }
		/// initialize value offset
		void set_value_offset(const Value& offset)
		{
			if (value_offset_initialized) // update already stored samples to new offset
				for (size_t i = get_sample_index_of_first_cached_sample(); i < get_nr_samples(); ++i) {
					auto& s = reinterpret_cast<std::pair<Time, Store>*>(get_void_sample_ptr(i))->second;
					s = Store(value_offset + s - offset);
				}
			else
				value_offset_initialized = true;
			value_offset = offset;
		}
		/// reconstruct value from stored value
		Value reconstruct_value(const Store& stored_value) const { return value_offset + stored_value; }
		/// contruct stored value from value and in case of first value, set the value offset
		Store construct_stored_value(const Value& value) const
		{
			if (!value_offset_initialized) {
				value_offset = value;
				value_offset_initialized = true;
			}
			return Store(value - value_offset);
		}
	};
	
	/// specialization of time_series_value_offset for case where no offset is used
	template <typename Time, typename Store, typename Value>
	class time_series_value_offset<Time,Store,Value,false> : public time_series_base
	{
	public:
		time_series_value_offset(size_t _ringbuffer_size = 0) : time_series_base(_ringbuffer_size) {}
		/// reconstruct value from stored value
		Value reconstruct_value(const Store& stored_value) const { return stored_value; }
		/// contruct stored value from value and in case of first value, set the value offset
		Store construct_stored_value(const Value& value) const { return Store(value); }
	};

	/// time series stores (time,value)-pairs of type (double,Value) in vectors optionally organized in ring buffers of type (Time,Store) relative to time- and value-offset extracted from first sample
	template <typename Time, typename Store, typename Value, bool use_value_offset = true>
	class time_series_cache : public time_series_value_offset<Time,Store,Value,use_value_offset>
	{
	public:
		typedef Time time_type;
		typedef Value value_type;
		typedef Store stored_type;
		typedef typename std::pair<double, Value> sample_type;
		typedef typename std::pair<Time, Store> stored_sample_type;
	protected:
		std::vector<stored_sample_type> sample_cache;
	public:
		/// construct time series - ring buffering is turned of if size parameter is 0
		time_series_cache(size_t _ringbuffer_size = 0) : time_series_value_offset<Time, Store, Value, use_value_offset>(_ringbuffer_size) {}
		/// return number of components per sample
		unsigned get_nr_components() const { return 1; }
		/// return current number of cached samples
		size_t get_nr_cached_samples() const { return sample_cache.size(); }
		/// return const pointer to stored sample of given index or null if this sample is currently not cached
		const stored_sample_type* get_sample_ptr(size_t si) const { return this->convert_to_cached_sample_index(si) ? &sample_cache[si] : 0; }
		/// return pointer to stored sample of given index or null if this sample is currently not cached
		stored_sample_type* get_sample_ptr(size_t si) { return this->convert_to_cached_sample_index(si) ? &sample_cache[si] : 0; }
		/// return const void pointer to stored sample
		const void* get_void_sample_ptr(size_t si) const { return get_sample_ptr(si); }
		/// return void pointer to stored sample
		void* get_void_sample_ptr(size_t si) { return get_sample_ptr(si); }
		/// set new time offset and update time values of already stored samples
		void set_time_offset(double offset)
		{
			if (this->initialized) // update time of already stored samples to new offset
				for (size_t si = this->get_sample_index_of_first_cached_sample(); si < this->get_nr_samples(); ++si) {
					auto& t = reinterpret_cast<stored_sample_type*>(get_void_sample_ptr(si))->first;
					t = Time(this->time_offset + t - offset);
				}
			else
				this->initialized = true;
			this->time_offset = offset;
		}
		/// construct stored time
		Time construct_stored_time(const double& time) const { return Time(time - this->time_offset); }
		/// reconstruct value from stored value
		double reconstruct_time(const Time& stored_time) const { return this->time_offset + stored_time; }
		/// reconstruct sample from stored sample
		sample_type reconstruct_sample(const stored_sample_type& stored_sample)
		{
			return sample_type(reconstruct_time(stored_sample.first), reconstruct_value(stored_sample.second));
		}
		/// append a new sample by conversion to internal types
		void append_sample(double time, const Value& value)
		{
			if (!this->initialized) {
				this->time_offset = time;
				this->initialized = true;
				if (this->has_ringbuffer())
					sample_cache.reserve(this->get_ringbuffer_size());
			}
			stored_sample_type s(this->construct_stored_time(time), this->construct_stored_value(value));
			if (this->has_ringbuffer() && this->get_nr_samples() >= this->ringbuffer_size)
				sample_cache[this->get_cached_sample_index(this->get_nr_samples())] = s;
			else
				sample_cache.push_back(s);
			++this->nr_samples;
		}
		void set_ringbuffer_size(size_t rbs)
		{
			time_series_base::set_ringbuffer_size(rbs);
			if (this->ringbuffer_size > 0)
				sample_cache.reserve(this->ringbuffer_size);
		}
	};

	/// time series stores (time,value)-pairs of type (double,Value) in vectors optionally organized in ring buffers of type (Time,Store) relative to time- and value-offset extracted from first sample
	template <typename Time, typename Store, typename Value, bool use_value_offset = true>
	class time_series : public time_series_cache<Time, Store, Value, use_value_offset>
	{
	public:
		time_series(size_t _ringbuffer_size = 0) : time_series_cache<Time, Store, Value, use_value_offset>(_ringbuffer_size)
		{
		}
		/// put time and value of queried sample component into passed references and return whether sample of given index was available
		bool put_sample_as_float(size_t fst_si_then_csi, float* output, TimeSeriesAccessor tsa = TSA_ALL) const
		{
			if (!this->convert_to_cached_sample_index(fst_si_then_csi))
				return false;
			const typename time_series_cache<Time, Store, Value, use_value_offset>::stored_sample_type& s = this->sample_cache[fst_si_then_csi];
			if ((tsa & TSA_TIME) != 0)
				*output++ = s.first;
			if ((tsa & TSA_X) != 0)
				*output++ = float(s.second);
			if ((tsa & TSA_LENGTH) != 0)
				*output++ = std::abs(float(s.second));
			return true;
		}
	};

	/// specialization for vector types
	template <typename Time, typename Store, typename Value, uint32_t N, bool use_value_offset>
	class time_series<Time, cgv::math::fvec<Store, N>, cgv::math::fvec<Value, N>, use_value_offset> : public time_series_cache<Time, cgv::math::fvec<Store, N>, cgv::math::fvec<Value, N>, use_value_offset>
	{
	public:
		time_series(size_t _ringbuffer_size = 0) : time_series_cache<Time, cgv::math::fvec<Store, N>, cgv::math::fvec<Value, N>, use_value_offset>(_ringbuffer_size)
		{
		}
		/// return number of components per sample
		unsigned get_nr_components() const { return N; }
		/// put time and value of queried sample component into passed references and return whether sample of given index was available
		bool put_sample_as_float(size_t fst_si_snd_csi, float* output, TimeSeriesAccessor tsa = TSA_ALL) const
		{
			if (!this->convert_to_cached_sample_index(fst_si_snd_csi))
				return false;
			const typename time_series_cache<Time, cgv::math::fvec<Store, N>, cgv::math::fvec<Value, N>, use_value_offset>::stored_sample_type& s = this->sample_cache[fst_si_snd_csi];
			if ((tsa & TSA_TIME) != 0)
				*output++ = s.first;
			if ((tsa & TSA_X) != 0)
				*output++ = float(s.second[0]);
			if ((tsa & TSA_Y) != 0)
				*output++ = float(s.second[1]);
			if ((tsa & TSA_Z) != 0)
				*output++ = N > 2 ? float(s.second[2]) : 0.0f;
			if ((tsa & TSA_W) != 0)
				*output++ = N > 3 ? float(s.second[3]) : 0.0f;
			if ((tsa & TSA_LENGTH) != 0)
				*output++ = float(s.second.length());
			if ((tsa & TSA_DIRECTION_ALL) != 0) {
				cgv::math::fvec<Store, N> dir;
				dir = normalize(s.second);
				if ((tsa & TSA_DIRECTION_X) != 0)
					*output++ = float(dir[0]);
				if ((tsa & TSA_DIRECTION_Y) != 0)
					*output++ = float(dir[1]);
				if ((tsa & TSA_DIRECTION_Z) != 0)
					*output++ = N > 2 ? float(dir[2]) : 0.0f;
				if ((tsa & TSA_DIRECTION_W) != 0)
					*output++ = N > 3 ? float(dir[3]) : 0.0f;
			}
			return true;
		}
	};
	/// specialization for quaternion types
	template <typename Time, typename Store, typename Value, bool use_value_offset>
	class time_series<Time, cgv::math::quaternion<Store>, cgv::math::quaternion<Value>, use_value_offset> : public time_series_cache<Time, cgv::math::quaternion<Store>, cgv::math::quaternion<Value>, use_value_offset>
	{
	public:
		time_series(size_t _ringbuffer_size = 0) : time_series_cache<Time, cgv::math::quaternion<Store>, cgv::math::quaternion<Value>, use_value_offset>(_ringbuffer_size)
		{
		}
		/// return number of components per sample
		unsigned get_nr_components() const { return 4; }
		/// put time and value of queried sample component into passed references and return whether sample of given index was available
		bool put_sample_as_float(size_t fst_si_then_csi, float* output, TimeSeriesAccessor tsa = TSA_ALL) const
		{
			if (!this->convert_to_cached_sample_index(fst_si_then_csi))
				return false;
			const typename time_series_cache<Time, cgv::math::quaternion<Store>, cgv::math::quaternion<Value>, use_value_offset>::stored_sample_type& s = this->sample_cache[fst_si_then_csi];
			if ((tsa & TSA_TIME) != 0)
				*output++ = s.first;
			if ((tsa & TSA_X) != 0)
				*output++ = float(s.second[0]);
			if ((tsa & TSA_Y) != 0)
				*output++ = float(s.second[1]);
			if ((tsa & TSA_Z) != 0)
				*output++ = float(s.second[2]);
			if ((tsa & TSA_W) != 0)
				*output++ = float(s.second[3]);
			if ((tsa & TSA_LENGTH) != 0)
				*output++ = float(s.second.length());
			if ((tsa & TSA_DIRECTION_ALL) != 0) {
				cgv::math::fvec<Store, 4> dir;
				dir = normalize(reinterpret_cast<const cgv::math::fvec<Store, 4>&>(s.second));
				if ((tsa & TSA_DIRECTION_X) != 0)
					*output++ = float(dir[0]);
				if ((tsa & TSA_DIRECTION_Y) != 0)
					*output++ = float(dir[1]);
				if ((tsa & TSA_DIRECTION_Z) != 0)
					*output++ = float(dir[2]);
				if ((tsa & TSA_DIRECTION_W) != 0)
					*output++ = float(dir[3]);
			}
			return true;
		}
	};
}

#include <cgv/config/lib_end.h>
