#include "stream_vis_context.h"
#include "parse.h"
#include <cgv/math/ftransform.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/file.h>
#include <limits>

namespace stream_vis {

	component_reference::component_reference(uint16_t _index, uint16_t _component_index)
	{
		index = _index;
		component_index = _component_index;
	}

	size_t stream_vis_context::get_component_index(TimeSeriesAccessor accessor, TimeSeriesAccessor accessors)
	{
		size_t component_index = 0;
		TimeSeriesAccessor tsa = TSA_FIRST;
		while (tsa != accessor) {
			if ((accessors & tsa) != TSA_NONE)
				++component_index;
			tsa = TimeSeriesAccessor(2 * tsa);
		}
		return component_index;
	}

	void stream_vis_context::construct_streaming_aabbs()
	{
		for (auto& tsrb : time_series_ringbuffers) {
			delete tsrb.streaming_aabb;
			switch (aabb_mode) {
			case AM_BRUTE_FORCE:
				switch (tsrb.nr_time_series_components) {
				case 1: tsrb.streaming_aabb = new streaming_aabb_brute_force<float, 1>(tsrb.time_series_ringbuffer_size); break;
				case 2: tsrb.streaming_aabb = new streaming_aabb_brute_force<float, 2>(tsrb.time_series_ringbuffer_size); break;
				case 3: tsrb.streaming_aabb = new streaming_aabb_brute_force<float, 3>(tsrb.time_series_ringbuffer_size); break;
				case 4: tsrb.streaming_aabb = new streaming_aabb_brute_force<float, 4>(tsrb.time_series_ringbuffer_size); break;
				case 5: tsrb.streaming_aabb = new streaming_aabb_brute_force<float, 5>(tsrb.time_series_ringbuffer_size); break;
				case 6: tsrb.streaming_aabb = new streaming_aabb_brute_force<float, 6>(tsrb.time_series_ringbuffer_size); break;
				case 7: tsrb.streaming_aabb = new streaming_aabb_brute_force<float, 7>(tsrb.time_series_ringbuffer_size); break;
				case 8: tsrb.streaming_aabb = new streaming_aabb_brute_force<float, 8>(tsrb.time_series_ringbuffer_size); break;
				case 9: tsrb.streaming_aabb = new streaming_aabb_brute_force<float, 9>(tsrb.time_series_ringbuffer_size); break;
				default: std::cerr << "found time series ringbuffer with more than 9 components" << std::endl; abort();
				}
				break;
			case AM_BLOCKED_8:
				switch (tsrb.nr_time_series_components) {
				case 1: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 1>(tsrb.time_series_ringbuffer_size, 8); break;
				case 2: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 2>(tsrb.time_series_ringbuffer_size, 8); break;
				case 3: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 3>(tsrb.time_series_ringbuffer_size, 8); break;
				case 4: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 4>(tsrb.time_series_ringbuffer_size, 8); break;
				case 5: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 5>(tsrb.time_series_ringbuffer_size, 8); break;
				case 6: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 6>(tsrb.time_series_ringbuffer_size, 8); break;
				case 7: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 7>(tsrb.time_series_ringbuffer_size, 8); break;
				case 8: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 8>(tsrb.time_series_ringbuffer_size, 8); break;
				case 9: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 9>(tsrb.time_series_ringbuffer_size, 8); break;
				default: std::cerr << "found time series ringbuffer with more than 9 components" << std::endl; abort();
				}
				break;
			case AM_BLOCKED_16:
				switch (tsrb.nr_time_series_components) {
				case 1: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 1>(tsrb.time_series_ringbuffer_size, 16); break;
				case 2: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 2>(tsrb.time_series_ringbuffer_size, 16); break;
				case 3: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 3>(tsrb.time_series_ringbuffer_size, 16); break;
				case 4: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 4>(tsrb.time_series_ringbuffer_size, 16); break;
				case 5: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 5>(tsrb.time_series_ringbuffer_size, 16); break;
				case 6: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 6>(tsrb.time_series_ringbuffer_size, 16); break;
				case 7: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 7>(tsrb.time_series_ringbuffer_size, 16); break;
				case 8: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 8>(tsrb.time_series_ringbuffer_size, 16); break;
				case 9: tsrb.streaming_aabb = new streaming_aabb_block_based<float, 9>(tsrb.time_series_ringbuffer_size, 16); break;
				default: std::cerr << "found time series ringbuffer with more than 9 components" << std::endl; abort();
				}
				break;
			}

		}
	}

