#include "declaration_reader.h"
#include <libs/plot/plot2d.h>
#include <libs/plot/plot3d.h>

namespace stream_vis {

	declaration_reader::declaration_reader(const std::string& _declarations) :
		declarations(_declarations)
	{
	}
	void declaration_reader::init(
		std::map<std::string, uint16_t>* _name2index_ptr,
		std::vector<stream_vis::streaming_time_series*>* _typed_time_series_ptr,
		std::vector<plot_info>* _plot_pool_ptr)
	{
		name2index_ptr = _name2index_ptr;
		typed_time_series_ptr = _typed_time_series_ptr;
		plot_pool_ptr = _plot_pool_ptr;
	}
	bool declaration_reader::construct_composed_time_series(const std::string& name, const std::string& type, const std::vector<std::string>& ts_names)
	{
		// for all other declarations extract stream indices
		std::vector<uint16_t> stream_indices;
		for (auto ts_name : ts_names) {
			if (name2index_ptr->find(ts_name) != name2index_ptr->end())
				stream_indices.push_back(name2index_ptr->at(ts_name));
			else {
				std::cerr << "could not find name " << ts_name << " in inputs or outputs" << std::endl;
				stream_indices.push_back(0);
			}
		}
		if (type.substr(0, 3) == "vec") {
			if (type.length() != 4) {
				std::cerr << "unknown type <" << type << ">" << std::endl;
				return false;
			}
			int dim = type[3] - '0';
			if (stream_indices.size() != dim) {
				std::cerr << "composed stream of type <" << type << "> needs to be defined from " << dim << " streams, but found only " << stream_indices.size() << " for " << name << ":" << type << std::endl;
				return false;
			}
			if (dim < 2 || dim > 4) {
				std::cerr << "only support composed vec type of dim 2-4 but got " << dim << " for " << name << ":" << type << std::endl;
				return false;
			}
			switch (dim) {
			case 2: typed_time_series_ptr->push_back(new stream_vis::fvec_time_series<2>(stream_indices[0], stream_indices[1])); break;
			case 3: typed_time_series_ptr->push_back(new stream_vis::fvec_time_series<3>(stream_indices[0], stream_indices[1], stream_indices[2])); break;
			case 4: typed_time_series_ptr->push_back(new stream_vis::fvec_time_series<4>(stream_indices[0], stream_indices[1], stream_indices[2], stream_indices[3])); break;
			}
		}
		else if (type == "quat") {
			if (stream_indices.size() != 4) {
				std::cout << "composed stream of type <" << type << "> needs to be defined from 4 streams, but found only " << stream_indices.size() << " in " << name << ":" << type << std::endl;
				return false;
			}
			typed_time_series_ptr->push_back(new stream_vis::quat_time_series(stream_indices[0], stream_indices[1], stream_indices[2], stream_indices[3]));
		}
		typed_time_series_ptr->back()->name = name;
		typed_time_series_ptr->back()->series().set_ringbuffer_size(1024);
		(*name2index_ptr)[name] = uint16_t(typed_time_series_ptr->size() - 1);
		return true;
	}
	bool declaration_reader::construct_mark(const std::string& name, cgv::plot::plot_base_config& cfg, int dim)
	{
		if (name == "p" || name == "points") {
			cfg.show_points = true;
			parse_color("color", cfg.point_color.color);
			parse_float("size", cfg.point_size.size);
		}
		else if (name == "s" || name == "sticks") {
			cfg.show_sticks = true;
			parse_color("color", cfg.stick_color.color);
			parse_float("width", cfg.stick_width.size);
		}
		else if (name == "b" || name == "bars") {
			cfg.show_bars = true;
			parse_color("color", cfg.bar_color.color);
			parse_color("outline_color", cfg.bar_outline_color.color);
			parse_float("outline_width", cfg.bar_outline_width.size);
			parse_float("percentual_width", cfg.bar_percentual_width.size);
			if (dim == 3)
				parse_float("percentual_depth", static_cast<cgv::plot::plot3d_config&>(cfg).bar_percentual_depth.size);
		}
		else if (name == "l" || name == "lines") {
			cfg.show_lines = true;
			parse_color("color", cfg.line_color.color);
			parse_float("width", cfg.line_width.size);
			if (dim == 3)
				parse_bool("show_orientation", static_cast<cgv::plot::plot3d_config&>(cfg).show_line_orientation);
		}
		else {
			std::cerr << "unknown mark " << name << std::endl;
			return false;
		}
		return true;
	}
	bool declaration_reader::construct_attribute_definitions(std::vector<std::string>& defs, std::vector<attribute_definition>& ads)
	{
		for (auto ts_name : defs) {
			attribute_definition ad;
			ad.accessor = TSA_ALL;
			// check for swizzle and extract it
			std::string swizzle;
			size_t dot_pos = ts_name.find_first_of('.');
			if (dot_pos != std::string::npos) {
				swizzle = ts_name.substr(dot_pos + 1);
				ts_name = ts_name.substr(0, dot_pos);
			}
			// determine index of time series corresponding to name and check for existance
			auto iter = name2index_ptr->find(ts_name);
			if (iter == name2index_ptr->end()) {
				std::cout << "expected time series reference but found <" << ts_name << ">" << std::endl;
				return false;
			}
			ad.time_series_index = iter->second;
			// if no swizzle is given, add one accessor per dimension of time series
			if (swizzle.empty()) {
				size_t dim = typed_time_series_ptr->at(ad.time_series_index)->get_io_indices().size();
				if (dim == 1)
					ads.push_back(ad);
				else {
					ad.accessor = TimeSeriesAccessor(TSA_X);
					ads.push_back(ad);
					int delta = 1;
					for (size_t d = 1; d < dim; ++d) {
						delta *= 2;
						ad.accessor = TimeSeriesAccessor(TSA_X + delta);
						ads.push_back(ad);
					}
				}
			}
			// in case of swizzle add one accessor per swizzle entry
			else {
				for (auto s : swizzle) {
					switch (s) {
					case 't': ad.accessor = TSA_TIME; break;
					case 'x': ad.accessor = TSA_X; break;
					case 'y': ad.accessor = TSA_Y; break;
					case 'z': ad.accessor = TSA_Z; break;
					case 'l': ad.accessor = TSA_LENGTH; break;
					case 'd': {
						ad.accessor = TSA_DIRECTION_X;
						size_t dim = typed_time_series_ptr->at(ad.time_series_index)->get_io_indices().size();
						if (dim > 1) {
							int delta = 2;
							for (size_t d = 0; d + 1 < dim; ++d) {
								ads.push_back(ad);
								ad.accessor = TimeSeriesAccessor(ad.accessor * 2);
							}
						}
						break;
					}
					default:
						std::cerr << "encountered unknown swizzle <" << s << "> and ignoring it" << std::endl;
						continue;
					}
					ads.push_back(ad);
				}
			}
		}
		return true;
	}
	bool declaration_reader::construct_subplot(plot_info& pi, int dim, std::vector<attribute_definition>& ads)
	{
		// in case of missing component insert time component by default 
		if (ads.size() == dim - 1) {
			if (ads.front().accessor == TSA_TIME) {
				std::cerr << "subplot" << dim << "d declaration with only " << dim - 1 << " accessors and auto extension of time not possible." << std::endl;
				return false;
			}
			attribute_definition ad;
			ad.time_series_index = ads.front().time_series_index;
			ad.accessor = TSA_TIME;
			ads.insert(ads.begin(), ad);
		}
		if (ads.size() > dim + 2) {
			std::cerr << "subplot" << dim << "d declaration found with " << ads.size() << " accessors and maximum of 2 additional attributes allowed." << std::endl;
			return false;
		}
		// todo add derivation of subplot name from attribute definitions
		std::string subplot_name = "<";
		uint16_t last_time_series_index = 65535;
		for (const auto& ad : ads) {
			if (ad.time_series_index != last_time_series_index) {
				if (last_time_series_index != 65535)
					subplot_name += ",";
				last_time_series_index = ad.time_series_index;
				subplot_name += typed_time_series_ptr->at(ad.time_series_index)->name;
				subplot_name += ".";
			}
			subplot_name += get_accessor_string(ad.accessor);			
		}
		subplot_name += ">";

		// add subplot info
		subplot_info spi;
		spi.attribute_definitions = ads;
		pi.subplot_infos.push_back(spi);
		auto& cfg = pi.plot_ptr->ref_sub_plot_config(pi.plot_ptr->add_sub_plot(subplot_name));

		// construct subplot config
		cfg.show_points = cfg.show_sticks = cfg.show_bars = cfg.show_lines = false;
		if (parse_color("color", cfg.ref_color.color))
			cfg.set_colors(cfg.ref_color.color);
		if (parse_float("size", cfg.ref_size.size))
			cfg.set_sizes(cfg.ref_size.size);

		parse_marks(pi, cfg, dim);


		return true;


	}
	void declaration_reader::construct_plot(const std::string& name, int dim)
	{
		plot_info pi;
		pi.outofdate = true;
		pi.dim = dim;
		pi.name = name;
		pi.offline_texture_resolution = ivec2(1024, 1024);
		pi.fixed_domain = box3(vec3(0.0f), vec3(1.0f));
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < dim; ++j) {
				pi.domain_adjustment[i][j] = DA_COMPUTE;
				pi.domain_bound_ts_index[i][j] = uint16_t(-1);
			}
		}
		parse_bound_vecn("view_min", pi.domain_adjustment[0], pi.domain_bound_ts_index[0], &pi.fixed_domain.ref_min_pnt()[0], dim);
		parse_bound_vecn("view_max", pi.domain_adjustment[1], pi.domain_bound_ts_index[1], &pi.fixed_domain.ref_max_pnt()[0], dim);
		parse_vecn("view_max", &pi.fixed_domain.ref_max_pnt()[0], dim);
		parse_ivec2("offline_resolution", pi.offline_texture_resolution);			
		if (dim == 2) {
			pi.plot_ptr = new cgv::plot::plot2d();
			pi.extent_on_texture = vec2(2.0f);
			parse_vec2("extent_on_texture", pi.extent_on_texture);
			vec2 ext = vec2(1.0f, float(pi.offline_texture_resolution[1]) / pi.offline_texture_resolution[0]);
			parse_vec2("extent", ext);
			pi.plot_ptr->set_extent(ext.to_vec());
		}
		else {
			pi.plot_ptr = new cgv::plot::plot3d();;
			vec3 ext;
			if (parse_vec3("extent", ext))
				pi.plot_ptr->set_extent(ext.to_vec());
		}
		vec3 center;
		if (parse_vec3("center", center))
			pi.plot_ptr->place_center(center);
		auto& dom_cfg = *pi.plot_ptr->get_domain_config_ptr();
		dom_cfg.fill = false;
		parse_bool("fill_domain", dom_cfg.fill);
		parse_bool("show_domain", dom_cfg.show_domain);
		parse_color("domain_color", dom_cfg.color);
		quat ori;
		if (parse_quat("orientation", ori))
			pi.plot_ptr->set_orientation(ori);
		parse_float("font_size", dom_cfg.label_font_size);
		parse_subplots(pi, dim);
		plot_pool_ptr->push_back(pi);
	}
}