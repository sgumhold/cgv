#include "number_format.h"

#include "convert_string.h"
#include "scan.h"

namespace cgv {
namespace utils {

std::string number_format::convert(float value) const {
	std::string s;

	if(decimal_integers) {
		s = std::to_string(static_cast<int>(std::round(value)));
	} else {
		s = cgv::utils::to_string(value, -1, precision, fixed);
		if(!trailing_zeros && s.length() > 1)
			cgv::utils::rtrim(cgv::utils::rtrim(s, "0"), ".");
	}

	if(grouping) {
		std::string sign;
		if(!s.empty() && s.front() == '-' || s.front() == '+') {
			sign = s.front();
			s = s.substr(1);
		}

		std::string suffix;
		for(size_t i = 0; i < s.size(); ++i) {
			if(!std::isdigit(s[i])) {
				suffix = s.substr(i);
				s = s.substr(0, i);
				break;
			}
		}
		s = apply_grouping(s);
		s = sign + s + suffix;
	}

	return s;
}

void number_format::precision_from_range(float first, float last) {
	const float delta = std::abs(last - first);
	precision = 0;

	float limit = 1.0f;
	unsigned max_precision = 7;
	for(unsigned i = 1; i <= max_precision; ++i) {
		if(delta > limit || i == max_precision) {
			precision = i;
			break;
		}
		limit /= 2.0f;
	}
}

std::string number_format::apply_grouping(const std::string& value) const {
	if(group_size <= 0)
		return value;

	std::vector<std::string> parts;
	int i = value.length();
	int g = group_size;
	size_t length = 0;

	while(i > 0) {
		if(length + g + 1 > value.length())
			g = std::max(static_cast<size_t>(1), value.length() - length);
		i -= g;
		parts.push_back(value.substr(i, g));
		length += g;
		if(length > value.length())
			break;
	}

	std::reverse(parts.begin(), parts.end());
	return cgv::utils::join(parts, group_separator);
}

} // namespace utils
} // namespace cgv
