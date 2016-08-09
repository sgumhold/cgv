#include "tiff_reader.h"
#include <cgv/base/register.h>
#include <iostream>

#pragma warning (disable:4996)


void myErrorHandler(const char*, const char*, va_list)
{
}

void myErrorHandlerExt(thandle_t, const char*, const char*, va_list)
{
}

/// default constructor
tiff_reader::tiff_reader() : fp(0)
{
	allows_row_based = false;
	nr_images = -1;
	row = 0;
	TIFFSetErrorHandler(myErrorHandler);
	TIFFSetErrorHandlerExt(myErrorHandlerExt);
	TIFFSetWarningHandler(myErrorHandler);
	TIFFSetWarningHandlerExt(myErrorHandlerExt);
}

/// close file in destructor
tiff_reader::~tiff_reader()
{
	if (fp)
		TIFFClose(fp);
	fp = 0;
}
/// overload to return the type name of this object
std::string tiff_reader::get_type_name() const
{
	return "tiff_reader";
}
/// return the last error message
const std::string& tiff_reader::get_last_error() const
{
	return last_error;
}

/// construct a copy of the reader
abst_image_reader* tiff_reader::clone() const
{
	return new tiff_reader();
}
/// return a string containing a colon separated list of extensions that can be read with this reader
const char* tiff_reader::get_supported_extensions() const
{
	return "tif;tiff";
}

/// whether the file can contain several images
bool tiff_reader::supports_multiple_images() const
{
	return fp ? (TIFFCurrentDirectory(fp) > 0 || !TIFFLastDirectory(fp)) : true;
}

/// return the number of images in the file, what can cause the whole file to be scanned
unsigned tiff_reader::get_nr_images() const
{
	if (!fp)
		return 0;
	if (nr_images != -1)
		return nr_images;
	int current_dir = TIFFCurrentDirectory(fp);
	nr_images = 1;
	TIFFSetDirectory(fp, 0);
	while (!TIFFLastDirectory(fp)) {
		if (!TIFFReadDirectory(fp))
			break;
		++nr_images;
	}
	TIFFSetDirectory(fp, current_dir);
	return nr_images;
}

/// return the index of the current image
unsigned tiff_reader::get_current_image() const
{
	return (fp != 0) ? TIFFCurrentDirectory(fp) : -1;
}

/// jump to a specific image and return whether this was successful
bool tiff_reader::seek_image(unsigned idx)
{
	return (fp != 0) ? (TIFFSetDirectory(fp, idx) != 0) : false;
}

