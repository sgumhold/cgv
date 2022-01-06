#pragma once

#include <cgv/render/drawable.h>
#include <cgv_gl/renderer.h>
#include "contact_info.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {
		/// class that manages bounding box recomputations
		class bounding_box_cache : public cgv::render::render_types
		{
		protected:
			///
			mutable box3 box;
			/// 
			mutable bool box_outofdate;
		public:
			bounding_box_cache();
			bool check_box_update(const box3& old_box, const box3& new_box) const;
			void set_box_outofdate();
			virtual uint32_t get_nr_primitives() const = 0;
			virtual box3 get_bounding_box(uint32_t i) const = 0;
			const box3& compute_bounding_box() const;
		};

	}
}

#include <cgv/config/lib_end.h>