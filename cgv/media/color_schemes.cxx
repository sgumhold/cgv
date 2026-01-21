#include "color_schemes.h"

namespace cgv {
namespace media {

size_t load_continuous_color_scheme_presets(continuous_color_scheme_registry& registry, ColorSchemeType type) {
	using namespace schemes;

	size_t count = 0;
	const auto load = [&registry, &count](const std::string& name, const continuous_color_scheme& scheme) {
		count += registry.add(name, scheme) ? 1 : 0;
	};
	bool load_all = type == ColorSchemeType::kUndefined;

	if(load_all || type == ColorSchemeType::kSequential) {
		// sequential schemes (single-hue)
		load("blues", interpolateBlues());
		load("greens", interpolateGreens());
		load("grays", interpolateGrays());
		load("oranges", interpolateOranges());
		load("purples", interpolatePurples());
		load("reds", interpolateReds());
		// sequential schemes (multi-hue)
		load("bone", interpolateBone());
		load("hot", interpolateHot());
		load("jet", interpolateJet());
		load("turbo", interpolateTurbo());
		load("viridis", interpolateViridis());
		load("magma", interpolateMagma());
		load("inferno", interpolateInferno());
		load("plasma", interpolatePlasma());
		load("cividis", interpolateCividis());
	}

	if(load_all || type == ColorSchemeType::kDiverging) {
		// diverging schemes
		load("BrBG", interpolateBrBG());
		load("PRGn", interpolatePRGn());
		load("PiYG", interpolatePiYG());
		load("PuOr", interpolatePuOr());
		load("RdBu", interpolateRdBu());
		load("RdGy", interpolateRdGy());
		load("RdYlBu", interpolateRdYlBu());
		load("RdYlGn", interpolateRdYlGn());
		load("spectral", interpolateSpectral());
	}

	if(load_all || type == ColorSchemeType::kCyclical) {
		// cyclical schemes
		load("twilight", interpolateTwilight());
		load("hues", interpolateHue());
	}

	return count;
}

size_t load_discrete_color_scheme_presets(discrete_color_scheme_registry& registry, ColorSchemeType type) {
	using namespace schemes;

	size_t count = 0;
	const auto load = [&registry, &count](const std::string& name, const discrete_color_scheme& scheme) {
		count += registry.add(name, scheme) ? 1 : 0;
	};
	bool load_all = type == ColorSchemeType::kUndefined;

	if(load_all || type == ColorSchemeType::kSequential) {
		// sequential schemes (single-hue)
		load("blues", schemeBlues());
		load("greens", schemeGreens());
		load("grays", schemeGrays());
		load("oranges", schemeOranges());
		load("purples", schemePurples());
		load("reds", schemeReds());
		// sequential schemes (multi-hue)
	}

	if(load_all || type == ColorSchemeType::kDiverging) {
		// diverging schemes
		load("BrBG", schemeBrBG());
		load("PRGn", schemePRGn());
		load("PiYG", schemePiYG());
		load("PuOr", schemePuOr());
		load("RdBu", schemeRdBu());
		load("RdGy", schemeRdGy());
		load("RdYlBu", schemeRdYlBu());
		load("RdYlGn", schemeRdYlGn());
	}

	if(load_all || type == ColorSchemeType::kCategorical) {
		// categorical schemes
		load("category10", schemeCategory10());
		load("accent8", schemeAccent8());
		load("dark8", schemeDark8());
		load("observable10", schemeObservable10());
		load("paired12", schemePaired12());
		load("pastel8_1", schemePastel8_1());
		load("pastel8_2", schemePastel8_2());
		load("set8", schemeSet8());
		load("set9", schemeSet9());
		load("set12", schemeSet12());
		load("tableau10", schemeTableau10());
	}

	return count;
}

} // namespace media
} // namespace cgv
