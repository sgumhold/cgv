#include <cgv/base/register.h>
#include <cgv/utils/convert.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/file.h>
#include <cgv/type/variant.h>

#include <algorithm>
#include <vector>
#include <set>

#if defined(_WIN32)
#include <Windows.h>
#include <strsafe.h>
#if defined(NDEBUG)
#define CGV_NDEBUG
#endif
#else
#if defined(DEBUG)
#else
#define CGV_NDEBUG
#endif
#endif

using namespace cgv::utils;

namespace cgv {
	namespace base {



/**************** additional types *********************/

struct registration_info 
{
	bool registration_enabled;
	bool permanent_registration;
	bool registration_event_cleanup;
	unsigned int nr_events_before_disable;
	registration_info() 
	{
		registration_enabled = false;
		permanent_registration = true;
		registration_event_cleanup = false;
		nr_events_before_disable = 0;
	}
};

struct registration_order_info
{
	std::string partial_order;
	bool before_contructor_execution;
	std::string when;
	registration_order_info() : before_contructor_execution(false) 
	{
	}
	registration_order_info(const std::string& _partial_order, bool _before_contructor_execution, const std::string& _when) :
		partial_order(_partial_order),
		before_contructor_execution(_before_contructor_execution),
		when(_when)
	{
	}
};

struct object_collection
{
	std::vector<base_ptr> objects;
	void add_object(base_ptr object)
	{
		objects.push_back(object);
	}
	void remove_object(base_ptr object)
	{
		for (unsigned int i=0; i<objects.size(); ++i) {
			if (object == objects[i]) {
				objects.erase(objects.begin()+i);
				++i;
			}
		}
	}
	void unregister_all_objects()
	{
		while (objects.size() > 0) {
			unregister_object(objects.back());
		}
	}
	named_ptr find_object_by_name(const std::string& name)
	{
		for (unsigned int oi=0; oi<objects.size(); ++oi) {
			named_ptr np = objects[oi]->cast<named>();
			if (np && np->get_name() == name)
				return np;
		}
		return named_ptr();
	}
	base_ptr find_object_by_type(const std::string& type_name)
	{
		for (unsigned int oi=0; oi<objects.size(); ++oi) {
			base_ptr bp = objects[oi];
			if (type_name == bp->get_type_name())
				return bp;
		}
		return base_ptr();
	}
};


/**************** static variables *********************/

object_collection& ref_object_collection()
{
	static object_collection oc;
	return oc;
}

std::vector<registration_order_info>& ref_registration_order_infos()
{
	static std::vector<registration_order_info> roi;
	return roi;
}

std::string& ref_prog_name()
{
	static std::string prog_name;
	return prog_name;
}

std::string& ref_prog_path_prefix()
{
	static std::string prog_path_prefix;
	return prog_path_prefix;
}

std::string& ref_plugin_name()
{
	static std::string plugin_name;
	return plugin_name;
}

std::map<std::string, resource_file_info>& ref_resource_file_map()
{
	static std::map<std::string, resource_file_info> resource_file_map;
	return resource_file_map;
}

registration_info& ref_info()
{
	static registration_info ri;
	return ri;
}

std::vector<std::pair<base_ptr,std::string> >& ref_registration_events()
{
	static std::vector<std::pair<base_ptr,std::string> > registration_events;
	return registration_events;
}

std::vector<base_ptr>& ref_listeners()
{
	static std::vector<base_ptr> listeners;
	return listeners;
}


/****************** helper functions **************/


void show_split_lines(const std::string& s)
{
	if (s.empty()) 
		return;
	std::vector<token> toks;
	bite_all(tokenizer(s).set_ws(";"), toks);
	for (unsigned i=0; i<toks.size(); ++i) 
		std::cout << "\n    " << to_string(toks[i]).c_str();
}

bool& ref_registration_debugging_enabled()
{
#if defined CGV_FORCE_STATIC && defined _DEBUG
	static bool is_debug = true;
#else
	static bool is_debug = false;
#endif
	return is_debug;
}

void show_object_debug_info(cgv::base::base_ptr o)
{
	std::cout << o->get_type_name();
	if (o->get_named())
		std::cout << "<" << o->get_named()->get_name() << ">";
	bool is_c = o->get_interface<object_constructor>() != 0;
	bool is_s = o->get_interface<server>() != 0;
	bool is_d = o->get_interface<driver>() != 0;
	bool is_l = o->get_interface<registration_listener>() != 0;

	if (is_c || is_s || is_d || is_l) {
		std::cout << " [";
		if (is_c)
			std::cout << "C";
		if (is_s)
			std::cout << "S";
		if (is_d)
			std::cout << "D";
		if (is_l)
			std::cout << "L";
		std::cout << "]";
	}
}


/// register an object and send event to all current registration ref_listeners()
void register_object_internal(base_ptr object, const std::string& options)
{
	if (is_registration_debugging_enabled()) {
		std::cout << "REG OBJECT ('" << options << "') ";
		show_object_debug_info(object);
		std::cout << std::endl;

		static std::map<cgv::base::base*, int> is_registered;
		if (is_registered.find(&(*object)) == is_registered.end())
			is_registered[&(*object)] = 1;
		else {
			++is_registered[&(*object)];
			std::cerr << "ERROR: last object registered " << is_registered[&(*object)] << " times" << std::endl;
		}
	}

	// send register event to all listeners
	for (unsigned i = 0; i<ref_listeners().size(); ++i)
		ref_listeners()[i]->get_interface<registration_listener>()->register_object(object, options);

	// perform permanent registration
	if (is_permanent_registration_enabled())
		ref_object_collection().add_object(object);
	// send register event to object
	object->on_register();
}


/****************** implementation of exported functions **************/

void define_registration_order(const std::string& partial_order, bool before_contructor_execution, const std::string& when)
{
	ref_registration_order_infos().push_back(registration_order_info(partial_order, before_contructor_execution, when));
}

void add_partially_ordered(const std::vector<std::set<unsigned> >& combined_partial_order, std::vector<unsigned>& permutation, std::vector<bool>& appended, std::vector<bool>& delayed, unsigned i)
{
	if (delayed[i]) {
		std::cout << "REG ORDER cyclic dependency of registration event <" << ref_registration_events()[i].first->get_type_name() << ">" << std::endl;
		return;
	}
	delayed[i] = true;
	// ensure that indices that have to come before i, are also added before i
	for (auto j : combined_partial_order[i]) {
		if (!appended[j])
			add_partially_ordered(combined_partial_order, permutation, appended, delayed, j);
	}
	delayed[i] = false;
	appended[i] = true;
	permutation.push_back(i);
}

void sort_registration_events(bool before_contructor_execution)
{
	// initialized combined partial order
	size_t N = ref_registration_events().size();
	std::vector<std::set<unsigned> > combined_partial_order;
	combined_partial_order.resize(N);
	unsigned nr_partial_orders = 0;

	// iterate all registration order infos
	for (auto roi : ref_registration_order_infos()) {
		// ignore if before_contructor_execution does not match
		if (roi.before_contructor_execution != before_contructor_execution)
			continue;

		// ignore if "when" does not match
		if (roi.when == "plugins") {
			if (ref_plugin_name().empty())
				continue;
		}
		else if (roi.when == "program") {
			if (!ref_plugin_name().empty())
				continue;
		}
		else if (roi.when != "always") {
			if (ref_plugin_name() != roi.when)
				continue;
		}

		// extract partial order
		std::vector<cgv::utils::token> toks;
		cgv::utils::tokenizer(roi.partial_order).set_ws(";").bite_all(toks);

		// first construct a vector with indices of registration events
		std::vector<unsigned> event_indices;
		unsigned nr_matched = 0;
		for (auto t : toks) {
			bool found = false;
			for (unsigned i = 0; i < N; ++i) {
				std::string tn = ref_registration_events()[i].first->get_type_name();
				if (t == tn) {
					event_indices.push_back(i);
					++nr_matched;
					found = true;
					break;
				}
			}
			if (!found) {
				std::cout << "REG ORDER: could not find event <" << t << ">" << std::endl;
			}
		}
		if (nr_matched < 2) {
			std::cout << "REG ORDER: partial order <" << roi.partial_order << "> did match only " << nr_matched << " object" << std::endl;
			continue;
		}

		// extend combined partial order by current partial order
		for (unsigned i = 1; i < event_indices.size(); ++i)
			combined_partial_order[event_indices[i]].insert(event_indices[i - 1]);
		++nr_partial_orders;
	}
	if (nr_partial_orders == 0)
		return;

	// compute permutation
	std::vector<unsigned> permutation;
	std::vector<bool> appended(N, false);
	std::vector<bool> delayed(N, false);
	unsigned i;
	for (i = 0; i < N; ++i) {
		if (!appended[i])
			add_partially_ordered(combined_partial_order, permutation, appended, delayed, i);
	}
	if (permutation.size() != N) {
		std::cerr << "ERROR: could not compute valid permutation of registration events" << std::endl;
		abort();
	}

	// permute registration events
	std::vector<std::pair<base_ptr, std::string> > permuted_registration_events;
	permuted_registration_events.resize(N);
	for (i = 0; i < N; ++i)
		permuted_registration_events[i] = ref_registration_events()[permutation[i]];
	ref_registration_events() = permuted_registration_events;
}

registration_order_definition::registration_order_definition(const std::string& partial_order, bool before_contructor_execution, const std::string& when)
{
	define_registration_order(partial_order, before_contructor_execution, when);
}


void enable_registration_debugging()
{
	ref_registration_debugging_enabled() = true;
}
/// disable registration debugging
void disable_registration_debugging()
{
	ref_registration_debugging_enabled() = false;
}

/// check whether registration debugging is enabled
bool is_registration_debugging_enabled()
{
	return ref_registration_debugging_enabled();
}
/// enable registration and send all registration events that where emitted during disabled registration
void enable_registration()
{
	if (is_registration_enabled())
		return;

	if (is_registration_debugging_enabled())
		std::cout << "REG ENABLE <" << (ref_plugin_name().empty() ? ref_prog_name() : ref_plugin_name()) << "> Begin" << std::endl;
	
	unsigned i, i0 = ref_info().nr_events_before_disable;

	sort_registration_events(true);

	// first execute delayed registrations by replacing constructor objects with constructed objects
	for (i=i0; i<ref_registration_events().size(); ++i) {
		base_ptr o = ref_registration_events()[i].first;
		object_constructor* obr = o->get_interface<object_constructor>();
		if (obr) {
			if (is_registration_debugging_enabled())
				std::cout << "REG CONSTRUCT " << obr->get_constructed_type_name() << "('" << ref_registration_events()[i].second << "')";
			
			ref_registration_events()[i].first = obr->construct_object();
			
			if (is_registration_debugging_enabled()) {
				std::cout << " -> ";
				show_object_debug_info(ref_registration_events()[i].first);
				std::cout << std::endl;
			}
		}
	}

	sort_registration_events(false);

	// next register all servers
	const std::vector<base_ptr>& L = ref_listeners();
	for (i=i0; i<ref_registration_events().size(); ++i) {
		base_ptr object = ref_registration_events()[i].first;
		if (object->get_interface<server>() == 0)
			continue;
		register_object_internal(object, ref_registration_events()[i].second);
	}

	// next register all drivers
	for (i=i0; i<ref_registration_events().size(); ++i) {
		base_ptr object = ref_registration_events()[i].first;
		if (object->get_interface<driver>() == 0)
			continue;
		register_object_internal(object, ref_registration_events()[i].second);
	}

	// next register all listeners
	for (i=i0; i<ref_registration_events().size(); ++i) {
		base_ptr object = ref_registration_events()[i].first;
		if (object->get_interface<registration_listener>() == 0)
			continue;
		register_object_internal(object, ref_registration_events()[i].second);

		ref_listeners().push_back(object);
		// send all buffered events
		for (unsigned j=0; j<i0; ++j)
			object->get_interface<registration_listener>()->register_object(ref_registration_events()[j].first,
								ref_registration_events()[j].second);
	}

	// next register all remaining objects
	for (i=i0; i<ref_registration_events().size(); ++i) {
		base_ptr object = ref_registration_events()[i].first;
		if (object->get_interface<registration_listener>() != 0 ||
			object->get_interface<driver>() != 0 ||
			object->get_interface<server>() != 0)
			continue;
		register_object_internal(object, ref_registration_events()[i].second);
	}

	// remove registration events
	if (is_registration_event_cleanup_enabled())
		ref_registration_events().clear();

	ref_info().nr_events_before_disable = (unsigned) ref_registration_events().size();
	ref_info().registration_enabled = true;

	if (is_registration_debugging_enabled())
		std::cout << "REG ENABLE <" << (ref_plugin_name().empty() ? ref_prog_name() : ref_plugin_name()) << "> End" << std::endl;
}

void disable_registration()
{
	if (!is_registration_enabled())
		return;
	if (is_registration_debugging_enabled())
		std::cout << "REG DISABLE <" << (ref_plugin_name().empty() ? ref_prog_name() : ref_plugin_name()) << ">" << std::endl;

	ref_info().nr_events_before_disable = (unsigned)ref_registration_events().size();
	ref_info().registration_enabled = false;
}

/// check whether registration is enabled
bool is_registration_enabled()
{
	return ref_info().registration_enabled;
}

void enable_permanent_registration()
{
	if (is_permanent_registration_enabled())
		return;
	if (is_registration_debugging_enabled()) {
		std::cout << "ENABLE PERMANENT REGISTRATION" << std::endl;
	}
	ref_info().permanent_registration = true;
}

void unregister_all_objects()
{
	ref_object_collection().unregister_all_objects();
}

/// access to number of permanently registered objects
unsigned get_nr_permanently_registered_objects()
{
	return (unsigned)ref_object_collection().objects.size();
}

/// access to i-th permanently registered object
base_ptr get_permanently_registered_object(unsigned i)
{
	if (i >= get_nr_permanently_registered_objects())
		return 0;
	return ref_object_collection().objects[i];
}

void disable_permanent_registration()
{
	if (!is_permanent_registration_enabled())
		return;
	if (is_registration_debugging_enabled()) {
		std::cout << "DISABLE PERMANENT REGISTRATION" << std::endl;
	}
	ref_info().permanent_registration = false;
}

/// check whether permanent registration is enabled
bool is_permanent_registration_enabled()
{
	return ref_info().permanent_registration;
}

void enable_registration_event_cleanup()
{
	if (is_registration_event_cleanup_enabled())
		return;
	if (is_registration_debugging_enabled()) {
		std::cout << "ENABLE REGISTRATION CLEANUP" << std::endl;
	}
	ref_info().registration_event_cleanup = true;
	if (is_registration_enabled())
		ref_registration_events().clear();
	else
		ref_registration_events().erase(
			ref_registration_events().begin(),
			ref_registration_events().begin()+ref_info().nr_events_before_disable);
	ref_info().nr_events_before_disable = 0;
}

//! disable cleanup of registration events (see enable_registration_event_cleanup).
void disable_registration_event_cleanup()
{
	if (!is_registration_event_cleanup_enabled())
		return;
	if (is_registration_debugging_enabled()) {
		std::cout << "DISABLE REGISTRATION CLEANUP" << std::endl;
	}
	ref_info().registration_event_cleanup = false;
}

bool is_registration_event_cleanup_enabled()
{
	return ref_info().registration_event_cleanup;
}

/// register an object and send event to all current registration ref_listeners()
void register_object(base_ptr object, const std::string& options)
{
	// if registration is disabled or if registratration event cleanup is disabled, store registration event
	if (!is_registration_enabled() || !is_registration_event_cleanup_enabled()) {
		ref_registration_events().push_back(std::pair<base_ptr, std::string>(object, options));
		
		if (is_registration_debugging_enabled()) {
			std::cout << "REG EVENT ('" << options << "') ";
			show_object_debug_info(object);
			std::cout << std::endl;
		}
	}
	if (!is_registration_enabled())
		return;

	// execute object constructor in case of delayed registration
	object_constructor* oc = object->get_interface<object_constructor>();
	if (oc)
		object = oc->construct_object();

	// register as listener if necessary
	registration_listener* rl = object->get_interface<registration_listener>();
	if (rl) {
		ref_listeners().push_back(object);
		// send all buffered events
		if (is_registration_enabled()) {
			// next register all remaining objects
			for (unsigned i = 0; i<ref_registration_events().size(); ++i)
				rl->register_object(ref_registration_events()[i].first,
				ref_registration_events()[i].second);
		}
	}

	register_object_internal(object, options);
}

/// unregister an object and send event to all current registration ref_listeners()
void unregister_object(base_ptr object, const std::string& options)
{
	unsigned int i;

	if (is_registration_debugging_enabled()) {
		std::cout << "UNREG " << object.operator->() << ", '" << options << "' (" << ref_object_collection().objects.size() << ")" << std::endl;
	}

	// remove from permanent registration
	ref_object_collection().remove_object(object);

	// remove from registration events
	for (i=0; i<ref_registration_events().size(); ++i)
		if (ref_registration_events()[i].first == object) {
			ref_registration_events().erase(ref_registration_events().begin()+i);
			--i;
		}

	// remove from listeners
	if (object->get_interface<registration_listener>()) {
		for (i=0; i<ref_listeners().size(); ++i) {
			if (ref_listeners()[i] == object) {
				ref_listeners().erase(ref_listeners().begin() + i);
				--i;
			}
		}
	}
	// send unregister events to remaining listeners
	for (unsigned int i=0; i<ref_listeners().size(); ++i)
		ref_listeners()[i]->get_interface<registration_listener>()->unregister_object(object, options);

	// send unregister event to object itself
	object->unregister();
}

/// in case permanent registration is active, look for a registered object by name
named_ptr find_object_by_name(const std::string& name)
{
	return ref_object_collection().find_object_by_name(name);
}

/// in case permanent registration is active, look for a registered object by type name
base_ptr find_object_by_type(const std::string& type_name)
{
	return ref_object_collection().find_object_by_type(type_name);
}

std::string get_config_file_name(const std::string& _file_name)
{
	std::string file_name = _file_name;
	if (file::get_extension(file_name) == "def") {
		std::string fn = file::drop_extension(file_name)+".cfg";
		if (file::exists(fn))
			file_name = fn;
	}
	return file_name;
}

//bool process_command_ext(const token& cmd, bool eliminate_quotes, bool* persistent = 0, config_file_observer* cfo = 0, const char* begin = 0);
bool process_command_ext(const command_info& info, bool* persistent = 0, config_file_observer* cfo = 0, const char* begin = 0);

config_file_driver*& ref_config_file_driver()
{
	static config_file_driver* driver = 0;
	return driver;
}

void register_config_file_driver(config_file_driver* cfd)
{
	if (ref_config_file_driver())
		std::cerr << "warning: registering more than one config_file_driver" << std::endl;
	ref_config_file_driver() = cfd;
}

config_file_observer* find_config_file_observer(const std::string& file_name, const std::string& content)
{
	if (!ref_config_file_driver()) {
		std::cerr << "warning: attempt to use permanent registration without a registered config_file_driver" << std::endl;
		return 0;
	}
	return ref_config_file_driver()->find_config_file_observer(file_name, content);
}

bool process_config_file_ext(const std::string& _file_name, bool* persistent = 0)
{
	// update file name extension
	std::string file_name = get_config_file_name(_file_name);
	if (file_name.empty())
		return false;

	// try to read file
	std::string content;
	if (!file::read(file_name, content, true)) {
		std::cerr << "couldn't read config file " << file_name.c_str() << std::endl;
		return false;
	}

	bool pers = false;
	if (!persistent)
		persistent = &pers;

	config_file_observer* cfo = find_config_file_observer(file_name, content);

	// split file content into lines
	std::vector<line> lines;
	split_to_lines(content,lines);

	// interpret each line as a command
	unsigned int i;
	for (i = 0; i < lines.size(); ++i) {
		command_info info;
		analyze_command((token&)(lines[i]), false, &info);
		process_command_ext(info, persistent, cfo, &content[0]);
		// process_command_ext((token&)(lines[i]), false, persistent, cfo, &content[0]);
	}
	return true;
}

bool process_config_file(const std::string& _file_name)
{
	return process_config_file_ext(_file_name);
}

/// interpret a gui file
bool process_gui_file(const std::string& file_name)
{
	config_file_driver* cfd = ref_config_file_driver();
	if (cfd)
		return cfd->process_gui_file(file_name);
	std::cerr << "attempt to process gui file without a config_file_driver registered" << std::endl;
	return false;
}


test::test(const std::string& _test_name, bool (*_test_func)())
	: test_name(_test_name), test_func(_test_func)
{
}

std::string test::get_test_name() const
{
	return test_name;
}

bool test::exec_test() const
{
	return test_func();
}

int cgv::base::test::nr_failed = 0;

std::string test::get_type_name() const
{
	return "test";
}

test_registration::test_registration(const std::string& _test_name, bool (*_test_func)())
{
	register_object(base_ptr(new test(_test_name,_test_func)),"");
}


/// construct 
factory::factory(const std::string& _created_type_name, bool _singleton, const std::string& _object_options)
	: created_type_name(_created_type_name), is_singleton(_singleton), object_options(_object_options)
{
}

/// return the options string used for object registration
std::string factory::get_object_options() const
{
	return object_options;
}


/// support creation of object by setting create property to true
std::string factory::get_property_declarations()
{
	return "create:bool";
}


/// 
bool factory::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "create") {
		bool do_create;
		cgv::type::get_variant(do_create, value_type, value_ptr);
		if (do_create)
			register_object(create_object());
		return true;
	}
	else
		return false;
}

