#ifdef WIN32
#pragma warning (disable:4996)
#endif

#include "bmp_reader.h"
#include <iostream>
#include <cgv/base/import.h>
#include <cgv/base/register.h>


namespace cgv {
	namespace media {
		namespace image {

/// default constructor
bmp_reader::bmp_reader() : fp(0) 
{
}

/// close file in destructor
bmp_reader::~bmp_reader()
{
	if (fp)
		fclose(fp);
	fp = 0;
}

/// overload to return the type name of this object
std::string bmp_reader::get_type_name() const
{
	return "bmp_reader";
}

/// construct a copy of the reader
abst_image_reader* bmp_reader::clone() const
{
	return new bmp_reader();
}

/// return a reference to the last error message
const std::string& bmp_reader::get_last_error() const
{
	return last_error;
}

/// return a string containing a colon separated list of extensions that can be read with this reader
const char* bmp_reader::get_supported_extensions() const
{
	return "bmp";
}

#define GET_2B(array,offset)  ( (unsigned int) array[offset] + (((unsigned int) array[offset+1]) << 8) )
#define GET_4B(array,offset)  ( (unsigned int) array[offset] + (((unsigned int) array[offset+1]) << 8) + \
							    (((unsigned int) array[offset+2]) << 16) + (((unsigned int) array[offset+3]) << 24) )

/// open the file and read the image header in order to determine the data format
bool bmp_reader::open(const std::string& file_name, data_format& df, std::vector<data_format>* palette_formats)
{
	if (fp != 0)
		close();
	palette.clear();

	fp = cgv::base::open_data_file(file_name.c_str(), "rb");
//	fp = fopen(file_name.c_str(), "rb");
	if (!fp) {
		last_error  = "could not open file: ";
		last_error += file_name;
		return false;
	}
	unsigned char bmp_header[14];
	if (fread(bmp_header, 1, 14, fp) != 14) {
		last_error = "could not read bmp header"; fclose(fp); fp = 0; return false;
	}
	if (GET_2B(bmp_header,0) != 0x4D42) {
		last_error = "bmp header does not start with BM"; fclose(fp); fp = 0; return false;
	}
	unsigned char bmp_info[64];
	if (fread(bmp_info, 1, 4, fp) != 4) {
		last_error = "could not read first 4 bytes of bmp info"; fclose(fp); fp = 0; return false; 
	}
	unsigned int info_size = GET_4B(bmp_info,0);
	if (!(info_size == 12 || info_size == 40 || info_size == 64)) { 
		last_error = "unknown size of bmp info"; fclose(fp); fp = 0; return false; 
	}
	if (fread(bmp_info+4, 1, info_size-4, fp) != info_size-4) {
		last_error = "could not read bmp info"; fclose(fp); fp = 0; return false; 
	}
	unsigned int width = 0, height = 0, planes = 1, bits_per_pixel = 1;
	switch ((int) info_size) {
	case 12: /* Decode OS/2 1.x header (Microsoft calls this a BITMAPCOREHEADER) */
		width     =      GET_2B(bmp_info,4);
		height    =      GET_2B(bmp_info,6);
		planes    =      GET_2B(bmp_info,8);
		bits_per_pixel = GET_2B(bmp_info,10);
		break;
	case 40:
	case 64:
		/* Decode Windows 3.x header (Microsoft calls this a BITMAPINFOHEADER) */
		/* or OS/2 2.x header, which has additional fields that we ignore */
		width              = GET_4B(bmp_info,4);
		height             = GET_4B(bmp_info,8);
		planes             = GET_2B(bmp_info,12);
		bits_per_pixel     = GET_2B(bmp_info,14);
		/*biXPelsPerMeter = GET_4B(bmp_info,24),  biYPelsPerMeter = GET_4B(bmp_info,28), biSizeImage, biClrImportant fields are ignored */
		if (GET_4B(bmp_info,16) != 0) {
			last_error = "compressed bmp files not supported"; fclose(fp); fp = 0; return false;
		}
		if (bits_per_pixel == 8 && GET_4B(bmp_info,32) != 256) {
			last_error = "only bmp palettes with 256 entries supported allowed"; fclose(fp); fp = 0; return false;
		}
		break;
	}	
	if (planes != 1) {
		last_error = "bad number of planes in bmp file"; fclose(fp); fp = 0; return false;
	}
	if (!(bits_per_pixel == 8 || bits_per_pixel == 24)) {
		last_error = "bad number of bits per pixel in bmp file"; fclose(fp); fp = 0; return false;
	}

	/* Compute distance to bitmap data --- will adjust for colormap below */
	int skip_bytes = GET_4B(bmp_header,10) - (info_size + 14);

	/* Read the colormap, if any */
	if (bits_per_pixel == 8) {
		int nr_entries = (info_size == 12) ? 3 : 4;
		palette.resize(nr_entries*256);
		if (fread(&palette[0], 1, nr_entries*256, fp) != (size_t)(nr_entries*256)) {
			last_error = "could not read bmp color map"; palette.clear(); fclose(fp); fp = 0; return false;
		}
		skip_bytes -= nr_entries*256;
		for (unsigned int i = 0; i < 256; ++i) {
			unsigned char tmp = palette[nr_entries*i];
			palette[3*i]   = palette[nr_entries*i+2];
			palette[3*i+1] = palette[nr_entries*i+1];
			palette[3*i+2] = tmp;
		}
	}
	/* Skip any remaining pad bytes */
	if (skip_bytes < 0) {
		last_error = "bmp incorrect bfOffBits value?"; palette.clear(); fclose(fp); fp = 0; return false;
	}
	while (--skip_bytes >= 0) {
		fgetc(fp);
	}
	if (bits_per_pixel == 8 && palette_formats) {
		df = data_format(width, height, TI_UINT8, "0");
		palette_formats->push_back(data_format(256, TI_UINT8, CF_RGB));
	}
	else
		df = data_format(width, height, TI_UINT8, CF_RGB);
	return true;
}

/// read the i-th palette in case of a paletted file format, the standard implementation returns false
bool bmp_reader::read_palette(unsigned int i, const data_view& dv)
{
	if (palette.empty()) {
		last_error = "bmp file has no palette";
		return false;
	}
	if (i > 0) {
		last_error = "only one palette in bmp file";
		return false;
	}
	std::copy(&palette[0], &palette[3*256], dv.get_ptr<unsigned char>());
	return true;
}

/// whether the reader supports per line reading (only valid after successful opening an image file
bool bmp_reader::supports_per_line_read() const
{
	return true;
}
/// read the next line into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done
bool bmp_reader::read_line(const data_format& df, const data_view& dv)
{
//	static unsigned char temp[4];
	unsigned char* data_ptr = dv.get_ptr<unsigned char>();
	unsigned int n = df.get_width();
	if (palette.empty()) {
		if (fread(data_ptr, 3, n, fp) != n) {
			last_error = "bmp read error"; return false;
		}
		for (unsigned short x = 0; x < n; ++x, data_ptr +=3)
			std::swap(data_ptr[0], data_ptr[2]);
		n *= 3;
	}
	else {
		if (fread(data_ptr, 1, n, fp) != n) {
			last_error = "bmp read error"; return false;
		}
		if (dv.get_format()->get_nr_components() == 3) {
			data_ptr += n;
			unsigned char* dest_ptr = data_ptr + 2*n;
			for (unsigned short x = 0; x < n; ++x) {
				--data_ptr;
				dest_ptr -= 3;
				const unsigned char* p = &palette[3*(*data_ptr)];
				dest_ptr[0] = p[0];
				dest_ptr[1] = p[1];
				dest_ptr[2] = p[2];
			}
		}
	}
	while ((n & 3) != 0) {
		fgetc(fp);
		n++;
	}
	return true;
}

/// read the whole image into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done. If image file has not been opened yet, open it and close it after reading
bool bmp_reader::read_image(const data_format& df, const data_view& dv)
{
	bool success = true;
	for (unsigned int y = 0; success && y < df.get_height(); ++y)
		success = read_line(df, dv(y));
	return success;
}
/// close the image file
bool bmp_reader::close()
{
	if (!fp) {
		last_error = "attempt to close a not open bmp file";
		return false;
	}
	int res = fclose(fp);
	if (res != 0)
		last_error = "closing of bmp file failed";
	fp = 0;
	return res == 0;
}

cgv::base::object_registration<bmp_reader> brr("");

		}
	}
}
