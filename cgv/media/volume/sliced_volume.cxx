#include <cgv/base/base.h>
#include "sliced_volume.h"
#include "sliced_volume_io.h"
#include <fstream>
#include <stdio.h>
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/convert_string.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/media/image/image_writer.h>

namespace cgv {
	namespace media {
		namespace volume {


			ooc_sliced_volume::ooc_sliced_volume()
			{
				nr_slices = 0;
				offset = 0;
			}

			volume::dimension_type ooc_sliced_volume::get_dimensions() const
			{
				return
					df.get_nr_dimensions() > 1 ?
					dimension_type(int(df.get_width()), int(df.get_height()), nr_slices) :
					dimension_type(0, 0, 0);
			}

			/// resize the data view to hold a slice and set the total number of slices as well
			void ooc_sliced_volume::resize(const dimension_type& S)
			{
				df.set_width(S(0));
				df.set_height(S(1));
				nr_slices = S(2);
				dv = cgv::data::data_view(&df);
			}


			std::string ooc_sliced_volume::get_slice_file_name(int i) const
			{
				static std::string long_placeholder = "00000000$";
				std::string file_name = file_name_pattern;
				std::string number = cgv::utils::to_string(i + offset);
				std::string placeholder = long_placeholder.substr(long_placeholder.size() - number.size());
				cgv::utils::replace(file_name, placeholder, number);
				return file_name;
			}

			bool ooc_sliced_volume::open_read(const std::string& file_name)
			{
				sliced_volume_info info;
				if (!read_sliced_header(file_name, info)) {
					std::cerr << "could not read header " << file_name << std::endl;
					return false;
				}
				df.set_nr_dimensions(2);
				df.set_width(info.dimensions(0));
				df.set_height(info.dimensions(1));
				nr_slices = info.dimensions(2);
				extent = info.extent;
				offset = info.offset;
				df.set_component_format(cgv::data::component_format(info.type_id, info.components));

				file_name_pattern = info.file_name_pattern;
				std::string path = cgv::utils::file::get_path(file_name);
				if (path.size() > 0)
					file_name_pattern = path + "/" + file_name_pattern;

				//if (file_name_pattern.find_first_of('*') == std::string::npos && get_slice_file_name(0) == get_slice_file_name(1)) {
				//	std::cerr << "found invalid file name pattern " << file_name_pattern << std::endl;
				//	return false;
				//}
				return true;
			}

			bool ooc_sliced_volume::open_write(const std::string& file_name, const std::string& _file_name_pattern, unsigned nr_slices)
			{
				file_name_pattern = _file_name_pattern;

				// pattern to be written in file should be with local path and no backslashes
				std::string pattern = _file_name_pattern;
				std::string path = cgv::utils::file::get_path(file_name);
				if (path.size() > 0) {
					if (pattern.substr(0, path.size()) == path)
						pattern = pattern.substr(path.size() + 1);
				}
				pattern = cgv::utils::file::clean_path(pattern);

				// writing header
				if (!write_sliced_header(file_name, *this)) {
					std::cerr << "could not write header " << file_name << std::endl;
					return false;
				}

				return true;
			}

			bool ooc_sliced_volume::is_open() const
			{
				return !file_name_pattern.empty();
			}

			unsigned ooc_sliced_volume::get_nr_slices() const
			{
				return nr_slices;
			}

			bool ooc_sliced_volume::read_slice(int i, const std::string& slice_file_name)
			{
				if (i < 0 || i >= (int)get_nr_slices()) {
					std::cerr << "slice index " << i << " out of range [0, " << get_nr_slices() << "[" << std::endl;
					return false;
				}
				std::string file_name = slice_file_name.empty() ? get_slice_file_name(i) : slice_file_name;

				// detect special case for binary files
				if (cgv::utils::file::get_extension(file_name).empty()) {
					size_t file_size = cgv::utils::file::size(file_name);
					size_t data_size = df.get_nr_bytes();
					if (data_size > file_size) {
						std::cerr << "slice file " << file_name << " too small: only contains " << file_size << " bytes, but " << df.get_nr_bytes() << " bytes needed." << std::endl;
						return false;
					}
					char* content = cgv::utils::file::read(file_name);
					if (!content) {
						std::cerr << "could not read slice file " << file_name << "." << std::endl;
						return false;
					}
					size_t offset = file_size - data_size;
					if (dv.empty())
						new(&dv) data_view(&df);
					std::copy(content + offset, content + file_size, dv.get_ptr<char>());
					delete[] content;
					return true;
				}
				else {
					cgv::media::image::image_reader ir(df);
					if (!ir.open(file_name)) {
						std::cerr << "could not open slice file " << file_name << std::endl;
						return false;
					}
					if (!ir.read_image(dv)) {
						std::cerr << "could not read slice file " << file_name << std::endl;
						return false;
					}
					ir.close();
					return true;
				}
			}

			bool ooc_sliced_volume::write_slice(int i) const
			{
				if (i < 0 || i >= (int)get_nr_slices()) {
					std::cerr << "slice index " << i << " out of range [0, " << get_nr_slices() << "[" << std::endl;
					return false;
				}
				std::string file_name = get_slice_file_name(i);
				cgv::media::image::image_writer iw(file_name);
				if (!iw.write_image(dv)) {
					std::cerr << "could not write slice file " << file_name << std::endl;
					return false;
				}
				iw.close();
				return true;
			}


			void ooc_sliced_volume::close()
			{
				clear();
				file_name_pattern.clear();
				nr_slices = 0;
			};

			sliced_volume_info::sliced_volume_info()
			{
				offset = 0;
			}

			std::string truncate_spaces(const std::string& s)
			{
				size_t p = 0;
				while (p < s.size() && cgv::utils::is_space(s[p]))
					++p;
				size_t q = s.size();
				while (q >= p && q > 0 && cgv::utils::is_space(s[q - 1]))
					--q;
				return s.substr(p, q - p);
			}

			bool unknown_line_callback(const std::string& line, const std::vector<cgv::utils::token>& toks, volume_info& info)
			{
				if (cgv::utils::to_upper(to_string(toks[0])) == "PATTERN") {
					sliced_volume_info& sinfo = (sliced_volume_info&)info;
					sinfo.file_name_pattern = truncate_spaces(line.substr(8));
					return true;
				}
				if (cgv::utils::to_upper(to_string(toks[0])) == "OFFSET") {
					sliced_volume_info& sinfo = (sliced_volume_info&)info;
					if (!cgv::utils::is_integer(truncate_spaces(line.substr(7)), sinfo.offset)) {
						std::cerr << "ERROR in OFFSET line: <" << truncate_spaces(line.substr(7)) << "> not defining an integer offset" << std::endl;
						return false;
					}
					return true;
				}
				return false;
			}

			bool read_sliced_header(const std::string& file_name, sliced_volume_info& info)
			{
				return read_vox_header(file_name, info, &unknown_line_callback);
			}

			bool write_sliced_header(const std::string& file_name, const ooc_sliced_volume& V)
			{
				if (!write_vox_header(file_name, V))
					return false;
				std::ofstream os(file_name, std::ofstream::app);
				os << "Pattern: " << V.file_name_pattern << std::endl;
				return !os.fail();
			}

		}
	}
}