/// 
bool factory::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "create") {
		cgv::type::set_variant(true, value_type, value_ptr);
		return true;
	}
	else
		return false;
}

/// overload to return the type name of the objects that the factory can create
const std::string& factory::get_created_type_name() const
{
	return created_type_name;
}
/// return whether the factory can only generate one instance of the given type
bool factory::is_singleton_factory() const
{
	return is_singleton;
}

/// return pointer to singleton
base_ptr factory::get_singleton() const
{
	return singleton;
}
/// release the singleton pointer
void factory::release_singleton()
{
	singleton.clear();
}

/// overload to create an object
base_ptr factory::create_object()
{
	base_ptr o = create_object_impl();
	if (is_singleton_factory())
		singleton = o;
	return o;
}

std::string guess_created_type_name(const char* item_text)
{
	std::vector<token> toks;
	tokenizer(item_text).set_ws("/").bite_all(toks);
	if (toks.empty())
		return "";
	return to_string(toks.back());
}

void register_factory_object(base_ptr fo, const char* item_text, char shortcut)
{
	std::string options("menu_text=\"");
	options += item_text;
	options += "\";shortcut='Ctrl-";
	options += shortcut;
	options += "'";
	register_object(fo, options);
}

void register_prog_name(const char* _prog_name)
{	
	ref_prog_name() = cgv::utils::file::get_file_name(_prog_name);
	std::string prog_path_prefix = cgv::utils::file::clean_path(cgv::utils::file::get_path(_prog_name));
	if (!prog_path_prefix.empty())
		prog_path_prefix += '/';
	ref_prog_path_prefix() = prog_path_prefix;
}

