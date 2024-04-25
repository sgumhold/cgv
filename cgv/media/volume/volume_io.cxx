#include <cgv/base/base.h>
#include "volume_io.h"
#include <fstream>
#include <stdio.h>
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/media/image/image_reader.h>
#include <cgv/media/image/image_writer.h>
#include <cgv/media/video/video_reader.h>

namespace cgv {
	namespace media {
		namespace volume {


			volume_info::volume_info() : dimensions(0, 0, 0), extent(1, 1, 1), position(0, 0, 0), type_id(cgv::type::info::TI_UINT8), components(cgv::data::CF_L)
			{
				orientation.identity();
			}
			volume_info::volume_info(const volume& V, const std::string& _path)
			{
				path = _path;
				cgv::type::info::TypeId type_id;
				cgv::data::ComponentFormat components;
				dimensions = V.get_dimensions();
				type_id = V.get_component_type();
				components = V.get_format().get_standard_component_format();
				extent = V.get_box().get_extent();
				position = V.get_box().get_center();
				orientation.identity();
			}
			size_t volume_info::get_data_size() const
			{
				return dimensions(0) * dimensions(1) * dimensions(2) * cgv::type::info::get_type_size(type_id);
			}

			bool read_vox(const std::string& file_name, volume& V, volume_info* info_ptr = 0);

			bool read_qim_header(const std::string& file_name, volume_info& info);
			bool read_qim(const std::string& file_name, volume& V, volume_info* info_ptr = 0);

			bool read_tiff(const std::string& file_name, volume& V, volume_info* info_ptr = 0);

			bool read_avi(const std::string& file_name, volume& V, volume_info* info_ptr = 0);


			bool write_vox(const std::string& file_name, const volume& V);

			bool write_qim_header(const std::string& file_name, const volume& V);
			bool write_qim(const std::string& file_name, const volume& V);

			bool write_tiff(const std::string& file_name, const volume& V, const std::string& options);


			bool read_header(const std::string& file_name, volume_info& info, bool(*unknown_line_callback)(const std::string& line, const std::vector<cgv::utils::token>&, volume_info& info))
			{
				std::string ext = cgv::utils::to_upper(cgv::utils::file::get_extension(file_name));
				if (ext == "HD")
					return read_vox_header(file_name, info, unknown_line_callback);
				if (ext == "QHA")
					return read_qim_header(file_name, info);
				return false;
			}

			bool read_volume(const std::string& file_name, volume& V, volume_info* info_ptr)
			{
				std::string ext = cgv::utils::to_upper(cgv::utils::file::get_extension(file_name));
				if (ext == "VOX" || ext == "HD")
					return read_vox(file_name, V, info_ptr);
				if (ext == "QIM" || ext == "QHA")
					return read_qim(file_name, V, info_ptr);
				if (ext == "TIF" || ext == "TIFF")
					return read_tiff(file_name, V, info_ptr);
				if (ext == "AVI")
					return read_avi(file_name, V, info_ptr);

				std::cerr << "unsupported extension " << ext << std::endl;
				return false;
			}

			bool write_header(const std::string& file_name, const volume& V)
			{
				std::string ext = cgv::utils::to_upper(cgv::utils::file::get_extension(file_name));
				if (ext == "HD")
					return write_vox_header(file_name, V);
				if (ext == "QHA")
					return write_qim_header(file_name, V);
				return false;
			}

			bool write_volume(const std::string& file_name, const volume& V, const std::string& options)
			{
				std::string ext = cgv::utils::to_upper(cgv::utils::file::get_extension(file_name));
				if (ext == "VOX" || ext == "HD")
					return write_vox(file_name, V);
				if (ext == "QIM" || ext == "QHA")
					return write_qim(file_name, V);
				if (ext == "TIF" || ext == "TIFF")
					return write_tiff(file_name, V, options);

				std::cerr << "unsupported extension " << ext << std::endl;
				return false;
			}

