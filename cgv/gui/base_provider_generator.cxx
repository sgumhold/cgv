#include "base_provider.h"
#include "base_provider_generator.h"
#include <cgv/gui/trigger.h>
#include <cgv/utils/file.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/tokenizer.h>

using namespace cgv::utils;

namespace cgv {
	namespace gui {

/// construct from instance and gui definition
base_provider_generator::base_provider_generator()
{
	check_file_update = false;
}

///
std::string base_provider_generator::get_type_name() const
{
	return "base_provider_generator";
}

/// 
bool base_provider_generator::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return rh.reflect_member("check_file_update", check_file_update);
}

/// 
void base_provider_generator::on_set(void* member_ptr)
{
	if (member_ptr != &check_file_update)
		return;
	if (check_file_update)
		connect(get_animation_trigger().shoot, this, &base_provider_generator::timer_event);
	else
		disconnect(get_animation_trigger().shoot, this, &base_provider_generator::timer_event);
}

std::string error_start(const std::string& content, const token& T) 
{
	std::string error("error (");
	const char* end = T.end;
	if (&content[content.size()-1] < end)
		end = &content[content.size()-1];

	unsigned line_nr = 1;
	for (const char* p = &content[0]; p<end;++p)
		if (*p == '\n')
			++line_nr;

	error += to_string(line_nr);
	error += ") : ";
	return error;
}

void base_provider_generator::timer_event(double,double)
{
	for (std::map<std::string,long long>::const_iterator i = gui_files.begin(); i != gui_files.end(); ++i) {
		long long lwt = cgv::utils::file::get_last_write_time(i->first);
		if (lwt > i->second)
			parse_gui_file(i->first);
	}
}

/// parse file and extract gui definitions
bool base_provider_generator::parse_gui_file(const std::string& file_name)
{
	std::string content;
	if (!cgv::utils::file::read(file_name, content, true))
		return false;
	gui_files[file_name] = cgv::utils::file::get_last_write_time(file_name);
	std::vector<token> T;
	tokenizer(content).set_sep(":(){}").set_skip("\"","\"").bite_all(T);
	unsigned i=0;
	while (i < T.size()) {
		bool is_type;
		if (!((is_type = (T[i] == "type")) || T[i] == "name")) {
			std::cerr << error_start(content, T[i]) << "expected type or name" << std::endl;
			++i;
			continue;
		}
		if (++i >= T.size()) {
			std::cerr << error_start(content, T[i-1]) << "incomplete definition" << std::endl;
			continue;
		}
		if (T[i] != "(") {
			std::cerr << error_start(content, T[i]) << "expected ( " << std::endl;
			continue;
		}
		if (++i >= T.size()) {
			std::cerr << error_start(content, T[i-1]) << "incomplete definition" << std::endl;
			continue;
		}
		if (T[i] == ")" || T[i] == "{" || T[i] == "}" || T[i] == ":") {
			std::cerr << error_start(content, T[i]) << "unexpected token" << std::endl;
			continue;
		}
		std::string key = to_string(T[i]);
		if (++i >= T.size()) {
			std::cerr << error_start(content, T[i-1]) << "incomplete definition" << std::endl;
			continue;
		}
		if (T[i] != ")") {
			std::cerr << error_start(content, T[i]) << "expected )" << std::endl;
			continue;
		}
		if (++i >= T.size()) {
			std::cerr << error_start(content, T[i-1]) << "incomplete definition" << std::endl;
			continue;
		}
		std::string options;
		if (T[i] == ":") {
			if (++i >= T.size()) {
				std::cerr << error_start(content, T[i-1]) << "incomplete definition" << std::endl;
				continue;
			}
			options = to_string(T[i]);
			++i;
		}
		if (T[i] != "{") {
			std::cerr << error_start(content, T[i]) << "expected {" << std::endl;
			continue;
		}
		unsigned j = i+1;
		unsigned k = 1;
		while (j < T.size()) {
			if (T[j] == "{")
				++k;
			else if (T[j] == "}") {
				--k;
				if (k==0)
					break;
			}
			++j;
		}
		const char* end_ptr;
		if (j == T.size()) {
			std::cerr << error_start(content, T[j-1]) << "missing }" << std::endl;
			end_ptr = &content[content.size()];
		}
		else
			end_ptr = T[j-1].end;

		std::string def(T[i+1].begin, end_ptr-T[i+1].begin);
		if (is_type) {
			defs_by_type[key] = gui_definition(def,options);
			// std::cout << "add by type def for " << key << ":\n" << defs_by_type[key] << "\n" << std::endl;
			// update guis of matched objects
			for (pvd_map_iter pi=providers.begin(); pi != providers.end(); ++pi) {
				if (!pi->second->is_named_gui_assignment() && pi->first->get_type_name() == key)
					pi->second->set_gui_definition(def);
				pi->second->multi_set(options, true);
			}
		}
		else {
			defs_by_name[key]  = gui_definition(def,options);
			// std::cout << "add by name def for " << key << ":\n" << defs_by_name[key] << "\n" << std::endl;
			// update guis of matched objects
			for (pvd_map_iter pi=providers.begin(); pi != providers.end(); ++pi) {
				if (pi->first->get_named() && pi->first->get_named()->get_name() == key) {
					pi->second->set_gui_definition(def);
					pi->second->multi_set(options, true);
					pi->second->set_named_gui_assignment(true);
				}
			}
		}

		i = j+1;
	}
	// try to match unmatched objects
	for (i=0; i<unmatched_objects.size(); ++i)
		if (generate_object_gui(unmatched_objects[i])) {
			unmatched_objects.erase(unmatched_objects.begin()+i);
			--i;
		}

	return true;
}

/// check whether gui description is available for object and if yes generate a base_provider
bool base_provider_generator::generate_object_gui(base_ptr object)
{
	if (object->get_named()) {
		const std::string& name = object->get_named()->get_name();
		def_map_iter iter = defs_by_name.find(name);
		if (iter != defs_by_name.end()) {
			base_provider_ptr pvd(new base_provider(object, iter->second.definition));
			if (!iter->second.options.empty())
				pvd->multi_set(iter->second.options, true);
			cgv::base::register_object(pvd);
			providers[object] = pvd;
			return true;
		}
	}
	def_map_iter iter = defs_by_type.find(object->get_type_name());
	if (iter != defs_by_type.end()) {
		base_provider_ptr pvd(new base_provider(object, iter->second.definition));
		if (!iter->second.options.empty())
			pvd->multi_set(iter->second.options, true);
		cgv::base::register_object(pvd);
		providers[object] = pvd;
		return true;
	}
	return false;
}

/// if object is registered that does not provide its own gui but matches type or name of a parsed gui definition, register a newly created base_provider for the object
void base_provider_generator::register_object(base_ptr object, const std::string& options)
{
	if (object->get_interface<provider>())
		return;
	if (!generate_object_gui(object))
		unmatched_objects.push_back(object);
}


/// remove also the base_provider of an object if created
void base_provider_generator::unregister_object(base_ptr object, const std::string& options)
{
	pvd_map_iter iter = providers.find(object);
	if (iter == providers.end())
		return;
	cgv::base::unregister_object(iter->second);
	providers.erase(iter);

	for (unsigned i=0; i<unmatched_objects.size(); ++i)
		if (unmatched_objects[i] == object) {
			unmatched_objects.erase(unmatched_objects.begin()+i);
			--i;
		}
}

	}
}