	void stream_vis_context::construct_storage_buffer()
	{
		// initialize component references to point to themselves
		time_series_component_references.resize(typed_time_series.size());
		for (size_t i = 0; i < typed_time_series.size(); ++i) {
			std::vector<uint16_t> indices = typed_time_series[i]->get_io_indices();
			time_series_component_references[i].resize(indices.size() + 1);
			time_series_component_references[i].front().index = i;
			time_series_component_references[i].front().component_index = 0;
			for (size_t j = 0; j < indices.size(); ++j) {
				time_series_component_references[i][j + 1].index = i;
				time_series_component_references[i][j + 1].component_index = j + 1;
			}
		}
		// collect all time series accesses
		std::vector<TimeSeriesAccessor> ts_accesses;
		ts_accesses.resize(typed_time_series.size(), TSA_NONE);
		for (const auto& pi : plot_pool)
			for (const auto& spi : pi.subplot_infos)
				for (const auto& ad : spi.attribute_definitions)
					ts_accesses[ad.time_series_index] = TimeSeriesAccessor(ts_accesses[ad.time_series_index] | ad.accessor);

		time_series_ringbuffer_references.resize(typed_time_series.size());
		// build ringbuffer storage for composed time series first
		size_t i, storage_buffer_offset = 0;
		for (i = get_first_composed_index(); i < typed_time_series.size(); ++i) {
			// ignore composed time series without reference
			if (ts_accesses[i] == TSA_NONE)
				continue;
			// add a new time series ringbuffer entry
			uint16_t rb_idx = uint16_t(time_series_ringbuffers.size());
			time_series_ringbuffers.resize(time_series_ringbuffers.size() + 1);
			// query the indices of the referenced time series
			std::vector<uint16_t> indices = typed_time_series[i]->get_io_indices();
			int16_t index_location = 0;
			// in case of use of time component, synchronize time components of referenced time series
			bool time_access = (ts_accesses[i] & TSA_TIME) != TSA_NONE;
			uint16_t nr_storage_components = time_access ? 1 : 0;
			if (time_access)
				time_series_ringbuffer_references[i].push_back(component_reference(rb_idx, 0));
			TimeSeriesAccessor tsa = TSA_FIRST;
			do {
				tsa = TimeSeriesAccessor(2 * tsa);
				if ((ts_accesses[i] & tsa) == TSA_NONE)
					continue;
				time_series_ringbuffer_references[i].push_back(component_reference(rb_idx, nr_storage_components));
				if (tsa >= TSA_X && tsa <= TSA_W) {
					uint16_t referenced_ts_idx = indices[index_location];
					auto& ts_refs = time_series_component_references[referenced_ts_idx];
					// if time component is stored, generate reference to composed time series component
					if (time_access) {
						ts_refs[0].index = i;
						ts_refs[0].component_index = 0;
					}
					// generate reference to value component
					ts_refs[1].index = i;
					ts_refs[1].component_index = nr_storage_components;
				}
				++nr_storage_components;
				++index_location;
			} while (tsa != TSA_LAST);
			time_series_ringbuffers.back().storage_buffer_index = 0;
			time_series_ringbuffers.back().time_series_access = ts_accesses[i];
			time_series_ringbuffers.back().time_series_index = (uint16_t)i;
			time_series_ringbuffers.back().nr_samples = 0;
			time_series_ringbuffers.back().storage_buffer_offset = storage_buffer_offset;
			time_series_ringbuffers.back().nr_time_series_components = nr_storage_components;
			time_series_ringbuffers.back().time_series_ringbuffer_size = (uint16_t)typed_time_series[i]->series().get_ringbuffer_size();
			time_series_ringbuffers.back().streaming_aabb = 0;
			storage_buffer_offset += nr_storage_components * typed_time_series[i]->series().get_ringbuffer_size();
		}
		// add ringbuffer entries for non referenced but accessed remaining non composed time series
		for (i = 0; i < get_first_composed_index(); ++i) {
			// ignore time series without reference
			if (ts_accesses[i] == TSA_NONE)
				continue;
			uint16_t nr_storage_components = 0;
			uint16_t reference_index = 0;
			TimeSeriesAccessor tsa = TSA_FIRST;
			while (true) {
				// check for access
				if ((ts_accesses[i] & tsa) != TSA_NONE) {
					// check whether component does not reference already allocated stored component
					if (reference_index > 1 || time_series_component_references[i][reference_index].index == i) {
						// add a new time series ringbuffer entry as soon as this is necessary
						if (nr_storage_components == 0)
							time_series_ringbuffers.resize(time_series_ringbuffers.size() + 1);
						time_series_ringbuffer_references[i].push_back(component_reference(uint16_t(time_series_ringbuffers.size() - 1), nr_storage_components));
						++nr_storage_components;
					}
					// otherwise copy ringbuffer reference from referenced component
					else {
						const auto& tscr = time_series_component_references[i][reference_index];
						time_series_ringbuffer_references[i].push_back(time_series_ringbuffer_references[tscr.index][tscr.component_index]);
					}
				}
				if (tsa == TSA_LAST)
					break;
				tsa = TimeSeriesAccessor(2 * tsa);
				++reference_index;
			}
			if (nr_storage_components > 0) {
				time_series_ringbuffers.back().storage_buffer_index = 0;
				time_series_ringbuffers.back().time_series_access = ts_accesses[i];
				time_series_ringbuffers.back().time_series_index = (uint16_t)i;
				time_series_ringbuffers.back().nr_samples = 0;
				time_series_ringbuffers.back().storage_buffer_offset = storage_buffer_offset;
				time_series_ringbuffers.back().nr_time_series_components = nr_storage_components;
				time_series_ringbuffers.back().time_series_ringbuffer_size = (uint16_t)typed_time_series[i]->series().get_ringbuffer_size();
				storage_buffer_offset += nr_storage_components * typed_time_series[i]->series().get_ringbuffer_size();
			}
		}
		// add ringbuffer references to subplot infos
		for (auto& pl : plot_pool) {
			for (unsigned i = 0; i < pl.subplot_infos.size(); ++i) {
				subplot_info& spi = pl.subplot_infos[i];
				spi.ringbuffer_references.clear();
				for (const auto& ad : spi.attribute_definitions) {
					spi.ringbuffer_references.push_back(
						time_series_ringbuffer_references[ad.time_series_index][
							get_component_index(ad.accessor, ts_accesses[ad.time_series_index])]);
				}
			}
		}
		// finally we can allocate the storage buffer
		storage_buffers.resize(1);
		storage_buffers.front().resize(storage_buffer_offset);
		construct_streaming_aabbs();
	}