			bool read_vox_header(const std::string& file_name, volume_info& info, bool (*unknown_line_callback)(const std::string& line, const std::vector<cgv::utils::token>&, volume_info& info))
			{
				std::ifstream is(file_name);
				if (is.fail()) {
					std::cerr << "could not open vox header " << file_name << std::endl;
					return false;
				}
				while (!is.eof()) {
					char buffer[500];
					is.getline(buffer, 500);
					std::string line(buffer);
					cgv::utils::tokenizer t(line);
					std::vector<cgv::utils::token> toks;
					t.set_ws(" \t,x:");
					t.set_skip("\"'", "\"'");
					t.bite_all(toks);
					bool success = false;
					if (toks.size() >= 2) {
						if (cgv::utils::to_upper(to_string(toks[0])) == "SIZE") {
							int c;
							for (c = 1; c < 4; ++c) {
								if (!cgv::utils::is_integer(toks[c].begin, toks[c].end, info.dimensions(c - 1)))
									break;
							}
							if (c == 4) {
								success = true;
								// check if value type is also given
								if (toks.size() > 4) {
									unsigned offset = 0;
									std::string type_name = cgv::utils::to_upper(to_string(cgv::utils::token(toks[4].begin, toks.back().end)));
									if (type_name.substr(0, 13) == "UNSIGNED BYTE") {
										info.type_id = cgv::type::info::TI_UINT8;
										offset = 14;
									}
									else if (type_name.substr(0, 14) == "UNSIGNED SHORT") {
										info.type_id = cgv::type::info::TI_UINT16;
										offset = 15;
									}
									else if (type_name.substr(0, 12) == "UNSIGNED INT") {
										info.type_id = cgv::type::info::TI_UINT32;
										offset = 13;
									}
									else if (type_name.substr(0, 11) == "SIGNED BYTE") {
										info.type_id = cgv::type::info::TI_INT8;
										offset = 12;
									}
									else if (type_name.substr(0, 12) == "SIGNED SHORT") {
										info.type_id = cgv::type::info::TI_INT16;
										offset = 13;
									}
									else if (type_name.substr(0, 10) == "SIGNED INT") {
										info.type_id = cgv::type::info::TI_INT32;
										offset = 11;
									}
									else if (type_name.substr(0, 5) == "FLOAT") {
										info.type_id = cgv::type::info::TI_FLT32;
										offset = 6;
									}
									else if (type_name.substr(0, 6) == "DOUBLE") {
										info.type_id = cgv::type::info::TI_FLT64;
										offset = 7;
									}
									else {
										std::cerr << "unknown voxel type " << type_name << std::endl;
										success = false;
									}
									if (success && offset < type_name.size()) {
										std::string components = cgv::utils::to_upper(type_name.substr(offset));
										if (components.substr(0, 9) == "LUMINANCE")
											info.components = cgv::data::CF_L;
										else if (components.substr(0, 9) == "INTENSITY")
											info.components = cgv::data::CF_I;
										else if (components.substr(0, 4) == "RGBA")
											info.components = cgv::data::CF_RGBA;
										else if (components.substr(0, 3) == "RGB")
											info.components = cgv::data::CF_RGB;
										else if (components.substr(0, 2) == "RG")
											info.components = cgv::data::CF_RG;
										else if (components.substr(0, 5) == "ALPHA")
											info.components = cgv::data::CF_A;
									}
								}
							}
						}
						else if (cgv::utils::to_upper(to_string(toks[0])) == "POSITION") {
							int c;
							for (c = 1; c < 4; ++c) {
								double d;
								if (cgv::utils::is_double(toks[c].begin, toks[c].end, d))
									info.position(c - 1) = (float)d;
								else
									break;
							}
							if (c == 4)
								success = true;
						}
						else if (cgv::utils::to_upper(to_string(toks[0])) == "ORIENTATION") {
							int c;
							for (c = 1; c < 10; ++c) {
								double d;
								if (cgv::utils::is_double(toks[c].begin, toks[c].end, d))
									static_cast<cgv::math::fmat<volume::coord_type, 3, 3>::base_type&>(info.orientation)(c - 1) = (float)d;
								else
									break;
							}
							if (c == 10)
								success = true;
						}
						else if (cgv::utils::to_upper(to_string(toks[0])) == "SCALING") {
							int c;
							for (c = 1; c < 4; ++c) {
								double d;
								if (cgv::utils::is_double(toks[c].begin, toks[c].end, d))
									info.extent(c - 1) = (float)d;
								else
									break;
							}
							if (c == 4)
								success = true;
						}
						else if (cgv::utils::to_upper(to_string(toks[0])) == "SPACING") {
							int c;
							for (c = 1; c < 4; ++c) {
								double d;
								if (cgv::utils::is_double(toks[c].begin, toks[c].end, d))
									info.extent(c - 1) = (float)(d * info.dimensions(c - 1));
								else
									break;
							}
							if (c == 4)
								success = true;
						}
						else if (cgv::utils::to_upper(to_string(toks[0])) == "PATH") {
							info.path = to_string(toks[1]);
							if (info.path.size() >= 2 && info.path.front() == info.path.back() &&
								(info.path.front() == '"' || info.path.front() == '\''))
								info.path = info.path.substr(1, info.path.size() - 2);
							success = true;
						}
					}
					if (!success) {
						if (toks.size() > 0) {
							if (unknown_line_callback && unknown_line_callback(line, toks, info)) {
								success = true;
							}
							else {
								std::cerr << "did not understand header line <" << line << ">!" << std::endl;
							}
						}
					}
				}
				return true;
			}

