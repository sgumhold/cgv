#pragma once

/// this header is dependency free

namespace cgv {
    namespace utils {

/// enum providing access to native endian of host system
enum class endian {
#if defined(_MSC_VER) && !defined(__clang__)
    little = 0, big = 1, native = little
#else
    little = __ORDER_LITTLE_ENDIAN__, big = __ORDER_BIG_ENDIAN__, native = __BYTE_ORDER__
#endif
};
namespace detail {
    void swap(uint8_t& a, uint8_t& b) { uint8_t t = a; a = b; b = t; }
    template <uint32_t N> void byte_swap(uint8_t* b);
    template <> void byte_swap<1>(uint8_t* b) {	}
    template <> void byte_swap<2>(uint8_t* b) { swap(b[0], b[1]); }
    template <> void byte_swap<4>(uint8_t* b) { swap(b[0], b[3]); swap(b[1], b[2]); }
    template <> void byte_swap<8>(uint8_t* b) { swap(b[0], b[7]); swap(b[1], b[6]); swap(b[2], b[5]); swap(b[3], b[4]); }

    template <typename T, bool no_swap = endian::big == endian::native>
    struct BE2native { static void exec(T& v) {}; };

    template <typename T> struct BE2native<T, false> {
        static void exec(T& v) { byte_swap<sizeof(T)>(reinterpret_cast<uint8_t*>(&v)); }
    };
}
/// convert value of a numeric type T from big endian to native endian of host
template <typename T> void big_endian_to_native(T& v) { detail::BE2native<T>::exec(v); }

    }
}