resource_file_info::resource_file_info(
	unsigned int _file_offset, 
	unsigned int _file_length, 
	const char* _file_data, const 
	std::string& _source_file)
	: file_offset(_file_offset), 
	  file_length(_file_length), 
	  file_data(_file_data), 
	  source_file(_source_file) {}

void register_resource_file(const std::string& file_path, unsigned int file_offset, unsigned int file_length, const char* file_data, const std::string& source_file)
{
	ref_resource_file_map()[file_path] = resource_file_info(file_offset, file_length, file_data, source_file);
}

void register_resource_string(const std::string& string_name, const char* string_data)
{
	ref_resource_file_map()[string_name] = resource_file_info(-1, (unsigned)std::string(string_data).size(), string_data);
}

void show_implementation(bool& implements_shown, const std::string& type_name)
{
	if (implements_shown)
		std::cout << ", ";
	else {
		std::cout << " implements ";
		implements_shown = true;
	}
	std::cout << type_name;
}

/// show information about all registered members
void show_all()
{
	const std::vector<base_ptr>& objects = ref_object_collection().objects;

	std::cout << "\n\n_______________ show all registered objects ______________________\n\n";
	for (unsigned int oi=0; oi<objects.size(); ++oi) {
		named_ptr np = objects[oi]->cast<named>();
		if (np)
			std::cout << "name(" << np->get_name().c_str() << "):" << np->get_type_name();
		else
			std::cout << "type(" << objects[oi]->get_type_name() << ")";
		bool implements_shown = false;
		if (objects[oi]->get_interface<server>())
			show_implementation(implements_shown,"server");
		if (objects[oi]->get_interface<driver>())
			show_implementation(implements_shown,"driver");
		if (objects[oi]->get_interface<registration_listener>())
			show_implementation(implements_shown,"registration_listener");
		if (objects[oi]->get_interface<factory>())
			show_implementation(implements_shown,"factory");
		show_split_lines(objects[oi]->get_property_declarations());
		std::cout << "\n\n";
	}
	std::cout << "__________________________________________________________________\n" << std::endl;
	return;
}

