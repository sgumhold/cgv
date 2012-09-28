#include "type_name.h"
#include <cgv/utils/scan.h>
namespace cgv {
	namespace type {
		/// namespace for templates that provide type information
		namespace info {

/// extract a type name from an type_info structure that does not contain the class, struct nor enum keywords
std::string extract_type_name(const std::type_info& ti)
{
	std::string res(ti.name());
	cgv::utils::replace(res, "class ", "");
	cgv::utils::replace(res, "struct ", "");
	cgv::utils::replace(res, "union ", "");
	cgv::utils::replace(res, "enum ", "");
	return res;
}


		}
	}
}