			bool write_vox_header(const std::string& file_name, const volume_info& VI)
			{
				std::ofstream os(file_name);
				if (os.fail()) {
					std::cerr << "cannot open vox header " << file_name << " for writing." << std::endl;
					return false;
				}
				os << "Size: " << VI.dimensions(0) << "x" << VI.dimensions(1) << "x" << VI.dimensions(2);
				switch (VI.type_id) {
				case cgv::type::info::TI_UINT8: os << " unsigned byte"; break;
				case cgv::type::info::TI_UINT16: os << " unsigned short"; break;
				case cgv::type::info::TI_UINT32: os << " unsigned int"; break;
				case cgv::type::info::TI_INT8: os << " signed byte"; break;
				case cgv::type::info::TI_INT16: os << " signed short"; break;
				case cgv::type::info::TI_INT32: os << " signed int"; break;
				case cgv::type::info::TI_FLT32: os << " float"; break;
				case cgv::type::info::TI_FLT64: os << " double"; break;
				}
				switch (VI.components) {
				case cgv::data::CF_L: os << " Luminance"; break;
				case cgv::data::CF_I: os << " Intensity"; break;
				case cgv::data::CF_RG: os << " RG"; break;
				case cgv::data::CF_RGB: os << " RGB"; break;
				case cgv::data::CF_RGBA: os << " RGBA"; break;
				case cgv::data::CF_A: os << " Alpha"; break;
				}
				os << std::endl;
				volume::extent_type scaling = VI.extent;
				os << "Scaling: " << scaling(0)/VI.dimensions(0) << "x" << scaling(1)/VI.dimensions(1) << "x" << scaling(2)/VI.dimensions(2) << std::endl;
				if (VI.position.length() > 1e-10)
					os << "Position: " << VI.position(0) << ", " << VI.position(1) << ", " << VI.position(2) << std::endl;
				
				cgv::math::fmat<volume::coord_type, 3, 3> I;
				I.identity();
				if ((VI.orientation - I).length() > 1e-10) {
					os << "Orientation: ";
					for (int i = 0; i < 9; ++i) {
						if (i > 0)
							os << ", ";
						os << reinterpret_cast<const cgv::math::fmat<volume::coord_type, 3, 3>::base_type&>(VI.orientation)(i);
					}
					os << std::endl;
				}
				if (!VI.path.empty())
					os << "Path: \"" << VI.path << "\"" << std::endl;
				return !os.fail();
			}

			// toggle endian
			void toggle_volume_endian(volume& V)
			{
				std::size_t n = V.get_nr_voxels();
				unsigned c = V.get_nr_components();
				unsigned N = cgv::type::info::get_type_size(V.get_component_type());
				if (N == 2) {
					unsigned char* ptr = V.get_data_ptr<unsigned char>();
					for (unsigned i = 0; i < n; ++i)
						for (unsigned j = 0; j < c; ++j) {
							std::swap(ptr[0], ptr[1]);
							ptr += 2;
						}
				}
				if (N == 4) {
					unsigned char* ptr = V.get_data_ptr<unsigned char>();
					for (unsigned i = 0; i < n; ++i)
						for (unsigned j = 0; j < c; ++j) {
							std::swap(ptr[0], ptr[3]);
							std::swap(ptr[1], ptr[2]);
							ptr += 4;
						}
				}
			}

