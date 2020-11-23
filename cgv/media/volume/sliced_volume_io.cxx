#if _MSC_VER > 1400
#pragma warning(disable:4996)
#endif

#include "sliced_volume_io.h"
#include <cgv/utils/scan.h>
#include <cgv/utils/file.h>

namespace cgv {
	namespace media {
		namespace volume {

			bool read_from_sliced_volume(const std::string& file_name, volume& V)
			{
				ooc_sliced_volume svol;
				if (!svol.open_read(file_name)) {
					std::cerr << "could not open " << file_name << std::endl;
					return false;
				}

				V.get_format().set_component_format(svol.get_format().get_component_format());
				V.resize(svol.get_dimensions());
				V.ref_extent() = svol.get_extent();

				std::vector<std::string> slice_file_names;
				std::string file_path;
				bool is_indexed = svol.file_name_pattern.find_first_of('*') == std::string::npos;
				if (!is_indexed) {
					file_path = cgv::utils::file::get_path(svol.file_name_pattern);
					if (!file_path.empty() && file_path.back() != '/')
						file_path += '/';
					void* handle = cgv::utils::file::find_first(svol.file_name_pattern);
					while (handle != 0) {
						if (!cgv::utils::file::find_directory(handle))
							slice_file_names.push_back(cgv::utils::file::find_name(handle));
						handle = cgv::utils::file::find_next(handle);
					}
				}

				std::size_t slize_size = V.get_voxel_size() * V.get_format().get_width() * V.get_format().get_height();
				cgv::type::uint8_type* dst_ptr = V.get_data_ptr<cgv::type::uint8_type>();
				for (int i = 0; i < (int)svol.get_nr_slices(); ++i) {
					if (is_indexed) {
						if (!svol.read_slice(i)) {
							std::cerr << "could not read slice " << i << " with filename \"" << svol.get_slice_file_name(i) << "\"." << std::endl;
							return false;
						}
					}
					else {
						int j = i + svol.offset;
						if ((unsigned)j > slice_file_names.size()) {
							std::cerr << "could not read slice " << i << " from with filename with index " << j << " as only " << slice_file_names.size() << " match pattern." << std::endl;
							return false;
						}
						if (!svol.read_slice(i, file_path + slice_file_names[j])) {
							std::cerr << "could not read slice " << i << " from file \"" << slice_file_names[j] << "\"." << std::endl;
							return false;
						}
					}
					const cgv::type::uint8_type* src_ptr = svol.get_data_ptr<cgv::type::uint8_type>();
					std::copy(src_ptr, src_ptr + slize_size, dst_ptr);
					dst_ptr += slize_size;
				}
				svol.close();
				return true;
			}

			bool write_as_sliced_volume(const std::string& file_name, const std::string& _file_name_pattern, const volume& V)
			{
				ooc_sliced_volume svol;
				svol.get_format().set_component_format(V.get_format().get_component_format());
				svol.resize(V.get_dimensions());
				svol.ref_extent() = V.get_extent();
				std::string file_name_pattern = _file_name_pattern;
				cgv::utils::replace(file_name_pattern, "$h", cgv::utils::file::get_file_name(file_name));
				std::string path = cgv::utils::file::get_path(file_name);
				if (path.size() > 0)
					file_name_pattern = path + "/" + file_name_pattern;
				if (!svol.open_write(file_name, file_name_pattern, V.get_format().get_depth())) {
					std::cerr << "could not open " << file_name << " for write." << std::endl;
					return false;
				}

				std::size_t slize_size = V.get_voxel_size() * V.get_format().get_width() * V.get_format().get_height();
				const cgv::type::uint8_type* src_ptr = V.get_data_ptr<cgv::type::uint8_type>();
				cgv::type::uint8_type* dst_ptr = svol.get_data_ptr<cgv::type::uint8_type>();
				for (int i = 0; i < (int)svol.get_nr_slices(); ++i) {
					std::copy(src_ptr, src_ptr + slize_size, dst_ptr);
					if (!svol.write_slice(i))
						return false;
					src_ptr += slize_size;
				}
				svol.close();
				return true;
			}

		}
	}
}