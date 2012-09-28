#include "io_reflection_handler.h"
#include <cgv/utils/scan.h>

using namespace cgv::reflect;

namespace cgv {
	namespace data {

enum_reflection_traits<IOReflectionError> get_reflection_traits(const IOReflectionError&) { 
	return enum_reflection_traits<IOReflectionError>("RE_NO_ERROR, RE_FILE_OPEN_ERROR, RE_FILE_READ_ERROR, RE_FILE_WRITE_ERROR, RE_CONTENT_MISMATCH, RE_VERSION_MISMATCH"); 
}

io_reflection_handler::io_reflection_handler(const std::string& _content, unsigned _ver)
{
	file_content = _content;
	version = _ver;
	in_header = false;
	last_error = RE_NO_ERROR;
}

///
bool io_reflection_handler::failed() const 
{
	return last_error != RE_NO_ERROR; 
}
///
IOReflectionError io_reflection_handler::get_error_code() const 
{
	return last_error;
}
///
std::string io_reflection_handler::get_error_message(IOReflectionError re)
{
	static const char* error_messages[] =
	{
		"success",
		"file open error",
		"file read error",
		"file write error",
		"content mismatch",
		"version mismatch"
	};
	return error_messages[re];
}

/// 
bool io_reflection_handler::reflect_method_void(const std::string& method_name, method_interface* mi_ptr,
									 abst_reflection_traits* return_traits, const std::vector<abst_reflection_traits*>& param_value_traits)
{
	return true;
}



	}
}

