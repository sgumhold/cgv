#pragma once

#include <string>
#include <cgv/type/info/type_id.h>
#include <cgv/type/standard_types.h>

namespace cgv {
	namespace type {
		namespace info {

/// access value whos type is given by a TypeId
template <typename T>
struct type_access
{
	/// return stored value converted into type T
	static T get(const void* ptr, TypeId tid)
	{ 
		static T dummy = T();
		switch (tid) {
		case TI_BOOL  : return(T)*static_cast<const bool*>(ptr);
		case TI_INT8  : return(T)*static_cast<const int8_type*>(ptr);
		case TI_INT16 : return(T)*static_cast<const int16_type*>(ptr);
		case TI_INT32 : return(T)*static_cast<const int32_type*>(ptr);
		case TI_INT64 : return(T)*static_cast<const int64_type*>(ptr);
		case TI_UINT8  : return(T)*static_cast<const uint8_type*>(ptr);
		case TI_UINT16 : return(T)*static_cast<const uint16_type*>(ptr);
		case TI_UINT32 : return(T)*static_cast<const uint32_type*>(ptr);
		case TI_UINT64 : return(T)*static_cast<const uint64_type*>(ptr);
		case TI_FLT32  : return(T)*static_cast<const flt32_type*>(ptr);
		case TI_FLT64  : return(T)*static_cast<const flt64_type*>(ptr);
		default:
			return dummy;
		}
		return dummy;
	}
	/// convert value from type T and store it
	static bool set(void* ptr, TypeId tid, const T& v)
	{
		switch (tid) {
		case TI_BOOL  : *static_cast<bool*>(ptr) = (v != 0); break;
		case TI_INT8  : *static_cast<int8_type*>(ptr) = (int8_type) v; break;
		case TI_INT16 : *static_cast<int16_type*>(ptr) = (int16_type) v; break;
		case TI_INT32 : *static_cast<int32_type*>(ptr) = (int32_type) v; break;
		case TI_INT64 : *static_cast<int64_type*>(ptr) = (int64_type) v; break;
		case TI_UINT8  : *static_cast<uint8_type*>(ptr) = (uint8_type) v; break;
		case TI_UINT16 : *static_cast<uint16_type*>(ptr) = (uint16_type) v; break;
		case TI_UINT32 : *static_cast<uint32_type*>(ptr) = (uint32_type) v; break;
		case TI_UINT64 : *static_cast<uint64_type*>(ptr) = (uint64_type) v; break;
		case TI_FLT32  : *static_cast<flt32_type*>(ptr) = (flt32_type) v; break;
		case TI_FLT64  : *static_cast<flt64_type*>(ptr) = (flt64_type) v; break;
		default:
			return false;
		}
		return true;
	}
};

		}
	}
}