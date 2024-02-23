#if _MSC_VER > 1400
#pragma warning(disable:4996)
#endif

#include "sliced_volume_io.h"
#include <cgv/utils/scan.h>
#include <cgv/utils/file.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/dir.h>
#include <cgv/os/cmdline_tools.h>
#include <cgv/media/video/video_reader.h>
#include <cgv/media/image/image_reader.h>


namespace cgv {
	namespace media {
		namespace volume {

			enum SourceType {
				ST_FILTER,
				ST_INDEX,
				ST_VIDEO
			};

			bool read_volume_from_video_with_ffmpeg(volume& V, const std::string& file_name,
				volume::dimension_type dims, volume::extent_type extent, const cgv::data::component_format& cf,
				size_t offset, FlipType flip_t, void (*on_progress_update)(int,void*), void* user_data, bool cycle_till_eof)
			{
				std::string fn_in_quotes = std::string("\"") + cgv::utils::file::platform_path(file_name) + "\"";
				if (dims(0) == -1 || dims(1) == -1 || dims(2) < 0) {
					int w, h, n = dims(2);
					while (true) {
						std::string cmd = "ffprobe -v error ";
						if (dims(2) == -2)
							cmd += "-count_frames ";
						cmd += "-select_streams v:0 -show_entries stream=width,height";
						if (dims(2) == -2)
							cmd += ",nb_read_frames";
						else
							cmd += ",nb_frames";
						cmd += " -show_entries side_data=rotation -of default=nokey=1:noprint_wrappers=1 ";
						cmd += fn_in_quotes;
						std::cout << "Analyze Video with " << cmd << std::endl;
						std::string video_info = cgv::os::query_system_output(cmd, false);
						std::vector<cgv::utils::line> lines;
						cgv::utils::split_to_lines(video_info, lines, true);
						bool do_swap_sides = false;
						if (lines.size() >= 4) {
							int rot;
							if (cgv::utils::is_integer(lines[3].begin, lines[3].end, rot)) {
								if (rot == 90 || rot == -90)
									do_swap_sides = true;
							}
						}
						std::stringstream ss(video_info);
						ss >> w >> h;
						if (do_swap_sides)
							std::swap(w, h);
						if (dims(2) < 0)
							ss >> n;
						// reattempt if nb_frames not supported by video format
						if (ss.fail()) {
							if (dims(2) == -1) {
								dims(2) = -2;
								std::cerr << "Video file <" << file_name << "> does not support nb_frames - need to could frames, please wait!" << std::endl;
								continue;
							}
							std::cerr << "Error: could not analyze video file <" << file_name << "> with ffprobe" << std::endl;
							return false;
						}
						else
							break;
					}
					if (dims(2) < 0 && offset > n) {
						std::cerr << "Error: frame offset " << offset << " larger than frame count of video file <" << file_name << "> with ffprobe" << std::endl;
						return false;
					}
					if (dims(0) == -1) {
						dims(0) = w;
						extent(0) *= -dims(0);
					}
					else if (dims(0) != w) {
						std::cerr << "Warning: mismatching video width - "
							<< dims(0) << " in svx file '" << file_name << "' vs "
							<< w << " in video file '" << file_name << "'" << std::endl;
					}
					if (dims(1) == -1) {
						dims(1) = h;
						extent(1) *= -dims(1);
					}
					else if (dims(1) != h) {
						std::cerr << "Warning: mismatching video height - "
							<< dims(1) << " in svx file '" << file_name << "' vs "
							<< h << " in video file '" << file_name << "'" << std::endl;
					}
					if (dims(2) < 0) {
						dims(2) = int(n - offset);
						extent(2) *= -dims(2);
					}
					//else if (dims(2) + offset > n) {
					//	std::cerr << "Warning: corrected video frame count from svx (" << dims(2)
					//		<< ") to smaller count found in video file (" << n - offset << ")" << std::endl;
					//	extent(2) *= float(n - offset) / dims(2);
					//	dims(2) = int(n - offset);
					//}
				}
				V.get_format().set_component_format(cf);
				V.get_format().set_width(dims(0));
				V.get_format().set_height(dims(1));
				V.get_format().set_depth(dims(2));
				if (on_progress_update)
					on_progress_update(-1, user_data);
				dims(2) = int(V.get_format().get_depth());
				V.resize(dims);
				V.ref_extent() = extent;
				std::string cmd = "ffmpeg -i ";
				cmd += fn_in_quotes;
				cmd += " -loglevel quiet";
				if (!cycle_till_eof) {
					cmd += " -frames:v ";
					cmd += cgv::utils::to_string(dims(2));
				}
				if (offset > 0) {
					cmd += " -vf \"select=gte(n\\,";
					cmd += cgv::utils::to_string(offset);
					cmd += ")\" ";
				}
				if(flip_t != FT_NO_FLIP)
				{
					if (offset == 0)
						cmd += " -vf ";
					if (flip_t == FT_HORIZONTAL)
						cmd += "hflip ";
					if (flip_t == FT_VERTICAL)
						cmd += "vflip ";
					if (flip_t == FT_VERTICAL_AND_HORIZONTAL)
						cmd += "hflip vflip ";
				}
				cmd += " -f rawvideo -pix_fmt ";
				switch (cf.get_standard_component_format()) {
				case cgv::data::CF_L:
					switch (cf.get_component_type()) {
					case cgv::type::info::TI_UINT8: cmd += "gray"; break;
					case cgv::type::info::TI_UINT16: cmd += "gray16be"; break;
					default:
						std::cerr << "unsupported luminance component type " << get_type_name(cf.get_component_type()) << std::endl;
						return false;
					}
					break;
				case cgv::data::CF_RGB:
					switch (cf.get_component_type()) {
					case cgv::type::info::TI_UINT8: cmd += "rgb24"; break;
					default:
						std::cerr << "unsupported rgb component type " << get_type_name(cf.get_component_type()) << std::endl;
						return false;
					}
					break;
				default:
					std::cerr << "unsupported component format " << (int)cf.get_standard_component_format() << std::endl;
					return false;
				}
				cmd += " pipe:1";
				std::cout << "COMMAND:\n" << cmd << "\n" << std::endl;
				if (on_progress_update)
					on_progress_update(0, user_data);
				
				size_t bytes_read = cgv::os::read_system_output(cmd, V.get_data_ptr<uint8_t>(), V.get_size(),
					"reading video", false, on_progress_update, user_data, V.get_slice_size(), cycle_till_eof);
				if (bytes_read < V.get_size()) {
					std::cerr << "Warning: could only read " << bytes_read << " of volume with size " << V.get_size() << std::endl;
				}
				if (on_progress_update)
					on_progress_update(V.get_dimensions()(2) + 1, user_data);
				return true;
			}
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
				SourceType st = ST_VIDEO;
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
				if (st == ST_VIDEO) {
					// first try to setup video reader
					vr_ptr = new cgv::media::video::video_reader(df);
					if (vr_ptr->open(svol.file_name_pattern)) {
						svol.set_component_type(df.get_component_type());
						svol.set_component_format(df.get_standard_component_format());
						if (dims(2) == -1) {
							dims(2) = vr_ptr->get<uint32_t>("nr_frames");
							svol.ref_extent()(2) *= -dims(2);
						}
						dv_ptr = new cgv::data::data_view(&df);
					}
					// if this does not work use ffmpeg to read video file
					else {
						delete vr_ptr;
						vr_ptr = 0;
						bool result = read_volume_from_video_with_ffmpeg(V, svol.file_name_pattern, dims, svol.get_extent(), svol.get_format().get_component_format(), svol.offset);
						svol.close();
						return result;
					}
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
					dims(0) = int(df.get_width());
					svol.ref_extent()(0) *= -dims(0);
				}
				if (dims(1) == -1) {
					dims(1) = int(df.get_height());
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
					case ST_VIDEO:
						if (!vr_ptr->read_frame(*dv_ptr)) {
							std::cerr << "could not frame " << i << " from avi file \"" << svol.file_name_pattern << "\"." << std::endl;
							return false;
						}
						break;
					}
					const cgv::type::uint8_type* src_ptr =
						st == ST_VIDEO ? dv_ptr->get_ptr<cgv::type::uint8_type>() :
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
				if (!svol.open_write(file_name, file_name_pattern, unsigned(V.get_format().get_depth()))) {
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