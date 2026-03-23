#include "gpmf.h"
#include "GPMF_mp4reader.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <array> 
#include <cgv/utils/endian.h>

namespace gpmf {
	/**** support for fourcc ****/
	bool operator == (uint32_t id, Key key)
	{
		return id == (const uint32_t&)key;
	}
	/**** support for char typed values ****/
	uint32_t type_size(char type)
	{
		switch (type) {
		case 'c':
		case 'u':
		case 'b':
		case 'B': return 1;
		case 'q':
		case 's':
		case 'S': return 2;
		case 'l':
		case 'L':
		case 'Q':
		case 'f':
		case 'F': return 4;
		case 'j':
		case 'J':
		case 'd': return 8;
		case 'G':
		case 'U': return 16;
		}
		return 0;
	}
	bool is_numeric(char type)
	{
		switch (type) {
		case 'b':
		case 'B':
		case 'q':
		case 's':
		case 'S':
		case 'l':
		case 'L':
		case 'Q':
		case 'f':
		case 'j':
		case 'J':
		case 'd': 
			return true;
		}
		return false;
	}
	uint32_t convert_to_native(char type, uint8_t* byte_ptr, uint32_t count)
	{
		uint32_t size = type_size(type);
		if (cgv::utils::endian::native != cgv::utils::endian::big) {
			switch (size) {
			case 2: for (uint32_t i = 0; i < count; ++i, byte_ptr += 2) cgv::utils::detail::byte_swap<2>(byte_ptr); break;
			case 4: for (uint32_t i = 0; i < count; ++i, byte_ptr += 4) cgv::utils::detail::byte_swap<4>(byte_ptr); break;
			case 8: for (uint32_t i = 0; i < count; ++i, byte_ptr += 8) cgv::utils::detail::byte_swap<8>(byte_ptr); break;
			}
		}
		return size;
	}
	template <int N>
	std::ostream& operator << (std::ostream& os, const std::array<char, N>& a)
	{
		std::string s(&std::get<0>(a), N);
		return os << "'" << s << "'";
	}
	template <typename F, typename T>
	void stream_out_as(std::ostream& os, const T* ptr, uint32_t count, uint32_t elems)
	{
		for (uint32_t i = 0; i < count; ++i) {
			if (i > 0)
				std::cout << ",";
			if (elems > 1)
				std::cout << "(";
			for (uint32_t j = 0; j < elems; ++j) {
				if (j > 0)
					std::cout << ",";
				os << F(*ptr++);
			}
			if (elems > 1)
				std::cout << ")";
		}
	}
	template <typename T>
	void stream_out(std::ostream& os, const T* ptr, uint32_t count, uint32_t elems)
	{
		stream_out_as<T>(os, ptr, count, elems);
	}
	uint32_t stream_out(std::ostream& os, char type, const uint8_t* byte_ptr, uint32_t count, uint32_t elems)
	{
		switch (type) {
		case 'c':
		case 'u':
		case 'q':
		case 'Q':
			std::cerr << "type 'c','u','q','Q','G','U' not supported yet in stream_out()" << std::endl;
			break;
		case 'U':
		case 'G' :stream_out(os, reinterpret_cast<const std::array<char, 16>*>(byte_ptr), count, elems); break;
		case 'F': stream_out(os, reinterpret_cast<const std::array<char,4>*>(byte_ptr), count, elems); break;
		case 'b': stream_out_as<int>(os, reinterpret_cast<const int8_t*>(byte_ptr), count, elems); break;
		case 'B': stream_out_as<unsigned>(os, reinterpret_cast<const uint8_t*>(byte_ptr), count, elems); break;
		case 's': stream_out(os, reinterpret_cast<const int16_t*>(byte_ptr), count, elems); break;
		case 'S': stream_out(os, reinterpret_cast<const uint16_t*>(byte_ptr), count, elems); break;
		case 'l': stream_out(os, reinterpret_cast<const int32_t*>(byte_ptr), count, elems); break;
		case 'L': stream_out(os, reinterpret_cast<const uint32_t*>(byte_ptr), count, elems); break;
		case 'j': stream_out(os, reinterpret_cast<const int64_t*>(byte_ptr), count, elems); break;
		case 'J': stream_out(os, reinterpret_cast<const uint64_t*>(byte_ptr), count, elems); break;
		case 'f': stream_out(os, reinterpret_cast<const float*>(byte_ptr), count, elems); break;
		case 'd': stream_out(os, reinterpret_cast<const double*>(byte_ptr), count, elems); break;
		}
		return type_size(type);
	}
	uint32_t convert_numeric_values(char type_in, const uint8_t* byte_in_ptr, char type_out, uint8_t* byte_out_ptr)
	{
		switch (type_out) {
		case 'b': return convert_numeric_value(type_in, byte_in_ptr, *reinterpret_cast<int8_t*>(byte_out_ptr));
		case 'B': return convert_numeric_value(type_in, byte_in_ptr, *reinterpret_cast<uint8_t*>(byte_out_ptr));
		case 's': return convert_numeric_value(type_in, byte_in_ptr, *reinterpret_cast<int16_t*>(byte_out_ptr));
		case 'S': return convert_numeric_value(type_in, byte_in_ptr, *reinterpret_cast<uint16_t*>(byte_out_ptr));
		case 'l': return convert_numeric_value(type_in, byte_in_ptr, *reinterpret_cast<int32_t*>(byte_out_ptr));
		case 'L': return convert_numeric_value(type_in, byte_in_ptr, *reinterpret_cast<uint16_t*>(byte_out_ptr));
		case 'j': return convert_numeric_value(type_in, byte_in_ptr, *reinterpret_cast<int64_t*>(byte_out_ptr));
		case 'J': return convert_numeric_value(type_in, byte_in_ptr, *reinterpret_cast<uint64_t*>(byte_out_ptr));
		case 'f': return convert_numeric_value(type_in, byte_in_ptr, *reinterpret_cast<float*>(byte_out_ptr));
		case 'd': return convert_numeric_value(type_in, byte_in_ptr, *reinterpret_cast<double*>(byte_out_ptr));
		}
		return 0;
	}
	/**** ios manipulation ****/
	int take::uid(std::ios_base& b) 
	{
		static int const uid = std::ios_base::xalloc();
		return uid;
	}	
	long& take::count(std::ios_base& b) 
	{
		return b.iword(uid(b));
	}	
	std::ostream& operator<<(std::ostream& o, take const& t) 
	{
		take::count(o) = t._count;
		return o;
	}
	const char*& complex_type::text(std::ios_base& b)
	{
		return const_cast<const char*&>(reinterpret_cast<char*&>(b.pword(take::uid(b))));
	}
	std::ostream& operator<<(std::ostream& o, complex_type const& ct)
	{
		complex_type::text(o) = ct._text;
		return o;
	}
	/**** key length value triples ****/
	uint32_t key_length_value_ptr::get_nr_elements() const
	{
		return size / type_size(type);
	}
	bool key_length_value_ptr::to_string(std::string& s) const
	{
		if (type != 'c')
			return false;
		auto char_ptr = reinterpret_cast<const char*>(value_ptr);
		s = std::string(char_ptr, size_t(repeat) * size);
		return true;
	}
	bool key_length_value_ptr::to_strings(std::vector<std::string>& ss) const
	{
		if (type != 'c')
			return false;
		const char* begin = reinterpret_cast<const char*>(value_ptr);
		for (unsigned i = 0; i<unsigned(repeat); ++i) {
			const char* end = begin + size - 1;
			while (end > begin && *end == 0)
				--end;
			ss.push_back(std::string(begin, end));
			begin += size;
		}
		return true;
	}
	uint32_t* key_length_value_ptr::construct_native(uint32_t* ptr, const std::string& complex_type)
	{
		id = *ptr;
		type = *reinterpret_cast<const char*>(ptr + 1);
		size = reinterpret_cast<const uint8_t*>(ptr + 1)[1];
		repeat = reinterpret_cast<const uint16_t*>(ptr + 1)[1];
		cgv::utils::big_endian_to_native(repeat);
		value_ptr = ptr + 2;
		if (type != 0) {
			if (type != '?') {
				uint32_t elems = get_nr_elements();
				convert_to_native(type, reinterpret_cast<uint8_t*>(value_ptr), repeat * elems);
			}
			else {
				if (!complex_type.empty()) {
					auto* byte_ptr = reinterpret_cast<uint8_t*>(value_ptr);
					for (uint32_t i = 0; i < repeat; ++i)
						for (uint32_t j = 0; j < complex_type.size(); ++j)
							byte_ptr += convert_to_native(complex_type[j], byte_ptr);
				}
			}
		}
		return value_ptr + (size*repeat + 3) / 4;
	}
	std::ostream& operator << (std::ostream& os, const key_length_value_ptr& klv)
	{
		os << klv.fourcc[0] << klv.fourcc[1] << klv.fourcc[2] << klv.fourcc[3] << " "
			<< (klv.type == 0 ? '.' : klv.type) << " " << (int)klv.size << " " << klv.repeat;
		if (klv.type == 0)
			return os;
		os << "=";
		if (klv.type == 'c' || klv.type == 'U' || klv.type == 'F') {
			std::string s(reinterpret_cast<const char*>(klv.value_ptr), size_t(klv.repeat) * klv.size);
			// treat special characters somehow
			for (size_t i = 0; i < s.length(); ++i) {
				switch (s[i]) {
				case '°': s[i] = '0'; break;
				case '²': s[i] = '2'; break;
				case '³': s[i] = '3'; break;
				case 'µ': s[i] = 'y'; break;
				}
			}
			if (klv.type == 'U')
				os << s.substr(6, 2) << ":" << s.substr(8, 2) << ":" << s.substr(10, 2) << "," << s.substr(13, 3) << " " << s.substr(4, 2) << "." << s.substr(2, 2) << ".20" << s.substr(0, 2);
			else {
				if (klv.size > 1) {
					for (int j = 0; j < klv.repeat; ++j) {
						if (j > 0)
							os << ",";
						// truncate shorter string instances
						unsigned count = klv.size;
						while (count > 1 && s[j * klv.size + count - 1] == 0)
							--count;
						os << s.substr(j * klv.size, count);
					}
				}
				else
					os << s;
			}
			return os;
		}
		long count = take::count(os) <= 0 ? klv.repeat : std::min(long(klv.repeat), take::count(os));
		auto* byte_ptr = reinterpret_cast<const uint8_t*>(klv.value_ptr);
		if (klv.type != '?')
			stream_out(os, klv.type, byte_ptr, count, klv.get_nr_elements());
		else {
			const char* ctype = complex_type::text(os);
			uint32_t elems = uint32_t(std::string(ctype).size());
			for (long i = 0; i < count; ++i) {
				if (i > 0)
					os << ',';
				if (elems > 1)
					os << '(';
				for (uint32_t j = 0; j < elems; ++j) {
					if (j > 0)
						os << ',';
					byte_ptr += stream_out(os, ctype[j], byte_ptr);
				}
				if (elems > 1)
					os << ')';
			}
		}
		return os;
	}
	/**** stream payloads ****/
	stream_payload::stream_payload()
	{
		temperature = std::numeric_limits<float>::quiet_NaN();
	}
	void stream_payload::construct_native(uint32_t* begin_ptr, uint32_t* end_ptr)
	{
		key_length_value_ptr klv;
		uint32_t total_samples = 0;
		while (begin_ptr < end_ptr) {
			uint32_t* next_ptr = klv.construct_native(begin_ptr, element_types);
			switch (klv.id) {
			case gpmf::Key::stream_name: klv.to_string(name); break;
			case gpmf::Key::units:
			case gpmf::Key::si_units: klv.to_strings(units); break;
			case gpmf::Key::scale: klv.to_numbers(scales); break;
			case gpmf::Key::tick: klv.to_number(tick); break;
			case gpmf::Key::tock: klv.to_number(tock); break;
			case gpmf::Key::time_stamp: klv.to_number(timestamp); break;
			case gpmf::Key::temperature_c: klv.to_number(temperature); break;
			case gpmf::Key::total_samples: klv.to_number(total_samples); break;
			case gpmf::Key::type: klv.to_string(element_types); break;
			case gpmf::Key::matrix: klv.to_numbers(matrix); break;
			case gpmf::Key::orientation_in:
			case gpmf::Key::orientation_out:
			case gpmf::Key::timing_offset:
			case gpmf::Key::time_stamps:
			case gpmf::Key::preformatted:
			case gpmf::Key::empty_payloads:
			case gpmf::Key::quantize:
			case gpmf::Key::version:
			case gpmf::Key::freespace:
			case gpmf::Key::remark:
				// std::cerr << "not handled key '" << std::string(klv.fourcc, 4) << "' in stream payload" << std::endl;
			default:
				additional_klvs[klv.id] = klv;
				break;
			}
			begin_ptr = next_ptr;
		}
		id = klv.id;
		nr_samples = klv.repeat;
		sample_size = klv.size;
		if (klv.type == '?') {
			assert(!element_types.empty());
			nr_elements = uint32_t(element_types.size());
			uint32_t element_offset = 0;
			element_offsets.resize(nr_elements);
			for (uint32_t i = 0; i < nr_elements; ++i) {
				element_offsets[i] = element_offset;
				element_offset += type_size(element_types[i]);
			}
			assert(sample_size == element_offset);
		}
		else {
			assert(element_types.empty());
			nr_elements = klv.get_nr_elements();
			element_types = std::string(1, klv.type);
			uint32_t element_size = type_size(klv.type);
			element_offsets.resize(nr_elements);
			for (uint32_t i = 0; i < nr_elements; ++i)
				element_offsets[i] = i * element_size;
		}
		value_ptr = klv.value_ptr;
	}
	bool stream_payload::has_temperature() const
	{
		return temperature != std::numeric_limits<float>::quiet_NaN();
	}
	std::string stream_payload::get_unit(unsigned element_index) const
	{
		assert(units.size() <= 1 || element_index < units.size());
		return units.size() == 1 ? units.front() : (has_units() ? units[element_index] : "");
	}
	double stream_payload::get_element_scale(unsigned element_index) const
	{
		assert(scales.size() <= 1 || element_index < scales.size());
		return scales.size() == 1 ? scales.front() : (has_scales() ? scales[element_index] : 1.0f);
	}
	const key_length_value_ptr* stream_payload::has_klv(uint32_t id) const {
		auto iter = additional_klvs.find(id);
		if (iter == additional_klvs.end())
			return 0;
		return &(iter->second);
	}
	char stream_payload::get_element_type(unsigned element_index) const
	{
		assert(element_types.size() == 1 || element_index < element_types.size());
		return element_types.size() == 1 ? element_types.front() : element_types[element_index];
	}
	
