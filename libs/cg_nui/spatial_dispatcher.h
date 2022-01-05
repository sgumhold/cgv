#pragma once 

#include <cgv/render/drawable.h>
#include <cgv/gui/event.h>
#include <libs/vr/vr_kit.h>
#include "grabable.h"
#include "pointable.h"
#include "transformed.h"
#include "dispatcher.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class CGV_API spatial_dispatcher : public dispatcher, public transformed
		{
			struct geometric_info
			{
				bool check_proximity = false;
				proximity_info prox_info;
				focus_info prox_foc_info;

				bool check_intersection = false;
				intersection_info inter_info;
				focus_info inter_foc_info;
			};
		protected:
			struct controller_info
			{
				bool pointing;
				bool grabbing;
				rgb color;
			};
			controller_info ctrl_infos[2] = { {true, true, {0.6f,0.3f,0.3f} },{true, true, {0.3f,0.3f,0.6f} } };
			/// when deciding between proximity and pointing, a factor multiplied to distance to first ray intersection when compared to closest point distance (defaults to 2)
			float intersection_bias = 2.0f;
			/// distance threshold up to which grabbing is possible (defaults to 5cm)
			float max_grabbing_distance = 0.05f;
			/// distance threshold up to which pointing is possible (defaults to 5 meters)
			float max_pointing_distance = 5.0f;
			///
			float min_pointing_distance = 0.2f;
			/// recursively traverse hierarchy and compute proximity & intersections to find to be grabbed or pointed to object
			void update_geometric_info_recursive(cgv::base::base_ptr root_ptr, cgv::base::base_ptr object_ptr, geometric_info& gi, bool recurse = true) const;
			/// provide implementation of spatial dispatching
			bool dispatch_spatial(const focus_attachment& foc_att, const cgv::gui::event& e, const hid_identifier& hid_id, refocus_info& rfi, bool* handle_called_ptr);
		public:
			spatial_dispatcher();
		};
	}
}

#include <cgv/config/lib_end.h>