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
				template <typename T>
				T* get_ptr() { return dv.get_ptr<T>(); }
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

			};
		}
	}
}

#include <cgv/config/lib_end.h>
