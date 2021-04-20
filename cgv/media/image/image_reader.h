#pragma once

#include <cgv/base/base.h>
#include <cgv/data/data_view.h>

#include "lib_begin.h"

using namespace cgv::data;

namespace cgv {
	namespace media {
		/// namespace for image processing
		namespace image {

class CGV_API image_reader;

/// abstract interface for image readers
class CGV_API abst_image_reader : public cgv::base::base
{
public:
	friend class image_reader;
	/// construct a copy of the reader
	virtual abst_image_reader* clone() const = 0;
	/// return a reference to the last error message
	virtual const std::string& get_last_error() const = 0;
	/// return a string containing a colon separated list of extensions that can be read with this reader
	virtual const char* get_supported_extensions() const = 0;
	/// open the file and read the image header in order to determine the data format
	virtual bool open(const std::string& file_name, data_format& df, std::vector<data_format>* palette_formats) = 0;
	/// whether the reader supports per line reading (only valid after successful opening an image file
	virtual bool supports_per_line_read() const = 0;
	/// whether the file can contain several images
	virtual bool supports_multiple_images() const;
	/// return the number of images in the file, what can cause the whole file to be scanned
	virtual unsigned get_nr_images() const;
	/// return the duration of the current image in seconds, if returned value is 0, no duration is available
	virtual float get_image_duration() const;
	/// return the index of the current image
	virtual unsigned get_current_image() const;
	/// jump to a specific image and return whether this was successful
	virtual bool seek_image(unsigned idx);
	//! read the i-th palette in case of a paletted file format, the standard implementation returns false
	/*! In case of files with animated images, the palette can change for each image. Therefore call this
	    method before each call to \c read_image(). */
	virtual bool read_palette(unsigned int i, const data_view& dv);
	/// read the next line into the given data pointer, set data format if not yet specified and allocate the data ptr if not yet done
	virtual bool read_line(const data_format& df, const data_view& dv) = 0;
	//! read an image into the given data pointer.
	/*! Only possible after successful open. 
	    If multiple images are contained in the file, this method can be called with success once for each contained image.
		The number of images can be determined despite of the method \c get_nr_images() by calling this method until it 
		returns false. */
	virtual bool read_image(const data_format& df, const data_view& dv) = 0;
	/// close the image file
	virtual bool close() = 0;

};

/** the image reader chooses a specific reader automatically based on 
    the extension of the given file name. In the open method it sets the
	 format passed by reference to the reader in the constructor. */
class CGV_API image_reader : public cgv::base::base
{
protected:
	/// store the data format
	data_format* file_format_ptr;
	/// store a pointer to the palette format vector
	std::vector<data_format>* palette_formats;
	/// store a pointer to the chosen reader
	abst_image_reader* rd;
	/// abstract interface for the setter, by default it simply returns false
	bool set_void(const std::string& property, const std::string& type, const void* value);
	/// abstract interface for the getter, by default it simply returns false
	bool get_void(const std::string& property, const std::string& type, void* value);
public:
	/** construct an image reader from a reference to a data format instance in which the format of the image file
	    will be stored after a call to the open method. If also images with palettes should be handled, provide also
		a vector of data formats for the palettes. By default no vector is provided such that the reader converts
		paletted image formats to non paletted ones. In case palettes are used, the components in the file_format
		will be '0', '1', ... for the components that reference the i-th palette. */
	image_reader(data_format& file_format, std::vector<data_format>* palette_formats = 0);
	~image_reader() {}
	/// overload to return the type name of this object
	std::string get_type_name() const;
	/// return a string with a list of supported extensions, where the list entries are separated with the passed character that defaults to a semicolon
	static const std::string& get_supported_extensions(char sep = ';');
	/// use this to generate a file_open_dialog or file_save_dialog
	static std::string construct_filter_string();
	/// return a semicolon separated list of property declarations of the form "name:type", by default an empty list is returned
	std::string get_property_declarations();
	/// return a reference to the last error message
	const std::string& get_last_error() const;
	/** read the whole image into the given data view. If the data view is
	    empty, set the format and allocate memory to hold the image data.
		 The newly allocate data pointer will belong to the data view afterwards.
		 If the data pointer of the data view is already allocated, use this in
		 the same way as the alternate version of read_image with the const
		 data_view argument does. */
	bool read_image(const std::string& file_name, data_view& dv, std::vector<data_view> *palettes = 0);
	/** read the image into the given data view that must have the correct 
	    format and an allocated data pointer. */
	bool read_image(const std::string& file_name, const data_view& dv, const std::vector<data_view> *palettes = 0);
	/// open the file and read the image header in order to determine the data format of the file, which is stored in the data format specified in the constructor
	bool open(const std::string& file_name);
	/// return the data format of the image file
	data_format* get_file_format() const;
	/// whether the file can contain several images
	bool supports_multiple_images() const;
	/// return the number of images in the file, what can cause the whole file to be scanned
	unsigned get_nr_images() const;
	/// return the duration of the current image in seconds, if returned value is 0, no duration is available
	float get_image_duration() const;
	/// return the index of the current image
	unsigned get_current_image() const;
	/// jump to a specific image and return whether this was successful
	bool seek_image(unsigned idx);
	//! read the i-th palette in case of a paletted file format, and handle the data view as in the read_image method the standard implementation returns false
	/*! In case of files with animated images, the palette can change for each image. Therefore call this
	    method before each call to \c read_image(). */
	bool read_palette(unsigned int i, data_view& dv);
	//! read the i-th palette in case of a paletted file format, and handle the data view as in the read_image method the standard implementation returns false
	/*! In case of files with animated images, the palette can change for each image. Therefore call this
	    method before each call to \c read_image(). */
	bool read_palette(unsigned int i, const data_view& dv);
	/// return whether the reader supports per line reading (only valid after successfully opening an image file)
	bool supports_per_line_read() const;
	/** read the next line into the given data view. If the data format 
		 of the view has not been specified, set it to the data format of 
		 the file, construct a 1d subview and allocate memory for a line
		 only. The data pointer is then owned by the view. */
	bool read_line(data_view& dv);
	/// read line into a preallocated data view of the correct format
	bool read_line(const data_view& dv);
	//! read an opened image data and palettes into a data views that are optionally allocated.
	/*! read an image from an opened image file into the given data view. 
		 If the data format of the view has not been specified, set it to 
		 the data format of the file. If the data pointer is empty, allocate 
		 enough memory to hold the image. The pointer is then owned by the
		 view. */
	bool read_image(data_view& dv, std::vector<data_view> *palettes = 0);
	/// read image to a data_view with the correct format and an allocated pointer
	bool read_image(const data_view& dv, const std::vector<data_view> *palettes = 0);
	/// close the image file
	bool close();
};

		}
	}
}

#include <cgv/config/lib_end.h>
