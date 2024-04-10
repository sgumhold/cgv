#pragma once

#include "time_series.h"

#include <cgv/os/mutex.h>
#include <cgv/type/info/type_id.h>
#include <cgv/math/geo_transform.h>
#include <cgv/media/color.h>

#include "lib_begin.h"

namespace stream_vis {
	extern "C"
	{
		typedef uint8_t value_type[8];
		/// combination of index and value
		struct indexed_value
		{
			uint16_t index;
			value_type value;
		};
	}
	enum TransformType
	{
		TT_NONE,
		TT_GEODETIC2ENU_WGS84
	};
	enum AABBMode
	{
		AM_NONE,
		AM_BRUTE_FORCE,
		AM_BLOCKED_8,
		AM_BLOCKED_16
	};
	enum NanMappingMode
	{
		NMM_DEFAULT,
		NMM_ATTRIBUTE_ZERO,
		NMM_ATTRIBUTE_ONE,
		NMM_ATTRIBUTE_MIN,
		NMM_ATTRIBUTE_MAX
	};
	/// interface for all time series used in the streaming visualization library
	class CGV_API streaming_time_series
	{
	protected:
		/// mutex used to support parallel appending of new values
		mutable cgv::os::mutex lock;
		/// keep track whether time series is out of date due to new sample
		mutable bool outofdate;
		/// store id of value type
		cgv::type::info::TypeId type_id;
		/// store name of streaming time series
		std::string name;
		/// whether a new value is not yet processed
		bool last_had_new_values;

