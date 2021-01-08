#include "parse.h"
#include <cgv/utils/advanced_scan.h>

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

bool parse_color(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::rgb& color)
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
bool parse_float(const std::map<std::string, std::string>& pp, const std::string& name, float& flt)
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
bool parse_vecn(const std::map<std::string, std::string>& pp, const std::string& name, float* v, uint32_t dim)
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
bool parse_quat(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::quat& quat)
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
	if (parse_vec4(pp, name, v)) {
		quat = cgv::render::render_types::quat((cgv::render::render_types::vec3&)v, float(M_PI/180*v[3]));
		return true;
	}
	return false;
}

bool parse_vec2(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::vec2& v)
{
	return parse_vecn(pp, name, &v[0], 2);
}
bool parse_vec3(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::vec3& v)
{
	return parse_vecn(pp, name, &v[0], 3);
}
bool parse_vec4(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::vec4& v)
{
	return parse_vecn(pp, name, &v[0], 4);
}
bool parse_ivecn(const std::map<std::string, std::string>& pp, const std::string& name, int32_t* v, uint32_t dim)
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
bool parse_ivec2(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::ivec2& v)
{
	return parse_ivecn(pp, name, &v[0], 2);
}
bool parse_ivec3(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::ivec3& v)
{
	return parse_ivecn(pp, name, &v[0], 3);
}
bool parse_ivec4(const std::map<std::string, std::string>& pp, const std::string& name, cgv::render::render_types::ivec4& v)
{
	return parse_ivecn(pp, name, &v[0], 4);
}
bool parse_bool(const std::map<std::string, std::string>& pp, const std::string& name, bool& b)
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
