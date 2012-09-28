#pragma once

#include <fstream>
#include <cgv/reflect/reflection_handler.h>
#include <cgv/reflect/reflect_enum.h>

#include "lib_begin.h"

namespace cgv {
	namespace data {

/// different error codes
enum IOReflectionError { RE_NO_ERROR, RE_FILE_OPEN_ERROR, RE_FILE_READ_ERROR, RE_FILE_WRITE_ERROR, RE_CONTENT_MISMATCH, RE_VERSION_MISMATCH };

extern CGV_API cgv::reflect::enum_reflection_traits<IOReflectionError> get_reflection_traits(const IOReflectionError&);

/** common base for all io reflection handlers */
class CGV_API io_reflection_handler : public cgv::reflect::reflection_handler
{
public:
protected:
	unsigned version;
	std::string file_content;

	bool in_header;
	IOReflectionError last_error;
public:
	///
	io_reflection_handler(const std::string& _content, unsigned _ver);
	///
	bool failed() const;
	///
	IOReflectionError get_error_code() const;
	///
	static std::string get_error_message(IOReflectionError ae);
	/// 
	bool reflect_method_void(const std::string& method_name, cgv::reflect::method_interface* mi_ptr,
						     cgv::reflect::abst_reflection_traits* return_traits, const std::vector<cgv::reflect::abst_reflection_traits*>& param_value_traits);
};


	}
}

#include <cgv/config/lib_end.h>