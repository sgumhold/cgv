#include "options.h"
#include "scan.h"
#include "advanced_scan.h"

namespace cgv {
	namespace utils {

		bool has_option(const std::string& option)
		{
			std::vector<std::string> options;
			enumerate_options(options);
			for (auto o : options)
				if (to_upper(o) == to_upper(option))
					return true;
			return false;

		}
		void enumerate_options(std::vector<std::string>& options)
		{
			char* cgv_options = getenv("CGV_OPTIONS");
			if (cgv_options) {
				std::vector<cgv::utils::token> toks;
				std::string option_str(cgv_options);
				split_to_tokens(option_str, toks, ";", true, "", "", "");
				for (auto tok : toks)
					options.push_back(to_string(tok));
			}
		}
	}
}