/// open the file and read the image header in order to determine the data format
bool tiff_reader::open(const std::string& file_name, data_format& df, std::vector<data_format>* palette_formats)
{
	fp = TIFFOpen(file_name.c_str(), "r");
	if (!fp)
		return false;

	nr_images = -1;

	uint32 width, height;		/* image width & height */	
	uint16 format, components, config, bps;
	TIFFGetField(fp, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(fp, TIFFTAG_IMAGELENGTH, &height);
	if (!TIFFGetField(fp, TIFFTAG_BITSPERSAMPLE, &bps))
		bps = 8;
	if (!TIFFGetField(fp, TIFFTAG_PLANARCONFIG, &config))
		config = PLANARCONFIG_CONTIG;
	if (!TIFFGetField(fp, TIFFTAG_SAMPLESPERPIXEL, &components))
		components = 1;
	if (!TIFFGetField(fp, TIFFTAG_SAMPLEFORMAT, &format))
		format = SAMPLEFORMAT_UINT;

	TIFFSetField(fp, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

	TypeId tid = TI_UINT8;
	switch (bps) {
	case  8 : 
		switch (format) {
		case SAMPLEFORMAT_INT : tid = TI_INT8; break;
		case SAMPLEFORMAT_UINT : tid = TI_UINT8; break;
		default: std::cerr << "float, void and complex types not supported by tiff reader for 8 bit" << std::endl;
			return false;
		}
		break;
	case 16 : 
		switch (format) {
		case SAMPLEFORMAT_INT : tid = TI_INT16; break;
		case SAMPLEFORMAT_UINT : tid = TI_UINT16; break;
		default: std::cerr << "float, void and complex types not supported by tiff reader for 16 bit" << std::endl;
			tid = TI_UINT16;
			return false;
		}
		break;
	case 32 : 
		switch (format) {
		case SAMPLEFORMAT_INT : tid = TI_INT32; break;
		case SAMPLEFORMAT_UINT : tid = TI_UINT32; break;
		case SAMPLEFORMAT_IEEEFP : tid = TI_FLT32; break;
		default: std::cerr << "void and complex types not supported by tiff reader" << std::endl;
			tid = TI_UINT32;
			return false;
		}
	default: std::cerr << "only bit depths 8, 16, and 32 supported by tiff reader" << std::endl;
		return false;
	}
	df = data_format(width,height,tid,components == 1 ? CF_L : CF_RGB);
	allows_row_based = config == PLANARCONFIG_CONTIG;
	row = 0;
	return true;
}
/// whether the reader supports per line reading (only valid after successful opening an image file
bool tiff_reader::supports_per_line_read() const
{
	return allows_row_based;
}
/// read the next line into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done
bool tiff_reader::read_line(const data_format& df, const data_view& dv)
{
	if (row >= df.get_height()) {
		if (!TIFFLastDirectory(fp))
			TIFFReadDirectory(fp);
		return false;
	}
	if (TIFFReadScanline(fp, dv.get_ptr<unsigned char>(), row) == 1) {
		if (++row == df.get_height()) {
			if (!TIFFLastDirectory(fp)) {
				TIFFReadDirectory(fp);
				row = 0;
			}
		}
		return true;
	}
	return false;
}
/// read the whole image into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done. If image file has not been opened yet, open it and close it after reading
bool tiff_reader::read_image(const data_format& df, const data_view& dv)
{
	if (!allows_row_based && (df.get_entry_size()/df.get_nr_components() == 1 && df.get_nr_components() >= 3)) {
		uint32* raster = (uint32*)_TIFFmalloc(df.get_width() * df.get_height() * sizeof (uint32));
		if (raster == 0) {
			last_error = "No space for raster buffer";
			return false;
		}

		/* Read the image in one chunk into an RGBA array */
		if (!TIFFReadRGBAImageOriented(fp, df.get_width(), df.get_height(), raster, ORIENTATION_TOPLEFT, 0)) {
			_TIFFfree(raster);
			last_error = "Error in reading RGBA image";
			return false;
		}
		/*
		 * XXX: raster array has 4-byte unsigned integer type, that is why
		 * we should rearrange it here.
		 */
#if HOST_BIGENDIAN
		TIFFSwabArrayOfLong(raster, df.get_size());
#endif
		int pixel_count = df.get_nr_entries();
		if (df.get_entry_size() == 3) {
			unsigned char *src, *dst;
			src = (unsigned char *) raster;
			dst = dv.get_ptr<unsigned char>();
			while (pixel_count > 0) {
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				src++, pixel_count--;
			}
		}
		else {
			memcpy(dv.get_ptr<unsigned char>(), raster, pixel_count*df.get_entry_size());
		}
		_TIFFfree( raster );
	}
	else {
		bool success = true;
		for (unsigned int y = 0; success && y < df.get_height(); ++y) {
			success = read_line(df, dv(df.get_height()-y-1));
		}
		return success;
	}
	if (!TIFFLastDirectory(fp))
		TIFFReadDirectory(fp);
	return true;
}
/// close the image file
bool tiff_reader::close()
{
	if (!fp)
		return false;
	TIFFClose(fp);
	fp = 0;
	return true;
}

cgv::base::object_registration<tiff_reader> prr("");
