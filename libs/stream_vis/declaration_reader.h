#pragma once

#include <string>
#include <map>
#include "plot_info.h"
#include "offset_info.h"
#include "view_info.h"
#include "streaming_time_series.h"
#include "lib_begin.h"

namespace stream_vis {
	class CGV_API declaration_reader
	{
	protected:
		const std::string& declarations;
		std::map<std::string, uint16_t>* name2index_ptr;
		std::vector<stream_vis::streaming_time_series*>* typed_time_series_ptr;
		std::vector<stream_vis::view_info>* view_infos_ptr;
		std::vector<stream_vis::offset_info>* offset_infos_ptr;
		std::vector<plot_info>* plot_pool_ptr;
	public:
		declaration_reader(const std::string& _declarations);
		void init(
			std::map<std::string, uint16_t>* _name2index_ptr,
			std::vector<stream_vis::streaming_time_series*>* _typed_time_series_ptr,
			std::vector<stream_vis::offset_info>* _offset_infos_ptr,
			std::vector<stream_vis::view_info>* _view_infos_ptr,
			std::vector<plot_info>* _plot_pool_ptr);
		virtual bool get_value(const std::string& name, std::string& v) = 0;
		virtual bool parse_accessor(char swizzle, TimeSeriesAccessor& tsa);
		virtual bool parse_bool(const std::string& name, bool& b) = 0;
		virtual bool parse_int(const std::string& name, int& i) = 0;
		virtual bool parse_float(const std::string& name, float& f) = 0;
		virtual bool parse_double(const std::string& name, double& f) = 0;
		virtual bool parse_color(const std::string& name, cgv::rgb& color) = 0;
		virtual bool parse_color(const std::string& name, cgv::rgba& color) = 0;
		virtual bool parse_quat (const std::string& name, cgv::quat& quat) = 0;
		virtual bool parse_vecn(const std::string& name, float* v, uint32_t dim) = 0;
		virtual bool parse_dvecn(const std::string& name, double* v, uint32_t dim) = 0;
		virtual bool parse_ivecn(const std::string& name, int32_t* v, uint32_t dim) = 0;
		virtual bool parse_bound_vecn(const std::string& name, DomainAdjustment domain_adjustment[3], uint16_t domain_bound_ts_index[3], float* fixed_domain_ptr, int dim, int nr_attributes) = 0;
		virtual bool parse_marks(plot_info& pi, cgv::plot::plot_base_config& cfg, int dim) = 0;
		virtual bool parse_subplots(plot_info& pi, int dim) = 0;

		void parse_mapped_rgba(const std::string& name_color, const std::string& name_opacity, cgv::plot::mapped_rgba& color);
		void parse_mapped_size(const std::string& name, cgv::plot::mapped_size& size);

		bool parse_vec2(const std::string& name, cgv::vec2& v) { return parse_vecn(name, &v[0], 2); }
		bool parse_vec3(const std::string& name, cgv::vec3& v) { return parse_vecn(name, &v[0], 3); }
		bool parse_vec4(const std::string& name, cgv::vec4& v) { return parse_vecn(name, &v[0], 4); }
		bool parse_dvec2(const std::string& name, cgv::dvec2& v) { return parse_dvecn(name, &v[0], 2); }
		bool parse_dvec3(const std::string& name, cgv::dvec3& v) { return parse_dvecn(name, &v[0], 3); }
		bool parse_dvec4(const std::string& name, cgv::dvec4& v) { return parse_dvecn(name, &v[0], 4); }
		bool parse_ivec2(const std::string& name, cgv::ivec2& v) { return parse_ivecn(name, &v[0], 2); }
		bool parse_ivec3(const std::string& name, cgv::ivec3& v) { return parse_ivecn(name, &v[0], 3); }
		bool parse_ivec4(const std::string& name, cgv::ivec4& v) { return parse_ivecn(name, &v[0], 4); }
		void finalize_time_series(uint16_t ts_idx, int default_ringbuffer_size = 1024);
		bool construct_time_series(const std::string& name, const std::string& type, const std::vector<std::string>& ts_names);
		bool construct_attribute_definitions(std::vector<std::string>& defs, std::vector<attribute_definition>& ads);
		bool construct_mark(const std::string& mark, cgv::plot::plot_base_config& cfg, int dim);
		bool construct_subplot(plot_info& pi, int dim, std::vector<attribute_definition>& ads);
		void construct_plot(const std::string& name, int dim);
		void construct_view(const std::string& name, int dim, const std::vector<std::string>& plot_refs);
		void construct_resample(const std::string& name, const std::string& resampled_ts, const std::string& sampling_ts);
		void construct_offset(const std::string& name, std::vector<std::string>& offset_refs);
		virtual bool parse_declarations() = 0;
	};
}

#include <cgv/config/lib_end.h>