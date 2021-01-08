#pragma once

#include <LOLA_vis.h>
#include "time_series.h"

#include <cgv/render/render_types.h>
#include <cgv/os/mutex.h>
#include <cgv/type/info/type_id.h>

#include "lib_begin.h"

namespace stream_vis {
	/// interface for all time series used in the streaming visualization library
	class CGV_API streaming_time_series : public cgv::render::render_types
	{
	protected:
		mutable cgv::os::mutex lock;
		mutable bool outofdate;
		/// store id of value type
		cgv::type::info::TypeId type_id;
	public:
		/// store name of streaming time series
		std::string name;
		/// construct empty streaming time series
		streaming_time_series(cgv::type::info::TypeId _type_id = cgv::type::info::TI_UNDEF);
		/// return list of unique indices to defining inputs/outputs
		virtual std::vector<uint16_t> get_io_indices() const = 0;
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
		virtual bool extract_from_values(uint16_t num_values, rtlola_viz::LV_IndexedValue* values, double timestamp) = 0;
	};

	struct CGV_API float_time_series : public streaming_time_series, public time_series<float, float, double>
	{
		uint16_t index;
		float_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, rtlola_viz::LV_IndexedValue* values, double timestamp);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
	};

	struct CGV_API int_time_series : public streaming_time_series, public time_series<float, int32_t, int64_t>
	{
		uint16_t index;
		int_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, rtlola_viz::LV_IndexedValue* values, double timestamp);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
	};

	struct CGV_API uint_time_series : public streaming_time_series, public time_series<float, uint32_t, uint64_t>
	{
		uint16_t index;
		uint_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, rtlola_viz::LV_IndexedValue* values, double timestamp);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
	};

	struct CGV_API bool_time_series : public streaming_time_series, public time_series<float, uint8_t, bool, false>
	{
		uint16_t index;
		bool_time_series(uint16_t i);
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(1, index); }
		bool extract_from_values(uint16_t num_values, rtlola_viz::LV_IndexedValue* values, double timestamp);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
	};

	template <uint32_t N>
	struct fvec_time_series : public streaming_time_series, public time_series<float, cgv::math::fvec<float, N>, cgv::math::fvec<double, N>>
	{
		uint16_t indices[N];
		fvec_time_series(uint16_t i0, uint16_t i1) : streaming_time_series(cgv::type::info::TI_ARRAY)
		{
			indices[0] = i0;
			indices[1] = i1;
		}
		fvec_time_series(uint16_t i0, uint16_t i1, uint16_t i2) : streaming_time_series(cgv::type::info::TI_ARRAY)
		{
			indices[0] = i0;
			indices[1] = i1;
			indices[2] = i2;
		}
		fvec_time_series(uint16_t i0, uint16_t i1, uint16_t i2, uint16_t i3) : streaming_time_series(cgv::type::info::TI_ARRAY)
		{
			indices[0] = i0;
			indices[1] = i1;
			indices[2] = i2;
			indices[3] = i3;
		}
		std::vector<uint16_t> get_io_indices() const { return std::vector<uint16_t>(indices, indices+N); }
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
		bool extract_from_values(uint16_t num_values, rtlola_viz::LV_IndexedValue* values, double timestamp)
		{
			cgv::math::fvec<double, N> pos;
			int cnt = 0;
			for (uint16_t i = 0; i < num_values; ++i) {
				for (int c = 0; c < 3; ++c)
					if (values[i].index == indices[c]) {
						pos[c] = reinterpret_cast<const double&>(values[i].value[0]);
						++cnt;
					}
			}
			if (cnt != N)
				return false;
			append_sample(timestamp, pos);
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
		bool extract_from_values(uint16_t num_values, rtlola_viz::LV_IndexedValue* values, double timestamp);
		const time_series_base& series() const { return *this; }
		time_series_base& series() { return *this; }
	};
}

#include <cgv/config/lib_end.h>