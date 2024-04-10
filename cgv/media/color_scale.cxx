#include "color_scale.h"
#include <map>
#include <algorithm>
#include <cgv/utils/scan.h>
#include <cgv/utils/convert_string.h>

namespace cgv {
	namespace media {

/// compute an rgb color according to the selected color scale
color<float,RGB> color_scale(double v, ColorScale cs, int polarity)
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
		if (int(cs - CS_NAMED) < query_color_scale_names(polarity).size())
			return sample_sampled_color_scale((float)v, query_named_color_scale(query_color_scale_names(polarity)[cs - CS_NAMED]));
		break;
	}
	return color<float,RGB>((float)v, (float)v, (float)v);
}

double color_scale_gamma_mapping(double v, double gamma, bool is_bipolar, double window_zero_position)
{
	if (is_bipolar) {
		double amplitude = std::max(window_zero_position, 1.0 - window_zero_position);
		if (v < window_zero_position)
			return window_zero_position - std::pow((window_zero_position - v) / amplitude, gamma) * amplitude;
		else
			return std::pow((v - window_zero_position) / amplitude, gamma) * amplitude + window_zero_position;
	}
	else
		return std::pow(v, gamma);
}


double adjust_zero_position(double v, double window_zero_position)
{
	if (window_zero_position <= 0.5)
		return 1.0 - 0.5 * (1.0 - v) / (1.0 - window_zero_position);
	else
		return 0.5 * v / window_zero_position;
}

typedef std::pair<std::vector<color<float, RGB>>, bool> scs_entry_type;
typedef std::map<std::string, scs_entry_type> scs_map_type;

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
		// https://gka.github.io/palettes/#/49|d|43ffff,00cdcd,0073ff,203434|332420,ac0000,ff6100,ffe000|0|0
		static unsigned bipo[8] = { 0x43ffff, 0x00cdcd, 0x0073ff, 0x203434, 0x332420, 0xac0000, 0xff6100, 0xffe000 };
		//
		static unsigned hue[7] = { 0xff0000,0xffff00,0x00ff00,0x00ffff,0x0000ff,0xff00ff,0xff0000 };
		scs_map["temperature_11"] = scs_entry_type(construct_sampled_color_scale(11, temp),false);
		scs_map["anaglyph_9"] = scs_entry_type(construct_sampled_color_scale(9, anag), false);
		scs_map["bipolar_8"] = scs_entry_type(construct_sampled_color_scale(8, bipo), true);
		scs_map["hue_7"] = scs_entry_type(construct_sampled_color_scale(7, hue),false);
		initialized = true;
	}
	return scs_map;
}

size_t& ref_named_color_scale_timestamp()
{
	static size_t timestamp = 1;
	return timestamp;
}

size_t get_named_color_scale_timestamp()
{
	return ref_named_color_scale_timestamp();
}

void register_named_color_scale(const std::string& name, const std::vector<color<float, RGB>>& samples, bool is_bipolar)
{
	ref_sampled_color_scale_map()[name] = scs_entry_type(samples, is_bipolar);
	++ref_named_color_scale_timestamp();
}

const std::vector<std::string>& query_color_scale_names(int polarity)
{
	static std::vector<std::string> names[3];
	static size_t timestamp[3] = { 0, 0, 0 };
	if (timestamp[polarity] < get_named_color_scale_timestamp()) {
		for (const auto& p : ref_sampled_color_scale_map())
			if (polarity == 0 || (polarity == (p.second.second ? 2 : 1)))
				names[polarity].push_back(p.first);
		timestamp[polarity] = get_named_color_scale_timestamp();
	}
	return names[polarity];
}

std::string get_color_scale_name(ColorScale cs)
{
	static const char* color_scale_names[] = { "red","green","blue","gray","temperature","hue","hue_luminance" };
	if (cs < CS_NAMED)
		return color_scale_names[cs];
	int idx = cs - CS_NAMED;
	if (idx < ref_sampled_color_scale_map().size())
		for (const auto& p : ref_sampled_color_scale_map()) {
			if (idx == 0)
				return p.first;
			--idx;
		}
	return std::string();
}

