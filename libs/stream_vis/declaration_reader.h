#pragma once

#include <string>
#include <map>
#include "plot_info.h"
#include "streaming_time_series.h"
#include "lib_begin.h"

namespace stream_vis {
	class CGV_API declaration_reader : public cgv::render::render_types
	{
	protected:
		const std::string& declarations;
		std::map<std::string, uint16_t>* name2index_ptr;
		std::vector<stream_vis::streaming_time_series*>* typed_time_series_ptr;
		std::vector<plot_info>* plot_pool_ptr;
	public:
		declaration_reader(const std::string& _declarations);
		void init(
			std::map<std::string, uint16_t>* _name2index_ptr,
			std::vector<stream_vis::streaming_time_series*>* _typed_time_series_ptr,
			std::vector<plot_info>* _plot_pool_ptr);
		virtual bool parse_bool (const std::string& name, bool& b) = 0;
		virtual bool parse_int(const std::string& name, int& i) = 0;
		virtual bool parse_float(const std::string& name, float& f) = 0;
		virtual bool parse_color(const std::string& name, rgb& color) = 0;
		virtual bool parse_color(const std::string& name, rgba& color) = 0;
		virtual bool parse_quat (const std::string& name, quat& quat) = 0;
		virtual bool parse_vecn (const std::string& name, float* v, uint32_t dim) = 0;
		virtual bool parse_ivecn(const std::string& name, int32_t* v, uint32_t dim) = 0;
		virtual bool parse_bound_vecn(const std::string& name, DomainAdjustment domain_adjustment[3], uint16_t domain_bound_ts_index[3], float* fixed_domain_ptr, int dim) = 0;
		virtual bool parse_marks(plot_info& pi, cgv::plot::plot_base_config& cfg, int dim) = 0;
		virtual bool parse_subplots(plot_info& pi, int dim) = 0;

		bool parse_vec2(const std::string& name, cgv::render::render_types::vec2& v) { return parse_vecn(name, &v[0], 2); }
		bool parse_vec3(const std::string& name, cgv::render::render_types::vec3& v) { return parse_vecn(name, &v[0], 3); }
		bool parse_vec4(const std::string& name, cgv::render::render_types::vec4& v) { return parse_vecn(name, &v[0], 4); }
		bool parse_ivec2(const std::string& name, cgv::render::render_types::ivec2& v) { return parse_ivecn(name, &v[0], 2); }
		bool parse_ivec3(const std::string& name, cgv::render::render_types::ivec3& v) { return parse_ivecn(name, &v[0], 3); }
		bool parse_ivec4(const std::string& name, cgv::render::render_types::ivec4& v) { return parse_ivecn(name, &v[0], 4); }
		bool construct_time_series(const std::string& name, const std::string& type, const std::vector<std::string>& ts_names);
		bool construct_attribute_definitions(std::vector<std::string>& defs, std::vector<attribute_definition>& ads);
		bool construct_mark(const std::string& mark, cgv::plot::plot_base_config& cfg, int dim);
		bool construct_subplot(plot_info& pi, int dim, std::vector<attribute_definition>& ads);
		void construct_plot(const std::string& name, int dim);
		virtual bool parse_declarations() = 0;
	};
}

#include <cgv/config/lib_end.h>