	/**** video meta data processor ****/
	void video_meta_data_accessor::free_payload()
	{
		if (payload_handle == 0)
			return;
		FreePayloadResource(mp4_handle, payload_handle);
		payload_size = 0;
		payload_data = 0;
		payload_handle = 0;
		payload_begin_time = payload_end_time = 0.0;
	}
	video_meta_data_accessor::video_meta_data_accessor()
	{
	}
	video_meta_data_accessor::~video_meta_data_accessor()
	{
		close();
	}
	bool video_meta_data_accessor::open(const std::string& file_name)
	{
		bool is_mp4 = false;
		// the following is simpler with cgv::utils::to_lower(cgv::utils::file::get_extension) 
		// but this would be only dependency on cgv_utils lib that we want to avoid
		size_t ext_start = file_name.rfind('.', file_name.length());
		if (ext_start != std::string::npos) {
			std::string ext = file_name.substr(ext_start + 1, file_name.length() - ext_start);
			for (char& c : ext)
				c = std::tolower(static_cast<unsigned char>(c));
			if (ext == "mp4")
				is_mp4 = true;
		}
		// open video file
		if (is_mp4)
			mp4_handle = OpenMP4Source(file_name.c_str(), MOV_GPMF_TRAK_TYPE, MOV_GPMF_TRAK_SUBTYPE, 0);
		else
			mp4_handle = OpenMP4SourceUDTA(file_name.c_str(), 0);
		if (mp4_handle == 0)
			return false;
		//
		meta_data_length = GetDuration(mp4_handle);
		nr_payloads = 0;
		if (meta_data_length > 0.0f) {
			// determine number of video frames and video frame rate
			uint32_t frame_numerator, frame_denominator;
			nr_video_frames = GetVideoFrameRateAndCount(mp4_handle, &frame_numerator, &frame_denominator);
			if (nr_video_frames > 0)
				video_fps = (float)frame_numerator / (float)frame_denominator;

			// iterate payloads
			nr_payloads = GetNumberPayloads(mp4_handle);
		}
		return true;
	}
	void video_meta_data_accessor::close()
	{
		free_payload();
		if (mp4_handle) {
			CloseSource(mp4_handle);
			mp4_handle = 0;
		}
	}
	uint32_t video_meta_data_accessor::get_payload_size(uint32_t payload_index) const
	{
		return GetPayloadSize(mp4_handle, payload_index);
	}
	uint32_t* video_meta_data_accessor::get_payload_end() const
	{
		return payload_data + (payload_size + 3) / 4;
	}
	bool video_meta_data_accessor::access_payload(uint32_t _payload_index)
	{
		free_payload();
		payload_index = _payload_index;
		payload_size = GetPayloadSize(mp4_handle, payload_index);
		payload_handle = GetPayloadResource(mp4_handle, payload_handle, payload_size);
		payload_data = GetPayload(mp4_handle, payload_handle, payload_index);
		if (payload_data == NULL) {
			payload_handle = payload_size = 0;
			return false;
		}
		uint32_t ret = GetPayloadTime(mp4_handle, payload_index, &payload_begin_time, &payload_end_time);
		if (ret != MP4_ERROR_OK) {
			free_payload();
			return false;
		}
		return true;
	}