CommandType update_info(command_info* info_ptr, CommandType cmd, cgv::utils::token* args_tok_ptr = 0)
{

	if (info_ptr) {
		info_ptr->command_type = cmd;
		if (args_tok_ptr)
			info_ptr->parameters.push_back(*args_tok_ptr);
	}
	return cmd;
}

CommandType analyze_command(const cgv::utils::token& cmd, bool eliminate_quotes, command_info* info_ptr)
{
	// remove unnecessary stuff
	token cmd_tok = cmd;
	cmd_tok.begin = skip_spaces(cmd_tok.begin, cmd_tok.end);
	cmd_tok.end = cutoff_spaces(cmd_tok.begin, cmd_tok.end);

	// detect empty lines and comments
	if (cmd_tok.empty())
		return update_info(info_ptr, CT_EMPTY);

	// detect comments
	if (cmd_tok.get_length() > 1 && cmd_tok[0] == '/' && cmd_tok[1] == '/')
		return update_info(info_ptr, CT_COMMENT);

	// detect predefined commands
	if (cmd_tok == "show all")
		return update_info(info_ptr, CT_SHOW);
	if (cmd_tok == "persistent")
		return update_info(info_ptr, CT_PERSISTENT);
	if (cmd_tok == "initial")
		return update_info(info_ptr, CT_INITIAL);

	// determine command header
	token cmd_header = tokenizer(cmd_tok).set_sep(":").set_ws("").set_skip("\"'", "\"'").bite();
	if (cmd_header.end == cmd_tok.end)
		return update_info(info_ptr, CT_UNKNOWN);

	// and command arguments
	token args_tok(cmd_header.end + 1, cmd_tok.end);

	// eliminate quotes around argument, which need to be used in commands specified on the command line
	if (eliminate_quotes && args_tok.get_length() >= 2 &&
		((args_tok[0] == '"'  && args_tok[(int)args_tok.get_length() - 1] == '"') ||
		(args_tok[0] == '\'' && args_tok[(int)args_tok.get_length() - 1] == '\''))) {
		++args_tok.begin;
		--args_tok.end;
	}
	std::string args(to_string(args_tok));

	// detect direct commands
	if (cmd_header == "plugin")
		return update_info(info_ptr, CT_PLUGIN, &args_tok);
	if (cmd_header == "config")
		return update_info(info_ptr, CT_CONFIG, &args_tok);
	if (cmd_header == "gui")
		return update_info(info_ptr, CT_GUI, &args_tok);

	// split composed commands into head and argument
	std::vector<token> toks;
	tokenizer(cmd_header).set_sep("()").set_ws("").bite_all(toks);

	// check for name or type command
	if (toks.size() == 4 && toks[1] == "(" && toks[3] == ")" &&
		(toks[0] == "name" || toks[0] == "type")) {

		std::string identifier = to_string(toks[2]);
		if (info_ptr)
			info_ptr->parameters.push_back(toks[2]);
		return update_info(info_ptr, toks[0] == "name"  ? CT_NAME : CT_TYPE, &args_tok);
	}
	return update_info(info_ptr, CT_UNKNOWN);
}

