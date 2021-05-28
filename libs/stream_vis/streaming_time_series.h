#pragma once

#include "time_series.h"

#include <cgv/render/render_types.h>
#include <cgv/os/mutex.h>
#include <cgv/type/info/type_id.h>

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
	enum AABBMode
	{
		AM_NONE,
		AM_BRUTE_FORCE,
		AM_BLOCKED_8,
		AM_BLOCKED_16
	};
	/// interface for all time series used in the streaming visualization library
	class CGV_API streaming_time_series : public cgv::render::render_types
	{
	protected:
		mutable cgv::os::mutex lock;
		mutable bool outofdate;
		/// store id of value type
		cgv::type::info::TypeId type_id;
		/// store name of streaming time series
		std::string name;
	public:
		/// store index of first visible sample
		size_t first_visible_sample_index;
		/// default color
		rgb default_color;
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
		/// return name
		const std::string& get_name() const;
		/// 
		void set_name(const std::string& new_name);
		/// construct empty streaming time series
		streaming_time_series(cgv::type::info::TypeId _type_id = cgv::type::info::TI_UNDEF);
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
		virtual bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp) = 0;
	};

	struct CGV_API float_time_series : public streaming_time_series, public time_series<float, float, double>
	{
		uint16_t index;
		float_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
	};

	struct CGV_API int_time_series : public streaming_time_series, public time_series<float, int32_t, int64_t>
	{
		uint16_t index;
		int_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
	};

	struct CGV_API uint_time_series : public streaming_time_series, public time_series<float, uint32_t, uint64_t>
	{
		uint16_t index;
		uint_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
	};

	struct CGV_API bool_time_series : public streaming_time_series, public time_series<float, uint8_t, bool, false>
	{
		uint16_t index;
		bool_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
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
			set_value_offset(cgv::math::fvec<double, N>(0.0));
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
			set_value_offset(cgv::math::fvec<double, N>(0.0));
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
			set_value_offset(cgv::math::fvec<double, N>(0.0));
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
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp)
		{
			cgv::math::fvec<double, N> pos;
			int cnt = 0;
			for (uint16_t i = 0; i < num_values; ++i) {
				for (int c = 0; c < N; ++c)
					if (values[i].index == indices[c]) {
						switch (source_types[c]) {
						case cgv::type::info::TI_FLT64: pos[c] = reinterpret_cast<const double&>(values[i].value[0]); break;
						case cgv::type::info::TI_UINT64: pos[c] = (double)reinterpret_cast<const uint64_t&>(values[i].value[0]); break;
						case cgv::type::info::TI_INT64: pos[c] = (double)reinterpret_cast<const int64_t&>(values[i].value[0]); break;
						}
						++cnt;
					}
			}
			if (cnt != N)
				return false;
			this->append_sample(timestamp, pos);
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
		bool extract_from_values(uint16_t num_values, indexed_value* values, double timestamp);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
	};
}

#include <cgv/config/lib_end.h>