bool find_color_scale(const std::string& name, ColorScale& cs)
{
	const auto& csm = ref_sampled_color_scale_map();
	if (csm.find(name) != csm.end()) {
		ColorScale _cs = CS_NAMED;
		for (const auto& p : csm) {
			if (p.first == name) {
				cs = _cs;
				return true;
			}
			++((int&)_cs);
		}
		return false;
	}
	std::string lname = cgv::utils::to_lower(name);
	if (lname == "red")
		cs = CS_RED;
	else if (lname == "green")
		cs = CS_GREEN;
	else if (lname == "blue")
		cs = CS_BLUE;
	else if (lname == "gray")
		cs = CS_GRAY;
	else if (lname == "temperature")
		cs = CS_TEMPERATURE;
	else if (lname == "hue")
		cs = CS_HUE;
	else if (lname == "hue_luminance")
		cs = CS_HUE_LUMINANCE;
	else 
		return false;
	return true;
}

const std::string& get_color_scale_enum_definition(bool include_fixed, bool include_named, int polarity)
{
	static std::string empty = "enums=''";
	if (!include_fixed && !include_named)
		return empty;
	unsigned i = (include_fixed ? 1 : 0) + (include_named ? 2 : 0) - 1 + 3*polarity;
	static std::string enum_definitions[9];
	static size_t timestamps[9] = { 0,0,0, 0,0,0, 0,0,0 };
	if (timestamps[i] < get_named_color_scale_timestamp()) {
		std::string def = "enums='";
		if (include_fixed)
			def += "red,green,blue,gray,temperature,hue,hue_luminance";
		if (include_named) {
			bool no_comma = !include_fixed;
			unsigned i = 7;
			for (const auto& p : ref_sampled_color_scale_map()) {
				if (polarity == 0 || (polarity == (p.second.second ? 2 : 1))) {
					if (no_comma)
						no_comma = false;
					else
						def += ",";
					def += p.first;
					def += "=";
					def += cgv::utils::to_string(i);
				}
				++i;
			}
		}
		def += "'";
		enum_definitions[i] = def;
		timestamps[i] = get_named_color_scale_timestamp();
	}
	return enum_definitions[i];
}


const std::vector<color<float, RGB>>& query_named_color_scale(const std::string& name, bool* is_bipolar_ptr)
{
	const auto& scs = ref_sampled_color_scale_map()[name];
	if (is_bipolar_ptr)
		*is_bipolar_ptr = scs.second;
	return scs.first;
}

::std::vector<color<float, RGB>> sample_named_color_scale(const std::string& name, size_t nr_samples, bool exact)
{
	const auto& scs_map = ref_sampled_color_scale_map();
	auto it = scs_map.find(name);
	if (it == scs_map.end())
		return ::std::vector<color<float, RGB>>();
	if (nr_samples == 0)
		return it->second.first;
	// determine size of to be returned sampling
	size_t size = it->second.first.size();
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
		result.push_back(sample_sampled_color_scale(value, it->second.first, it->second.second));
	}
	return result;
}

color<float, RGB> sample_sampled_color_scale(float value, const ::std::vector<color<float, RGB>>& samples, bool is_bipolar)
{
	// first check if values needs to be clamped to 0
	if (value <= 0.0f)
		return samples.front();
	// than check if values needs to be clamped to 1 and make sure that values is really smaller than 1
	if (value > 0.99999f)
		return samples.back();
	float v, f;
	size_t i;
	// in case of bipolar color map central to samples correspond to -0 and +0
	if (is_bipolar && ((samples.size() & 1) == 0)) {
		// scale value up to [0,n-2]
		v = value * (samples.size() - 2);
		// compute index of smaller sampled necessary for linear interpolation
		i = size_t(v);
		// correct indices in second half
		if (i+1 >= samples.size()/2)
			++i;
		// compute fractional part
		f = v - i;
	}
	else {
		// scale value up to [0,n-1]
		v = value * (samples.size() - 1);
		// compute index of smaller sampled necessary for linear interpolation
		i = size_t(v);
		// compute fractional part
		f = v - i;
	}
	// return affine combination of two adjacent samples
	return (1.0f - f) * samples[i] + f * samples[i + 1];
}


	}
}