		/// used for resampled time series to keep track whether new value is cached
		bool have_new_value;
		/// used for resampled time series to cached timestamp of new value
		double new_timestamp;
	public:
		/// for 2D or 3D vector valued time series the transformation
		TransformType transform;
		/// origin used for transformation (geodetic reference point)
		cgv::dvec3 transform_origin;
		/// whether NAN-values are used; defaults to false
		bool uses_nan;
		/// float encoding of nan value; defaults to std::numeric_limits<float>::quiet_NaN()
		float nan_value;
		/// mode of how to map nan values; defaults to NMM_DEFAULT
		NanMappingMode nan_mapping_mode;
		/// store index of first visible sample
		size_t first_visible_sample_index;
		/// default color
		cgv::rgb default_color;
		/// default opacity
		float default_opacity;
		/// default size
		float default_size;
		/// index of time series value defining minimum
		uint16_t lower_bound_index;
		/// index of time series value defining minimum
		uint16_t upper_bound_index;
		/// type of automatic bound computation
		AABBMode aabb_mode;
		/// message currently used only for triggers
		std::string message;
		/// return name
		const std::string& get_name() const;
		/// 
		void set_name(const std::string& new_name);
		/// check for new value
		bool has_new_value() const;
		/// set status of new value
		void set_new_value(bool has_new);
		/// return whether time series is a resampled time series
		virtual bool is_resample() const;
		/// copy all type independent members from another time series
		virtual void copy_from(const streaming_time_series& sts);
		/// construct empty streaming time series
		streaming_time_series(cgv::type::info::TypeId _type_id = cgv::type::info::TI_UNDEF);
		/// construct a time series that resamples this time series according to the sampling time series passed in the argument
		virtual streaming_time_series* construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const;
		/// define virtual destructor for abstract base class
		virtual ~streaming_time_series() {}
		/// return list of unique indices to defining inputs/outputs
		virtual std::vector<uint16_t> get_io_indices() const = 0;
		/// return type id of value
		virtual cgv::type::info::TypeId get_value_type_id() const;
		/// return name of value type
		virtual std::string get_value_type_name() const;
		/// return reference to time series implementation for access to sample cache
		virtual const time_series_base& series() const = 0;
		/// return reference to time series base interface
		virtual time_series_base& series() = 0;
		/// return whether time series is out of date
		bool is_outofdate() const;
		/// append all cached samples converted to float at end of given samples container
		//void append_cached_samples(std::vector<vec2>& samples, unsigned component_index = 0) const;
		/// extract new sample and return whether new value was provided in given values
		virtual bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx) = 0;
	};

	struct CGV_API float_time_series : public streaming_time_series, public time_series<float, float, double>
	{
		uint16_t index;
		float_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
		streaming_time_series* construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const;
	};
	struct CGV_API resampled_float_time_series : public float_time_series
	{
		streaming_time_series* sampling_ts_ptr;
		float new_value;
		bool is_resample() const;
		resampled_float_time_series(const std::string& name, streaming_time_series* _sampling_ts_ptr);
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx);
	};

	struct CGV_API int_time_series : public streaming_time_series, public time_series<float, int32_t, int64_t>
	{
		uint16_t index;
		int_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
		streaming_time_series* construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const;
	};
	struct CGV_API resampled_int_time_series : public int_time_series
	{
		streaming_time_series* sampling_ts_ptr;
		int32_t new_value;
		bool is_resample() const;
		resampled_int_time_series(const std::string& name, streaming_time_series* _sampling_ts_ptr);
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx);
	};

	struct CGV_API uint_time_series : public streaming_time_series, public time_series<float, uint32_t, uint64_t>
	{
		uint16_t index;
		uint_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
		streaming_time_series* construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const;
	};
	struct CGV_API resampled_uint_time_series : public uint_time_series
	{
		streaming_time_series* sampling_ts_ptr;
		uint32_t new_value;
		bool is_resample() const;
		resampled_uint_time_series(const std::string& name, streaming_time_series* _sampling_ts_ptr);
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx);
	};

	struct CGV_API bool_time_series : public streaming_time_series, public time_series<float, uint8_t, bool, false>
	{
		uint16_t index;
		bool_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
		streaming_time_series* construct_resampled_time_series(const std::string& name, streaming_time_series* sampling_ts) const;
	};
	struct CGV_API resampled_bool_time_series : public bool_time_series
	{
		streaming_time_series* sampling_ts_ptr;
		bool new_value;
		bool is_resample() const;
		resampled_bool_time_series(const std::string& name, streaming_time_series* _sampling_ts_ptr);
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx);
	};

	template <uint32_t N>
	struct fvec_time_series : public streaming_time_series, public time_series<float, cgv::math::fvec<float, N>, cgv::math::fvec<double, N>>
	{
		uint16_t indices[N];
		cgv::type::info::TypeId source_types[N];
		fvec_time_series(uint16_t i0, uint16_t i1, 
			cgv::type::info::TypeId t0 = cgv::type::info::TI_FLT64, cgv::type::info::TypeId t1 = cgv::type::info::TI_FLT64) : streaming_time_series(cgv::type::info::TI_ARRAY)
		{
			indices[0] = i0;
			indices[1] = i1;
			source_types[0] = t0;
			source_types[1] = t1;
			this->set_value_offset(cgv::math::fvec<double, N>(0.0));
		}
		fvec_time_series(uint16_t i0, uint16_t i1, uint16_t i2,
			cgv::type::info::TypeId t0 = cgv::type::info::TI_FLT64, cgv::type::info::TypeId t1 = cgv::type::info::TI_FLT64, 
			cgv::type::info::TypeId t2 = cgv::type::info::TI_FLT64) : streaming_time_series(cgv::type::info::TI_ARRAY)
		{
			indices[0] = i0;
			indices[1] = i1;
			indices[2] = i2;
			source_types[0] = t0;
			source_types[1] = t1;
			source_types[2] = t2;
			this->set_value_offset(cgv::math::fvec<double, N>(0.0));
		}
		fvec_time_series(uint16_t i0, uint16_t i1, uint16_t i2, uint16_t i3,
			cgv::type::info::TypeId t0 = cgv::type::info::TI_FLT64, cgv::type::info::TypeId t1 = cgv::type::info::TI_FLT64, 
			cgv::type::info::TypeId t2 = cgv::type::info::TI_FLT64, cgv::type::info::TypeId t3 = cgv::type::info::TI_FLT64) : streaming_time_series(cgv::type::info::TI_ARRAY)
		{
			indices[0] = i0;
			indices[1] = i1;
			indices[2] = i2;
			indices[3] = i3;
			source_types[0] = t0;
			source_types[1] = t1;
			source_types[2] = t2;
			source_types[3] = t3;
			this->set_value_offset(cgv::math::fvec<double, N>(0.0));
		}
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(indices, indices + N); }
		/// return name of value type
		std::string get_value_type_name() const
		{
			switch (N) {
			case 2: return "vec2";
			case 3: return "vec3";
			case 4: return "vec4";
			}
			return "vecn";
		}
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx)
		{
			cgv::math::fvec<double, N> pos;
			for (int ci = 0; ci < N; ++ci) {
				uint16_t vi = val_idx_from_ts_idx[indices[ci]];
				if (vi == uint16_t(-1))
					return false;
				switch (source_types[ci]) {
				case cgv::type::info::TI_FLT64: pos[ci] = reinterpret_cast<const double&>(values[vi].value[0]); break;
				case cgv::type::info::TI_UINT64: pos[ci] = (double)reinterpret_cast<const uint64_t&>(values[vi].value[0]); break;
				case cgv::type::info::TI_INT64: pos[ci] = (double)reinterpret_cast<const int64_t&>(values[vi].value[0]); break;
				}
			}
			switch (transform) {
			case TT_GEODETIC2ENU_WGS84: {
				cgv::dvec3 pos_geod = cgv::dvec3(pos[0], pos[1], N == 2 ? 0.0 : pos[2]);
				cgv::dvec3 pos_ENU = cgv::math::ENU_from_geodetic(pos_geod, transform_origin);
				pos = cgv::math::fvec<double, N>(N, &pos_ENU[0]);
			}
				break;
			}
			lock.lock();
			this->append_sample(timestamp, pos);
			outofdate = true;
			lock.unlock();
			return true;
		}
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
	};
	struct CGV_API quat_time_series : public streaming_time_series, public time_series<float, cgv::math::quaternion<float>, cgv::math::quaternion<double>, false>
	{
		uint16_t indices[4];
		quat_time_series(uint16_t i0, uint16_t i1, uint16_t i2, uint16_t i3);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(indices, indices+4); }
		/// return name of value type
		std::string get_value_type_name() const;
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
	};
}

#include <cgv/config/lib_end.h>