bool process_command_ext(const command_info& info, bool* persistent, config_file_observer* cfo, const char* begin)
{
	switch (info.command_type) {
	case CT_SHOW:
		show_all();
		return true;
	case CT_PERSISTENT:
		if (persistent)
			*persistent = true;
		return true;
	case CT_INITIAL:
		if (persistent)
			*persistent = false;
		return true;
	case CT_PLUGIN: {
		if (load_plugin(to_string(info.parameters[0]))) {
			std::cout << "read plugin " << info.parameters[0] << std::endl;
			return true;
		}
		std::cerr << "error reading plugin " << info.parameters[0] << std::endl;

		// Reaching this point means the library could not be loaded.
		// Therefore use the approriate system facilities to print an error
		// message.
#ifdef _WIN32
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);

		std::wcerr << (LPTSTR)lpMsgBuf;

		LocalFree(lpMsgBuf);
#else
		auto error_string = dlerror();
		if (nullptr != error_string)
			std::cerr << error_string;
#endif

		return false;
	}
	case CT_CONFIG:
		if (process_config_file_ext(to_string(info.parameters[0]), persistent)) {
			std::cout << "read config file " << get_config_file_name(to_string(info.parameters[0])) << std::endl;
			return true;
		}
		std::cerr << "error reading config file " << info.parameters[0] << std::endl;
		return false;
	case CT_GUI:
		if (process_gui_file(to_string(info.parameters[0]))) {
			std::cout << "read gui file " << info.parameters[0] << std::endl;
			return true;
		}
		std::cerr << "error reading gui file " << info.parameters[0] << std::endl;
		return false;
	case CT_NAME:
	case CT_TYPE:
		{
			base_ptr bp;
			if (info.command_type == CT_NAME) {
				named_ptr np = find_object_by_name(to_string(info.parameters[0]));
				if (np) {
					std::cout << "name(" << np->get_name().c_str() << ")";
					bp = np;
				}
				else
					std::cerr << "could not find object of name '" << info.parameters[0] << "'" << std::endl;
			}
			else {
				bp = find_object_by_type(to_string(info.parameters[0]));
				if (bp)
					std::cout << "type(" << bp->get_type_name() << ")";
				else
					std::cerr << "could not find object of type <" << info.parameters[0] << ">" << std::endl;
			}
			if (bp) {
				std::string args = to_string(info.parameters[1]);
				// replace single quotes by double quotes
				for (unsigned int x = 0; x < args.size(); ++x)
					if (args[x] == '\'')
						args[x] = '"';

				show_split_lines(args);
				std::cout << "\n" << std::endl;
				if (persistent && *persistent && cfo)
					cfo->multi_observe(bp, args, info.parameters[1].begin - begin);
				else
					bp->multi_set(to_string(info.parameters[1]), true);
				return true;
			}
			return false;
		}
	}
	return false;
}

