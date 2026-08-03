#pragma once

#include <cgv/math/adjacency_list.h>

#include "error.h"
#include "sl.h"

#include "lib_begin.h"

namespace cgv {
namespace gpgpu {

struct compile_result {
	error err;
	std::string str;
};

/// 
class CGV_API type_compiler {
public:
	void add_type(const sl::data_type& type) {
		if(type.is_compound())
			_types.push_back(type);
	}

	void add_types(const std::vector<sl::data_type>& types) {
		for(const sl::data_type& type : types)
			add_type(type);
	}

	void add_types(const sl::named_variable_list& variables) {
		for(const sl::named_variable& variable : variables)
			add_type(variable.type());
	}

	compile_result compile() const;

private:
	struct dependency_graph_vertex : cgv::math::vertex<cgv::math::edge> {
		sl::data_type type;
	};

	using graph_type = cgv::math::adjacency_list<dependency_graph_vertex>;

	std::vector<sl::data_type> _types;
};

} // namespace gpgpu
} // namespace cgv

#include <cgv/config/lib_end.h>
