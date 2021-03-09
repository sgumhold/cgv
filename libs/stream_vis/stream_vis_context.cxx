#include "stream_vis_context.h"
#include <cgv/math/ftransform.h>
#include <cgv/utils/advanced_scan.h>
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
		reader.init(&name2index, &typed_time_series, &plot_pool);
		if (reader.parse_declarations())
			return;
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
	void stream_vis_context::show_plots() const
	{
		for (const auto& pl : plot_pool) {
			std::cout << pl.name << ":";
			for (const auto& spi : pl.subplot_infos) {
				std::cout << "<";
				for (const auto& ad : spi.attribute_definitions) {
					std::cout << typed_time_series[ad.time_series_index]->name << "." << (int)ad.accessor << "|";
				}
				std::cout << "> [";
				for (const auto& rr : spi.ringbuffer_references) {
					std::cout << typed_time_series[time_series_ringbuffers[rr.index].time_series_index]->name 
						<< "." << rr.component_index << "|";
				}
				std::cout << "]";
			}
			std::cout << std::endl;
		}
	}

	bool stream_vis_context::init(cgv::render::context& ctx)
	{
		ctx.set_bg_clr_idx(4);
		bool success = true;
		for (auto& pl : plot_pool) {
			if (false && pl.dim == 2) {
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
			pl.plot_ptr->adjust_tick_marks();
		}
	}
	void stream_vis_context::init_frame(cgv::render::context& ctx)
	{
		update_plot_samples(ctx);
		update_plot_domains();
		for (auto& pl : plot_pool) {
			pl.plot_ptr->init_frame(ctx);
			if (false && pl.dim == 2) {
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
				if (false && pl.fbo.is_created()) {
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