	stream_vis_context::stream_vis_context(const std::string& name) : node(name)
	{
		outofdate = true;
		last_use_vbo = use_vbo = false;
		plot_attributes_initialized = false;
		aabb_mode = last_aabb_mode = AM_BRUTE_FORCE;
	}
	stream_vis_context::~stream_vis_context()
	{
		for (auto& tsp : typed_time_series)
			delete tsp;
	}
	void stream_vis_context::on_set(void* member_ptr)
	{
		// iterate all plots
		for (auto& pl : plot_pool) {
			for (unsigned i = 0; i < pl.plot_ptr->get_nr_sub_plots(); ++i) {
				auto& cfg = pl.plot_ptr->ref_sub_plot_config(i);
				if (member_ptr == &cfg.ref_size) {
					cfg.set_size(cfg.ref_size);
					update_member(&cfg.point_size);
					update_member(&cfg.line_width);
					update_member(&cfg.stick_width);
					update_member(&cfg.bar_outline_width);
				}
				if (member_ptr == &cfg.ref_color) {
					cfg.set_colors(cfg.ref_color);
					update_member(&cfg.point_color);
					update_member(&cfg.line_color);
					update_member(&cfg.stick_color);
					update_member(&cfg.bar_color);
					update_member(&cfg.bar_outline_color);
				}
			}
			if ((member_ptr >= &pl.plot_ptr->ref_domain_min()(0) && member_ptr < &pl.plot_ptr->ref_domain_min()(0) + pl.plot_ptr->get_dim()) ||
				(member_ptr >= &pl.plot_ptr->ref_domain_max()(0) && member_ptr < &pl.plot_ptr->ref_domain_max()(0) + pl.plot_ptr->get_dim())) {
				pl.plot_ptr->adjust_tick_marks_to_domain();
			}
			update_member(member_ptr);
			post_redraw();
		}
	}

