#include "color_scale.h"
#include <map>

namespace cgv {
	namespace media {

/// compute an rgb color according to the selected color scale
color<float,RGB> color_scale(double v, ColorScale cs)
{
	switch (cs) {
	case CS_RED: return color<float, RGB>((float)v, 0, 0);
	case CS_GREEN: return color<float, RGB>(0, (float)v, 0);
	case CS_BLUE: return color<float, RGB>(0, 0, (float)v);
	case CS_GRAY: return color<float, RGB>((float)v, (float)v, (float)v);
	case CS_TEMPERATURE:
		if (v < 1.0 / 3)
			return color<float, RGB>((float)(3 * v), 0, 0);
		if (v < 2.0 / 3)
			return color<float, RGB>(1, (float)(3 * v) - 1, 0);
		return color<float, RGB>(1, 1, (float)(3 * v) - 2);
	case CS_HUE:
		return color<float, RGB>(color<float, HLS>((float)v, 0.5f, 1));
	case CS_HUE_LUMINANCE:
		return color<float, RGB>(color<float, HLS>((float)v, (float)(0.5 * v + 0.25), 1));
	default:
		if (int(cs - CS_NAMED) < query_color_scale_names().size())
			return sample_sampled_color_scale((float)v, query_named_color_scale(query_color_scale_names()[cs - CS_NAMED]));
		break;
	}
	return color<float,RGB>((float)v, (float)v, (float)v);
}

double color_scale_gamma_mapping(double v, double gamma, bool is_bipolar, double window_zero_position)
{
	if (is_bipolar) {
		double amplitude = std::max(window_zero_position, 1.0 - window_zero_position);
		if (v < window_zero_position)
			return window_zero_position - pow((window_zero_position - v) / amplitude, gamma) * amplitude;
		else
			return pow((v - window_zero_position) / amplitude, gamma) * amplitude + window_zero_position;
	}
	else
		return pow(v, gamma);
}


double adjust_zero_position(double v, double window_zero_position)
{
	if (window_zero_position <= 0.5)
		return 1.0 - 0.5 * (1.0 - v) / (1.0 - window_zero_position);
	else
		return 0.5 * v / window_zero_position;
}

typedef std::map<std::string, std::vector<color<float, RGB>>> scs_map_type;

std::vector<color<float, RGB>> construct_sampled_color_scale(unsigned count, unsigned* data)
{
	::std::vector<color<float, RGB>> samples;
	for (unsigned i = 0; i < count; ++i)
		samples.push_back(color<float, RGB>(((data[i] & 0xff0000) >> 16) / 255.0f, ((data[i] & 0xff00) >> 8) / 255.0f, (data[i] & 0xff) / 255.0f));
	return samples;
}

scs_map_type& ref_sampled_color_scale_map()
{
	static scs_map_type scs_map;
	static bool initialized = false;
	if (!initialized) {
		// register default color maps
		// https://gka.github.io/palettes/#/11|s|000000,760000,ff0000,ffa500,f7f825,ffffff|ffffe0,ff005e,93003a|0|0
		static unsigned temp[11] = { 0x000000,0x3b0000,0x760000,0xba0000,0xff0000,0xff5300,0xffa500,0xfbce12,0xf7f825,0xfbfc92,0xffffff };
		// https://learnui.design/tools/data-color-picker.html
		static unsigned anag[9] = { 0x000000,0x32172e,0x5d2851,0x873e6f,0xaf5c85,0xd37e93,0xefa79b,0xffd4a4,0xffffff };
		// https://learnui.design/tools/data-color-picker.html#divergent
		static unsigned bipo[11] = { 0x0079d6,0x5387d3,0x7796d1,0x95a6ce,0xaeb6ca,0xc6c6c6,0xd5aba1,0xde8e7c,0xe17059,0xe14d37,0xde1212 };
		//
		static unsigned hue[7] = { 0xff0000,0xffff00,0x00ff00,0x00ffff,0x0000ff,0xff00ff,0xff0000 };
		scs_map["temperature_11"] = construct_sampled_color_scale(11, temp);
		scs_map["anaglyph_9"] = construct_sampled_color_scale(9, anag);
		scs_map["bipolar_11"] = construct_sampled_color_scale(11, bipo);
		scs_map["hue_7"] = construct_sampled_color_scale(7, hue);
		initialized = true;
	}
	return scs_map;
}

size_t& ref_named_color_scale_timestamp()
{
	static size_t timestamp = 1;
	return timestamp;
}

void register_named_color_scale(const std::string& name, const std::vector<color<float, RGB>>& samples)
{
	ref_sampled_color_scale_map()[name] = samples;
	++ref_named_color_scale_timestamp();
}

const std::vector<std::string>& query_color_scale_names()
{
	static std::vector<std::string> names;
	static size_t timestamp = 0;
	if (timestamp < get_named_color_scale_timestamp()) {
		for (const auto& p : ref_sampled_color_scale_map())
			names.push_back(p.first);
		timestamp = get_named_color_scale_timestamp();
	}
	return names;
}

const std::string& get_color_scale_enum_definition(bool include_fixed, bool include_named)
{
	static std::string empty = "enums=''";
	if (!include_fixed && !include_named)
		return empty;
	unsigned i = (include_fixed ? 1 : 0) + (include_named ? 2 : 0) - 1;
	static std::string enum_definitions[3];
	static size_t timestamps[3] = { 0,0,0 };
	if (timestamps[i] < get_named_color_scale_timestamp()) {
		std::string def = "enums='";
		if (include_fixed)
			def += "red,green,blue,gray,temperature,hue,hue_luminance";
		if (include_named) {
			bool no_comma = !include_fixed, fst = true;
			for (const auto& p : ref_sampled_color_scale_map()) {
				if (no_comma)
					no_comma = false;
				else
					def += ",";
				def += p.first;
				if (fst) {
					def += "=7";
					fst = false;
				}
			}
		}
		def += "'";
		enum_definitions[i] = def;
		timestamps[i] = get_named_color_scale_timestamp();
	}
	return enum_definitions[i];
}


size_t get_named_color_scale_timestamp()
{
	return ref_named_color_scale_timestamp();
}

const std::vector<color<float, RGB>>& query_named_color_scale(const std::string& name)
{
	return ref_sampled_color_scale_map()[name];
}

::std::vector<color<float, RGB>> sample_named_color_scale(const std::string& name, size_t nr_samples, bool exact)
{
	const auto& scs_map = ref_sampled_color_scale_map();
	auto it = scs_map.find(name);
	if (it == scs_map.end())
		return ::std::vector<color<float, RGB>>();
	if (nr_samples == 0)
		return it->second;
	// determine size of to be returned sampling
	size_t size = it->second.size();
	size_t target_size = nr_samples;
	// in case of not exactly given size
	if (!exact) {
		// if queried size is larger than size of color scale sampling
		if (nr_samples > size)
			// find smallest multiple of size at least as big as queried size
			target_size = size * size_t(ceil(double(nr_samples) / size));
		else {
			// otherwise find smallest divisor of size that is at least as large as fraction of size to queried size
			size_t divisor = size_t(ceil(double(size) / nr_samples));
			while (divisor > 1) {
				if (size == divisor * (size / divisor))
					break;
				--divisor;
			}
			target_size = divisor * (size / divisor);
		}
	}
	// resampled color scale
	::std::vector<color<float, RGB>> result(target_size);
	for (size_t i = 0; i < target_size; ++i) {
		float value = float(i) / (target_size - 1);
		result.push_back(sample_sampled_color_scale(value, it->second));
	}
	return result;
}

color<float, RGB> sample_sampled_color_scale(float value, const ::std::vector<color<float, RGB>>& samples)
{
	// first check if values needs to be clamped to 0
	if (value <= 0.0f)
		return samples.front();
	// than check if values needs to be clamped to 1 and make sure that values is really smaller than 1
	if (value > 0.99999f)
		return samples.back();
	// scale value up to [0,n-1]
	float v = value * (samples.size()-1);
	// compute index of smaller sampled necessary for linear interpolation
	size_t i = size_t(v);
	// compute fractional part
	float f = v - i;
	// return affine combination of two adjacent samples
	return (1.0f - f) * samples[i] + f * samples[i + 1];
}


	}
}