	void ostream_payload_content(std::ostream& os, uint32_t* payload_ptr, const uint32_t* payload_end, int depth = 0)
	{
		key_length_value_ptr klv;
		std::string ctype;
		while (payload_ptr < payload_end) {
			uint32_t* next_ptr = klv.construct_native(payload_ptr, ctype);
			if (klv.id == Key::type) {
				klv.to_string(ctype);
				os << complex_type(ctype.c_str());
			}
			os << std::string(2 * depth, ' ') << klv << std::endl;
			if (klv.type == 0)
				ostream_payload_content(os, payload_ptr + 2, next_ptr, depth + 1);
			payload_ptr = next_ptr;
		}
	}
	bool ostream_meta_data(std::ostream& os, const std::string& file_name, uint32_t max_nr_payloads)
	{
		video_meta_data_accessor vmda;
		if (!vmda.open(file_name))
			return false;
		uint32_t nr = std::min(vmda.nr_payloads, max_nr_payloads);
		for (uint32_t i = 0; i < nr; ++i) {
			if (!vmda.access_payload(i))
				continue;
			os << "payload " << i << ": " << vmda.payload_size << " [" << vmda.payload_begin_time << "->"
				<< vmda.payload_end_time << "]" << std::endl;
			ostream_payload_content(os, vmda.payload_data, vmda.get_payload_end());
		}
		return true;
	}
	void read_payload_content(uint32_t* payload_ptr, const uint32_t* payload_end, 
		const video_meta_data_accessor& vmda, stream_payload_callback callback, void* user_data, int depth = 0)
	{
		key_length_value_ptr klv;
		std::string ctype;
		while (payload_ptr < payload_end) {
			uint32_t* next_ptr = klv.construct_native(payload_ptr, ctype);
			if (klv.id == Key::type)
				klv.to_string(ctype);
			if (klv.type == 0) {
				if (klv.id == Key::stream) {
					stream_payload spl;
					spl.construct_native(payload_ptr + 2, next_ptr);
					callback(vmda, spl, user_data);
				}
				else {
					read_payload_content(payload_ptr + 2, next_ptr, vmda, callback, user_data, depth + 1);
				}
			}
			payload_ptr = next_ptr;
		}
	}
	bool read_meta_data(const std::string& file_name, stream_payload_callback callback, void* user_data, uint32_t max_nr_payloads)
	{
		video_meta_data_accessor vmda;
		if (!vmda.open(file_name))
			return false;
		uint32_t nr = std::min(vmda.nr_payloads, max_nr_payloads);
		for (uint32_t i = 0; i < nr; ++i) {
			if (!vmda.access_payload(i))
				continue;
			read_payload_content(vmda.payload_data, vmda.get_payload_end(), vmda, callback,user_data);
		}
		return true;
	}
		
