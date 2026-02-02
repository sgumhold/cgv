#pragma once

#include <ostream>
#include <vector>
#include <string>
#include <cassert>

#define MAKEID(a,b,c,d)			(((d&0xff)<<24)|((c&0xff)<<16)|((b&0xff)<<8)|(a&0xff))

#include "lib_begin.h"

/// gpmf (GoPro Meta Format) namespace and library only depends on stl but uses the dll import export handling of CGV framework
namespace gpmf {

	/*@name support for fourcc*/
	//@{
	/// fourcc keys used by GoPro 
	enum class Key {
		// Internal Metadata structure and formatting tags
		device         = MAKEID('D', 'E', 'V', 'C'),//DEVC - nested device data to speed the parsing of multiple devices in post 
		device_id      = MAKEID('D', 'V', 'I', 'D'),//DVID - unique id per stream for a metadata source (in camera or external input) (single 4 byte int)
		device_name    = MAKEID('D', 'V', 'N', 'M'),//DVNM - human readable device type/name (char string)
		stream         = MAKEID('S', 'T', 'R', 'M'),//STRM - nested channel/stream of telemetry data
		stream_name    = MAKEID('S', 'T', 'N', 'M'),//STNM - human readable telemetry/metadata stream type/name (char string)
		si_units       = MAKEID('S', 'I', 'U', 'N'),//SIUN - Display string for metadata units where inputs are in SI units "uT","rad/s","km/s","m/s","mm/s" etc.
		units          = MAKEID('U', 'N', 'I', 'T'),//UNIT - Freeform display string for metadata units (char string like "RPM", "MPH", "km/h", etc)
		matrix         = MAKEID('M', 'T', 'R', 'X'),//MTRX - 2D matrix for any sensor calibration.
		orientation_in = MAKEID('O', 'R', 'I', 'N'),//ORIN - input 'n' channel data orientation, lowercase is negative, e.g. "Zxy" or "ABGR".
		orientation_out= MAKEID('O', 'R', 'I', 'O'),//ORIO - output 'n' channel data orientation, e.g. "XYZ" or "RGBA".
		scale          = MAKEID('S', 'C', 'A', 'L'),//SCAL - divisor for input data to scale to the correct units.
		type           = MAKEID('T', 'Y', 'P', 'E'),//TYPE - Type define for complex data structures
		total_samples  = MAKEID('T', 'S', 'M', 'P'),//TSMP - Total Sample Count including the current payload
		tick           = MAKEID('T', 'I', 'C', 'K'),//TICK - Beginning of data timing (arrival) in milliseconds. 
		tock           = MAKEID('T', 'O', 'C', 'K'),//TOCK - End of data timing (arrival)  in milliseconds. 
		time_offset    = MAKEID('T', 'I', 'M', 'O'),//TIMO - Time offset of the metadata stream that follows (single 4 byte float)
		timing_offset  = MAKEID('T', 'I', 'M', 'O'),//TIMO - duplicated, as older code might use the other version of TIMO
		time_stamp     = MAKEID('S', 'T', 'M', 'P'),//STMP - Time stamp for the first sample.
		time_stamps    = MAKEID('S', 'T', 'P', 'S'),//STPS - Stream of all the timestamps delivered (Generally don't use this. This would be if your sensor has no periodic times, yet precision is required, or for debugging.)
		preformatted   = MAKEID('P', 'F', 'R', 'M'),//PFRM - GPMF data
		temperature_c  = MAKEID('T', 'M', 'P', 'C'),//TMPC - Temperature in Celsius
		empty_payloads = MAKEID('E', 'M', 'P', 'T'),//EMPT - Payloads that are empty since the device start (e.g. BLE disconnect.)
		quantize       = MAKEID('Q', 'U', 'A', 'N'),//QUAN - quantize used to enable stream compression - 1 -  enable, 2+ enable and quantize by this value
		version        = MAKEID('V', 'E', 'R', 'S'),//VERS - version of the metadata stream (debugging)
		freespace      = MAKEID('F', 'R', 'E', 'E'),//FREE - n bytes reserved for more metadata added to an existing stream
		remark         = MAKEID('R', 'M', 'R', 'K'),//RMRK - adding comments to the bitstream (debugging)

