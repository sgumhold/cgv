#pragma once

#include <string>
#include <vector>
#include "lib_begin.h"

namespace cgv {
	namespace gui {
		/// tell the user something with a message box
		extern CGV_API void message(const std::string& _message);
		/// ask the user with \c question to select one of the \c answers, where \c default_answer specifies index of default answer
		extern CGV_API int question(const std::string& question, const std::vector<std::string>& answers, int default_answer = -1);
		/// second question interface, where \c answers is a comma seprated list of enum declaration
		extern CGV_API int question(const std::string& question, const std::string& answers, int default_answer = -1);
	}
}

#include <cgv/config/lib_end.h>