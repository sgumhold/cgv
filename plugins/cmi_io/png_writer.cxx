#include "png_writer.h"
#include <cgv/base/register.h>
#include <iostream>


/// default constructor
png_writer::png_writer() : fp(0)
{
	interlacing = false;
	compression_level =  Z_BEST_COMPRESSION;
}
/// close file in destructor
png_writer::~png_writer()
{
	if (fp)
		fclose(fp);
	fp = 0;
}

/// overload to return the type name of this object
std::string png_writer::get_type_name() const
{
	return "png_writer";
}

///
bool png_writer::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return rh.reflect_member("compression_level", compression_level);
}

/// construct a copy of the reader
abst_image_writer* png_writer::clone() const
{
	return new png_writer();
}
/// return a string containing a colon separated list of extensions that can be read with this reader
const char* png_writer::get_supported_extensions() const
{
	return "png";
}

bool png_writer::determine_params(const component_format& cf, int& color_type, int& bit_depth) const
{
	switch (cf.get_standard_component_format()) {
	case CF_L :
	case CF_R :
	case CF_G :
	case CF_B :
	case CF_A :
		color_type = PNG_COLOR_TYPE_GRAY;
		break;
	case CF_LA :
		color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
		break;
	case CF_RGB :
		color_type = PNG_COLOR_TYPE_RGB;
		break;
	case CF_RGBA :
		color_type = PNG_COLOR_TYPE_RGBA;
		break;
	default:
		last_error = "unsupported component format";
		return false;
	}
	if (cf.is_packing()) {
		bit_depth = cf.get_bit_depth(0);
		if (color_type != PNG_COLOR_TYPE_GRAY) {
			last_error = "bit depths below 8 are only supported for gray value images or paletted ones";
			return false;
		}
		if (bit_depth != 1 && bit_depth != 2 && bit_depth != 4 && bit_depth) {
			last_error = "only bit depths 1, 2, 4, 8 or 16 supported for gray value images";
			return false;
		}
		for (unsigned int i=1; i<cf.get_nr_components(); ++i)
			if (cf.get_bit_depth(i) != bit_depth) {
				last_error = "only a common bit depth for all components supported";
				return false;
			}
	}
	else {
		switch (cf.get_component_type()) {
		case TI_UINT8 : bit_depth = 8; break;
		case TI_UINT16 : bit_depth = 16; break;
		default : return false;
		}
	}
	return true;
}

/// check if the chosen writer supports the given component format
bool png_writer::is_format_supported(const component_format& cf, const std::vector<component_format>* palette_formats) const
{
	int ct, bd;
	return determine_params(cf,ct,bd);
}
/// return a reference to the last error message
const std::string& png_writer::get_last_error() const
{
	return last_error;
}

/// open image file to write
bool png_writer::open(const std::string& _file_name)
{
	file_name = _file_name;
	return true;
}

/** write the data stored in the data view to a file with the file name given in the constructor. */
bool png_writer::write_image(const const_data_view& dv, const std::vector<const_data_view>* palettes, double duration)
{
	int color_type, bit_depth;
	if (!determine_params(*dv.get_format(), color_type, bit_depth))
		return false;

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 
		png_voidp_NULL, png_error_ptr_NULL, png_error_ptr_NULL);
	if (!png_ptr) {
		last_error = "could not create png write struct";
		return false;
	}

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		last_error = "could not create info struct";
		return false;
    }

	fp = fopen(file_name.c_str(), "wb");
    if (!fp) {
		last_error = "could not open file";
		return false;
    }

	if (setjmp(png_jmpbuf(png_ptr))) {
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(fp);
		fp = 0;
		/* If we get here, we had a problem reading the file */
		return false;
	}

	png_init_io(png_ptr, fp);

	png_set_compression_level(png_ptr,compression_level);

	int h = dv.get_format()->get_height();
	png_set_IHDR(png_ptr, info_ptr, dv.get_format()->get_width(), h,
				 bit_depth, color_type, interlacing?PNG_INTERLACE_ADAM7
				 :PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_byte** row_data = new png_byte*[h];
	const unsigned char* data = dv.get_ptr<unsigned char>();
	int step_size = dv.get_step_size(0);
	for (int i=0; i<h; ++i)
		row_data[i] = const_cast<unsigned char*>(data) + i*step_size;
	png_set_rows(png_ptr, info_ptr, row_data);
#ifdef WIN32
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY | PNG_TRANSFORM_SWAP_ENDIAN, NULL);
#else
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
#endif
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	delete [] row_data;
	fclose(fp);
	return true;
}

/// close image [stream]
bool png_writer::close()
{
	return true;
}

cgv::base::object_registration<png_writer> pwr2("");