		gps5           = MAKEID('G', 'P', 'S', '5'),       // GPS5 (Lat., Long., Alt., 2D speed, 3D speed)
		magnetometer   = MAKEID('M', 'A', 'G', 'N'),       // Magnetometer
		accelerometer  = MAKEID('A', 'C', 'C', 'L'),       // Accelerometer
		gyroscope      = MAKEID('G', 'Y', 'R', 'O'),       // Gyroscope
		exposure_time  = MAKEID('S', 'H', 'U', 'T'),	   // Exposure time (shutter speed)
		white_bal_temp = MAKEID('W', 'B', 'A', 'L'),	   // White Balance temperature (Kelvin)
		white_bal_RGB_gains = MAKEID('W', 'R', 'G', 'B'),  // White Balance RGB gains
		ISO            = MAKEID('I', 'S', 'O', 'E'),	   // Sensor ISO
		ISO_gain       = MAKEID('I', 'S', 'O', 'G'),	   // Sensor gain (ISO x100)
		avg_luminance  = MAKEID('Y', 'A', 'V', 'G'),	   // Average luminance
		img_uniformity = MAKEID('U', 'N', 'I', 'F'),	   // Image uniformity
		scene_class    = MAKEID('S', 'C', 'E', 'N'),	   // Scene classification[[CLASSIFIER_FOUR_CC,prob], ...]
		predominant_hue= MAKEID('H', 'U', 'E', 'S'),	   // Predominant hue[[hue, weight], ...]
		faces          = MAKEID('F', 'A', 'C', 'E'),	   // Face Coordinates and details
		cam_orientation= MAKEID('C', 'O', 'R', 'I'),	   // CameraOrientation
		img_orientation= MAKEID('I', 'O', 'R', 'I'),	   // ImageOrientation
		gravity        = MAKEID('G', 'R', 'A', 'V'),	   // Gravity Vector
		wind_proc      = MAKEID('W', 'N', 'D', 'M'),	   // Wind Processing[wind_enable, meter_value(0 - 100)]
		microphone_wet = MAKEID('M', 'W', 'E', 'T'),	   // Microphone Wet[mic_wet, all_mics, confidence]
		AGC_audio_level= MAKEID('A', 'A', 'L', 'P'),	   // AGC audio level[rms_level ,peak_level]

		end            = 0                          //(null)
	};
	/// comparison of int to Key
	extern CGV_API bool operator == (uint32_t id, Key key);
	//@}

	/*@name support of values where type is defined by a char.
	    Numeric Types:
		  Integer Types:
	        'b' : int8_t,  'B' : uint8_t, 's' : int16_t, 'S' : uint16_t,
		    'l' : int32_t, 'L' : uint32_t, 'j' : int64_t, 'J' : uint64_t,
		  Float Type
		    'q' : fixflt16,'Q' : fixflt32, 'f' : float,   'd' : double,
		Additional Type:
		  'c' : ASCII text string, 
		  'u' : UTF-8 formatted text string,
		  'F' : fourcc
		  'U' : 16 byte UTC Time format yymmddhhmmss.sss,
		  'G' : 128-bit ID (like UUID)
		None Types:
		  '?' : complex type,
		  '#' : Huffman compression STRM payloads.  
		        4-CC <type><size><rpt> <data ...> is compressed as 
				4-CC '#'<new size/rpt> <type><size><rpt> <compressed data ...>
		  0   : nested type
		*/
	//@{
	/// byte size of value of \c type
	extern CGV_API uint32_t type_size(char type);
	/// returns whether \c type is numeric and therefore supports access to scaled value
	extern CGV_API bool is_numeric(char type);
	/// convert typed value[s] from big endian to native endian and return type size
	extern CGV_API uint32_t convert_to_native(char type, uint8_t* byte_ptr, uint32_t count = 1);
	/// stream out [comma separated] value[s] or if \c elems > 1 value tuples enclosded in parantheses to \c os
	extern CGV_API uint32_t stream_out(std::ostream& os, char type, const uint8_t* byte_ptr, uint32_t count = 1, uint32_t elems = 1);
	/// convert value of numeric \c type at \c value_ptr to \c value of numeric type \c T and return byte size of \c type to simplify advancing \c value_ptr
	template <typename T>
	uint32_t convert_numeric_value(char type, const uint8_t* byte_ptr, T& value) {
		switch (type) {
		case 'b': value = T(*reinterpret_cast<const int8_t*>(byte_ptr)); return 1;
		case 'B': value = T(*reinterpret_cast<const uint8_t*>(byte_ptr)); return 1;
		case 's': value = T(*reinterpret_cast<const int16_t*>(byte_ptr)); return 2;
		case 'S': value = T(*reinterpret_cast<const uint16_t*>(byte_ptr)); return 2;
		case 'l': value = T(*reinterpret_cast<const int32_t*>(byte_ptr)); return 4;
		case 'L': value = T(*reinterpret_cast<const uint32_t*>(byte_ptr)); return 4;
		case 'j': value = T(*reinterpret_cast<const int64_t*>(byte_ptr)); return 8;
		case 'J': value = T(*reinterpret_cast<const uint64_t*>(byte_ptr)); return 8;
		case 'f': value = T(*reinterpret_cast<const float*>(byte_ptr)); return 4;
		case 'd': value = T(*reinterpret_cast<const double*>(byte_ptr)); return 8;
		}
		return 0;
	}
	/// convert from value of numeric \c type_in at \c value_in_ptr to value of numeric \c type_out at \c value_out_ptr and return byte size of \c type_in to simplify advancing \c value_in_ptr
	extern CGV_API uint32_t convert_numeric_values(char type_in, const uint8_t* byte_in_ptr, char type_out, uint8_t* byte_out_ptr);
	//@}

