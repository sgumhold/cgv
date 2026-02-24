#include "schemes.h"

namespace crameri {

size_t load_continuous_color_scheme_presets(cgv::media::continuous_color_scheme_registry& registry, const std::set<cgv::media::ColorSchemeType> types) {
	using namespace schemes;

	cgv::media::continuous_color_scheme_registry_helper r(registry, types);

	if(r.can_load(cgv::media::ColorSchemeType::kSequential)) {
		r.load("acton", interpolateActon());
		r.load("bamako", interpolateBamako());
		r.load("batlow", interpolateBatlow());
		r.load("batlowK", interpolateBatlowK());
		r.load("batlowW", interpolateBatlowW());
		r.load("bilbao", interpolateBilbao());
		r.load("buda", interpolateBuda());
		r.load("davos", interpolateDavos());
		r.load("devon", interpolateDevon());
		r.load("glasgow", interpolateGlasgow());
		r.load("grayC", interpolateGrayC());
		r.load("hawaii", interpolateHawaii());
		r.load("imola", interpolateImola());
		r.load("lajolla", interpolateLajolla());
		r.load("lapaz", interpolateLapaz());
		r.load("lipari", interpolateLipari());
		r.load("navia", interpolateNavia());
		r.load("naviaW", interpolateNaviaW());
		r.load("nuuk", interpolateNuuk());
		r.load("oslo", interpolateOslo());
		r.load("tokyo", interpolateTokyo());
		r.load("turku", interpolateTurku());
		r.load("bukavu", interpolateBukavu());
		r.load("fes", interpolateFes());
		r.load("oleron", interpolateOleron());
	}

	if(r.can_load(cgv::media::ColorSchemeType::kDiverging)) {
		r.load("bam", interpolateBam());
		r.load("berlin", interpolateBerlin());
		r.load("broc", interpolateBroc());
		r.load("cork", interpolateCork());
		r.load("lisbon", interpolateLisbon());
		r.load("managua", interpolateManagua());
		r.load("roma", interpolateRoma());
		r.load("tofino", interpolateTofino());
		r.load("vanimo", interpolateVanimo());
		r.load("vik", interpolateVik());
	}

	if(r.can_load(cgv::media::ColorSchemeType::kCyclical)) {
		r.load("bamO", interpolateBamO());
		r.load("brocO", interpolateBrocO());
		r.load("corkO", interpolateCorkO());
		r.load("romaO", interpolateRomaO());
		r.load("vikO", interpolateVikO());
	}

	return r.loaded_count();
}

size_t load_discrete_color_scheme_presets(cgv::media::discrete_color_scheme_registry& registry, const std::set<cgv::media::ColorSchemeType> types) {
	using namespace schemes;

	cgv::media::discrete_color_scheme_registry_helper r(registry, types);

	if(r.can_load(cgv::media::ColorSchemeType::kCategorical)) {
		r.load("acton10", schemeActon10());
		r.load("bamako10", schemeBamako10());
		r.load("batlow10", schemeBatlow10());
		r.load("batlowK10", schemeBatlowK10());
		r.load("batlowW10", schemeBatlowW10());
		r.load("bilbao10", schemeBilbao10());
		r.load("buda10", schemeBuda10());
		r.load("davos10", schemeDavos10());
		r.load("devon10", schemeDevon10());
		r.load("glasgow10", schemeGlasgow10());
		r.load("grayC10", schemeGrayC10());
		r.load("hawaii10", schemeHawaii10());
		r.load("imola10", schemeImola10());
		r.load("lajolla10", schemeLajolla10());
		r.load("lapaz10", schemeLapaz10());
		r.load("lipari10", schemeLipari10());
		r.load("navia10", schemeNavia10());
		r.load("naviaW10", schemeNaviaW10());
		r.load("nuuk10", schemeNuuk10());
		r.load("oslo10", schemeOslo10());
		r.load("tokyo10", schemeTokyo10());
		r.load("turku10", schemeTurku10());
	}

	return r.loaded_count();
}

} // namespace crameri
