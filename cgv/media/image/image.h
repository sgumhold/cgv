#pragma once

#include <algorithm>
#include "image_reader.h"
#include "image_writer.h"
#include <cgv/math/functions.h>

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
				image(const image &I) : dv()
				{
					copy(I);
				}
				template <typename T> T* get_ptr() { return dv.get_ptr<T>(); }
				template <typename T> const T* get_ptr() const { return dv.get_ptr<T>(); }
				bool read(const std::string& file_name)
				{
					image_reader ir(*this);
					if (ir.open(file_name)) {
						if (ir.read_image(dv)) {
							return true;
						}
					}
					return false;
				}
				bool write(const std::string& file_name) {
					image_writer iw(file_name);
					if(iw.write_image(dv)) {
						return true;
					}
					return false;
				}
				void hflip()
				{
					// swap order of rows
					size_t row_size = get_entry_size() * get_width();
					unsigned h = (unsigned)get_height();
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
				/// constructs a copy of given image
				void copy(const image& I)
				{
					int w = (int)I.get_width();
					int h = (int)I.get_height();

					// copy format and set dimensions
					*static_cast<cgv::data::data_format*>(this) = I;
					set_width(w);
					set_height(h);

					// allocate data
					new(&dv) data_view(this);

					const cgv::type::uint8_type* src_ptr = I.get_ptr<cgv::type::uint8_type>();
					cgv::type::uint8_type* dst_ptr = get_ptr<cgv::type::uint8_type>();

					unsigned entry_size = get_entry_size();

					size_t size = entry_size * w * h;
					memcpy(dst_ptr, src_ptr, size);
				}
				/// construct a resized smaller version of given image using area averaging
				void downscale(unsigned int size_x, unsigned int size_y, const image& I)
				{
					int w = (int)I.get_width();
					int h = (int)I.get_height();
					float wf = float(w);
					float hf = float(h);
					float x_scale = float(size_x) / wf;
					float y_scale = float(size_y) / hf;
					float y_end = 0.0f;

					// copy format and set downscaled dimensions
					*static_cast<cgv::data::data_format*>(this) = I;
					set_width(size_x);
					set_height(size_y);

					// allocate data
					new(&dv) data_view(this);

					const cgv::type::uint8_type* src_ptr = I.get_ptr<cgv::type::uint8_type>();
					cgv::type::uint8_type* dst_ptr = get_ptr<cgv::type::uint8_type>();

					unsigned n_c = get_nr_components();
					unsigned entry_size = get_entry_size();

					for(unsigned j = 0; j < size_y; ++j) {
						float y_start = y_end;
						y_end = (j + 1) / y_scale;

						if(y_end >= hf) y_end = hf - 0.000001f;

						float x_end = 0.0f;

						for(unsigned i = 0; i < size_x; ++i) {
							float x_start = x_end;
							x_end = (i + 1) / x_scale;

							if(x_end >= wf) x_end = wf - 0.000001f;

							double sum = 0.0;
							double c[4] = { 0.0, 0.0, 0.0, 0.0 };

							for(int y = (int)y_start; y <= (int)y_end; ++y) {
								float y_portion = 1.0f;

								if(y == (int)y_start) y_portion -= y_start - y;
								if(y == (int)y_end) y_portion -= y + 1.0f - y_end;

								for(int x = (int)x_start; x <= (int)x_end; ++x) {
									float x_portion = 1.0f;
									if(x == (int)x_start) x_portion -= x_start - x;
									if(x == (int)x_end) x_portion -= x + 1.0f - x_end;

									double scaling = y_portion * x_portion;
									sum += scaling;

									for(unsigned ci = 0; ci < n_c; ++ci)
										c[ci] += get<double>(ci, src_ptr + entry_size * (x + w * y)) * scaling;
								}
							}
							for(unsigned ci = 0; ci < n_c; ++ci)
								set<double>(ci, dst_ptr + entry_size * (i + (unsigned)size_x*j), c[ci] / sum);
						}
					}
				}
				/// downsample image in x and y direction by given downsampling factors fx and fy
				void downsample(unsigned fx, unsigned fy, const image& I)
				{
					downscale((unsigned)I.get_width() / fx, (unsigned)I.get_height() / fy, I);
				}
				/// construct a resized larger version of given image using bilinear interpolation
				void upscale(unsigned int size_x, unsigned int size_y, const image& I)
				{
					int w = (int)I.get_width();
					int h = (int)I.get_height();
					
					// copy format and set upscaled dimensions
					*static_cast<cgv::data::data_format*>(this) = I;
					set_width(size_x);
					set_height(size_y);

					// allocate data
					new(&dv) data_view(this);

					const cgv::type::uint8_type* src_ptr = I.get_ptr<cgv::type::uint8_type>();
					cgv::type::uint8_type* dst_ptr = get_ptr<cgv::type::uint8_type>();

					unsigned n_c = get_nr_components();
					unsigned entry_size = get_entry_size();



					unsigned n = 8;
					unsigned m = 4;
					float step_x = 1.0f / float(size_x);
					float step_y = 1.0f / float(size_y);

					for(unsigned j = 0; j < size_y; ++j) {
						float ty = (float(j) + 0.5f) * step_y;
						float fiy = ty * float(h) - 0.5f;
						unsigned iy = unsigned(fiy);
						ty = cgv::math::clamp(fiy - iy, 0.0f, 1.0f);

						if(iy == h - 1) {
							iy = h - 2;
							ty = 1.0f;
						}
						
						for(unsigned i = 0; i < size_x; ++i) {
							float tx = (float(i) + 0.5f) * step_x;
							float fix = tx * float(w) - 0.5f;
							unsigned ix = unsigned(fix);
							tx = cgv::math::clamp(fix - ix, 0.0f, 1.0f);

							if(ix == w - 1) {
								ix = w - 2;
								tx = 1.0f;
							}

							unsigned idx00 = entry_size * (ix + w * iy);
							unsigned idx10 = entry_size * (ix + 1 + w * iy);
							unsigned idx01 = entry_size * (ix + w * (iy + 1));
							unsigned idx11 = entry_size * (ix + 1 + w * (iy + 1));

							double c[4] = { 0.0, 0.0, 0.0, 0.0 };

							for(unsigned ci = 0; ci < n_c; ++ci) {
								double y0 = (1.0f - tx) * get<double>(ci, src_ptr + idx00) + tx * get<double>(ci, src_ptr + idx10);
								double y1 = (1.0f - tx) * get<double>(ci, src_ptr + idx01) + tx * get<double>(ci, src_ptr + idx11);
								
								c[ci] = (1.0f - ty) * y0 + ty * y1;
							}
								
							for(unsigned ci = 0; ci < n_c; ++ci)
								set<double>(ci, dst_ptr + entry_size * (i + (unsigned)size_x*j), c[ci]);
						}
					}
				}
				/// construct a resized version of given image using the down- and upscale methods
				void resize(unsigned size_x, unsigned size_y, const image& I)
				{
					unsigned w = (int)I.get_width();
					unsigned h = (int)I.get_height();
					
					// cannot produce an image with resolution of zero in either dimension
					assert(size_x != 0);
					assert(size_y != 0);

					bool shrink_x = size_x <= w;
					bool shrink_y = size_y <= h;

					if(w == size_x && h == size_y) {
						// copy the image if the same size is requested
						copy(I);
					} else {
						// choose an optimal resize scheme depending on the requested size
						if(shrink_x && shrink_y) {
							downscale(size_x, size_y, I);
						} else if(!shrink_x && !shrink_y) {
							upscale(size_x, size_y, I);
						} else {// if(shrink_x && !shrink_y) {
							image temp;
							temp.downscale(shrink_x ? size_x : w, shrink_y ? size_y : h, I);
							upscale(size_x, size_y, temp);
						}
					}
				}
			};
		}
	}
}

#include <cgv/config/lib_end.h>
