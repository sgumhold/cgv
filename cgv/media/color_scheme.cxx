#include "color_scheme.h"

namespace cgv {
namespace media {

continuous_color_scheme_registry& get_global_continuous_color_scheme_registry() {
	static continuous_color_scheme_registry registry;
	return registry;
}

discrete_color_scheme_registry& get_global_discrete_color_scheme_registry() {
	static discrete_color_scheme_registry registry;
	return registry;
}

} // namespace media
} // namespace cgv
