#include "guid.h"

namespace cgv {
    namespace utils {
        guid guid_from_string(const std::string& str)
        {
            guid id;
            sscanf(str.c_str(), "{%8x-%4hx-%4hx-%2hhx%2hhx-%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx}",
                   &id.d1, &id.d2, &id.d3, &id.d4[0], &id.d4[1], &id.d4[2], &id.d4[3], &id.d4[4], &id.d4[5], &id.d4[6], &id.d4[7]);
            return id;
        }
        std::string to_string(const guid& id)
        {
            char str[39];
            snprintf(str, sizeof(str), "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
                     id.d1, id.d2, id.d3, id.d4[0], id.d4[1], id.d4[2], id.d4[3], id.d4[4], id.d4[5], id.d4[6], id.d4[7]);
            return std::string(str);
        }
    }
}
