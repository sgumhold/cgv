#pragma once

#include <cgv/math/fmat.h>
#include <cgv/utils/token.h>
#include "volume.h"

#include "../lib_begin.h"

namespace cgv {
	namespace media {
		namespace volume {

			struct CGV_API volume_info
			{
				volume::dimension_type dimensions;
				volume::extent_type extent;
				volume::point_type position;
				cgv::math::fmat<volume::coord_type, 3, 3> orientation;
				std::string path;

				cgv::type::info::TypeId type_id;
				cgv::data::ComponentFormat components;
				size_t get_data_size() const;
				volume_info();
				volume_info(const volume& V, const std::string& path = "");
			};

			extern CGV_API bool read_volume(const std::string& file_name, volume& V, volume_info* info_ptr = 0);

			extern CGV_API bool write_volume(const std::string& file_name, const volume& V, const std::string& options = "");

			extern CGV_API bool read_vox_header(const std::string& file_name, volume_info& info, bool(*unknown_line_callback)(const std::string& line, const std::vector<cgv::utils::token>&, volume_info& info) = 0);

			extern CGV_API bool read_header(const std::string& file_name, volume_info& info, bool(*unknown_line_callback)(const std::string& line, const std::vector<cgv::utils::token>&, volume_info& info) = 0);

			extern CGV_API bool write_vox_header(const std::string& file_name, const volume_info& info);

			extern CGV_API bool write_header(const std::string& file_name, const volume& V);

			extern CGV_API bool read_volume_binary(const std::string& file_name, const volume_info& info, volume& V, size_t offset = 0);

			extern CGV_API void toggle_volume_endian(volume& V);

			extern CGV_API bool write_volume_binary(const std::string& file_name, const volume& V, size_t offset = 0);


			enum FileOrganisation {
				FO_SINGLE,
				FO_SLICES,
				FO_TILES,
				FO_OCTREE
			};

			enum FileFormat {
				FF_ASCII,
				FF_BINARY,
				FF_PNG,
				FF_TIFF
			};

			struct CGV_API file_organisation_info
			{
				std::string file_name_pattern;
				int first_file_index, file_index_step, last_file_index;
				int first_channel_file_index, file_channel_index_step, last_channel_file_index;
			};

			struct CGV_API slice_file_organisation_info : public file_organisation_info
			{
			};

			struct CGV_API tile_file_organisation_info : public file_organisation_info
			{
				volume::dimension_type tile_counts;
				volume::dimension_type tile_size;
			};

			struct CGV_API octree_file_organisation_info : public file_organisation_info
			{
				std::string child_directory_names[8];
			};

			struct CGV_API voxel_file_info
			{
				/// array of voxel counts in all three dimension, i.e. 128,128,128
				volume::dimension_type voxel_counts;
				/// array with size of voxel in x-, y-, and z-direction
				volume::extent_type voxel_scaling;
				/// voxel format
				cgv::data::component_format voxel_format;
				/// 
				bool byte_order_MSB;
				/// if header size is -1, it is computed from file size minus the size needed for all elements
				int header_size;
				/// file organisation
				FileOrganisation file_organisation;
				/// depending on the file organisation either store a pointer to the file_name or to a corresponding info structure
				union {
					std::string* file_name_ptr;
					slice_file_organisation_info* slice_file_info_ptr;
					tile_file_organisation_info* tile_file_info_ptr;
					octree_file_organisation_info* octree_file_info_ptr;
				};
				///
				FileFormat file_format;
				/// 
				voxel_file_info();
				voxel_file_info(const volume::dimension_type& D, const volume::extent_type& E, const cgv::data::component_format& cf);
			};

		}
	}
}

#include <cgv/config/lib_end.h>