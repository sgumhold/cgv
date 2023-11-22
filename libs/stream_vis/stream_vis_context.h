#pragma once

#include "plot_info.h"
#include "view_info.h"
#include "offset_info.h"
#include "view_overlay.h"
#include "streaming_time_series.h"
#include "streaming_aabb.h"
#include <cgv/base/node.h>
#include <cgv/os/thread.h>
#include <cgv/os/mutex.h>
#include <cgv/render/drawable.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv_app/application_plugin.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <plot/plot2d.h>
#include <plot/plot3d.h>
#include <atomic>

#include "lib_begin.h"

namespace stream_vis {

	/// ringbuffer 
	struct time_series_ringbuffer
	{
		uint16_t time_series_index;
		TimeSeriesAccessor time_series_access;
		uint16_t nr_time_series_components;
		uint16_t time_series_ringbuffer_size;
		size_t   nr_samples;
		uint16_t storage_buffer_index;
		size_t   storage_buffer_offset;
		streaming_aabb_base<float>* streaming_aabb;
	};

	struct time_series_info
	{
		TimeSeriesAccessor time_series_access;
		std::vector<uint16_t> ringbuffer_indices;
		std::vector<uint16_t> component_indices;
	};

	class CGV_API stream_vis_context : 
		public cgv::app::application_plugin,
		public view_update_handler
	{
	protected:
		std::vector<view_overlay_ptr> view_overlays;
		std::atomic<bool> outofdate;
		bool use_vbo, last_use_vbo, plot_attributes_initialized;
		AABBMode aabb_mode, last_aabb_mode;
		/// maps name to time series indices
		std::map<std::string, uint16_t> name2index;
		/// vector with all time series
		std::vector<stream_vis::streaming_time_series*> typed_time_series;
		/// vector with all offset infos
		std::vector<offset_info> offset_infos;
		/// keep track if there are uninitialized offset definitions
		size_t nr_uninitialized_offsets;
		/// vector of all plots
		std::vector<plot_info> plot_pool;
		/// vector of view infos
		std::vector<view_info> view_infos;
		/// vector of all layouts
//		std::vector<layout_info> layouts;

		bool paused;
		unsigned sleep_ms;

		/// store view pointer and last view
		cgv::render::view* view_ptr = 0;
		cgv::render::view  last_view;

		/// store for each typed time series and each of its components a list of references where the time series component is stored in the storage buffers/vbos
		std::vector<std::vector<component_reference>> time_series_component_references;
		/// 
		std::vector<std::vector<component_reference>> time_series_ringbuffer_references;
		std::vector<time_series_ringbuffer> time_series_ringbuffers;
		std::vector<std::vector<float>> storage_buffers;
		std::vector<cgv::render::vertex_buffer*> storage_vbos;

		static size_t get_component_index(TimeSeriesAccessor accessor, TimeSeriesAccessor accessors);

		void construct_streaming_aabbs();
		void construct_storage_buffer();
		bool is_paused() const { return paused; }
		unsigned get_sleep_ms() const { return sleep_ms; }
	public:
		stream_vis_context(const std::string& name);
		~stream_vis_context();
		virtual size_t get_first_composed_index() const = 0;
		void announce_values(uint16_t num_values, indexed_value* values, double timestamp, const uint16_t* val_idx_from_ts_idx);
		void on_set(void* member_ptr);
		std::string get_type_name() const { return "stream_vis_context"; }
		virtual void extract_time_series() = 0;
		void parse_declarations(const std::string& declarations);
		void show_time_series() const;
		void show_plots() const;
		void show_ringbuffers() const;
		bool is_outofdate() const { return outofdate; }
		bool handle_event(cgv::gui::event& e);
		void stream_help(std::ostream& os);
		bool init(cgv::render::context& ctx);
		void clear(cgv::render::context& ctx);
		void update_plot_samples(cgv::render::context& ctx);
		void update_plot_domains();
		void init_frame(cgv::render::context& ctx);
		void draw(cgv::render::context& ctx);
		void create_gui();
	};
}

#include <cgv/config/lib_end.h>