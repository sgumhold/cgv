#pragma once

#include "hit_info.h"
#include "focusable.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {
		struct proximity_info : public hit_info
		{
			vec3 query_point;
			float closest_distance = std::numeric_limits<float>::max();
		};

		struct CGV_API proximity_dispatch_info : public hit_dispatch_info, public proximity_info
		{
			void copy(const dispatch_info& dis_info);
			const hit_info* get_hit_info() const;
			dispatch_info* clone() const;
		};

		/// interface for object that provides a find closest point method
		class CGV_API grabbable
		{
		public:
			/// empty constructor
			grabbable();
			/// implement closest point algorithm and return whether this was found (failure only for invisible objects) and in this case set \c prj_point to closest point and \c prj_normal to corresponding surface normal
			virtual bool compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx) = 0;
			/// check whether event changes the default grabbing state
			bool is_grab_change(const cgv::gui::event& e, bool& grabbed) const;
			/// check whether event is a grabbing event
			bool is_grabbing(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info) const;
			/// in case is_grabbing returned true, this provides access to proximity info of dispatch info
			const proximity_info& get_proximity_info(const dispatch_info& dis_info) const;
		};
	}
}
#include <cgv/config/lib_end.h>