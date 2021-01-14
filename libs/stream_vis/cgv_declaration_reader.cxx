#include "cgv_declaration_reader.h"
#include <cgv/utils/advanced_scan.h>

namespace stream_vis {
	cgv_declaration_reader::cgv_declaration_reader(const std::string& _declarations) : declaration_reader(_declarations)
	{

	}
	bool parse_parameters(const cgv::utils::token& tok, std::map<std::string, std::string>& pp)
	{
		std::vector<cgv::utils::token> tokens;
		cgv::utils::split_to_tokens(
			tok.begin, tok.end, tokens,
			"=;", false, "\"'", "\"'"
		);
		if (tokens.size() < 3)
			return false;
		size_t i = 0;
		do {
			std::string name = cgv::utils::to_string(tokens[i]);
			if (tokens[i + 1] != "=")
				return false;
			std::string value = cgv::utils::to_string(tokens[i + 2]);
			pp[name] = value;
			i += 3;
			if (i == tokens.size())
				break;
			if (i + 4 > tokens.size())
				return false;
			if (tokens[i] != ";")
				return false;
			++i;
		} while (true);
		return true;
	}
	int parse_hex(char c)
	{
		if (cgv::utils::is_digit(c))
			return int(c - '0');
		if (!cgv::utils::is_letter(c))
			return 0;
		int v = int(cgv::utils::to_upper(c) - 'A') + 10;
		if (v > 15)
			return 0;
		return v;
	}
	bool cgv_declaration_reader::parse_bool(const std::string& name, bool& b)
	{
		auto iter = pp.find(name);
		if (iter == pp.end())
			return false;
		std::string value = iter->second;
		if (cgv::utils::to_lower(value) == "true") {
			b = true;
			return true;
		}
		if (cgv::utils::to_lower(value) == "false") {
			b = false;
			return true;
		}
		return false;
	}
	bool cgv_declaration_reader::parse_float(const std::string& name, float& flt)
	{
		auto iter = pp.find(name);
		if (iter == pp.end())
			return false;
		std::string value = iter->second;
		double d;
		if (!cgv::utils::is_double(value, d))
			return false;
		flt = float(d);
		return true;
	}
	bool cgv_declaration_reader::parse_color(const std::string& name, rgb& color)
	{
		auto iter = pp.find(name);
		if (iter == pp.end())
			return false;
		std::string value = iter->second;
		if (value.size() != 6)
			return false;
		for (unsigned j = 0; j < 3; ++j)
			color[j] = float(16 * parse_hex(value[2 * j]) + parse_hex(value[2 * j + 1])) / 255.0f;
		return true;
	}
	bool cgv_declaration_reader::parse_quat(const std::string& name, quat& quat)
	{
		auto iter = pp.find(name);
		if (iter == pp.end())
			return false;
		std::string value = iter->second;
		// check for permuations
		bool is_permutation = false;
		if (value.length() == 3) {
			is_permutation = true;
			cgv::render::render_types::mat3 R;
			R.zeros();
			for (int i = 0; i < 3; ++i) {
				switch (value[i]) {
				case 'x':
				case 'y':
				case 'z':
					R(value[i] - 'x', i) = 1.0f;
					break;
				case 'X':
				case 'Y':
				case 'Z':
					R(value[i] - 'X', i) = -1.0f;
					break;
				default:
					is_permutation = false;
					break;
				}
			}
			if (is_permutation) {
				quat = cgv::render::render_types::quat(R);
				return true;
			}
		}
		// check for vec4 and interpret as axis and angle in degree
		cgv::render::render_types::vec4 v;
		if (parse_vec4(name, v)) {
			quat = cgv::render::render_types::quat((cgv::render::render_types::vec3&)v, float(M_PI / 180 * v[3]));
			return true;
		}
		return false;
	}
	bool cgv_declaration_reader::parse_vecn(const std::string& name, float* v, uint32_t dim)
	{
		auto iter = pp.find(name);
		if (iter == pp.end())
			return false;
		std::string value = iter->second;
		std::vector<cgv::utils::token> tokens;
		cgv::utils::split_to_tokens(value, tokens, "|", false);
		if (tokens.size() != 2 * dim - 1)
			return false;
		uint32_t i;
		for (i = 0; i < dim - 1; ++i)
			if (tokens[2 * i + 1] != "|")
				return false;
		for (i = 0; i < dim; ++i) {
			double d;
			if (!cgv::utils::is_double(tokens[2 * i].begin, tokens[2 * i].end, d))
				return false;
			v[i] = float(d);
		}
		return true;
	}
	bool cgv_declaration_reader::parse_ivecn(const std::string& name, int32_t* v, uint32_t dim)
	{
		auto iter = pp.find(name);
		if (iter == pp.end())
			return false;
		std::string value = iter->second;
		std::vector<cgv::utils::token> tokens;
		cgv::utils::split_to_tokens(value, tokens, "|", false);
		if (tokens.size() != 2 * dim - 1)
			return false;
		uint32_t i;
		for (i = 0; i < dim - 1; ++i)
			if (tokens[2 * i + 1] != "|")
				return false;
		for (i = 0; i < dim; ++i) {
			int x;
			if (!cgv::utils::is_integer(tokens[2 * i].begin, tokens[2 * i].end, x))
				return false;
			v[i] = x;
		}
		return true;
	}
	bool cgv_declaration_reader::parse_bound_vecn(const std::string& name, DomainAdjustment domain_adjustments[3], uint16_t bound_ts_indices[3], float* fixed_vec, int dim)
	{
		std::vector<std::string> bounds;
		bounds.resize(dim);
		// first check for vector definition with n '|'-separated values
		auto iter = pp.find(name);
		if (iter != pp.end()) {
			std::string value = iter->second;
			std::vector<cgv::utils::token> tokens;
			cgv::utils::split_to_tokens(value, tokens, "|", false);
			int j = 0;
			// in case only a single bound specification is given, copy this to all coords
			if (tokens.size() == 1 && tokens[0] != "|") {
				for (int i = 0; i < dim; ++i)
					bounds[i] = cgv::utils::to_string(tokens.front());
			}
			// otherwise extract given values and skip empty slots
			else {
				for (uint32_t i = 0; i < tokens.size(); ++i) {
					if (tokens[i] == "|")
						++j;
					else
						if (j < dim)
							bounds[j] = cgv::utils::to_string(tokens[i]);
				}
			}
		}
		// next check for component definitions
		static char comp[4] = { 'x', 'y', 'z', 'w' };
		int i;
		for (i = 0; i < dim; ++i) {
			auto iter = pp.find(name + '_' + comp[i]);
			if (iter != pp.end())
				bounds[i] = iter->second;
		}
		// check all bound definitions
		for (i = 0; i < dim; ++i) {
			//domain_adjustments, uint16_t* bound_ts_indices, float* fixed_vec
			if (bounds[i].empty() || cgv::utils::to_lower(bounds[i]) == "compute")
				domain_adjustments[i] = DA_COMPUTE;
			else {
				double d;
				if (cgv::utils::is_double(bounds[i], d)) {
					domain_adjustments[i] = DA_FIXED;
					fixed_vec[i] = float(d);
				}
				else {
					auto iter = name2index_ptr->find(bounds[i]);
					if (iter == name2index_ptr->end()) {
						std::cerr << "WARNING: Did not find bound reference <" << bounds[i] << std::endl;
						domain_adjustments[i] = DA_COMPUTE;
					}
					else {
						domain_adjustments[i] = DA_TIME_SERIES;
						bound_ts_indices[i] = iter->second;
					}
				}
			}
		}
		return true;
	}
	bool cgv_declaration_reader::parse_marks(plot_info& pi, cgv::plot::plot_base_config& cfg, int dim)
	{
		std::vector<cgv::utils::token> toks;
		cgv::utils::split_to_tokens(tok_marks.begin, tok_marks.end, toks, ",", false, "\"'[", "\"']");
		// iterate through time series references
		size_t i = 0;
		while (i < toks.size()) {
			// extract time series name and prepare accessor
			bool has_params = i + 1 < toks.size() && toks[i + 1].begin[-1] == '[';
			pp.clear();
			if (has_params)
				parse_parameters(toks[i + 1], pp);
			construct_mark(cgv::utils::to_string(toks[i]), cfg, dim);
			i += has_params ? 2 : 1;
			if (i < toks.size()) {
				if (toks[i] != ",") {
					std::cerr << "expected mark definitions to be separated by ','" << std::endl;
					return false;
				}
				++i;
			}
		}
		return true;
	}
	bool cgv_declaration_reader::parse_components(cgv::utils::token tok, std::vector<attribute_definition>& ads)
	{
		std::vector<cgv::utils::token> tokens;
		cgv::utils::split_to_tokens(tok.begin, tok.end, tokens, ",", false, "\"'", "\"'");
		// iterate through time series references
		std::vector<std::string> defs;
		size_t i = 0;
		while (i < tokens.size()) {
			// extract time series name and prepare accessor
			defs.push_back(cgv::utils::to_string(tokens[i]));
			if (++i < tokens.size()) {
				if (tokens[i] != ",") {
					std::cerr << "expected component definitions to be separated by ','" << std::endl;
					return false;
				}
				++i;
			}
		}
		return construct_attribute_definitions(defs, ads);		
	}
	bool cgv_declaration_reader::parse_subplots(plot_info& pi, int dim)
	{		
		std::vector<cgv::utils::token> toks;
		cgv::utils::split_to_tokens(tok_plot, toks, ";=", false, "\"'<[(", "\"'>])");
		size_t ti = 0;
		while (ti + 2 < toks.size()) {
			pp.clear();
			bool has_params = toks[ti + 1].begin[-1] == '[';
			size_t di = has_params ? 2 : 1;
			if (ti + di + 1 >= toks.size()) {
				std::cerr << "not enough tokens to define subplot." << std::endl;
				break;
			}
			if (toks[ti].begin[-1] != '<') {
				std::cerr << "expected '<' Components '>' at beginning of subplot definitions." << std::endl;
				break;
			}
			if (toks[ti + di] != "=") {
				std::cerr << "expected '=' inside of subplot definition." << std::endl;
				break;
			}
			if (toks[ti + di+1].begin[-1] != '(') {
				std::cerr << "expected parentheses '(' Marks ')' around mark list." << std::endl;
				break;
			}
			if (has_params)
				parse_parameters(toks[ti + 1], pp);
			std::vector<attribute_definition> ads;
			parse_components(toks[ti], ads);
			tok_marks = toks[ti + di + 1];
			construct_subplot(pi, dim, ads);
			ti += di + 2;
			if (ti < toks.size()) {
				if (toks[ti] != ";") {
					std::cerr << "expected subplots to be separated by ';'." << std::endl;
					break;
				}
				++ti;
			}
		}
		return true;
	}
	bool cgv_declaration_reader::parse_declarations()
	{
		std::vector<cgv::utils::token> toks;
		cgv::utils::split_to_tokens(declarations, toks, ":=", false, "\"'[{", "\"']}");
		size_t ti = 0;
		while (ti + 5 <= toks.size()) {
			bool has_params = toks[ti + 3].begin[-1] == '[';
			size_t delta = has_params ? 6 : 5;
			size_t di = ti + delta - 1;
			if (ti + 5 == toks.size())
				break;
			if (toks[ti + 1] != ":") {
				std::cerr << "parsing error: expected second token in declaration to be ':' but found '" << toks[ti + 1] << "'" << std::endl;
				ti += delta;
				continue;
			}
			if (toks[di].begin[-1] != '{') {
				std::cerr << "parsing error: expected '{' Definition '}' but found '" << toks[di] << "'" << std::endl;
				ti += delta;
				continue;
			}
			std::string name = to_string(toks[ti]);
			std::string type = to_string(toks[ti + 2]);
			// first check for plot declarations
			if (type.substr(0, 4) == "plot") {
				if (type.length() < 6 || type[5] != 'd') {
					std::cout << "unknown type <" << type << ">" << std::endl;
					ti += delta;
					continue;
				}
				int dim = type[4] - '0';
				if (dim < 2 || dim > 3) {
					std::cout << "only support plots of dim 2 or 3 but got " << dim << ":\n" << to_string(toks[ti + 2]) << std::endl;
					ti += delta;
					continue;
				}
				pp.clear();
				if (has_params) {
					parse_parameters(toks[ti + 3], pp);
				}
				tok_plot = toks[di];
				construct_plot(name, dim);
			}
			else {
				std::vector<std::string> ts_names;
				std::vector<cgv::utils::token> tokens;
				cgv::utils::split_to_tokens(toks[di], tokens, ",", false);
				size_t i = 0;
				while (i < tokens.size()) {
					ts_names.push_back(cgv::utils::to_string(tokens[i]));
					if (++i < tokens.size()) {
						if (tokens[i] != ",") {
							std::cerr << "expected time series references to be separated by ','" << std::endl;
							break;
						}
						++i;
					}
				}
				construct_composed_time_series(name, type, ts_names);
			}
			ti += delta;
		}
		return true;
	}
}