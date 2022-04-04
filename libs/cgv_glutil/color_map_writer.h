#pragma once

#include <string>

#include <cgv/render/render_types.h>
#include <cgv/utils/file.h>
#include <cgv/utils/xml.h>

#include "color_map.h"

//#include "lib_begin.h"

namespace cgv {
namespace glutil {

class color_map_writer : cgv::render::render_types {
private:
	static std::string stringify(const std::string& name, const color_map& cm, bool add_tab) {
		auto put = [](const std::string& n, const std::string& v) {
			return n + "=\"" + v + "\" ";
		};

		std::string content = "";
		std::string tab = "  ";
		std::string t = add_tab ? tab : "";

		content += t + "<ColorMap " + put("name", name) + ">\n";
		t += tab;
	
		const auto& color_points = cm.ref_color_points();
		const auto& opacity_points = cm.ref_opacity_points();

		for(size_t i = 0; i < color_points.size(); ++i) {
			const auto& p = cm.ref_color_points()[i];

			content += t + "<ColorPoint ";
			content += "x=\"" + std::to_string(p.first) + "\" ";
			content += "r=\"" + std::to_string(p.second.R()) + "\" ";
			content += "g=\"" + std::to_string(p.second.G()) + "\" ";
			content += "b=\"" + std::to_string(p.second.B()) + "\"";
			content += "/>\n";
		}
		
		for(size_t i = 0; i < opacity_points.size(); ++i) {
			const auto& p = cm.ref_opacity_points()[i];

			content += t + "<OpacityPoint ";
			content += "x=\"" + std::to_string(p.first) + "\" ";
			content += "o=\"" + std::to_string(p.second) + "\" ";
			content += "/>\n";
		}

		t = add_tab ? tab : "";
		content += t + "</ColorMap>\n";
		return content;
	}

public:
	static std::string to_xml(const std::string& name, const color_map& cm, bool put_parent_tag = true) {
		std::string content = "";
		std::string end = "";
		bool tab = false;

		if(put_parent_tag) {
			content = "<ColorMaps>\n";
			end = "</ColorMaps>\n";
			tab = true;
		}

		return content + stringify(name, cm, tab) + end;
	}

	static std::string to_xml(const std::vector<std::string>& names, const std::vector<color_map>& color_maps, bool put_parent_tag = true) {
		
		std::string content = "";
		std::string end = "";
		bool tab = false;

		if(put_parent_tag) {
			content = "<ColorMaps>\n";
			end = "</ColorMaps>\n";
			tab = true;
		}

		if(names.size() == color_maps.size()) {
			for(size_t i = 0; i < names.size(); ++i) {
				content += stringify(names[i], color_maps[i], tab);
			}
		}

		return content + end;
	}

	static bool write_to_xml(const std::string& file_name, const std::string& name, const color_map& cm, bool put_parent_tag = true) {
		
		return cgv::utils::file::write(file_name, to_xml(name, cm, put_parent_tag), true);
	}

	static bool write_to_xml(const std::string& file_name, const std::vector<std::string>& names, const std::vector<color_map>& color_maps, bool put_parent_tag = true) {

		return cgv::utils::file::write(file_name, to_xml(names, color_maps, put_parent_tag), true);
	}
};

}
}

//#include <cgv/config/lib_end.h>