	void ostream_scaled_streams_callback(const video_meta_data_accessor& vmda, stream_payload& spl, void* user_data)
	{
		std::ostream& os = *reinterpret_cast<std::ostream*>(user_data);
		std::cout 
			<< std::string(spl.fourcc, 4) 
			<< "[" << vmda.payload_index << "|" << vmda.payload_begin_time << "," << vmda.payload_end_time << "] "
			<< spl.name << std::endl;
		uint32_t count = take::count(os);
		if (count == 0 || count > spl.nr_samples)
			count = spl.nr_samples;
		double value;
		for (uint32_t i = 0; i < count; ++i) {
			if (i > 0)
				os << ',';
			if (spl.nr_elements > 1)
				os << '[';
			for (uint32_t j = 0; j < spl.nr_elements; ++j) {
				if (is_numeric(spl.get_element_type(j))) {
					spl.put_scaled_value(i, j, value);
					if (j > 0)
						os << ',';
					os << value;
				}
			}
			if (spl.nr_elements > 1)
				os << ']';
		}
		os << std::endl;
	}
	/// uses read meta data to stream out all streams in meta data scaled to units
	bool ostream_scaled_streams(std::ostream& os, const std::string& file_name, uint32_t max_nr_payloads)
	{
		return read_meta_data(file_name, ostream_scaled_streams_callback, &os, max_nr_payloads);
	}
}
