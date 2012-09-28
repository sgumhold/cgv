#include "variant.h"

using cgv::utils::to_string;
using namespace cgv::type::info;

namespace cgv {
	namespace type {

void assign_variant(const std::string& dst_value_type, void* dst_value_ptr, 
						  const std::string& src_value_type, const void* src_value_ptr)
{
		if (dst_value_type == get_type_name(TI_BOOL))
			get_variant(*static_cast<bool*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_INT8))
			get_variant(*static_cast<int8_type*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_INT16))
			get_variant(*static_cast<int16_type*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_INT32))
			get_variant(*static_cast<int32_type*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_INT64))
			get_variant(*static_cast<int64_type*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_UINT8))
			get_variant(*static_cast<uint8_type*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_UINT16))
			get_variant(*static_cast<uint16_type*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_UINT32))
			get_variant(*static_cast<uint32_type*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_UINT64))
			get_variant(*static_cast<uint64_type*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_FLT32))
			get_variant(*static_cast<flt32_type*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_FLT64))
			get_variant(*static_cast<flt64_type*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_WCHAR))
			get_variant(*static_cast<wchar_type*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_STRING))
			get_variant(*static_cast<std::string*>(dst_value_ptr), src_value_type, src_value_ptr);
		else if (dst_value_type == get_type_name(TI_WSTRING))
			get_variant(*static_cast<std::wstring*>(dst_value_ptr), src_value_type, src_value_ptr);
}

	}
}