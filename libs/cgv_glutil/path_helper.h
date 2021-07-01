#pragma once

#include <string>
#include <cgv/defines/quote.h>

#include "lib_begin.h"

namespace cgv {
namespace glutil {

class CGV_API path_helper {
private:
	//static std::string ws2s(const std::wstring& wstr);

public:
	path_helper() = delete;
	~path_helper() = delete;

	static std::string get_executable_name();
	static std::string get_executable_path();
	// TODO: Do not use currently. This will return the input path of the cgv_glutil project.
	//static std::string get_input_path() { return QUOTE_SYMBOL_VALUE(INPUT_DIR); }
};

}
}

#include <cgv/config/lib_end.h>
