#pragma once

#include "declaration_reader.h"

#include "lib_begin.h"

namespace stream_vis {
	class CGV_API cgv_declaration_reader : public declaration_reader
	{
		std::map<std::string, std::string> pp;
		cgv::utils::token tok_plot, tok_marks;
	public:
		cgv_declaration_reader(const std::string& _declarations);
		bool get_value(const std::string& name, std::string& v);
		bool parse_bool(const std::string& name, bool& b);
		bool parse_int(const std::string& name, int& i);
		bool parse_float(const std::string& name, float& f);
		bool parse_double(const std::string& name, double& d);
		bool parse_color(const std::string& name, cgv::rgb& color);
		bool parse_color(const std::string& name, cgv::rgba& color);
		bool parse_quat(const std::string& name, cgv::quat& quat);
		bool parse_vecn(const std::string& name, float* v, uint32_t dim);
		bool parse_dvecn(const std::string& name, double* v, uint32_t dim);
		bool parse_ivecn(const std::string& name, int32_t* v, uint32_t dim);
		bool parse_bound_vecn(const std::string& name, DomainAdjustment domain_adjustment[3], uint16_t domain_bound_ts_index[3], float* fixed_domain_ptr, int dim, int nr_attributes);
		bool parse_marks(plot_info& pi, cgv::plot::plot_base_config& cfg, int dim);
		bool parse_components(cgv::utils::token tok, std::vector<attribute_definition>& ads);
		bool parse_subplots(plot_info& pi, int dim);
		bool parse_declarations();
	};
}

#include <cgv/config/lib_end.h>