/*
bool process_command_ext(const token& cmd, bool eliminate_quotes, bool* persistent, config_file_observer* cfo, const char* begin)
{
	// remove unnecessary stuff
	token cmd_tok = cmd;
	cmd_tok.begin = skip_spaces(cmd_tok.begin, cmd_tok.end);
	cmd_tok.end   = cutoff_spaces(cmd_tok.begin, cmd_tok.end);

	// ignore empty lines and comments
	if (cmd_tok.empty() || cmd_tok[0] == '/')
		return true;

	// interpret predefined commands
	if (cmd_tok == "show all") {
		show_all();
		return true;
	}
	if (cmd_tok == "persistent") {
		if (persistent)
			*persistent = true;
		return true;
	}
	if (cmd_tok == "initial") {
		if (persistent)
			*persistent = false;
		return true;
	}

	// determine command header
	token cmd_header = tokenizer(cmd_tok).set_sep(":").set_ws("").set_skip("\"'","\"'").bite();
	if (cmd_header.end == cmd_tok.end) {
		std::cerr << "could not interpret command >" << to_string(cmd) << "< (probably missing a ':')!" << std::endl;
		return false;
	}
	// and command arguments
	token args_tok(cmd_header.end+1,cmd_tok.end);

	// eliminate quotes around argument, which need to be used in commands specified on the command line
	if (eliminate_quotes && args_tok.get_length() >= 2 &&
		 ( (args_tok[0] == '"'  && args_tok[(int)args_tok.get_length()-1] == '"') ||
		   (args_tok[0] == '\'' && args_tok[(int)args_tok.get_length()-1] == '\'') ) ) {
			 ++args_tok.begin;
			 --args_tok.end;
	}
	std::string args(to_string(args_tok));

	// perform direct commands
	if (cmd_header == "plugin") {
		if (load_plugin(args)) {
			std::cout << "read plugin " << args << std::endl;
			return true;
		}
		else {
			std::cerr << "error reading plugin " << args << std::endl;
			return false;
		}
	}
	if (cmd_header == "config") {
		if (process_config_file_ext(args, persistent)) {
			std::cout << "read config file " << get_config_file_name(args) << std::endl;
			return true;
		}
		else {
			std::cerr << "error reading config file " << args << std::endl;
			return false;
		}
	}
	if (cmd_header == "gui") {
		if (process_gui_file(args)) {
			std::cout << "read gui file " << args << std::endl;
			return true;
		}
		else {
			std::cerr << "error reading gui file " << args << std::endl;
			return false;
		}
	}

	// split composed commands into head and argument
	std::vector<token> toks;
	tokenizer(cmd_header).set_sep("()").set_ws("").bite_all(toks);

	// check for name or type command
	if (toks.size() == 4 && toks[1] == "(" && toks[3] == ")" && 
		(toks[0]=="name" || toks[0]=="type") ) {
		

		// replace single quotes by double quotes
		for (unsigned int x=0; x<args.size(); ++x)
			if (args[x] == '\'')
				args[x] = '"';


		std::string identifier = to_string(toks[2]);
		if (toks[0]=="name") {
			named_ptr np = find_object_by_name(identifier);
			if (np) {
				std::cout << "name(" << np->get_name().c_str() << ")";
				show_split_lines(args);
				std::cout << "\n" << std::endl;
				if (persistent && *persistent && cfo)
					cfo->multi_observe(np, args, (unsigned)(args_tok.begin - begin));
				else
					np->multi_set(args, true);
			}
			else {
				std::cerr << "could not find object of name '" << identifier << "'" << std::endl;
				return false;
			}
		}
		else {
			base_ptr bp = find_object_by_type(identifier);
			if (bp) {
				std::cout << "type(" << bp->get_type_name() << ")";
				show_split_lines(args);
				std::cout << "\n" << std::endl;
				if (persistent && *persistent && cfo)
					cfo->multi_observe(bp, args, (unsigned)(args_tok.begin - begin));
				else
					bp->multi_set(args, true);
			}
			else {
				std::cerr << "could not find object of type <" << identifier << ">" << std::endl;
				return false;
			}
		}
		return true;
	}
	else {
		std::cerr << "could not interpret command >" << to_string(cmd) << "< !" << std::endl;
		return false;
	}
}
*/

