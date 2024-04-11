#include "stream_vis_context.h"
#include "view2d_overlay.h"
#include "view3d_overlay.h"
#include <cgv/math/ftransform.h>
#include <cgv/gui/theme_info.h>
#include <cgv/utils/scan.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/gui/key_event.h>
#include <cgv/utils/file.h>
#include "cgv_declaration_reader.h"
#include <nlohmann/json.hpp>
#include <limits>

namespace stream_vis {

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
			// ignore resample time series
			if (typed_time_series[i]->is_resample())
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
			std::vector<component_reference> refs;
			if (time_access)
				refs.push_back(component_reference(rb_idx, 0));
			TimeSeriesAccessor tsa = TSA_FIRST;
			do {
				tsa = TimeSeriesAccessor(2 * tsa);
				if ((ts_accesses[i] & tsa) == TSA_NONE)
					continue;
				refs.push_back(component_reference(rb_idx, nr_storage_components));
				if (tsa >= TSA_X && tsa <= TSA_W) {
					uint16_t referenced_ts_idx = indices[index_location];
					// if time component of referenced time series is used, also store it for composed time series
					if (!time_access && (ts_accesses[referenced_ts_idx] & TSA_TIME) != TSA_NONE) {
						ts_accesses[i] = TimeSeriesAccessor(ts_accesses[i] | TSA_TIME);
						for (auto& r : refs)
							++r.component_index;
						++nr_storage_components;
						refs.insert(refs.begin(), component_reference(rb_idx, 0));
						time_access = true;
					}
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
			time_series_ringbuffer_references[i] = refs;
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
		for (i = 0; i < typed_time_series.size(); ++i) {
			if (i >= get_first_composed_index() && !typed_time_series[i]->is_resample())
				continue;
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

		show_plots();
		show_ringbuffers();

		construct_streaming_aabbs();

	}
	stream_vis_context::stream_vis_context(const std::string& name) : application_plugin(name)
	{
		paused = false;
		sleep_ms = 20;
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
		update_member(member_ptr);
		post_redraw();
	}
	void stream_vis_context::parse_declarations(const std::string& declarations)
	{
		const std::string* content_ptr = &declarations;
		std::string content;
		if (cgv::utils::file::exists(declarations)) {
			if (cgv::utils::file::read(declarations, content, true))
				content_ptr = &content;
		}
		cgv_declaration_reader reader(*content_ptr);
		reader.init(&name2index, &typed_time_series, &offset_infos, &view_infos, &plot_pool);
		if (reader.parse_declarations()) {
			nr_uninitialized_offsets = offset_infos.size();
			// construct views
			for (const auto& vi : view_infos) {
				view_overlay_ptr v_overlay = 0;
				if (vi.dim == 2) {
					view2d_overlay_ptr v2d_overlay = register_overlay<view2d_overlay>(vi.name);
					v2d_overlay->set_update_handler(this);
					v_overlay = v2d_overlay;
				}
				else {
					view3d_overlay_ptr v3d_overlay = register_overlay<view3d_overlay>(vi.name);
					v3d_overlay->set_update_handler(this);
					v3d_overlay->set_current_view(vi.current_view);
					v3d_overlay->set_default_view(vi.default_view);
					v_overlay = v3d_overlay;
				}
				if (v_overlay) {
					v_overlay->set_stretch(cgv::app::overlay::SO_PERCENTUAL, vi.stretch);
					v_overlay->set_alignment(cgv::app::overlay::AO_PERCENTUAL, cgv::app::overlay::AO_PERCENTUAL, vi.offset);
					v_overlay->toggle_key = vi.toggle;
					view_overlays.push_back(v_overlay);
				}
			}
			// assign plots to views
			for (size_t pi = 0; pi < plot_pool.size(); ++pi) {
				const auto& pl = plot_pool[pi];
				if (pl.view_index >= view_overlays.size())
					continue;
				view2d_overlay_ptr v2d_overlay = view_overlays[pl.view_index]->cast<stream_vis::view2d_overlay>();
				view3d_overlay_ptr v3d_overlay = view_overlays[pl.view_index]->cast<stream_vis::view3d_overlay>();
				if (v2d_overlay)
					v2d_overlay->add_plot((int)pi, pl.plot_ptr);
				if (v3d_overlay)
					v3d_overlay->add_plot((int)pi, pl.plot_ptr);
			}
			return;
		}
	}
	void stream_vis_context::show_time_series() const
	{
		for (const auto& tts : typed_time_series) {
			std::cout << tts->get_name() << ":" << tts->get_value_type_name();
			if (!tts->message.empty())
				std::cout << " '" << tts->message << "'";
			std::stringstream ss;
			std::string fill;
			if (tts->default_color[0] != -1) {
				cgv::rgb8 c = tts->default_color;
				ss << "color=" << cgv::utils::to_hex(c[0]) << cgv::utils::to_hex(c[1]) << cgv::utils::to_hex(c[2]);
				fill = ";";
			}
			if (tts->default_opacity != -1) {
				ss << fill << "opacity=" << tts->default_opacity;
				fill = ";";
			}
			if (tts->default_size != -1) {
				ss << fill << "size=" << tts->default_size;
				fill = ";";
			}
			if (tts->aabb_mode != AM_BLOCKED_16) {
				ss << fill << "aabb_mode=";
				switch (tts->aabb_mode)
				{
				case AM_NONE: ss << "none"; break;
				case AM_BRUTE_FORCE: ss << "brute_force"; break;
				case AM_BLOCKED_8: ss << "block_8"; break;
				}
				fill = ";";
			}
			ss.flush();
			if (!ss.str().empty())
				std::cout << "[" << ss.str() << "]";
			auto vi = tts->get_io_indices();
			if (vi.size() > 1) {
				std::cout << "(";
				bool first = true;
				for (uint16_t i : vi) {
					if (!first)
						std::cout << ",";
					std::cout << typed_time_series[i]->get_name();
					first = false;
				}
				std::cout << ")";
			}
			else {
				static const uint16_t nv = -1;
				if (tts->lower_bound_index != nv || tts->upper_bound_index != nv) {
					std::cout << " = {";
					if (tts->lower_bound_index != nv)
						std::cout << typed_time_series[tts->lower_bound_index]->get_name();
					std::cout << ", ";
					if (tts->upper_bound_index != nv)
						std::cout << typed_time_series[tts->upper_bound_index]->get_name();
					std::cout << "}";
				}
			}
			std::cout << std::endl;
		}
	}
	void stream_vis_context::announce_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx)
	{
		if (nr_uninitialized_offsets == 0)
			return;
		for (auto& oi : offset_infos) {
			if (oi.initialized)
				continue;
			if (oi.mode == OIM_FIRST) {
				// check for accessors
				if (!oi.accessors.empty() && oi.accessors.front() == TSA_TIME) {
					oi.offset_value = timestamp;
				}
				else {
					// check if values contain first value
					bool found = false;
					for (auto idx : oi.time_series_indices) {
						uint16_t vi = val_idx_from_ts_idx[idx];
						if (vi == uint16_t(-1))
							continue;
						switch (typed_time_series[idx]->get_value_type_id()) {
						case cgv::type::info::TI_FLT64:
							oi.offset_value = reinterpret_cast<const double&>(values[vi].value[0]);
							break;
						case cgv::type::info::TI_UINT64:
							oi.offset_value = (double)reinterpret_cast<const uint64_t&>(values[vi].value[0]);
							break;
						case cgv::type::info::TI_INT64:
							oi.offset_value = (double)reinterpret_cast<const int64_t&>(values[vi].value[0]);
							break;
						default:
							std::cerr << "unknown offset type" << std::endl;
						}
						found = true;
						break;
					}
					if (!found)
						continue;
				}
			}
			// go through all time series and set offset
			for (auto* ts_ptr : typed_time_series) {
				// check for time offset
				if (oi.accessors.size() > 0 && oi.accessors.front() == TSA_TIME) {
					ts_ptr->series().set_time_offset(oi.offset_value);
				}
				std::vector<uint16_t> ids = ts_ptr->get_io_indices();
				for (int i = 0; i < ids.size(); ++i) {
					for (auto idx : oi.time_series_indices) {
						if (ids[i] == idx) {
							if (ids.size() == 1) {
								// set value offset in scalar time series
								switch (typed_time_series[idx]->get_value_type_id()) {
								case cgv::type::info::TI_FLT64:
									dynamic_cast<float_time_series*>(ts_ptr)->set_value_offset(oi.offset_value);
									break;
								case cgv::type::info::TI_UINT64:
									dynamic_cast<uint_time_series*>(ts_ptr)->set_value_offset(uint64_t(oi.offset_value));
									break;
								case cgv::type::info::TI_INT64:
									dynamic_cast<int_time_series*>(ts_ptr)->set_value_offset(int64_t(oi.offset_value));
									break;
								}
							}
							else {
								switch (ids.size()) {
								case 2: {
									cgv::dvec2 voff = dynamic_cast<fvec_time_series<2>*>(ts_ptr)->get_value_offset();
									voff[i] = oi.offset_value;
									dynamic_cast<fvec_time_series<2>*>(ts_ptr)->set_value_offset(voff);
									break;
								}
								case 3: {
									cgv::dvec3 voff = dynamic_cast<fvec_time_series<3>*>(ts_ptr)->get_value_offset();
									voff[i] = oi.offset_value;
									dynamic_cast<fvec_time_series<3>*>(ts_ptr)->set_value_offset(voff);
									break;
								}
								case 4: {
									cgv::dvec4 voff = dynamic_cast<fvec_time_series<4>*>(ts_ptr)->get_value_offset();
									voff[i] = oi.offset_value;
									dynamic_cast<fvec_time_series<4>*>(ts_ptr)->set_value_offset(voff);
									break;
								}
								}
							}
						}
					}
				}
			}
			oi.initialized = true;
			--nr_uninitialized_offsets;
		}
	}
	void stream_vis_context::show_plots() const
	{
		for (const auto& pl : plot_pool) {
			std::cout << pl.name << ":";
			for (const auto& spi : pl.subplot_infos) {
				std::cout << "<";
				bool first = true;
				for (const auto& ad : spi.attribute_definitions) {
					if (first)
						first = false;
					else
						std::cout << "|";
					std::cout << typed_time_series[ad.time_series_index]->get_name() << "." << get_accessor_string(ad.accessor);
				}
				std::cout << "> ";
				first = true;
				for (const auto& rr : spi.ringbuffer_references) {
					if (first)
						first = false;
					else
						std::cout << ",";
					std::cout << rr.index << ":" << typed_time_series[time_series_ringbuffers[rr.index].time_series_index]->get_name() <<
						"[" << rr.component_index << "]";
				}
				std::cout << "; ";
			}
			std::cout << std::endl;
		}
	}
	void stream_vis_context::show_ringbuffers() const
	{
		for (const auto& tsrb : time_series_ringbuffers) {
			size_t idx = &tsrb - time_series_ringbuffers.data();
			std::cout << idx << ":" << typed_time_series[tsrb.time_series_index]->get_name() <<
				"." << get_accessor_string(tsrb.time_series_access) << "|" << tsrb.nr_time_series_components <<
				" = " << tsrb.nr_samples << "(" << tsrb.time_series_ringbuffer_size << ") -> " <<
				tsrb.storage_buffer_index << "|" << tsrb.storage_buffer_offset << std::endl;
		}
	}
	bool stream_vis_context::handle_event(cgv::gui::event& e)
	{
		if (e.get_kind() == cgv::gui::EID_KEY) {
			auto& ke = reinterpret_cast<cgv::gui::key_event&>(e);
			if (ke.get_action() != cgv::gui::KA_RELEASE) {
				// first check for changes in overlay visibility
				for (view_overlay_ptr vo : view_overlays) 
					if (ke.get_key() == vo->toggle_key) {
						vo->set_visibility(!vo->is_visible());
						return true;
					}

				switch (ke.get_char()) {
				case '+':
					sleep_ms += 1;
					on_set(&sleep_ms);
					return true;
				case '-':
					if (sleep_ms > 0) {
						sleep_ms -= 1;
						on_set(&sleep_ms);
					}
					return true;
				}
				switch (ke.get_key()) {
				case 'X':
					sleep_ms = 0;
					on_set(&sleep_ms);
					return true;
				case 'S':
					sleep_ms = 20;
					on_set(&sleep_ms);
					return true;
				case cgv::gui::KEY_Space:
					if (ke.get_modifiers() == 0) {
						paused = !paused;
						on_set(&paused);
						return true;
					}
					break;
				}
			}
		}
		return false;
	}
	void stream_vis_context::stream_help(std::ostream& os)
	{
		os << "stream_vis_context: <Space> toggles pause, <+|-> control sleep ms" << std::endl;
	}
	bool stream_vis_context::init(cgv::render::context& ctx)
	{
		ctx.set_bg_clr_idx(4);
		bool success = true;
		for (auto& pl : plot_pool) {
			if (!pl.plot_ptr->init(ctx))
				success = false;
		}
		return success;
	}
	void stream_vis_context::clear(cgv::render::context& ctx)
	{
		for (auto& pl : plot_pool)
			pl.plot_ptr->clear(ctx);
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
		int i = 0;
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
				if (tsrr.streaming_aabb)
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
		std::vector<bool> update_tts(typed_time_series.size(), false);
		std::vector<float> tmp_buffer(18);
		float* flt_ptr = &tmp_buffer.front();
		for (auto& pl : plot_pool) {
			// update sample ranges of subplots
			for (unsigned i = 0; i < pl.subplot_infos.size(); ++i) {
				subplot_info& spi = pl.subplot_infos[i];
				auto& cfg = pl.plot_ptr->ref_sub_plot_config(i);
				unsigned buffer_size = 0;
				size_t nr_samples = std::numeric_limits<size_t>::max();
				for (unsigned ai = 0; ai < pl.nr_axes; ++ai) {
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
					//cfg.begin_sample;
					cfg.end_sample = nr_samples;
				}
				else {
					cfg.begin_sample = (nr_samples - buffer_size) % buffer_size;
					cfg.end_sample = nr_samples % buffer_size;
				}
			}
			// update plot domain
			float ranges[2][8];
			bool compute[2][8] = { {false,false,false,false,false,false,false,false}, {false,false,false,false,false,false,false,false} };
			int nr_compute = 0;
			for (unsigned ai = 0; ai < pl.nr_axes; ++ai) {
				for (int j = 0; j < 2; ++j) {
					switch (pl.domain_adjustment[j][ai]) {
					case DA_FIXED:
						ranges[j][ai] = (j == 0 ? pl.fixed_domain.get_min_pnt() : pl.fixed_domain.get_max_pnt())[ai];
						break;
					case DA_TIME_SERIES:
					{
						TimeSeriesAccessor ta = TSA_X;
						if (ai > 0 && typed_time_series[pl.domain_bound_ts_index[j][ai]]->series().get_nr_components() - 1 >= ai)
							ta = ((ai == 1) ? TSA_Y : TSA_Z);
						typed_time_series[pl.domain_bound_ts_index[j][ai]]->series().put_sample_as_float(
							typed_time_series[pl.domain_bound_ts_index[j][ai]]->series().get_nr_samples() - 1,
							&ranges[j][ai], ta);
					}
						break;
					case DA_COMPUTE:
						compute[j][ai] = true;
						++nr_compute;
						break;
					case DA_SHIFTED_TIME_SERIES:
						typed_time_series[pl.domain_bound_ts_index[j][ai]]->series().put_sample_as_float(
							typed_time_series[pl.domain_bound_ts_index[j][ai]]->series().get_nr_samples() - 1,
							&ranges[j][ai], TSA_X);
						ranges[j][ai] += (j == 0 ? pl.fixed_domain.get_min_pnt() : pl.fixed_domain.get_max_pnt())[ai];
						break;
					}
				}
			}
			if (nr_compute > 0) {
				bool initialized[2][8] = { {false,false,false,false,false,false,false,false}, {false,false,false,false,false,false,false,false} };
				for (unsigned i = 0; i < pl.subplot_infos.size(); ++i) {
					subplot_info& spi = pl.subplot_infos[i];
					auto& cfg = pl.plot_ptr->ref_sub_plot_config(i);
					for (unsigned ai = 0; ai < pl.nr_axes; ++ai) {
						if (!compute[0][ai] && !compute[1][ai])
							continue;
						// check if time series has bounds
						bool has_bound[2] = { false, false };
						float bound[2];
						const auto& ad = spi.attribute_definitions[ai];
						if (ad.accessor != TSA_TIME) {
							const auto* tts = typed_time_series[ad.time_series_index];
							// in case of composed time series find component time series
							const auto& is = tts->get_io_indices();
							if (is.size() > 1) {
								switch (ad.accessor) {
								case TSA_X: tts = typed_time_series[is[0]]; break;
								case TSA_Y: tts = typed_time_series[is[1]]; break;
								case TSA_Z:
									if (is.size() > 2)
										tts = typed_time_series[is[2]];
									break;
								case TSA_W:
									if (is.size() > 3)
										tts = typed_time_series[is[3]];
									break;
								}
							}
							// finally check for lower and upper bound
							if (compute[0][ai] && tts->lower_bound_index != uint16_t(-1)) {
								const auto& ts_lb = typed_time_series[tts->lower_bound_index]->series();
								ts_lb.put_sample_as_float(ts_lb.get_nr_samples() - 1, &bound[0], TSA_X);
								has_bound[0] = true;
							}
							if (compute[1][ai] && tts->upper_bound_index != uint16_t(-1)) {
								const auto& ts_ub = typed_time_series[tts->upper_bound_index]->series();
								ts_ub.put_sample_as_float(ts_ub.get_nr_samples() - 1, &bound[1], TSA_X);
								has_bound[1] = true;
							}
						}
						// if not all bounds have been given by time series
						if ((compute[0][ai] && !has_bound[0]) || (compute[1][ai] && !has_bound[1])) {
							const auto& rbr = spi.ringbuffer_references[ai];
							const auto& tsrb = time_series_ringbuffers[rbr.index];
							// and a streaming aabb is available
							if (tsrb.streaming_aabb) {
								tsrb.streaming_aabb->put_aabb(flt_ptr);
								for (int j = 0; j < 2; ++j) {
									if (!compute[j][ai] || has_bound[j])
										continue;
									bound[j] = flt_ptr[rbr.component_index + j * tsrb.nr_time_series_components];
									has_bound[j] = true;
								}
							}
						}
						// update to be computed bounds if values are given
						if (compute[0][ai] && has_bound[0])
							if (initialized[0][ai])
								ranges[0][ai] = std::min(ranges[0][ai], bound[0]);
							else {
								ranges[0][ai] = bound[0];
								initialized[0][ai] = true;
							}
						if (compute[1][ai] && has_bound[1])
							if (initialized[1][ai])
								ranges[1][ai] = std::max(ranges[1][ai], bound[1]);
							else {
								ranges[1][ai] = bound[1];
								initialized[1][ai] = true;
							}
					}
				}
			}
			// validate axis ranges and set in domain configuration
			auto& cfg = *pl.plot_ptr->get_domain_config_ptr();
			for (unsigned ai = 0; ai < pl.nr_axes; ++ai) {
				if (ranges[1][ai] - ranges[0][ai] < 1e-10)
					ranges[1][ai] = ranges[0][ai] + 0.1f;
				cfg.axis_configs[ai].set_attribute_range(ranges[0][ai], ranges[1][ai]);
			}
			// finally adjust tick marks
			pl.plot_ptr->adjust_tick_marks();
		}
	}
	void stream_vis_context::init_frame(cgv::render::context& ctx)
	{
		if (!view_ptr) {
			view_ptr = find_view_as_node();
			if (view_ptr)
				last_view = *view_ptr;
			//on_view_change(true);
		}
//		else {
//			if (view_ptr->get_y_extent_at_focus() )
//		}
		update_plot_samples(ctx);
		update_plot_domains();
		for (auto& pl : plot_pool)
			pl.plot_ptr->init_frame(ctx);
	}
	void stream_vis_context::draw(cgv::render::context& ctx)
	{
		// get theme colors
		auto& ti = cgv::gui::theme_info::instance();
		ctx.push_projection_matrix();
		ctx.push_modelview_matrix();
		ctx.set_projection_matrix(cgv::math::identity4<float>());
		ctx.set_modelview_matrix(cgv::math::identity4<float>());
		glDepthMask(GL_FALSE);
		ctx.ref_default_shader_program().enable(ctx);
		ctx.set_color(ti.background());
		ctx.tesselate_unit_square();
		ctx.ref_default_shader_program().disable(ctx);
		glDepthMask(GL_TRUE);
		ctx.pop_modelview_matrix();
		ctx.pop_projection_matrix();
	}
	void stream_vis_context::create_gui()
	{
		add_decorator("stream vis context", "heading", "level=1");
		add_member_control(this, "pause", paused, "toggle");
		add_member_control(this, "sleep_ms", sleep_ms, "value_slider", "min=0;max=1000;log=true;ticks=true");
		add_member_control(this, "use_vbo", use_vbo, "check");
		if (begin_tree_node("Plots", plot_pool, true)) {
			align("\a");
			for (auto& pl : plot_pool) {
				if (begin_tree_node(pl.name + std::string(pl.dim == 2 ? ":plot2d" : ":plot3d"), pl)) {
					align("\a");
					pl.plot_ptr->create_gui(this, *this);
					align("\b");
					end_tree_node(pl.name);
				}
			}
			align("\b");
			end_tree_node(plot_pool);
		}
		if (begin_tree_node("Views", view_overlays, true)) {
			align("\a");
			for (view_overlay_ptr vo : view_overlays) {
				if (begin_tree_node(vo->get_name(), vo)) {
					align("\a");
					inline_object_gui(vo);
					align("\b");
					end_tree_node(vo);
				}
			}
			align("\b");
			end_tree_node(view_overlays);
		}
	}

}