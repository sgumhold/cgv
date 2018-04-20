#pragma once

#include <cgv/render/context.h>
#include <cgv/reflect/reflect_enum.h>

#include <cgv_reflect_types/lib_begin.h>

#ifdef REFLECT_IN_CLASS_NAMESPACE
namespace cgv {
	namespace render {
#else
namespace cgv {
	namespace reflect {
#endif
		extern CGV_API cgv::reflect::enum_reflection_traits<cgv::render::IlluminationMode> get_reflection_traits(const cgv::render::IlluminationMode&);
		extern CGV_API cgv::reflect::enum_reflection_traits<cgv::render::CullingMode> get_reflection_traits(const cgv::render::CullingMode&);
		extern CGV_API cgv::reflect::enum_reflection_traits<cgv::render::MaterialSide> get_reflection_traits(const cgv::render::MaterialSide&);
	}
}

#include <cgv/config/lib_end.h>