bool process_command(const command_info& info)
{
	return process_command_ext(info);
}

bool process_command(const std::string& cmd, bool eliminate_quotes)
{
	command_info info;
	analyze_command(cmd, eliminate_quotes, &info);
	return process_command_ext(info);
	//	return process_command_ext(token(cmd), eliminate_quotes);
}

/// process the command line arguments: extract program name and load all plugins
void process_command_line_args(int argc, char** argv)
{
	cgv::base::register_prog_name(argv[0]);
	for (int ai=1; ai<argc; ++ai)
		process_command(argv[ai]);
}


resource_file_registration::resource_file_registration(const char* symbol)
{
	const char* file_data = symbol+(int)symbol[0]+9;
	std::string file_path(symbol+1,(std::string::size_type)symbol[0]);
	symbol += symbol[0]+1;
	unsigned int file_offset = (unsigned char) symbol[0];
	file_offset += ((unsigned int) (unsigned char) symbol[1] << 8);
	file_offset += ((unsigned int) (unsigned char) symbol[2] << 16);
	file_offset += ((unsigned int) (unsigned char) symbol[3] << 24);
	symbol += 4;
	unsigned int file_size   = (unsigned char) symbol[0]+((unsigned int) (unsigned char) symbol[1] << 8)+((unsigned int) (unsigned char) symbol[2] << 16)+((unsigned int) (unsigned char) symbol[3] << 24);
	std::string source_file = ref_plugin_name();
	if (source_file.empty())
		source_file = ref_prog_name();
	register_resource_file(file_path, file_offset,file_size,file_data,source_file);
}

