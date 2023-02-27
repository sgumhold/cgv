#include "jpg_writer.h"
#include <cgv/base/register.h>
#include <iostream>

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr)cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

/// default constructor
jpg_writer::jpg_writer()
{
	quality = 90;
}

/// close file in destructor
jpg_writer::~jpg_writer()
{

}
///
bool jpg_writer::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return rh.reflect_member("quality", quality);
}

/// overload to return the type name of this object
std::string jpg_writer::get_type_name() const
{
	return "jpg_writer";
}

/// construct a copy of the reader
abst_image_writer* jpg_writer::clone() const
{
	return new jpg_writer(*this);
}
/// return a string containing a colon separated list of extensions that can be read with this reader
const char* jpg_writer::get_supported_extensions() const
{
	return "jpg;jpeg";
}
/// check if the chosen writer supports the given component format
bool jpg_writer::is_format_supported(const component_format& cf, const std::vector<component_format>* palette_formats) const
{
	if (cf.get_component_type() != TI_UINT8 && cf.get_component_type() != TI_INT8)
		return false;
	switch (cf.get_standard_component_format()) {
	case cgv::data::CF_RGB: return true;
	case cgv::data::CF_L: return true;
	default: return false;
	}
}
/// return a reference to the last error message
const std::string& jpg_writer::get_last_error() const
{
	return last_error;
}

/// open image file to write
bool jpg_writer::open(const std::string& file_name)
{
	/* Step 1: allocate and initialize JPEG compression object */

	/* We have to set up the error handler first, in case the initialization
	 * step fails.  (Unlikely, but it could happen if you are out of memory.)
	 * This routine fills in the contents of struct jerr, and returns jerr's
	 * address which we place into the link field in cinfo.
	 */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */
		last_error = "jpeg error";
		fclose(outfile);
		jpeg_destroy_compress(&cinfo);
		return false;
	}
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */

	/* Here we use the library-supplied code to send compressed data to a
	 * stdio stream.  You can also write your own code to do something else.
	 * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	 * requires it in order to write binary files.
	 */
	outfile = fopen(file_name.c_str(), "wb");
	if (outfile == NULL) {
		last_error = "can't open file ";
		last_error += file_name;
		return false;
	}
	jpeg_stdio_dest(&cinfo, outfile);
	return true;
}

/** write the data stored in the data view to a file with the file name given in the constructor. */
bool jpg_writer::write_image(const const_data_view& dv, const std::vector<const_data_view>* palettes, double duration)
{
	const auto* fp = dv.get_format();
	/* Step 3: set parameters for compression */

	/* First we supply a description of the input image.
	 * Four fields of the cinfo struct must be filled in:
	 */
	cinfo.image_width =  fp->get_width(); 	/* image width and height, in pixels */
	cinfo.image_height = fp->get_height();
	cinfo.input_components = fp->get_nr_components();		/* # of color components per pixel */
	
	cinfo.in_color_space = cinfo.input_components == 3 ? JCS_RGB : JCS_GRAYSCALE; 	/* colorspace of input image */
	/* Now use the library's routine to set default compression parameters.
	 * (You must set at least cinfo.in_color_space before calling this,
	 * since the defaults depend on the source color space.)
	 */
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	 * Here we just illustrate the use of quality (quantization table) scaling:
	 */
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */

	/* TRUE ensures that we will write a complete interchange-JPEG file.
	 * Pass TRUE unless you are very sure of what you're doing.
	 */
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 * To keep things simple, we pass one scanline per call; you can pass
	 * more if you wish, though.
	 */
	row_stride = dv.get_step_size(0);	/* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could pass
		 * more than one scanline at a time if that's more convenient.
		 */
		row_pointer[0] = const_cast<JSAMPROW>(dv.get_ptr<JSAMPLE>(cinfo.next_scanline));
		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	return true;
}

/// close image [stream]
bool jpg_writer::close()
{
	/* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);
	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	/* Step 7: release JPEG compression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

	/* And we're done! */
	return true;
}

cgv::base::object_registration<jpg_writer> jwr2("");
