#pragma once
@exclude <cgv/config/ppp.ppp>
namespace cgv {
	namespace type {

/// use this type to mark the result of an invalid meta function
struct invalid_type {};

template <typename T>
struct is_valid
{
	static const int value = 1;
};
template <>
struct is_valid<invalid_type>
{
	static const int value = 0;
};

template <@["typename T1"; ", "; "typename T".N_ARG]>
struct count_valid_types
{
	static const int value = @["is_valid<T1>::value"; " + "; "is_valid<T".N_ARG.">::value"];
};

	}
}