resource_string_registration::resource_string_registration(const std::string& string_name, const char* string_data)
{
	register_resource_string(string_name, string_data);
}

std::string extend_plugin_name(const std::string& fn)
{
	std::string n = cgv::utils::file::drop_extension(fn);
#ifdef _WIN64
	n += "64";
#endif // _WIN64

#if defined(_WIN32) || !defined(CGV_NDEBUG)
	n += "_";
#endif
#ifndef CGV_NDEBUG
	n += "d";
#endif
#ifdef _WIN32
#if defined(_MSC_VER) && _MSC_VER < 1500
	n += "8";
#elif defined(_MSC_VER) && _MSC_VER < 1600
	n += "9";
#elif defined(_MSC_VER) && _MSC_VER < 1700
	n += "10";
#elif defined(_MSC_VER) && _MSC_VER < 1800
	n += "11";
#elif defined(_MSC_VER) && _MSC_VER < 1900
	n += "12";
#elif defined(_MSC_VER) && _MSC_VER < 1910
	n += "14";
#elif defined(_MSC_VER)
	n += "141";
#endif
	n += ".dll";
#else
	n = std::string("lib")+n+".so";
#endif
	return n;
}

	}
}

#ifdef _WIN32
#	include <windows.h>
#	include <winbase.h>
#else
#	include <unistd.h>
#	ifdef __APPLE__
#		include "dlload_osx.cxx"
#		define RTLD_NOW 1 // set to anything for now
#	else
#		include <dlfcn.h>
#	endif
#endif

namespace cgv {
	namespace base {

void* load_plugin(const std::string& file_name)
{
	std::vector<token> names;
	bite_all(tokenizer(file_name).set_ws(",|;"), names);

	bool enabled = is_registration_enabled();
	if (enabled)
		disable_registration();

	void* result = 0;
	for (unsigned i=0; i<names.size(); ++i) {
		std::string fn[2];
		fn[0] = to_string(names[i]);
		fn[1] = extend_plugin_name(fn[0]);
#ifdef NDEBUG
#else
		std::swap(fn[0], fn[1]);
#endif
		result = 0;
		for (int j=0; j<2; ++j) {
			ref_plugin_name() = fn[j];
#ifdef _WIN32
#ifdef _UNICODE
			result = LoadLibrary(cgv::utils::str2wstr(fn[j]).c_str());
#else
			result = LoadLibrary(fn[j].c_str());
#endif
#else
			result = dlopen(fn[j].c_str(), RTLD_NOW);
#endif
			if (result)
				break;
		}
	}
	if (enabled)
		enable_registration();
	ref_plugin_name().clear();
	return result;
}
bool unload_plugin(void* handle)
{
#ifdef _WIN32
	return FreeLibrary((HMODULE)handle) != 0;
#else
	return dlclose(handle) != 0;
#endif
}

	}
}