	//! custom ios manipulator to store limitation count for streaming out klv repeated values
	/*! typical use:
	    key_length_value klv;
		klv.construct_native(data_ptr);
		std::cout << gpmf::take(3) << klv << std::endl;
	*/
	class CGV_API take {
	public:
		/// access to repeat count of stream
		static long& count(std::ios_base& b);
		/// construct repeat count object passable to ostream
		explicit constexpr take(unsigned n) : _count(n) { }
	protected:
		static int uid(std::ios_base& b);
		friend class complex_type;
	private:
		unsigned const _count;
		extern CGV_API friend std::ostream& operator<<(std::ostream& o, take const& t);
	};
	//! custom ios manipulator to store complex type information for streaming out klv values
	/*! typical use:
		key_length_value klv;
		klv.construct_native(data_ptr);
		std::cout << gpmf::complex_type("lllf") << klv << std::endl;
	*/
	class CGV_API complex_type {
	public:
		/// access to complex type string
		static const char*& text(std::ios_base& b);
		/// construct repeat count object passable to ostream
		explicit constexpr complex_type(const char* t) : _text(t) { }
	private:
		const char* _text;
		extern CGV_API friend std::ostream& operator<<(std::ostream& o, complex_type const& ct);
	};

	/// a struct to represent key length value triple (klv) with value represented as a pointer
	struct CGV_API key_length_value_ptr
	{
		union {
			uint32_t id;
			char fourcc[4];
		};
		char type;
		uint8_t size;
		uint16_t repeat;
		uint32_t* value_ptr = 0;
		/// based on size and type_size(type) compute number elements per sample
		uint32_t get_nr_elements() const;
		/// convert to single string
		bool to_string(std::string& s) const;
		/// convert to multiple strings
		bool to_strings(std::vector<std::string>& ss) const;
		/// convert to number
		template <typename T> uint32_t to_number(T& nb) const {
			return convert_numeric_value(type, reinterpret_cast<const uint8_t*>(value_ptr), nb);
		}
		/// convert to numbers
		template <typename T>
		uint32_t to_numbers(std::vector<T>& ns) const {
			uint32_t count = repeat * get_nr_elements(), ret = 0;
			ns.resize(count);
			const auto* byte_ptr = reinterpret_cast<const uint8_t*>(value_ptr);
			for (uint32_t i = 0; i < count; ++i)
				byte_ptr += ret = convert_numeric_value(type, byte_ptr, ns[i]);
			return ret;
		}
		/// construct klv triple from memory converting the pointed to value to native format
		uint32_t* construct_native(uint32_t* ptr, const std::string& complex_type = "");
		/// operator to stream out klv, use repeat 
		friend std::ostream& operator << (std::ostream& os, const key_length_value_ptr& klv);
	};

