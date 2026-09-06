#include "type_compiler.h"

#include <queue>

#include "function_definition_registry.h"

namespace cgv {
namespace gpgpu {

compile_result type_compiler::compile() const {
	compile_result result;

	// the queue of unprocessed discovered types
	std::queue<sl::data_type> type_queue;
	// keep track of unique type names and associated indices of the dependency graph vertices
	std::map<std::string, size_t> index_by_type_name;
	// store pairs of type names to represent dependencies of form { parent_name, child_name }
	std::vector<std::pair<std::string, std::string>> dependencies;
	// the dependency graph stores types as vertices and their dependencies as directed edges
	graph_type dependency_graph = { cgv::math::EdgeOrientation::Directed };

	const auto queue_types = [&type_queue](const std::vector<sl::data_type>& types) {
		for(const auto& type : types)
			type_queue.push(type);
	};

	// Add all collected types to the queue to be processed.
	queue_types(_types);
	
	// Collect all types used in global function signatures and return types and add the to the queue.
	const function_definition_registry& function_registry = get_function_registry();
	for(const sl::function_definition& function : function_registry.get_global_functions())
		queue_types(function.get_used_types());

	while(!type_queue.empty()) {
		sl::data_type type = type_queue.front();
		type_queue.pop();

		// Only need to define custom struct types
		if(type.is_compound()) {
			// Only consider the type if it was not already added.
			auto type_index_it = index_by_type_name.find(type.type_name());
			if(type_index_it == index_by_type_name.end()) {
				dependency_graph_vertex vertex;
				vertex.type = type;
				index_by_type_name[type.type_name()] = dependency_graph.add_vertex(vertex);

				for(const sl::function_definition& function : function_registry.get_type_scoped_functions(type))
					queue_types(function.get_used_types());

				for(const sl::named_variable& member : type.members()) {
					if(member.type().is_compound()) {
						type_queue.push(member.type());
						// Collect dependencies and store type names since the graph may not contain the type vertex at this point.
						dependencies.push_back({ type.type_name(), member.type().type_name() });
					}
				}
			} else {
				const sl::data_type& known_type = dependency_graph.vertex(type_index_it->second).type;

				// Raise an error if the compound types have the same type name but different member declarations, indicating duplicate type names.
				if(known_type.is_compound() && type.is_compound()) {
					if(known_type.members() != type.members())
						result.err = error(errc::duplicate_type_name, type.type_name());
				}
			}
		}
	}

	// Add edges to represent dependencies in graph
	for(const auto& dependency : dependencies) {
		auto parent_it = index_by_type_name.find(dependency.first);
		auto child_it = index_by_type_name.find(dependency.second);

		// Add an edge to define a dependency. The cgv::math::adjacency_list allows reverse edges even in directed graphs, which will produce a cycle even if two types form a direct cyclic dependency.
		if(parent_it != index_by_type_name.end() && child_it != index_by_type_name.end())
			dependency_graph.add_edge(child_it->second, parent_it->second);
	}

	// Sort the type vertices by their dependency order and convert them to their string representations.
	// Additionally collect their associated functions and define those as well.
	std::vector<size_t> ordered_indices = dependency_graph.topological_sort();
	if(ordered_indices.size() == dependency_graph.vertex_count()) {
		std::string typedefs;
		std::string type_functions_str;
		for(size_t index : ordered_indices) {
			const sl::data_type& type = dependency_graph.vertex(index).type;
			typedefs += sl::get_type_definition_string(type) + "\n";

			sl::function_definition_list type_functions = function_registry.get_type_scoped_functions(type);
			type_functions_str += to_string(type_functions) + "\n";
		}

		typedefs += "\n" + type_functions_str + "\n";

		sl::function_definition_list global_functions = function_registry.get_global_functions();
		std::string global_functions_str = to_string(global_functions);
		typedefs += "\n" + global_functions_str + "\n";

		result.str = typedefs;
	} else {
		result.err = errc::cyclic_type_dependency;
	}

	return result;
}

} // namespace gpgpu
} // namespace cgv
