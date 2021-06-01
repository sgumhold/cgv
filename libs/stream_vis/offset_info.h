#pragma once

#include "time_series.h"

namespace stream_vis {

	/// different initialization modes for offset
	enum OffsetInitializationMode {
		OIM_FIXED,
		OIM_FIRST
	};
	/// store information of joint offsets
	struct offset_info
	{
		/// name of offset
		std::string name;
		/// whether offset has been initialized
		bool initialized;
		/// initialization mode
		OffsetInitializationMode mode;
		/// value of offset
		double offset_value;
		/// list of time series accessors that should receive the offset
		std::vector<TimeSeriesAccessor> accessors;
		/// indices of referenced time series 
		std::vector<uint16_t> time_series_indices;
		/// names of referenced time series 
		std::vector<std::string> time_series_names;
	};
}
