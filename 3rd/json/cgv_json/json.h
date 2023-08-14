#pragma once

/// This file provides implementation of basic json routines.

#include <fstream>

#include "../nlohmann/json.hpp"

namespace cgv {
namespace json {

/// @brief Read a file into a json instance. The file is expected to have json-formatted content.
/// This method does not check if the file exists.
/// 
/// @param [in] filename the json filename.
/// @param [out] optional return value for error message.
/// @return true if reading was successful, false otherwise.
static bool read_file(const std::string& filename, nlohmann::json& json, std::string* error = nullptr) {

	std::ifstream fs(filename);
	try {
		fs >> json;
	} catch (const std::exception& e) {
		if(error)
			*error = e.what();
		return false;
	}

	return true;
}

}
}
