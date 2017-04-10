#include "context.h"
#include <cgv/reflect/set_reflection_handler.h>

#ifdef REFLECT_IN_CLASS_NAMESPACE
namespace cgv {
	namespace render {
#else
namespace cgv {
	namespace reflect {
#endif
		cgv::reflect::enum_reflection_traits<cgv::render::IlluminationMode> get_reflection_traits(const cgv::render::IlluminationMode&)
		{
			return cgv::reflect::enum_reflection_traits<cgv::render::IlluminationMode>("OFF,ONESIDED,TWOSIDED");
		}
		cgv::reflect::enum_reflection_traits<cgv::render::CullingMode> get_reflection_traits(const cgv::render::CullingMode&)
		{
			return cgv::reflect::enum_reflection_traits<cgv::render::CullingMode>("OFF,BACKFACE,FRONTFACE");
		}
		cgv::reflect::enum_reflection_traits<cgv::render::MaterialSide> get_reflection_traits(const cgv::render::MaterialSide&)
		{
			return cgv::reflect::enum_reflection_traits<cgv::render::MaterialSide>("NONE,FRONT,BACK,FRONT_AND_BACK");
		}

	}
}

