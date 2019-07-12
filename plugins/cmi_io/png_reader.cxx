#include "png_reader.h"
#include <cgv/base/register.h>
#include <cgv/base/import.h>
#include <iostream>

/// default constructor
png_reader::png_reader() : fp(0)
{
	interlacing = false;
}

/// close file in destructor
png_reader::~png_reader()
{
	if (fp)
		fclose(fp);
	fp = 0;
}
/// overload to return the type name of this object
std::string png_reader::get_type_name() const
{
	return "png_reader";
}
/// return the last error message
const std::string& png_reader::get_last_error() const
{
	return last_error;
}

/// construct a copy of the reader
abst_image_reader* png_reader::clone() const
{
	return new png_reader();
}
/// return a string containing a colon separated list of extensions that can be read with this reader
const char* png_reader::get_supported_extensions() const
{
	return "png";
}

/// open the file and read the image header in order to determine the data format
bool png_reader::open(const std::string& file_name, data_format& df, std::vector<data_format>* palette_formats)
{
	if ((fp = cgv::base::open_data_file(file_name.c_str(), "rb")) == NULL)
		return false;

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also supply the
	* the compiler header file version, so that we know if the application
	* was compiled with a compatible version of the library.  REQUIRED
	*/
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
				png_voidp_NULL, png_error_ptr_NULL, png_error_ptr_NULL);

	if (png_ptr == NULL) {
		fclose(fp);
		fp = 0;
		return false;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		fp = 0;
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return false;
	}

	/* Set error handling if you are using the setjmp/longjmp method (this is
	* the normal method of doing things with libpng).  REQUIRED unless you
	* set up your own error handlers in the png_create_read_struct() earlier.
	*/

	if (setjmp(png_jmpbuf(png_ptr))) {
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
		fclose(fp);
		fp = 0;
		/* If we get here, we had a problem reading the file */
		return false;
	}

	/* Set up the input control if you are using standard C streams */
	png_init_io(png_ptr, fp);

	/* If we have already read some of the signature */
	unsigned int sig_read = 0;
	png_set_sig_bytes(png_ptr, sig_read);

   /* OK, you're doing it the hard way, with the lower-level functions */

   /* The call to png_read_info() gives us all of the information from the
    * PNG file before the first IDAT (image data chunk).  REQUIRED
    */
   png_read_info(png_ptr, info_ptr);

   png_uint_32 width, height;
   int bit_depth, color_type, interlace_type;

   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
       &interlace_type, int_p_NULL, int_p_NULL);

   interlacing = interlace_type != PNG_INTERLACE_NONE;

	std::string components;
	TypeId component_type = bit_depth == 16 ? TI_UINT16 : TI_UINT8;
	bool has_rgb = color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGBA;
	if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
		has_rgb = true;
	}
	bool has_alpha = color_type == PNG_COLOR_TYPE_RGBA || color_type == PNG_COLOR_TYPE_GA;
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(png_ptr);
		has_alpha = true;
	}
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) 
		png_set_expand_gray_1_2_4_to_8(png_ptr);

	ComponentFormat cf;
	if (has_alpha)
		if (has_rgb)
			cf = CF_RGBA;
		else
			cf = CF_LA;
	else
		if (has_rgb)
			cf = CF_RGB;
		else
			cf = CF_L;
	if (bit_depth == 16)
        png_set_swap(png_ptr);
	df = data_format(width,height,component_type,cf);
	return true;
}
/// whether the reader supports per line reading (only valid after successful opening an image file
bool png_reader::supports_per_line_read() const
{
	return !interlacing;
}
/// read the next line into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done
bool png_reader::read_line(const data_format& df, const data_view& dv)
{
	if (interlacing) {
		last_error = "per line reading not supported for interlaced files";
		return false;
	}
	unsigned char* data_ptr = dv.get_ptr<unsigned char>();
	unsigned char** row_pointers = &data_ptr;
	if (setjmp(png_jmpbuf(png_ptr)))
		return false;
	png_read_rows(png_ptr, row_pointers, NULL,1);
	return true;
}
/// read the whole image into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done. If image file has not been opened yet, open it and close it after reading
bool png_reader::read_image(const data_format& df, const data_view& dv)
{
	unsigned char** row_pointers = new unsigned char*[df.get_height()];
	for (unsigned int y=0; y<df.get_height(); ++y)
		row_pointers[y] = dv.get_ptr<unsigned char>() + y*dv.get_step_size(0);
	if (setjmp(png_jmpbuf(png_ptr))) {
		delete [] row_pointers;
		return false;
	}
	png_read_image(png_ptr, row_pointers);
	delete [] row_pointers;
	return true;
}
/// close the image file
bool png_reader::close()
{
	if (!fp)
		return false;

	if (setjmp(png_jmpbuf(png_ptr))) {
		fclose(fp);
		fp = 0;
		return false;
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

	int res = fclose(fp);
	fp = 0;

	return res == 0;
}

cgv::base::object_registration<png_reader> prr2("");
