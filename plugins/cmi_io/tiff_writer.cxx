#include "tiff_writer.h"
#include <cgv/base/register.h>
#include <iostream>
#include <memory.h>

#pragma warning (disable:4996)

/// default constructor
tiff_writer::tiff_writer() : fp(0)
{
	allows_row_based = false;
	row = 0;
	nr_images = 0;
}

/// close file in destructor
tiff_writer::~tiff_writer()
{
	if (fp)
		TIFFClose(fp);
	fp = 0;
}
/// overload to return the type name of this object
std::string tiff_writer::get_type_name() const
{
	return "tiff_writer";
}
/// return the last error message
const std::string& tiff_writer::get_last_error() const
{
	return last_error;
}

/// construct a copy of the reader
abst_image_writer* tiff_writer::clone() const
{
	return new tiff_writer();
}
/// return a string containing a colon separated list of extensions that can be read with this reader
const char* tiff_writer::get_supported_extensions() const
{
	return "tif;tiff";
}

/// check if the chosen writer supports the given component format
bool tiff_writer::is_format_supported(const component_format& cf, const std::vector<component_format>* palette_formats) const
{
	if (palette_formats)
		return false;

	switch (cf.get_component_type()) {
	case TI_UINT8 :
	case TI_UINT16 :
	case TI_UINT32 :
	case TI_INT8 :
	case TI_INT16 :
	case TI_INT32 :
	case TI_FLT32 :
		break;
	default:
		return false;
	}
	return true;
//	if (cf.get_nr_components() != 2 ||
}

/// return whether multiple images are supported, default implementation returns false
bool tiff_writer::supports_multiple_images() const
{
	return true;
}

/// open image file to write
bool tiff_writer::open(const std::string& file_name)
{
	// open tiff file
	fp = TIFFOpen(file_name.c_str(), "w");
	if (fp == NULL) {
		last_error  = "cannot open tif-file ";
		last_error += file_name+" for writing";
		return false;
	}
	nr_images = 0;
	return true;
}

/// open the file and read the image header in order to determine the data format
bool tiff_writer::write_image(const const_data_view& dv, const std::vector<const_data_view>* palettes, double duration)
{
	if (nr_images > 0)
		TIFFWriteDirectory(fp);

	TIFFSetField(fp, TIFFTAG_IMAGEWIDTH, (uint32) dv.get_format()->get_width());
	TIFFSetField(fp, TIFFTAG_IMAGELENGTH, (uint32) dv.get_format()->get_height());
	TIFFSetField(fp, TIFFTAG_BITSPERSAMPLE, (uint32) 8*get_type_size(dv.get_format()->get_component_type()));
	TIFFSetField(fp, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	uint32 format;
	switch (dv.get_format()->get_component_type()) {
	case TI_UINT8 :
	case TI_UINT16 :
	case TI_UINT32 : format = SAMPLEFORMAT_UINT; break;
	case TI_INT8 :
	case TI_INT16 :
	case TI_INT32 : format = SAMPLEFORMAT_INT; break;
	case TI_FLT32 : format = SAMPLEFORMAT_IEEEFP; break;
	}
	TIFFSetField(fp, TIFFTAG_SAMPLEFORMAT, format);
	switch (dv.get_format()->get_standard_component_format()) {
	case CF_RGB :
	case CF_RGBA :
		TIFFSetField(fp, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		break;
	}
	TIFFSetField(fp, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	TIFFSetField(fp, TIFFTAG_SAMPLESPERPIXEL, (uint32) dv.get_format()->get_nr_components());
//	TIFFSetField(fp, TIFFTAG_MINSAMPLEVALUE, (uint16) 0);
//	TIFFSetField(fp, TIFFTAG_MAXSAMPLEVALUE, (uint16) (pow2(bitsPerSample())-1));
	TIFFSetField(fp, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(fp, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(fp, (uint32) -1));


	tdata_t buf = _TIFFmalloc(TIFFScanlineSize(fp));
	const unsigned char* data_ptr = dv.get_ptr<unsigned char>();
	unsigned int delta = dv.get_step_size(0);
	for (unsigned int y = 0; y < dv.get_format()->get_height(); ++y) {
		memcpy(buf, data_ptr, delta);
		if (TIFFWriteScanline(fp, buf, y, 0) < 0) {
			_TIFFfree(buf);
			return false;
		}
		data_ptr += delta;
	}
	_TIFFfree(buf);
	++nr_images;
	return true;
}

/// close image [stream]
bool tiff_writer::close()
{
	TIFFClose(fp);
	return true;
}

cgv::base::object_registration<tiff_writer> tiff_writer_reg("");
