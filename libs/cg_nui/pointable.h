#pragma once

#include "focusable.h"
#include "hit_info.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		struct intersection_info : public hit_info
		{
			vec3  ray_origin;
			vec3  ray_direction;
			float ray_param = std::numeric_limits<float>::max();
		};

		struct intersection_dispatch_info : public hit_dispatch_info, public intersection_info
		{
			void copy(const dispatch_info& dis_info);
			const hit_info* get_hit_info() const;
			dispatch_info* clone() const;
		};

		/// interface for object that provides a ray intersection method
		class CGV_API pointable
		{
		public:
			/// empty constructor
			pointable();
			/// implement ray object intersection and return whether intersection was found and in this case set \c hit_param to ray parameter and optionally \c hit_normal to surface normal of intersection
			virtual bool compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx) = 0;
			/// check whether event changes the default trigger for pointing
			bool is_trigger_change(const cgv::gui::event& e, bool& triggered) const;
			/// check whether event is a pointing event
			bool is_pointing(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info) const;
			/// in case is_pointing returned true, this provides access to intersection info of dispatch info
			const intersection_info& get_intersection_info(const dispatch_info& dis_info) const;
		};
	}
}
#include <cgv/config/lib_end.h>