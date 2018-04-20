#include "jpg_reader.h"
#include <cgv/base/register.h>
#include <cgv/base/import.h>
#include <iostream>

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

/// default constructor
jpg_reader::jpg_reader() : fp(0)
{
}

/// close file in destructor
jpg_reader::~jpg_reader()
{
	if (fp)
		fclose(fp);
	fp = 0;
}

/// overload to return the type name of this object
std::string jpg_reader::get_type_name() const
{
	return "jpg_reader";
}

/// return the last error message
const std::string& jpg_reader::get_last_error() const
{
	return last_error;
}

/// construct a copy of the reader
abst_image_reader* jpg_reader::clone() const
{
	return new jpg_reader();
}

/// return a string containing a colon separated list of extensions that can be read with this reader
const char* jpg_reader::get_supported_extensions() const
{
	return "jpg;jpeg";
}

/// open the file and read the image header in order to determine the data format
bool jpg_reader::open(const std::string& file_name, data_format& df, std::vector<data_format>* palette_formats)
{
	/* In this example we want to open the input file before doing anything else,
	* so that the setjmp() error recovery below can assume the file is open.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to read binary files.
	*/
	if ((fp = cgv::base::open_data_file(file_name.c_str(), "rb")) == NULL) {
		last_error = "file could not be opened: ";
		last_error += file_name;
		return false;
	}

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		last_error = "jpeg error";
		jpeg_destroy_decompress(&cinfo);
		fclose(fp);
		return false;
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src(&cinfo, fp);

	/* Step 3: read file parameters with jpeg_read_header() */

	(void) jpeg_read_header(&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.doc for more info.
	*/

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	* jpeg_read_header(), so we do nothing here.
	*/

	/* Step 5: Start decompressor */

	(void) jpeg_start_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/
	/* JSAMPLEs per row in output buffer */
	std::string components;
	switch (cinfo.out_color_space) {
	case JCS_GRAYSCALE : components = "L"; break;
	case JCS_RGB :       components = "R,G,B"; break;
	case JCS_YCbCr :     components = "Y,Cb,Cr"; break;
	case JCS_CMYK :      components = "C,M,Y,K"; break;
	case JCS_YCCK :      components = "Y,Cb,Cr,K"; break;
	}
	df = data_format(cinfo.output_width, cinfo.output_height, TI_UINT8, components);
	return true;
}

/// whether the reader supports per line reading (only valid after successful opening an image file
bool jpg_reader::supports_per_line_read() const
{
	return true;
}

/// read the next line into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done
bool jpg_reader::read_line(const data_format& df, const data_view& dv)
{
	if (cinfo.output_scanline >= cinfo.output_height)
		return false;
	unsigned char* data_ptr = dv.get_ptr<unsigned char>();
	unsigned char** buffer = &data_ptr;
	if (setjmp(jerr.setjmp_buffer))
		return false;
	jpeg_read_scanlines(&cinfo, buffer, 1);
	return true;
}

/// read the whole image into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done. If image file has not been opened yet, open it and close it after reading
bool jpg_reader::read_image(const data_format& df, const data_view& dv)
{
	bool success = true;
	if (setjmp(jerr.setjmp_buffer))
		return false;
	for (unsigned int y = 0; success && y < df.get_height(); ++y) {
		success = read_line(df, dv(y));
	}
	return success;
}
/// close the image file
bool jpg_reader::close()
{
	if (!fp)
		return false;

	if (setjmp(jerr.setjmp_buffer)) {
		fclose(fp);
		fp = 0;
		return false;
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	int res = fclose(fp);
	fp = 0;

	return res == 0;
}

cgv::base::object_registration<jpg_reader> jrr("");
