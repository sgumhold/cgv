#include "volume.h"
#include <fstream>
#include <stdio.h>
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/tokenizer.h>

#pragma warning(disable:4996)

namespace cgv {
	namespace media {
		namespace volume {

			volume::volume() : extent(1, 1, 1), df("uint8[L]")
			{
			}

			/// copy construct volume by allocating a copy of the volume data
			volume::volume(const volume& V) : extent(V.extent), df(V.df)
			{
				new (&dv) cgv::data::data_view(&df);
				std::copy(V.dv.get_ptr<cgv::type::uint8_type>(), V.dv.get_ptr<cgv::type::uint8_type>() + df.get_nr_bytes(), dv.get_ptr<cgv::type::uint8_type>());
			}

			/// return the dimensions or (0,0,0) if not available
			volume::dimension_type volume::get_dimensions() const
			{
				return
					df.get_nr_dimensions() > 2 ?
					dimension_type(df.get_width(), df.get_height(), df.get_depth()) :
					dimension_type(0, 0, 0);
			}

			/// return the bounding box in volume coordinates, which is computed from spacing and dimensions and centered around the origin
			volume::box_type volume::get_box() const
			{
				return box_type(coord_type(-0.5) * extent, coord_type(0.5) * extent);
			}

			/// return the voxel spacing, computed by extent/dimensions
			volume::extent_type volume::get_spacing() const
			{
				volume::dimension_type dims = get_dimensions();
				return extent_type(extent(0) / dims(0), extent(1) / dims(1), extent(2) / dims(2));
			}

			std::size_t volume::get_nr_voxels() const
			{
				dimension_type dimensions = get_dimensions();
				std::size_t n = dimensions(0);
				n *= dimensions(1);
				n *= dimensions(2);
				return n;
			}

			void volume::resize(const dimension_type& S)
			{
				df.set_width(S(0));
				df.set_height(S(1));
				df.set_depth(S(2));
				dv = cgv::data::data_view(&df);
			}
		}
	}
}