	/// struct to represent a payload of a specific gpmf stream
	struct CGV_API stream_payload
	{
		union {
			char fourcc[4] = { 0,0,0,0 };
			uint32_t id;
		};
		std::string name;
		uint32_t tick = uint32_t(-1);
		uint32_t tock = uint32_t(-1);
		uint64_t timestamp = uint64_t(-1);
		float temperature;
		std::vector<std::string> units;
		std::vector<double> scales;
		std::vector<double> matrix;
		std::string element_types;
		uint32_t nr_samples = 0;
		uint32_t nr_elements = 1;
		uint32_t sample_size;
		std::vector<uint32_t> element_offsets;
		uint32_t* value_ptr;
		/// construct empty stream payload
		stream_payload();
		/// construct stream payload from memory range by constructing and analyzing native klv
		void construct_native(uint32_t* begin_ptr, uint32_t* end_ptr);
		bool has_complex_type() const { return element_types.size() > 1; }
		bool has_matrix() const { return !matrix.empty(); }
		bool has_tick() const { return tick != uint32_t(-1); }
		bool has_tock() const { return tock != uint32_t(-1); }
		bool has_timestamp() const { return timestamp != uint64_t(-1); }
		bool has_temperature() const;
		bool has_units() const { return !units.empty(); }
		std::string get_unit(unsigned element_index = 0) const;
		bool has_scales() const { return !scales.empty(); }
		double get_element_scale(unsigned element_index = 0) const;
		char get_element_type(unsigned element_index = 0) const;
		/// put single sample element value into \c dest_value, what only works if this stream_payload has no matrix
		template <typename T>
		void put_scaled_value(uint32_t sample_index, uint32_t element_index, T& dest_value) const {
			char type = get_element_type(element_index);
			assert(is_numeric(type));
			const auto* byte_ptr = reinterpret_cast<const uint8_t*>(value_ptr) + 
				sample_size * sample_index + element_offsets[element_index];
			convert_numeric_value(type, byte_ptr, dest_value);
			if (has_scales())
				dest_value /= T(get_element_scale(element_index));
			assert(!has_matrix());
		}
		/// put scaled values of samples in \c dest_value_ptr which needs to have alocated nr_samples*nr_elements values
		template <typename T>
		void put_scaled_samples(T* dest_value_ptr, uint32_t max_sample_count = -1) const {
			const auto* byte_ptr = reinterpret_cast<const uint8_t*>(value_ptr);
			uint32_t sample_count = std::min(max_sample_count, nr_samples);
			for (uint32_t i = 0; i < sample_count; ++i) {
				for (uint32_t j = 0; j < nr_elements; ++j) {
					byte_ptr += convert_numeric_value(get_element_type(j), byte_ptr, *dest_value_ptr);
					if (has_scales())
						*dest_value_ptr /= T(get_element_scale(j));
					++dest_value_ptr;
				}
				if (has_matrix()) {
					assert(matrix.size() == nr_elements * nr_elements);
					T* v = dest_value_ptr - nr_elements;
					std::vector<T> tmp(nr_elements, T(0));
					for (uint32_t y = 0; y < nr_elements; ++y)
						for (uint32_t x = 0; x < nr_elements; ++x)
							tmp[y] += T(matrix[y * nr_elements + x]) * v[x];
					for (uint32_t x = 0; x < nr_elements; ++x)
						v[x] = tmp[x];
				}
			}
		}
	};

	/// helper class zu access meta data in a video file
	class CGV_API video_meta_data_accessor
	{
	private:
		size_t mp4_handle = 0;
		size_t payload_handle = 0;
		void free_payload();
	public:
		/// information about video file set after open succeeded
		float meta_data_length = 0.0f;
		float video_fps = 0.0f;
		uint32_t nr_video_frames = 0;
		uint32_t nr_payloads = 0;

		/// information about current payload set after call to access_payload succeeded
		double payload_begin_time = 0.0;
		double payload_end_time = 0.0;
		uint32_t payload_index = 0;
		uint32_t payload_size = 0;
		uint32_t* payload_data = 0;

		/// construct meta data accessor
		video_meta_data_accessor();
		/// destructor releases potentially allocated payload and closes video file
		~video_meta_data_accessor();
		/// open a mp4 or a move file
		bool open(const std::string& file_name);
		/// close video file explicitly (destructor does this automatically)
		void close();
		/// query size of a specific payload without reading it
		uint32_t get_payload_size(uint32_t payload_index) const;
		/// convenience function to compute pointer to end of payload data
		uint32_t* get_payload_end() const;
		/// read a certain payload and set playload_size and payload_data members
		bool access_payload(uint32_t payload_index);
	};

	/// stream out meta data of file to given stream
	extern CGV_API bool ostream_meta_data(std::ostream& os, const std::string& file_name, uint32_t max_nr_payloads = -1);
	/// callback type for handling stream payloads while reading meta data
	typedef void (*stream_payload_callback)(const video_meta_data_accessor& vmda, stream_payload& spl, void* user_data);
	/// read meta data and pass stream payloads with payload information in video meta data accessor to callback
	extern CGV_API bool read_meta_data(const std::string& file_name, stream_payload_callback callback, void* user_data = 0, uint32_t max_nr_payloads = -1);
	/// uses read meta data to stream out all streams in meta data scaled to units
	extern CGV_API bool ostream_scaled_streams(std::ostream& os, const std::string& file_name, uint32_t max_nr_payloads = -1);
}
#include <cgv/config/lib_end.h>