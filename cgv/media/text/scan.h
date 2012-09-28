#pragma once

#include <string>

#include <cgv/utils/scan.h>

namespace cgv {
	namespace media {
		namespace text {

#define USE_OF_DEPRECATED_HEADER_cgv_media_text_scan_h___USE_cgv_utils_scan_h_INSTEAD 0
#define USE_OF_DEPRECATED_HEADER_cgv_media_text_scan_h___USE_cgv_utils_scan_h_INSTEAD 1

using cgv::utils::skip_spaces;
using cgv::utils::cutoff_spaces;
using cgv::utils::is_space;
using cgv::utils::is_url_special;
using cgv::utils::is_digit;
using cgv::utils::is_letter;
using cgv::utils::to_lower;
using cgv::utils::to_upper;
using cgv::utils::replace_special;
using cgv::utils::replace;
using cgv::utils::interpret_special;
using cgv::utils::find_name;
using cgv::utils::is_element;
using cgv::utils::get_element_index;
using cgv::utils::is_integer;
using cgv::utils::is_double;
using cgv::utils::is_year;
using cgv::utils::is_day;
using cgv::utils::is_month;
using cgv::utils::is_time;
using cgv::utils::is_date;
using cgv::utils::is_url;

		}
	}
}