	bool stream_vis_context::parse_plot_param(const cgv::utils::token& tok, std::map<std::string,std::string>& pp) const
	{
		std::vector<cgv::utils::token> tokens;
		cgv::utils::split_to_tokens(
			tok.begin, tok.end, tokens,
			"#;", false, "\"'", "\"'"
		);
		if (tokens.size() < 3)
			return false;
		size_t i = 0;
		do {
			std::string name = cgv::utils::to_string(tokens[i]);
			if (tokens[i + 1] != "#")
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
	bool stream_vis_context::analyze_subplot_declaration(cgv::utils::token tok, std::vector<cgv::utils::token>& tokens, std::string& subplot_name, std::vector<attribute_definition>& ads, size_t& i, std::map<std::string, std::string>& pp) const
	{
		// split subplot declaration into parts
		cgv::utils::split_to_tokens(tok.begin, tok.end, tokens, "<|>[]~&", false, "\"'", "\"'");
		if (tokens.size() < 3) {
			std::cerr << "subplot declaration incomplete as less then 3 tokens have been specified <" << tok << ">." << std::endl;
			return false;
		}
		// first check if subplot name is given
		i = 0;
		if (tokens[0] != "<") {
			subplot_name = cgv::utils::to_string(tokens[0]);
			++i;
		}
		// next validate that time series list is specified
		if (tokens[i] != "<") {
			std::cerr << "subplot declaration must start with '<' after optional name, but found <" << tokens[i] << ">." << std::endl;
			return false;
		}
		// iterate through time series references
		++i;
		while (true) {
			// extract time series name and prepare accessor
			std::string ts_name = cgv::utils::to_string(tokens[i]);
			attribute_definition ad;
			ad.accessor = TSA_ALL;
			// check for swizzle and extract it
			std::string swizzle;
			const char* dot_pos = std::find(tokens[i].begin, tokens[i].end, '.');
			if (dot_pos != tokens[i].end) {
				swizzle = ts_name.substr(dot_pos - tokens[i].begin + 1);
				ts_name = ts_name.substr(0, dot_pos - tokens[i].begin);
			}
			// determine index of time series corresponding to name and check for existance
			auto iter = name2index.find(ts_name);
			if (iter == name2index.end()) {
				std::cout << "expected time series reference but found <"
					<< cgv::utils::to_string(tokens[i]) << ">" << std::endl;
				return false;
			}
			ad.time_series_index = iter->second;
			// if no swizzle is given, add one accessor per dimension of time series
			if (swizzle.empty()) {
				size_t dim = typed_time_series[ad.time_series_index]->get_io_indices().size();
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
						size_t dim = typed_time_series[ad.time_series_index]->get_io_indices().size();
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
			++i;
			if (tokens[i] == ">")
				break;
			if (tokens[i] != "|")
				return false;
			++i;
			if (i + 1 >= tokens.size())
				return false;
		};
		++i;
		// validate existance of subplot definition
		if (i + 1 >= tokens.size())
			return false;
		// check for subplot parameter definition and parse it if it is specified
		if (tokens[i] == "[") {
			if (i + 4 >= tokens.size()) {
				std::cerr << "encountered incomplete subplot parameter definition in <" << tok << ">." << std::endl;
				return false;
			}
			if (tokens[i + 2] != "]") {
				std::cerr << "expected cosing ']' after parameter definition but found <" << tokens[i + 2] << ">." << std::endl;
				return false;
			}
			if (!parse_plot_param(tokens[i + 1], pp))
				return false;
			i += 3;
		}
		if (tokens[i] != "~") {
			std::cerr << "expected subplot definitions '~' but found <" << tokens[i] << ">." << std::endl;
			return false;
		}
		++i;
		return true;
	}
	bool stream_vis_context::auto_complete_subplot_declaration(const std::string& name, std::string& subplot_name, std::vector<attribute_definition>& ads, int dim) const
	{
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
		if (ads.size() != dim) {
			std::cerr << "subplot" << dim << "d declaration found with " << ads.size() << " accessors." << std::endl;
			return false;
		}
		// todo add derivation of subplot name from attribute definitions
		if (subplot_name.empty()) {
			subplot_name = name;
		}
		return true;
	}

	bool stream_vis_context::parse_base_config_params(const cgv::utils::token& tok, const std::map<std::string, std::string>& ppp, cgv::plot::plot_base_config& cfg) 
	{
		if (tok == "p") {
			cfg.show_points = true;
			if (parse_color(ppp, "color", cfg.point_color))
				on_set(&cfg.point_color);
			if (parse_float(ppp, "size", cfg.point_size))
				on_set(&cfg.point_size);
		}
		else if (tok == "s") {
			cfg.show_sticks = true;
			if (parse_color(ppp, "color", cfg.stick_color))
				on_set(&cfg.stick_color);
			if (parse_float(ppp, "width", cfg.stick_width))
				on_set(&cfg.stick_width);
		}
		else if (tok == "b") {
			cfg.show_bars = true;
			if (parse_color(ppp, "color", cfg.bar_color))
				on_set(&cfg.bar_color);
			if (parse_color(ppp, "outline_color", cfg.bar_outline_color))
				on_set(&cfg.bar_outline_color);
			if (parse_float(ppp, "outline_width", cfg.bar_outline_width))
				on_set(&cfg.bar_outline_width);
			if (parse_float(ppp, "percentual_width", cfg.bar_percentual_width))
				on_set(&cfg.bar_percentual_width);
		}
		else if (tok == "l") {
			cfg.show_lines = true;
			if (parse_color(ppp, "color", cfg.line_color))
				on_set(&cfg.line_color);
			if (parse_float(ppp, "width", cfg.line_width))
				on_set(&cfg.line_width);
		}
		else
			return false;
		return true;
	}

	bool stream_vis_context::add_subplot2d(const std::string& name, cgv::plot::plot2d* p2d_ptr, cgv::utils::token tok, std::vector<attribute_definition>& ads)
	{
		std::vector<cgv::utils::token> tokens;
		size_t i;
		std::map<std::string, std::string> pp;
		std::string subplot_name;
		if (!analyze_subplot_declaration(tok, tokens, subplot_name, ads, i, pp))
			return false;
		auto_complete_subplot_declaration(name, subplot_name, ads, 2);
		unsigned spi = p2d_ptr->add_sub_plot(subplot_name);
		auto& cfg = p2d_ptr->ref_sub_plot2d_config(spi);
		cfg.show_points = cfg.show_sticks = cfg.show_bars = cfg.show_lines = false;
		if (parse_color(pp, "color", cfg.ref_color))
			on_set(&cfg.ref_color);
		if (parse_float(pp, "size", cfg.ref_size))
			on_set(&cfg.ref_size);
		while (i < tokens.size()) {
			std::map<std::string, std::string> ppp;
			bool has_options = i + 3 < tokens.size() && tokens[i + 1] == "[" && tokens[i + 3] == "]" && parse_plot_param(tokens[i + 2], ppp);
			if (!parse_base_config_params(tokens[i], ppp, cfg))
				return false;
			i += has_options ? 4 : 1;
			if (i >= tokens.size())
				break;
			if (tokens[i] != "&")
				return false;
			++i;
			if (i >= tokens.size())
				return false;
		}
		return true;
	}
	bool stream_vis_context::add_subplot3d(const std::string& name, cgv::plot::plot3d* p3d_ptr, cgv::utils::token tok, std::vector<attribute_definition>& ads)
	{
		std::vector<cgv::utils::token> tokens;
		size_t i;
		std::map<std::string, std::string> pp;
		std::string subplot_name;
		if (!analyze_subplot_declaration(tok, tokens, subplot_name, ads, i, pp))
			return false;
		auto_complete_subplot_declaration(name, subplot_name, ads, 3);
		unsigned spi = p3d_ptr->add_sub_plot(subplot_name);
		auto& cfg = p3d_ptr->ref_sub_plot3d_config(spi);
		cfg.show_points = cfg.show_sticks = cfg.show_bars = cfg.show_surface = false;
		if (parse_color(pp, "color", cfg.ref_color))
			on_set(&cfg.ref_color);
		if (parse_float(pp, "size", cfg.ref_size))
			on_set(&cfg.ref_size);
		while (i < tokens.size()) {
			std::map<std::string, std::string> ppp;
			bool has_options = i + 3 < tokens.size() && tokens[i + 1] == "[" && tokens[i + 3] == "]" && parse_plot_param(tokens[i + 2], ppp);
			if (!parse_base_config_params(tokens[i], ppp, cfg))
				return false;
			if (tokens[i] == "l") {
				if (parse_bool(ppp, "show_orientation", cfg.show_line_orientation))
					on_set(&cfg.show_line_orientation);
			}
			else if (tokens[i] == "b") {
				if (parse_float(ppp, "percentual_depth", cfg.bar_percentual_depth))
					on_set(&cfg.bar_percentual_depth);
			}
			i += has_options ? 4 : 1;
			if (i >= tokens.size())
				break;
			if (tokens[i] != "&")
				return false;
			++i;
			if (i >= tokens.size())
				return false;
		}
		return true;
	}
	bool stream_vis_context::parse_bound_vecn(const std::map<std::string, std::string>& pp, const std::string& name, DomainAdjustment* domain_adjustments, uint16_t* bound_ts_indices, float* fixed_vec, int dim) const
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
					auto iter = name2index.find(bounds[i]);
					if (iter == name2index.end()) {
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
	void stream_vis_context::parse_declarations(const std::string& declarations)
	{
		const std::string* content_ptr = &declarations;
		std::string content;
		if (cgv::utils::file::exists(declarations)) {
			if (cgv::utils::file::read(declarations, content, true))
				content_ptr = &content;
		}
		std::vector<cgv::utils::line> lines;
		cgv::utils::split_to_lines(*content_ptr, lines);
		for (const auto& l : lines) {
			// skip empty lines
			if (l.empty())
				continue;
			// split line into basic structure type=n1,...,nd and validate
			std::vector<cgv::utils::token> tokens;
			cgv::utils::split_to_tokens(l.begin, l.end, tokens, ":,=", false, "\"'", "\"'");
			if (tokens.size() < 5) {
				std::cout << "skipping declaration line <" << to_string(l) << ">" << std::endl;
				continue;
			}
			bool valid = to_string(tokens[1]) == ":" && to_string(tokens[3]) == "=";
			size_t i = 5;
			while (valid && i < tokens.size()) {
				valid = to_string(tokens[i]) == ",";
				i += 2;
			}
			if (!valid) {
				std::cout << "expected declaration syntax name:type=n1,...,nd but found line <" << to_string(l) << ">" << std::endl;
				continue;
			}
			std::string name = to_string(tokens[0]);
			std::string type = to_string(tokens[2]);
			// first check for plot declarations
			if (type.substr(0, 4) == "plot") {
				if (type.length() < 6 || type[5] != 'd') {
					std::cout << "unknown type <" << type << ">" << std::endl;
					continue;
				}
				int dim = type[4] - '0';
				if (dim < 2 || dim > 3) {
					std::cout << "only support plots of dim 2 or 3 but got " << dim << ":\n" << to_string(l) << std::endl;
					continue;
				}
				std::map<std::string, std::string> pp;
				if (type.length() > 6 && type[6] == '[' && type.back() == ']') {
					cgv::utils::token tok;
					tok.begin = &type[7];
					tok.end = &type.back();
					parse_plot_param(tok, pp);
				}
				plot_info pi;
				pi.outofdate = true;
				pi.dim = dim;
				pi.name = name;
				pi.fixed_domain = box3(vec3(0.0f), vec3(1.0f));
				for (int i = 0; i < 2; ++i) {
					for (int j = 0; j < dim; ++j) {
						pi.domain_adjustment[i][j] = DA_COMPUTE;
						pi.domain_bound_ts_index[i][j] = uint16_t(-1);
					}
				}
				parse_bound_vecn(pp, "view_min", pi.domain_adjustment[0], pi.domain_bound_ts_index[0], &pi.fixed_domain.ref_min_pnt()[0], dim);
				parse_bound_vecn(pp, "view_max", pi.domain_adjustment[1], pi.domain_bound_ts_index[1], &pi.fixed_domain.ref_max_pnt()[0], dim);
				parse_vecn(pp, "view_max", &pi.fixed_domain.ref_max_pnt()[0], dim);
				if (!parse_ivec2(pp, "offline_resolution", pi.offline_texture_resolution))
					pi.offline_texture_resolution = ivec2(1024, 1024);
				if (dim == 2) {
					cgv::plot::plot2d* p2d_ptr = new cgv::plot::plot2d();
					pi.plot_ptr = p2d_ptr;
					pi.extent_on_texture = vec2(1.8f);
					parse_vec2(pp, "extent_on_texture", pi.extent_on_texture);
					vec2 ext = vec2(1.0f, float(pi.offline_texture_resolution[1]) / pi.offline_texture_resolution[0]);
					parse_vec2(pp, "extent", ext);
					pi.plot_ptr->set_extent(ext.to_vec());
					plot_pool.push_back(pi);
					for (i = 4; i < tokens.size(); i += 2) {
						subplot_info spi;
						if (add_subplot2d(std::string("subplot") + cgv::utils::to_string(p2d_ptr->get_nr_sub_plots()), p2d_ptr, tokens[i], spi.attribute_definitions))
							plot_pool.back().subplot_infos.push_back(spi);
					}
				}
				else {
					cgv::plot::plot3d* p3d_ptr = new cgv::plot::plot3d();
					pi.plot_ptr = p3d_ptr;
					vec3 ext;
					if (parse_vec3(pp, "extent", ext))
						pi.plot_ptr->set_extent(ext.to_vec());
					plot_pool.push_back(pi);
					for (i = 4; i < tokens.size(); i += 2) {
						subplot_info spi;
						if (add_subplot3d(std::string("subplot") + cgv::utils::to_string(p3d_ptr->get_nr_sub_plots()), p3d_ptr, tokens[i], spi.attribute_definitions))
							plot_pool.back().subplot_infos.push_back(spi);
					}
				}
				vec3 center;
				if (parse_vec3(pp, "center", center))
					plot_pool.back().plot_ptr->place_center(center);
				auto& dom_cfg = *plot_pool.back().plot_ptr->get_domain_config_ptr();
				dom_cfg.fill = false; 
				parse_bool(pp, "fill_domain", dom_cfg.fill);
				parse_bool(pp, "show_domain", dom_cfg.show_domain);
				parse_color(pp, "domain_color", dom_cfg.color);
				quat ori;
				if (parse_quat(pp, "orientation", ori))
					plot_pool.back().plot_ptr->set_orientation(ori);
				parse_float(pp, "font_size", dom_cfg.label_font_size);
			}
			else {
				// for all other declarations extract stream indices
				std::vector<uint16_t> stream_indices;
				i = 3;
				do {
					++i;
					if (name2index.find(to_string(tokens[i])) != name2index.end())
						stream_indices.push_back(name2index[to_string(tokens[i])]);
					else {
						std::cerr << "could not find name " << to_string(tokens[i]) << " in inputs or outputs" << std::endl;
						stream_indices.push_back(0);
					}
					++i;
				} while (i + 1 < tokens.size() && to_string(tokens[i]) == ",");
				if (type.substr(0, 3) == "vec") {
					if (type.length() != 4) {
						std::cout << "unknown type <" << type << ">" << std::endl;
						continue;
					}
					int dim = type[3] - '0';
					if (stream_indices.size() != dim) {
						std::cout << "composed stream of type <" << type << "> needs to be defined from " << dim << " streams, but found only " << stream_indices.size() << ":\n" << to_string(l) << std::endl;
						continue;
					}
					if (dim < 2 || dim > 4) {
						std::cout << "only support composed vec type of dim 2-4 but got " << dim << ":\n" << to_string(l) << std::endl;
						continue;
					}
					switch (dim) {
					case 2: typed_time_series.push_back(new stream_vis::fvec_time_series<2>(stream_indices[0], stream_indices[1])); break;
					case 3: typed_time_series.push_back(new stream_vis::fvec_time_series<3>(stream_indices[0], stream_indices[1], stream_indices[2])); break;
					case 4: typed_time_series.push_back(new stream_vis::fvec_time_series<4>(stream_indices[0], stream_indices[1], stream_indices[2], stream_indices[3])); break;
					}
				}
				else if (type == "quat") {
					if (stream_indices.size() != 4) {
						std::cout << "composed stream of type <" << type << "> needs to be defined from 4 streams, but found only " << stream_indices.size() << ":\n" << to_string(l) << std::endl;
						continue;
					}
					typed_time_series.push_back(new stream_vis::quat_time_series(stream_indices[0], stream_indices[1], stream_indices[2], stream_indices[3])); break;
				}
				typed_time_series.back()->name = name;
				typed_time_series.back()->series().set_ringbuffer_size(1024);
				name2index[name] = uint16_t(typed_time_series.size() - 1);
			}
		}
	}
	void stream_vis_context::show_time_series() const
	{
		for (const auto& tts : typed_time_series) {
			std::cout << tts->name << ":" << tts->get_value_type_name();
			auto vi = tts->get_io_indices();
			if (vi.size() > 1) {
				std::cout << "(";
				bool first = true;
				for (uint16_t i : vi) {
					if (!first)
						std::cout << ",";
					std::cout << typed_time_series[i]->name;
					first = false;
				}
				std::cout << ")";
			}
			std::cout << std::endl;
		}
	}

	bool stream_vis_context::init(cgv::render::context& ctx)
	{
		bool success = true;
		for (auto& pl : plot_pool) {
			if (pl.dim == 2) {
				// create GPU objects for offline rendering
				pl.tex.set_data_format("[R,G,B,A]");
				pl.depth.set_component_format("[D]");
				pl.tex.create(ctx, cgv::render::TT_2D, pl.offline_texture_resolution[0], pl.offline_texture_resolution[1]);
				pl.depth.create(ctx, pl.offline_texture_resolution[0], pl.offline_texture_resolution[1]);
				pl.fbo.create(ctx, pl.offline_texture_resolution[0], pl.offline_texture_resolution[1]);
				pl.fbo.attach(ctx, pl.depth);
				pl.fbo.attach(ctx, pl.tex);
				if (!pl.fbo.is_complete(ctx))
					success = false;
			}
			if (!pl.plot_ptr->init(ctx))
				success = false;
		}
		return success;
	}
	void stream_vis_context::clear(cgv::render::context& ctx)
	{
		for (auto& pl : plot_pool) {
			pl.plot_ptr->clear(ctx);
			if (pl.dim == 2) {
				pl.depth.destruct(ctx);
				pl.tex.destruct(ctx);
				pl.fbo.destruct(ctx);
			}
		}
	}

	void stream_vis_context::update_plot_samples(cgv::render::context& ctx)
	{
		// ensure construction of sorage buffers and vbos
		if (storage_buffers.empty())
			construct_storage_buffer();
		while (storage_vbos.size() < storage_buffers.size()) {
			cgv::render::vertex_buffer* vbo_ptr = new cgv::render::vertex_buffer();
			vbo_ptr->create(ctx, storage_buffers[storage_vbos.size()].size() * sizeof(GLfloat));
			storage_vbos.push_back(vbo_ptr);
		}
		// define subplot attribute array pointers
		if (!plot_attributes_initialized || use_vbo != last_use_vbo) {
			// iterate all plots, subplots and attributes
			for (auto& pl : plot_pool) {
				for (size_t i = 0; i < pl.subplot_infos.size(); ++i) {
					subplot_info& spi = pl.subplot_infos[i];
					for (size_t ai = 0; ai < spi.ringbuffer_references.size(); ++ai) {
						// define attribute storage location based on ringbuffer reference
						const auto& rr = spi.ringbuffer_references[ai];
						const auto& tsr = time_series_ringbuffers[rr.index];
						if (use_vbo)
							pl.plot_ptr->set_sub_plot_attribute(
							(unsigned)i, (unsigned)ai, storage_vbos[tsr.storage_buffer_index],
								(tsr.storage_buffer_offset + rr.component_index) * sizeof(GLfloat),
								tsr.time_series_ringbuffer_size, tsr.nr_time_series_components * sizeof(GLfloat));
						else
							pl.plot_ptr->set_sub_plot_attribute((unsigned)i, (unsigned)ai,
								&storage_buffers[tsr.storage_buffer_index][tsr.storage_buffer_offset + rr.component_index],
								tsr.time_series_ringbuffer_size, tsr.nr_time_series_components * sizeof(float));
					}
				}
			}
			last_use_vbo = use_vbo;
			plot_attributes_initialized = true;
		}
		// update time series ringbuffers
		for (auto& tsrr : time_series_ringbuffers) {
			const auto& tts = typed_time_series[tsrr.time_series_index];
			// check if new samples are available
			size_t count = tts->series().get_nr_samples() - tsrr.nr_samples;
			if (count == 0)
				continue;
			// convert samples to float and put them directly into CPU side storage buffer
			float* storage_ptr = &storage_buffers[tsrr.storage_buffer_index][tsrr.storage_buffer_offset];
			for (size_t s = 0; s < count; ++s) {
				size_t si = tsrr.nr_samples + s;
				size_t csi = si % tsrr.time_series_ringbuffer_size;
				tts->series().put_sample_as_float(si, storage_ptr + tsrr.nr_time_series_components * csi, tsrr.time_series_access);
				tsrr.streaming_aabb->add_samples_base(storage_ptr + tsrr.nr_time_series_components * csi, 1);
			}
			// three cases exist for the upload of the new samples to GPU:

			// first check for case when complete ringbuffer needs to be replaces
			if (count >= tsrr.time_series_ringbuffer_size)
				storage_vbos[tsrr.storage_buffer_index]->replace(ctx, sizeof(GLfloat) * tsrr.storage_buffer_offset,
					storage_ptr, tsrr.nr_time_series_components * tsrr.time_series_ringbuffer_size);
			// otherwise compute begin and end insertion location
			else {
				size_t csi_fst = tsrr.nr_samples % tsrr.time_series_ringbuffer_size;
				size_t csi_lst = (tsrr.nr_samples + count - 1) % tsrr.time_series_ringbuffer_size;
				// check if single copy is sufficient
				if (csi_lst >= csi_fst)
					storage_vbos[tsrr.storage_buffer_index]->replace(ctx,
						sizeof(GLfloat) * (tsrr.storage_buffer_offset + tsrr.nr_time_series_components * csi_fst),
						storage_ptr + tsrr.nr_time_series_components * csi_fst, tsrr.nr_time_series_components * count);
				// otherwise do two replacement commands
				else {
					storage_vbos[tsrr.storage_buffer_index]->replace(ctx,
						sizeof(GLfloat) * (tsrr.storage_buffer_offset + tsrr.nr_time_series_components * csi_fst),
						storage_ptr + tsrr.nr_time_series_components * csi_fst, tsrr.nr_time_series_components * (tsrr.time_series_ringbuffer_size - csi_fst));
					storage_vbos[tsrr.storage_buffer_index]->replace(ctx,
						sizeof(GLfloat) * tsrr.storage_buffer_offset,
						storage_ptr, tsrr.nr_time_series_components * (csi_lst + 1));
				}
			}
			tsrr.nr_samples += count;
		}
	}
	void stream_vis_context::update_plot_domains()
	{
		std::vector<float> tmp_buffer(18);
		float* flt_ptr = &tmp_buffer.front();
		for (auto& pl : plot_pool) {
			// update sample ranges of subplots
			for (unsigned i = 0; i < pl.subplot_infos.size(); ++i) {
				subplot_info& spi = pl.subplot_infos[i];
				auto& cfg = pl.plot_ptr->ref_sub_plot_config(i);
				unsigned buffer_size = 0;
				size_t nr_samples = std::numeric_limits<size_t>::max();
				for (unsigned ai = 0; ai < unsigned(pl.dim); ++ai) {
					const auto& rbr = spi.ringbuffer_references[ai];
					const auto& tsrb = time_series_ringbuffers[rbr.index];
					nr_samples = std::min(nr_samples, tsrb.nr_samples);
					if (buffer_size == 0)
						buffer_size = tsrb.time_series_ringbuffer_size;
					else
						if (buffer_size != tsrb.time_series_ringbuffer_size)
							std::cerr << "ERROR: mismatch in ringbuffer size" << std::endl;
				}
				if (nr_samples <= buffer_size) {
					cfg.begin_sample;
					cfg.end_sample = nr_samples;
				}
				else {
					cfg.begin_sample = (nr_samples - buffer_size) % buffer_size;
					cfg.end_sample = nr_samples % buffer_size;
				}
			}
			// update plot domain
			box3 dom;
			bool compute[2][3] = { {false,false,false}, {false,false,false} };
			int nr_compute = 0;
			for (unsigned ai = 0; ai < unsigned(pl.dim); ++ai) {
				for (int j = 0; j < 2; ++j) {
					vec3& v = (j == 0 ? dom.ref_min_pnt() : dom.ref_max_pnt());
					switch (pl.domain_adjustment[j][ai]) {
					case DA_FIXED:
						v[ai] = (j == 0 ? pl.fixed_domain.get_min_pnt() : pl.fixed_domain.get_max_pnt())[ai];
						break;
					case DA_TIME_SERIES:
						typed_time_series[pl.domain_bound_ts_index[j][ai]]->series().put_sample_as_float(
							typed_time_series[pl.domain_bound_ts_index[j][ai]]->series().get_nr_samples() - 1,
							&v[ai], TSA_X);
						break;
					case DA_COMPUTE:
						compute[j][ai] = true;
						++nr_compute;
						break;
					}
				}
			}
			if (nr_compute > 0) {
				bool initialized[2][3] = { {false,false,false}, {false,false,false} };
				for (unsigned i = 0; i < pl.subplot_infos.size(); ++i) {
					subplot_info& spi = pl.subplot_infos[i];
					auto& cfg = pl.plot_ptr->ref_sub_plot_config(i);
					for (unsigned ai = 0; ai < unsigned(pl.dim); ++ai) {
						if (!compute[0][ai] && !compute[1][ai])
							continue;
						const auto& rbr = spi.ringbuffer_references[ai];
						const auto& tsrb = time_series_ringbuffers[rbr.index];
						tsrb.streaming_aabb->put_aabb(flt_ptr);
						for (int j = 0; j < 2; ++j) {
							float bound = flt_ptr[rbr.component_index+j*tsrb.nr_time_series_components];
							vec3& v = (j == 0 ? dom.ref_min_pnt() : dom.ref_max_pnt());
							if (pl.domain_adjustment[j][ai] != DA_COMPUTE)
								continue;
							if (initialized[j][ai]) {
								if (j == 0)
									v[ai] = std::min(v[ai], bound);
								else
									v[ai] = std::max(v[ai], bound);
							}
							else {
								v[ai] = bound;
								initialized[j][ai] = true;
							}
						}
					}
				}
			}
			// validate domain
			vec3 dom_ext = dom.get_extent();
			for (unsigned ai = 0; ai < unsigned(pl.dim); ++ai) {
				if (abs(dom_ext[ai]) < 1e-10)
					dom.ref_max_pnt()[ai] += 0.1f;
			}
			// set domain
			pl.plot_ptr->set_domain3(dom);
			pl.plot_ptr->adjust_tick_marks_to_domain();
		}
	}
	void stream_vis_context::init_frame(cgv::render::context& ctx)
	{
		update_plot_samples(ctx);
		update_plot_domains();
		for (auto& pl : plot_pool) {
			pl.plot_ptr->init_frame(ctx);
			if (pl.dim == 2) {
				// first call init frame of plot
				if (!pl.fbo.is_created())
					continue;
				if (!pl.outofdate)
					continue;
				// if fbo is created, perform offline rendering with world space in the range [-1,1]² and white background
				pl.fbo.enable(ctx);
				pl.fbo.push_viewport(ctx);
				glClearColor(1, 1, 1, 1);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				ctx.push_modelview_matrix();
				ctx.set_modelview_matrix(cgv::math::identity4<double>());
				ctx.push_projection_matrix();
				ctx.set_projection_matrix(cgv::math::identity4<double>());
				glDepthMask(GL_FALSE);
				vecn ext = pl.plot_ptr->get_extent();
				vec3 ctr = pl.plot_ptr->get_center();
				quat ori = pl.plot_ptr->get_orientation();
				pl.plot_ptr->set_extent(pl.extent_on_texture.to_vec());
				pl.plot_ptr->place_center(vec3(0.0f));
				pl.plot_ptr->set_orientation(quat());
				pl.plot_ptr->draw(ctx);
				pl.plot_ptr->set_orientation(ori);
				pl.plot_ptr->place_center(ctr);
				pl.plot_ptr->set_extent(ext);
				glDepthMask(GL_TRUE);
				ctx.pop_projection_matrix();
				ctx.pop_modelview_matrix();

				pl.fbo.pop_viewport(ctx);
				pl.fbo.disable(ctx);

				// generate mipmaps in rendered texture and in case of success enable anisotropic filtering
				if (pl.tex.generate_mipmaps(ctx))
					pl.tex.set_min_filter(cgv::render::TF_ANISOTROP, 16.0f);
			}
		}
	}
	void stream_vis_context::draw(cgv::render::context& ctx)
	{
		ctx.push_modelview_matrix();
		for (auto& pl : plot_pool) {			
			if (pl.dim == 2) {
				if (pl.fbo.is_created()) {
					// use default shader with texture support to draw offline rendered plot
					glDisable(GL_CULL_FACE);
					auto& prog = ctx.ref_default_shader_program(true);
					pl.tex.enable(ctx);
					prog.enable(ctx);
					ctx.set_color(rgba(1, 1, 1, 1));
					// scale down in y-direction according to texture resolution
					vecn ext(pl.plot_ptr->get_extent());
					ctx.push_modelview_matrix();
					ctx.mul_modelview_matrix(
						cgv::math::translate4<float>(pl.plot_ptr->get_center())*
						pl.plot_ptr->get_orientation().get_homogeneous_matrix()
					);
					ctx.mul_modelview_matrix(cgv::math::scale4<double>(ext[0]/pl.extent_on_texture[0], ext[1]/pl.extent_on_texture[1], 1.0));
					ctx.tesselate_unit_square();
					prog.disable(ctx);
					pl.tex.disable(ctx);
					ctx.pop_modelview_matrix();
					glEnable(GL_CULL_FACE);
				}
				else
					pl.plot_ptr->draw(ctx);
			}
			else
				pl.plot_ptr->draw(ctx);
		}
	}
	void stream_vis_context::create_gui()
	{
		add_member_control(this, "use_vbo", use_vbo, "check");
		for (auto& pl : plot_pool) {
			add_decorator(pl.name, "heading", "level=2");
			pl.plot_ptr->create_gui(this, *this);
			add_decorator("separator", "separator");
		}
	}

}