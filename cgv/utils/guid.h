#pragma once

#include <string>

#include "lib_begin.h"

namespace cgv {
    namespace utils {
        /// simple struct to represent guid
        struct guid
        {
            uint32_t d1;
            uint16_t d2, d3;
            uint8_t d4[8];
        };
        /// convert string to guid
        extern CGV_API guid guid_from_string(const std::string& str);
        /// convert guid to string
        extern CGV_API std::string to_string(const guid& _id);
    }
}
 