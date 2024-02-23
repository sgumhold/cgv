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
					dimension_type(int(df.get_width()), int(df.get_height()), int(df.get_depth())) :
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

			/// add new component on the end (if result has <=4 components)
			bool volume::add_new_component(data::data_view& component_dv)
			{
				const data::data_format* existing_format_ptr = dv.get_format();
				const data::data_format* component_format_ptr = component_dv.get_format();
				cgv::type::info::TypeId existing_component_type = existing_format_ptr->get_component_type();

				if(existing_component_type != component_format_ptr->get_component_type())
				{
					std::cerr << "Can not add different component types." << std::endl;
					return false;
				}

				unsigned int nr_existing_components = existing_format_ptr->get_component_format().get_nr_components();
				unsigned int nr_component_components = component_format_ptr->get_component_format().get_nr_components();

				if(nr_existing_components < 1)
				{
					std::cerr << "Existing data view has no component." << std::endl;
					return false;
				}
				if(nr_existing_components > 3)
				{
					std::cerr << "Existing data view already more than 3 components (" << std::to_string(nr_existing_components) << "). Cant add to that." << std::endl;
					return false;
				}

				if(nr_component_components != 1)
				{
					std::cerr << "New component may only have one component, but has" << std::to_string(nr_component_components) << "." << std::endl;
					return false;
				}

				unsigned int component_number_of_dimensions = component_format_ptr->get_nr_dimensions();
				unsigned int existing_number_of_dimensions = existing_format_ptr->get_nr_dimensions();

				if(component_number_of_dimensions < 3 || component_number_of_dimensions > 4 || component_number_of_dimensions != existing_number_of_dimensions)
				{
					std::cerr << "Both data views need to have 3 or 4 dimensions (and both the same number). They have " << std::to_string(existing_number_of_dimensions) << " (existing) and" << std::to_string(component_number_of_dimensions) << " (new component)." << std::endl;
					return false;
				}
				
				if(component_format_ptr->get_height() != existing_format_ptr->get_height() || component_format_ptr->get_width() != existing_format_ptr->get_width() || component_format_ptr->get_depth() != existing_format_ptr->get_depth())
				{
					std::cerr << "The dimensions do not match: existing (" << std::to_string(existing_format_ptr->get_width()) << ", " << std::to_string(existing_format_ptr->get_height()) << ", " << std::to_string(existing_format_ptr->get_depth()) << ") vs. new component ("<< std::to_string(component_format_ptr->get_width()) << ", "<< std::to_string(component_format_ptr->get_height()) << ", " << std::to_string(component_format_ptr->get_depth()) <<")." << std::endl;
					return false;
				}

				data::ComponentFormat result_component_format = data::CF_RG;
				switch(nr_existing_components+nr_component_components) {
				case 3: result_component_format = data::CF_RGB;  break;
				case 4: result_component_format = data::CF_RGBA; break;
				default: result_component_format = data::CF_RG; break;
				}

				size_t w = existing_format_ptr->get_width();
				size_t h = existing_format_ptr->get_height();
				size_t d = existing_format_ptr->get_depth();
				size_t t = 1;
				if(component_number_of_dimensions == 4)
					t = existing_format_ptr->get_nr_time_steps();

				unsigned mask_t = 0;
				if(component_number_of_dimensions == 4)
					mask_t = 1;


				data::data_format* result_data_format_ptr =  new data::data_format(w, h, d, existing_component_type, result_component_format);

				cgv::data::data_view combined_dv = data::data_view(result_data_format_ptr);

				//component_type
				unsigned component_size = cgv::type::info::get_type_size(existing_component_type);

				for (unsigned x = 0; x < w; ++x) {
					for (unsigned y = 0; y < h; ++y) {
						for (unsigned z = 0; z < d; ++z) {
							for (unsigned i = 0; i < t; ++i) {

								unsigned char* existing_ptr = dv.get_ptr<unsigned char>(x, y, z , mask_t* i);
								unsigned char* component_ptr = component_dv.get_ptr<unsigned char>(x, y, z, mask_t* i);
								unsigned char* result_ptr = combined_dv.get_ptr<unsigned char>(x, y, z, mask_t* i);

								const unsigned copy_size = component_size * nr_existing_components;
								memcpy(result_ptr, existing_ptr, copy_size);
								memcpy(result_ptr, component_ptr, component_size);

							}
						}
					}
				}

				dv = combined_dv;
				return true;
				
			}

			bool volume::replace_component(unsigned i, data::data_view& component_dv)
			{
				const data::data_format* existing_format_ptr = dv.get_format();
				const data::data_format* component_format_ptr = component_dv.get_format();
				if(existing_format_ptr->get_component_type() != component_format_ptr->get_component_type())
				{
					std::cerr << "Can not replace with different component types." << std::endl;
					return false;
				}

				unsigned int nr_components = component_format_ptr->get_component_format().get_nr_components();
				if(nr_components != 1)
				{
					std::cerr << "Replacement component may only have one component, but has" << std::to_string(nr_components) << "." << std::endl;
					return false;
				}

				unsigned int component_number_of_dimensions = component_format_ptr->get_nr_dimensions();

				if(component_number_of_dimensions < 3 || component_number_of_dimensions > 4)
				{
					std::cerr << "The replacement data view needs to have 3 or 4 dimensions, but has " << std::to_string(component_number_of_dimensions) << "." << std::endl;
					return false;
				}
				if(component_number_of_dimensions != dv.get_format()->get_nr_dimensions())
				{
					std::cerr << "The replacement data view needs to have the same number of dimensions, but has " << std::to_string(component_number_of_dimensions) << " (existing has "<< std::to_string(dv.get_format()->get_nr_dimensions()) <<")." << std::endl;
					return false;
				}

				if(component_format_ptr->get_height() != existing_format_ptr->get_height() || component_format_ptr->get_width() != existing_format_ptr->get_width() || component_format_ptr->get_depth() != existing_format_ptr->get_depth())
				{
					std::cerr << "The dimensions do not match: existing (" << std::to_string(existing_format_ptr->get_width()) << ", " << std::to_string(existing_format_ptr->get_height()) << ", " << std::to_string(existing_format_ptr->get_depth()) << ") vs. new component ("<< std::to_string(component_format_ptr->get_width()) << ", "<< std::to_string(component_format_ptr->get_height()) << ", " << std::to_string(component_format_ptr->get_depth()) <<")." << std::endl;
					return false;
				}

				size_t w = existing_format_ptr->get_width();
				size_t h = existing_format_ptr->get_height();
				size_t d = existing_format_ptr->get_depth();
				size_t t = 1;
				if(component_number_of_dimensions == 4)
					t = existing_format_ptr->get_nr_time_steps();


				unsigned mask_t = 0;
				if(component_number_of_dimensions == 4)
					mask_t = 1;

				unsigned component_size = cgv::type::info::get_type_size(existing_format_ptr->get_component_type());

				for (unsigned i = 0; i < t; ++i) {
					for (unsigned z = 0; z < d; ++z) {
						for (unsigned y = 0; y < h; ++y) {
							for (unsigned x = 0; x < w; ++x) {

								unsigned char* existing_ptr = dv.get_ptr<unsigned char>(x, y, z, mask_t * i);
								unsigned char* component_ptr = component_dv.get_ptr<unsigned char>(x, y, z, mask_t * i);

								memcpy(existing_ptr, component_ptr, component_size);

							}
						}
					}
				}
				
				return false;
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