			bool read_volume_binary(const std::string& file_name, const volume_info& info, volume& V, size_t offset)
			{
				// update volume data structure and reserve space
				if (V.get_component_type() != info.type_id)
					V.set_component_type(info.type_id);
				if (V.get_component_format() != info.components)
					V.set_component_format(info.components);
				if (V.get_dimensions() != info.dimensions)
					V.resize(info.dimensions);
				if (V.get_extent() != info.extent)
					V.ref_extent() = info.extent;

				// open file and jump to offset
				FILE* fp = fopen(file_name.c_str(), "rb");
				if (!fp) {
					std::cerr << "cannot open file " << file_name << std::endl;
					return false;
				}
				if (offset > 0) {
					fseek(fp, (long)offset, SEEK_SET);
				}

				// read data
				std::size_t n = V.get_nr_voxels();
				unsigned N = V.get_voxel_size();
				std::size_t nr = fread(V.get_data_ptr<unsigned char>(), N, n, fp);
				if (nr != n) {
					std::cerr << "could not read the expected number " << n << " of voxels but only " << nr << std::endl;
					fclose(fp);
					return false;
				}

				// close and return success
				fclose(fp);
				return true;
			}

			bool read_vox(const std::string& file_name, volume& V, volume_info* info_ptr)
			{
				volume_info local_info;
				volume_info& info = info_ptr ? *info_ptr : local_info;
				if (!read_vox_header(cgv::utils::file::drop_extension(file_name) + ".hd", info))
					return false;
				return read_volume_binary(cgv::utils::file::drop_extension(file_name) + ".vox", info, V);
			}

			bool read_qim_header(const std::string& file_name, volume_info& info)
			{
				std::ifstream is(file_name);
				if (is.fail()) {
					std::cerr << "could not open qim header " << file_name << std::endl;
					return false;
				}
				int nr_components;
				volume::point_type c, x, y, z;
				is >> nr_components >> c;
				is >> info.dimensions(0) >> x;
				is >> info.dimensions(1) >> y;
				is >> info.dimensions(2) >> z;
				info.extent = volume::extent_type(x.length(), y.length(), z.length());
				if (nr_components != 1) {
					std::cerr << "no support for more than one component in qim file yet" << std::endl;
					exit(1);
				}
				return !is.fail();
			}

			bool read_qim(const std::string& file_name, volume& V, volume_info* info_ptr)
			{
				volume_info local_info;
				volume_info& info = info_ptr ? *info_ptr : local_info;
				if (!read_qim_header(cgv::utils::file::drop_extension(file_name) + ".qha", info))
					return false;
				return read_volume_binary(cgv::utils::file::drop_extension(file_name) + ".qim", info, V);
			}

			bool read_tiff(const std::string& file_name, volume& V, volume_info* info_ptr)
			{
				cgv::data::data_format df;
				cgv::media::image::image_reader ir(df);
				if (!ir.open(file_name)) {
					std::cerr << "could not open tiff file " << file_name << std::endl;
					return false;
				}
				int n = ir.get_nr_images();
				V.get_format().set_component_format(df.get_component_format());
				if (info_ptr) {
					info_ptr->type_id = df.get_component_type();
					info_ptr->components = df.get_standard_component_format();
				}
				volume::dimension_type size(int(df.get_width()), int(df.get_height()), n);
				if (info_ptr) {
					info_ptr->dimensions = size;
					info_ptr->position.zeros();
					info_ptr->orientation.identity();
				}
				V.resize(size);
				V.ref_extent() = volume::point_type(1, 1, 1) * size / (float)cgv::math::max_value(size);
				if (info_ptr)
					info_ptr->extent = V.get_extent();
				for (int i = 0; i < n; ++i) {
					cgv::data::data_view dv(&df, V.get_slice_ptr<void>(i));
					if (!ir.read_image(dv)) {
						std::cerr << "could not read slice " << i << " of file " << file_name << std::endl;
						return false;
					}
				}
				ir.close();
				return true;
			}

