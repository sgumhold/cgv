#pragma once

#include <algorithm>
#include "image_reader.h"
#include "image_writer.h"

#include "lib_begin.h"

namespace cgv {
	namespace media {
		namespace image {

			class image : public cgv::data::data_format
			{
			protected:
				cgv::data::data_view dv;
			public:
				image() : dv() {}
				template <typename T> T* get_ptr() { return dv.get_ptr<T>(); }
				template <typename T> const T* get_ptr() const { return dv.get_ptr<T>(); }
				bool read(const std::string& file_name)
				{
					cgv::media::image::image_reader ir(*this);
					if (ir.open(file_name)) {
						if (ir.read_image(dv)) {
							return true;
						}
					}
					return false;
				}
				void hflip()
				{
					// swap order of rows
					size_t row_size = get_entry_size() * get_width();
					unsigned h = get_height();
					for (unsigned y = 0; y < (h - 1) / 2; ++y) {
						std::swap_ranges(dv.get_ptr<char>() + y*row_size, dv.get_ptr<char>() + (y + 1)*row_size, dv.get_ptr<char>() + (h - y - 1)*row_size);
					}
				}
				/// combine a rectangular image region into a single pixel
				void combine(const cgv::type::uint8_type* src_ptr, unsigned n_x, unsigned entry_size, unsigned n_y, unsigned row_size, cgv::type::uint8_type* dst_ptr)
				{
					double c[4] = { 0,0,0,0 };
					unsigned n_c = get_nr_components();
					for (unsigned y = 0; y < n_y; ++y) {
						for (unsigned x = 0; x < n_x; ++x) {
							for (unsigned ci = 0; ci < n_c; ++ci)
								c[ci] += get<double>(ci, src_ptr + entry_size*x + row_size*y);
						}
					}
					double scale = 1.0 / (n_x*n_y);
					for (unsigned ci = 0; ci < n_c; ++ci)
						set<double>(ci, dst_ptr, scale*c[ci]);
				}
				/// construct a downsampled version of given image; dimensions of I must be dividable by downsampling factors
				void downsample(unsigned x_downsample_factor, unsigned y_downsample_factor, const image& I)
				{
					// extract downsampled image dimensions
					unsigned w = I.get_width() / x_downsample_factor;
					unsigned h = I.get_height() / y_downsample_factor;

					// check assumption of dividable image dimension
					assert(x_downsample_factor*w == I.get_width());
					assert(y_downsample_factor*h == I.get_height());

					// copy format with and set downsampled dimensions
					*static_cast<cgv::data::data_format*>(this) = I;
					set_width(w);
					set_height(h);

					// allocate data
					new(&dv) data_view(this);

					// prepare iteration
					unsigned n = w*h;
					unsigned entry_size = get_entry_size();
					cgv::type::uint8_type* dst_ptr = get_ptr<cgv::type::uint8_type>();
					unsigned src_row_size = entry_size*I.get_width();
					const cgv::type::uint8_type* src_ptr = I.get_ptr<cgv::type::uint8_type>();

					for (unsigned y = 0; y < h; ++y) {
						for (unsigned x = 0; x < w; ++x) {
							combine(
								src_ptr + src_row_size*y_downsample_factor*y+entry_size*x_downsample_factor*x, 
								x_downsample_factor, entry_size, y_downsample_factor, src_row_size, 
								dst_ptr);
							dst_ptr += entry_size;
						}
					}
				}

			};
		}
	}
}

#include <cgv/config/lib_end.h>
