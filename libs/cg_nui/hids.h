#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <limits>
#include <iostream>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
		extern CGV_API void consider_flag(bool f, const std::string& name, std::string& res, char sep);

		enum class hid_category : uint8_t {
			keyboard,
			mouse,
			joystick,
			gamepad,
			hmd,
			controller,
			tracker
		};
		/// provide conversion of hid_category to string
		extern CGV_API std::string to_string(hid_category hc);

		enum class kit_category : uint8_t {
			none,
			keyboard_mouse,
			vr
		};
		/// provide conversion of kit_category to string
		extern CGV_API std::string to_string(kit_category kc);

		extern CGV_API bool is_part_of_kit(hid_category hid_cat);
		extern CGV_API kit_category get_kit_category(hid_category hid_cat);
		extern CGV_API std::vector<hid_category> get_hid_categories(kit_category kit_cat);
		extern CGV_API int get_min_index(hid_category hid_cat);
		extern CGV_API int get_max_index(hid_category hid_cat);

		struct CGV_API hid_selection {
			bool keyboard : 1;
			bool mouse : 1;
			bool joystick : 1;
			bool gamepad : 1;
			bool hmd : 1;
			bool controller : 1;
			bool tracker : 1;
			bool contains(hid_category hid_cat) const;
			bool overlaps(kit_category kit_cat) const;
			bool overlaps(hid_selection sel) const;
			std::vector<hid_category> get_categories() const;
			bool operator == (const hid_selection& hs) const;
		};
		/// provide conversion of hid_selection to string
		extern CGV_API std::string to_string(hid_selection hs, char sep = ',');

		/// unique identifier for a hid
		struct CGV_API hid_identifier
		{
			hid_category category;
			void*        kit_ptr = 0; // pointer that uniquely defines a hid kit
			int16_t      index = 0;
			bool operator < (const hid_identifier& di) const;
			bool operator == (const hid_identifier& di) const;
		};
		extern CGV_API std::ostream& operator << (std::ostream& os, const hid_identifier& hid_id);

		/// unique identifier for a kit
		struct CGV_API kit_identifier
		{
			kit_category category;
			void* kit_ptr = 0; // pointer that uniquely defines a hid kit
			bool operator < (const kit_identifier& di) const;
			bool operator == (const kit_identifier& di) const;
		};
		extern CGV_API std::ostream& operator << (std::ostream& os, const kit_identifier& kit_id);

		extern CGV_API int get_hid_unique_index(const hid_identifier& hid_id);
	}
}

#include <cgv/config/lib_end.h>