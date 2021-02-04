#if _MSC_VER > 1400
#pragma warning(disable:4996)
#endif

#include "sliced_volume_io.h"
#include <cgv/utils/scan.h>
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>
#include <cgv/media/video/video_reader.h>
#include <cgv/media/image/image_reader.h>

namespace cgv {
	namespace media {
		namespace volume {

			enum SourceType {
				ST_FILTER,
				ST_INDEX,
				ST_AVI
			};

			bool read_from_sliced_volume(const std::string& file_name, volume& V)
			{
				ooc_sliced_volume svol;
				if (!svol.open_read(file_name)) {
					std::cerr << "could not open " << file_name << std::endl;
					return false;
				}
				volume::dimension_type dims = svol.get_dimensions();

				std::vector<std::string> slice_file_names;
				std::string file_path;
				SourceType st = ST_AVI;
				if (svol.file_name_pattern.find_first_of('*') != std::string::npos)
					st = ST_FILTER;
				else if (svol.file_name_pattern.find_first_of('$') != std::string::npos)
					st = ST_INDEX;
				if (st == ST_FILTER) {
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
				cgv::data::data_format df;
				cgv::media::video::video_reader* vr_ptr = 0;
				cgv::data::data_view* dv_ptr = 0;
				if (st == ST_AVI) {
					svol.set_component_type(cgv::type::info::TI_UINT8);
					svol.set_component_format(cgv::data::CF_RGB);
					vr_ptr = new cgv::media::video::video_reader(df);
					if (!vr_ptr->open(svol.file_name_pattern)) {
						std::cerr << "ERROR: could not open avi file <" << svol.file_name_pattern << ">" << std::endl;
						return false;
					}
					if (dims(2) == -1) {
						dims(2) = vr_ptr->get<uint32_t>("nr_frames");
						svol.ref_extent()(2) *= -dims(2);
					}
					dv_ptr = new cgv::data::data_view(&df);
				}
				else {
					if (dims(2) == -1) {
						if (st == ST_FILTER)
							dims(2) = int(slice_file_names.size() - svol.offset);
						else {
							dims(2) = 0;
							while (cgv::utils::file::exists(svol.get_slice_file_name(dims(2))))
								++dims(2);
						}
						svol.ref_extent()(2) *= -dims(2);
					}
					if ((dims(0) == -1 || dims(1) == -1)) {
						std::string fst_file_name;
						if (st == ST_FILTER) {
							if (svol.offset >= slice_file_names.size()) {
								std::cerr << "ERROR: offset larger than number of files" << std::endl;
								return false;
							}
							fst_file_name = file_path + slice_file_names[svol.offset];
						}
						else {
							fst_file_name = svol.get_slice_file_name(0);
						}
						if (!cgv::utils::file::exists(fst_file_name)) {
							std::cerr << "ERROR could not find first slice <" << fst_file_name << ">" << std::endl;
							return false;
						}
						cgv::media::image::image_reader ir(df);
						if (!ir.open(fst_file_name)) {
							std::cerr << "ERROR could not open first slice <" << fst_file_name << ">" << std::endl;
							return false;
						}
					}
				}
				if (dims(0) == -1) {
					dims(0) = df.get_width();
					svol.ref_extent()(0) *= -dims(0);
				}
				if (dims(1) == -1) {
					dims(1) = df.get_height();
					svol.ref_extent()(1) *= -dims(1);
				}
				svol.resize(dims);

				V.get_format().set_component_format(svol.get_format().get_component_format());
				V.resize(dims);
				V.ref_extent() = svol.get_extent();

				std::size_t slize_size = V.get_voxel_size() * V.get_format().get_width() * V.get_format().get_height();
				cgv::type::uint8_type* dst_ptr = V.get_data_ptr<cgv::type::uint8_type>();

				for (int i = 0; i < (int)dims(2); ++i) {
					switch (st) {
					case ST_INDEX:
						if (!svol.read_slice(i)) {
							std::cerr << "could not read slice " << i << " with filename \"" << svol.get_slice_file_name(i) << "\"." << std::endl;
							return false;
						}
						break;
					case ST_FILTER: {
						int j = i + svol.offset;
						if ((unsigned)j > slice_file_names.size()) {
							std::cerr << "could not read slice " << i << " from with filename with index " << j << " as only " << slice_file_names.size() << " match pattern." << std::endl;
							return false;
						}
						if (!svol.read_slice(i, file_path + slice_file_names[j])) {
							std::cerr << "could not read slice " << i << " from file \"" << slice_file_names[j] << "\"." << std::endl;
							return false;
						}
						break;
					}
					case ST_AVI:
						if (!vr_ptr->read_frame(*dv_ptr)) {
							std::cerr << "could not frame " << i << " from avi file \"" << svol.file_name_pattern << "\"." << std::endl;
							return false;
						}
						break;
					}
					const cgv::type::uint8_type* src_ptr =
						st == ST_AVI ? dv_ptr->get_ptr<cgv::type::uint8_type>() :
						svol.get_data_ptr<cgv::type::uint8_type>();
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