			bool read_avi(const std::string& file_name, volume& V, volume_info* info_ptr)
			{
				cgv::data::data_format df;
				cgv::media::video::video_reader vr(df);
				if (!vr.open(file_name)) {
					std::cerr << "could not open avi file " << file_name << std::endl;
					return false;
				}
				int nr_frames = vr.get<int>("nr_frames");
				if (nr_frames == 0) {
					std::cerr << "could not determine nr frames in avi file " << file_name << std::endl;
					return false;
				}
				V.get_format().set_component_format(cgv::data::component_format(df.get_component_type(), cgv::data::CF_L));
				if (info_ptr) {
					info_ptr->type_id = V.get_format().get_component_type();
					info_ptr->components = V.get_format().get_standard_component_format();
				}
				volume::dimension_type size(int(df.get_width()), int(df.get_height()), nr_frames);
				if (info_ptr) {
					info_ptr->dimensions = size;
					info_ptr->position.zeros();
					info_ptr->orientation.identity();
				}
				V.resize(size);
				V.ref_extent() = volume::point_type(1, 1, 1) * size / (float)cgv::math::max_value(size);
				if (info_ptr)
					info_ptr->extent = V.get_extent();
				cgv::data::data_view dv(&df);
				unsigned char* src_ptr = dv.get_ptr<unsigned char>();
				size_t n = df.get_width() * df.get_height();
				for (int i = 0; i < nr_frames; ++i) {
					if (!vr.read_frame(dv)) {
						std::cerr << "could not read slice " << i << " of file " << file_name << std::endl;
						return false;
					}
					unsigned char* dst_ptr = V.get_slice_ptr<unsigned char>(i);
					for (size_t j = 0; j < n; ++j)
						dst_ptr[j] = src_ptr[3 * j];
				}
				vr.close();
				return true;
			}

			bool write_volume_binary(const std::string& file_name, const volume& V, size_t offset)
			{
				std::size_t n = V.get_nr_voxels();
				unsigned N = V.get_voxel_size();
				FILE* fp = fopen(file_name.c_str(), "wb");
				if (!fp) {
					std::cerr << "cannot open volume file " << file_name << " for write." << std::endl;
					return false;
				}
				if (offset > 0) {
					if (fseek(fp, (long)offset, SEEK_SET) != 0) {
						std::cerr << "could not seek position " << offset << " in file " << file_name << std::endl;
						fclose(fp);
						return false;
					}
				}
				std::size_t nr = fwrite(V.get_data_ptr<cgv::type::uint8_type>(), N, n, fp);
				fclose(fp);
				if (nr != n) {
					std::cerr << "could not write the expected number " << n << " of voxels but only " << nr << std::endl;
					return false;
				}
				std::cout << "write volume '" << file_name << "' of size " << V.get_dimensions() << " and scaling " << V.get_box().get_extent();
				if (offset > 0)
					std::cout << " at offset " << offset;
				std::cout << std::endl;
				return true;
			}

			bool write_vox(const std::string& file_name, const volume& V)
			{
				if (!write_vox_header(cgv::utils::file::drop_extension(file_name) + ".hd", volume_info(V)))
					return false;
				return write_volume_binary(cgv::utils::file::drop_extension(file_name) + ".vox", V);
			}

			bool write_qim_header(const std::string& file_name, const volume& V)
			{
				std::ofstream os(file_name);
				if (os.fail()) {
					std::cerr << "cannot open qim header " << file_name << " for writing." << std::endl;
					return false;
				}
				os << V.get_nr_components() << " 0 0 0" << std::endl;
				os << V.get_dimensions()(0) << " " << V.get_box().get_extent()(0) << " 0 0" << std::endl;
				os << V.get_dimensions()(1) << " 0 " << V.get_box().get_extent()(1) << " 0" << std::endl;
				os << V.get_dimensions()(2) << " 0 0 " << V.get_box().get_extent()(2) << std::endl;
				return !os.fail();
			}

			bool write_qim(const std::string& file_name, const volume& V)
			{
				if (!write_vox_header(cgv::utils::file::drop_extension(file_name) + ".qha", V))
					return false;
				return write_volume_binary(cgv::utils::file::drop_extension(file_name) + ".qim", V);
			}

			bool write_tiff(const std::string& file_name, const volume& V, const std::string& options)
			{
				cgv::data::data_format df;
				df.set_component_format(V.get_format().get_component_format());
				df.set_width(V.get_dimensions()(0));
				df.set_height(V.get_dimensions()(1));
				cgv::media::image::image_writer iw(file_name);
				iw.multi_set(options);
				for (unsigned i = 0; i < (unsigned)(V.get_dimensions()(2)); ++i) {
					cgv::data::const_data_view dv(&df, V.get_slice_ptr<void>(i));
					if (!iw.write_image(dv)) {
						std::cerr << "could not write slice " << i << " to tiff file " << file_name << std::endl;
						return false;
					}
				}
				return iw.close();
			}

		}
	}
}