#include "named_color_schemes.h"

namespace cgv {
namespace media {

size_t load_continuous_color_scheme_presets(continuous_color_scheme_registry& registry, const std::set<ColorSchemeType>& types) {
	using namespace schemes;
	
	continuous_color_scheme_registry_helper r(registry, types);

	if(r.can_load(ColorSchemeType::kSequential)) {
		// sequential schemes (single-hue)
		r.load("blues", interpolateBlues());
		r.load("greens", interpolateGreens());
		r.load("grays", interpolateGrays());
		r.load("oranges", interpolateOranges());
		r.load("purples", interpolatePurples());
		r.load("reds", interpolateReds());
		// sequential schemes (multi-hue)
		r.load("bone", interpolateBone());
		r.load("hot", interpolateHot());
		r.load("jet", interpolateJet());
		r.load("turbo", interpolateTurbo());
		r.load("viridis", interpolateViridis());
		r.load("magma", interpolateMagma());
		r.load("inferno", interpolateInferno());
		r.load("plasma", interpolatePlasma());
		r.load("cividis", interpolateCividis());
	}

	if(r.can_load(ColorSchemeType::kDiverging)) {
		// diverging schemes
		r.load("BrBG", interpolateBrBG());
		r.load("PRGn", interpolatePRGn());
		r.load("PiYG", interpolatePiYG());
		r.load("PuOr", interpolatePuOr());
		r.load("RdBu", interpolateRdBu());
		r.load("RdGy", interpolateRdGy());
		r.load("RdYlBu", interpolateRdYlBu());
		r.load("RdYlGn", interpolateRdYlGn());
		r.load("spectral", interpolateSpectral());
	}

	if(r.can_load(ColorSchemeType::kCyclical)) {
		// cyclical schemes
		r.load("twilight", interpolateTwilight());
		r.load("hues", interpolateHue());
	}

	return r.loaded_count();
}

size_t load_discrete_color_scheme_presets(discrete_color_scheme_registry& registry, const std::set<ColorSchemeType>& types) {
	using namespace schemes;

	discrete_color_scheme_registry_helper r(registry, types);

	if(r.can_load(ColorSchemeType::kSequential)) {
		// sequential schemes (single-hue)
		r.load("blues", schemeBlues());
		r.load("greens", schemeGreens());
		r.load("grays", schemeGrays());
		r.load("oranges", schemeOranges());
		r.load("purples", schemePurples());
		r.load("reds", schemeReds());
		// sequential schemes (multi-hue)
	}

	if(r.can_load(ColorSchemeType::kDiverging)) {
		// diverging schemes
		r.load("BrBG", schemeBrBG());
		r.load("PRGn", schemePRGn());
		r.load("PiYG", schemePiYG());
		r.load("PuOr", schemePuOr());
		r.load("RdBu", schemeRdBu());
		r.load("RdGy", schemeRdGy());
		r.load("RdYlBu", schemeRdYlBu());
		r.load("RdYlGn", schemeRdYlGn());
	}

	if(r.can_load(ColorSchemeType::kCategorical)) {
		// categorical schemes
		r.load("category10", schemeCategory10());
		r.load("accent8", schemeAccent8());
		r.load("dark8", schemeDark8());
		r.load("observable10", schemeObservable10());
		r.load("paired12", schemePaired12());
		r.load("pastel8_1", schemePastel8_1());
		r.load("pastel8_2", schemePastel8_2());
		r.load("set8", schemeSet8());
		r.load("set9", schemeSet9());
		r.load("set12", schemeSet12());
		r.load("tableau10", schemeTableau10());
	}

	return r.loaded_count();
}

} // namespace media
} // namespace cgv
