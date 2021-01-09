#pragma once

#include "streaming_time_series.h"
#include "streaming_aabb.h"
#include <cgv/base/node.h>
#include <cgv/os/thread.h>
#include <cgv/os/mutex.h>
#include <cgv/render/drawable.h>
#include <cgv/render/render_buffer.h>
#include <cgv/render/frame_buffer.h>
#include <cgv/render/vertex_buffer.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <plot/plot2d.h>
#include <plot/plot3d.h>
#include <atomic>

#include "lib_begin.h"

namespace stream_vis {

	/// store multiple accesses to a time series
	struct attribute_definition
	{
		///
		uint16_t  time_series_index : 16;
		///
		TimeSeriesAccessor accessor : 16;
	};

	/// reference to a component of an indexed quantity
	struct component_reference
	{
		///
		uint16_t index : 16;
		///
		uint16_t component_index   : 16;
		/// constructor
		component_reference(uint16_t _index = 0, uint16_t _component_index = 0);
	};

	/// for each subplot we store per attribute its definition and reference to the ringbuffer where the data is stored
	struct subplot_info
	{
		std::vector<attribute_definition> attribute_definitions;
		std::vector<component_reference> ringbuffer_references;
	};

	enum DomainAdjustment
	{
		DA_FIXED,
		DA_COMPUTE,
		DA_TIME_SERIES
	};

	/// per plot information
	struct plot_info : public cgv::render::render_types
	{
		/// flag used to check whether offline rendering of 2d plot has to be performed
		bool outofdate;
		/// plot dimension
		int dim;
		/// plot name
		std::string name;
		/// fixed domain definition via view_min and view_max
		box3 fixed_domain;
		/// for each domain component the mode of adjustment
		DomainAdjustment domain_adjustment[2][3];
		/// for each domain component the index of the time series used for adjustment
		uint16_t domain_bound_ts_index[2][3];
		/// flags telling which domain bounds have been set 
		ivec2 offline_texture_resolution;
		/// extent of 2d plot in texture
		vec2 extent_on_texture;
		/// pointer to plot instance
		cgv::plot::plot_base* plot_ptr;
		/// subplot information
		std::vector<subplot_info> subplot_infos;
		// texture for offline rendering of 2d plots
		cgv::render::texture tex;
		// depth buffer for offline rendering of 2d plots
		cgv::render::render_buffer depth;
		// framebuffer obect for offline rendering of 2d plots
		cgv::render::frame_buffer fbo;
	};

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

	enum AABBMode
	{
		AM_BRUTE_FORCE,
		AM_BLOCKED_8,
		AM_BLOCKED_16
	};

	class CGV_API stream_vis_context : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider
	{
	protected:
		std::atomic<bool> outofdate;
		bool use_vbo, last_use_vbo, plot_attributes_initialized;
		AABBMode aabb_mode, last_aabb_mode;
		std::map<std::string, uint16_t> name2index;
		std::vector<stream_vis::streaming_time_series*> typed_time_series;
		
		std::vector<plot_info> plot_pool;
		
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
		bool parse_bound_vecn(const std::map<std::string, std::string>& pp, const std::string& name, DomainAdjustment* domain_adjustments, uint16_t* bound_ts_indices, float* fixed_vec, int dim) const;
		/// parse individual configuration parameter definition
		bool parse_plot_param(const cgv::utils::token& tok, std::map<std::string, std::string>& pp) const;
		/// parse main structure of subplot declaration and extract attribute definitions and configuration parameters
		bool analyze_subplot_declaration(cgv::utils::token tok, std::vector<cgv::utils::token>& tokens,
			std::string& subplot_name, std::vector<attribute_definition>& ads, size_t& i, std::map<std::string, std::string>& pp) const;
		bool auto_complete_subplot_declaration(const std::string& name, std::string& subplot_name, std::vector<attribute_definition>& ads, int dim) const;
		bool parse_base_config_params(const cgv::utils::token& tok, const std::map<std::string, std::string>& ppp, cgv::plot::plot_base_config& cfg);
		/// parse subplot2d declaration and add new plot
		bool add_subplot2d(const std::string& name, cgv::plot::plot2d* p2d_ptr, cgv::utils::token tok, std::vector<attribute_definition>& ads);
		/// parse subplot3d declaration and add new plot
		bool add_subplot3d(const std::string& name, cgv::plot::plot3d* p3d_ptr, cgv::utils::token tok, std::vector<attribute_definition>& ads);
	public:
		stream_vis_context(const std::string& name);
		~stream_vis_context();
		virtual size_t get_first_composed_index() const = 0;
		void on_set(void* member_ptr);
		std::string get_type_name() const { return "stream_vis_context"; }
		virtual void extract_time_series() = 0;
		void parse_declarations(const std::string& declarations);
		void show_time_series() const;
		void show_plots() const;
		bool is_outofdate() const { return outofdate; }
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