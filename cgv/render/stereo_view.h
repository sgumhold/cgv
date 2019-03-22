#pragma once

#include "clipped_view.h"

#include "lib_begin.h"

namespace cgv {
	namespace render {

		/** extends the clipped view with information on eye distance and parallax zero information. */
		class CGV_API stereo_view : public clipped_view
		{
		protected:
			/// distance between eyes
			double eye_distance;
			/// scale of parallax zero depth with respect to eye focus distance
			double parallax_zero_scale;
		public:
			/// construct with standard values
			stereo_view();
			///
			virtual void set_default_values();
			/// query distance between eyes
			double get_eye_distance() const;
			/// set distance between eyes
			virtual void set_eye_distance(double e);
			/// query scale of parallax zero depth with respect to eye focus distance
			double get_parallax_zero_scale() const;
			/// query parallax zero depth
			double get_parallax_zero_depth() const;
			/// set parallax zero scale
			virtual void set_parallax_zero_scale(double pzs);
		};
	}
}

#include <cgv/config/lib_end.h>