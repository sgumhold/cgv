#pragma once

#include <cgv/base/base.h>
#include <cgv/base/named.h>
#include <cgv/utils/token.h>
#include <cgv/type/info/type_name.h>
#include <string>
#include <iostream>
#include <map>

#include "lib_begin.h"

#ifndef _WIN32
#include <unistd.h>
#include <dlfcn.h>
#endif

/// the cgv namespace
namespace cgv {
	/// the base namespace holds the base hierarchy, support for plugin registration and signals
	namespace base {


/**@name control over the registration process */
//@{
//! Enable registration (default is that registration is disabled).
/*! If registration has been disabled before, send all registration events that
    where emitted during disabled registration. */
extern void CGV_API enable_registration();
/// if registration is disable, all registration events are stored and sent at the momement when registration is enabled again. This feature is used when loading dlls
extern void CGV_API disable_registration();
/// check whether registration is enabled
extern bool CGV_API is_registration_enabled();
//! specify a partial order of objects for registration
/*! \c partial_order is a semicolon separated list of type names that can ignore name spaces.
    \c before_constructor_execution tells whether the reordering should happen before constructors of delayed registration events are called.
	\c when specifies in which call to \c enable_registration the reordering should happen. Possible values are
	- "always"
	- "program" only once for the enable event of the executed program
	- "plugins" for enable events of all loaded plugins
	- <plugin_name> only for the enable event of the plugin with the given name
	If several partial orders are defined for an enable call, a combined partial order is computed and used to find the order closest to the actual
	registration order that is in accordance to the combined partial order. */
extern void CGV_API define_registration_order(const std::string& partial_order, bool before_contructor_execution = false, const std::string& when = "always");

/// helper class whose constructor calls the \c define_registration_order() function
struct CGV_API registration_order_definition
{
	registration_order_definition(const std::string& partial_order, bool before_contructor_execution = false, const std::string& when = "always");
};

/// enable registration debugging
extern void CGV_API enable_registration_debugging();
/// disable registration debugging
extern void CGV_API disable_registration_debugging();
/// check whether registration debugging is enabled
extern bool CGV_API is_registration_debugging_enabled();

/// register a registration listener that stores pointers to all registered objects
extern void CGV_API enable_permanent_registration();
/// deregister registration listener and dereference pointers to registered objects
extern void CGV_API disable_permanent_registration();
/// check whether permanent registration is enabled
extern bool CGV_API is_permanent_registration_enabled();
/// unregister all existing objects to clean up
extern void CGV_API unregister_all_objects();
/// calls the on_exit_request method for all registered objects and return true if exiting is allowed
extern bool CGV_API request_exit_from_all_objects();
/// access to number of permanently registered objects
extern unsigned CGV_API get_nr_permanently_registered_objects();
/// access to i-th permanently registered object
extern base_ptr CGV_API get_permanently_registered_object(unsigned i);


//! Enable cleanup of registration events (default).
/*! If registration event cleanup is disabled, registration events are not
    discarded as soon as objects have been registered. This makes objects
	available to listeners that are registered later. */
extern void CGV_API enable_registration_event_cleanup();
/// disable cleanup of registration events (see enable_registration_event_cleanup).
extern void CGV_API disable_registration_event_cleanup();
/// return whether registration cleanup is enabled
extern bool CGV_API is_registration_event_cleanup_enabled();

//@}

/**@name object registration */
//@{
//! register an object.
/*! This will send an event to all currently registered registration listeners. The options parameter
    can be used to select a specific listener. */
extern void CGV_API register_object(base_ptr object, const std::string& options = "");
/// unregister an object and send event to all currently registered registration listeners
extern void CGV_API unregister_object(base_ptr object, const std::string& options = "");

/// abstract base class of helpers to perform delayed registration and creation of objects in case that the registration is currently disabled
struct CGV_API object_constructor : public cgv::base::base
{
public:
	/// return the type name of the object constructor class
	std::string get_type_name() const { return cgv::type::info::type_name<object_constructor>::get_name(); }
	/// return the type name of the to be constructed object
	virtual std::string get_constructed_type_name() const = 0;
	/// creation function
	virtual base_ptr construct_object() const = 0;
};

// type specific specialization of helper class to perform delayed registration and creation of objects in case that the registration is disabled
template <class T>
class object_constructor_impl : public object_constructor
{
public:
	/// return the type name of the object constructor class
	std::string get_type_name() const { return cgv::type::info::type_name<object_constructor_impl<T> >::get_name(); }
	/// return the type name of the to be constructed object
	std::string get_constructed_type_name() const { return cgv::type::info::type_name<T>::get_name(); }
	// creation function
	base_ptr construct_object() const { return base_ptr(new T()); }
};

// type specific specialization of helper class to perform delayed registration and creation of objects in case that the registration is disabled
template <class T, typename CA>
class object_constructor_impl_1 : public object_constructor
{
	// buffer constructor argument
	CA ca;
public:
	// construct from option
	object_constructor_impl_1(const CA& _ca) : ca(_ca) {}
	/// return the type name of the object constructor class
	std::string get_type_name() const { return cgv::type::info::type_name<object_constructor_impl_1<T, CA> >::get_name(); }
	/// return the type name of the to be constructed object
	std::string get_constructed_type_name() const { return cgv::type::info::type_name<T>::get_name(); }
	// creation function
	base_ptr construct_object() const { return base_ptr(new T(ca)); }
};

// type specific specialization of helper class to perform delayed registration and creation of objects in case that the registration is disabled
template <class T, typename CA1, typename CA2>
class object_constructor_impl_2 : public object_constructor
{
	// buffer constructor arguments
	CA1 ca1;
	CA2 ca2;
public:
	// construct from option
	object_constructor_impl_2(const CA1& _ca1, const CA2& _ca2) : ca1(_ca1), ca2(_ca2) {}
	/// return the type name of the object constructor class
	std::string get_type_name() const { return cgv::type::info::type_name<object_constructor_impl_2<T,CA1,CA2> >::get_name(); }
	/// return the type name of the to be constructed object
	std::string get_constructed_type_name() const { return cgv::type::info::type_name<T>::get_name(); }
	// creation function
	base_ptr construct_object() const { return base_ptr(new T(ca1,ca2)); }
};

/// convenience class to register an object of the given class type
template <class T>
struct object_registration
{
	/// pass information about the target registration listener in the options argument
	explicit object_registration(const std::string& options) {
		if (is_registration_enabled())
			register_object(base_ptr(new T()),options);
		else
			register_object(base_ptr(new object_constructor_impl<T>()), options);
	}
};

/// convenience class to register an object of the given class type with one constructor argument
template <class T, typename CA>
struct object_registration_1
{
	/// pass information about the target registration listener in the options argument
	object_registration_1(const CA& arg, const std::string& options = "") {
		if (is_registration_enabled())
			register_object(base_ptr(new T(arg)),options);
		else
			register_object(base_ptr(new object_constructor_impl_1<T,CA>(arg)), options);
	}
};

/// convenience class to register an object of the given class type with two constructor arguments
template <class T, typename CA1, typename CA2>
struct object_registration_2
{
	/// pass information about the target registration listener in the options argument
	object_registration_2(const CA1& a1, const CA2& a2, const std::string& options = "")
	{
		if (is_registration_enabled())
			register_object(base_ptr(new T(a1,a2)),options);
		else
			register_object(base_ptr(new object_constructor_impl_2<T,CA1,CA2>(a1,a2)),options);
	}
};
//@}

/**@name support for driver, listener and factory registration*/
//@{

//! interfaces that add provides very basic functionality.
/*! Servers are registered before drivers and before listeners. */
struct CGV_API server
{
};

//! interfaces that add several listeners and objects.
/*! Drivers are special classes that provide extended functionality. They are registered
    after servers and before listeners. */
struct CGV_API driver
{
};

//! interfaces that allows to listen to registration events.
/*! In order to allow registration of a registration listener, the implementation must
    also inherit cgv::base::base. When registration had been disabled and is enabled again,
	registration listeners are registered before all other objects. */
struct CGV_API registration_listener
{
	/// overload to handle registration events
	virtual void register_object(base_ptr object, const std::string& options = "") = 0;
	/// overload to handle unregistration events
	virtual void unregister_object(base_ptr object, const std::string& options = "") = 0;
};

//! interface for a factory that allows to create objects derived from cgv::base::base
struct CGV_API factory : public base
{
protected:
	/// store the type name of the to be created objects
	std::string created_type_name;
	/// store whether the factory can only create one object
	bool is_singleton;
	/// pointer to the single object created by the factory in case is_singleton is true
	base_ptr singleton;
	/// store the options used for registering newly created objects
	std::string object_options;
public:
	/// construct
	factory(const std::string& _created_type_name, bool _singleton = false, const std::string& _object_options = "");
	/// return the options string used for object registration
	virtual std::string get_object_options() const;
	/// overload to return the type name of the objects that the factory can create
	const std::string& get_created_type_name() const;
	/// support creation of object by setting create property to true
	std::string get_property_declarations();
	///
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	///
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return whether the factory can only generate one instance of the given type
	bool is_singleton_factory() const;
	/// return pointer to singleton
	base_ptr get_singleton() const;
	/// release the singleton pointer
	void release_singleton();
	/// overload to create an object
	base_ptr create_object();
	/// overload to create an object
	virtual base_ptr create_object_impl() = 0;
};

/// implementation of factory for objects of type T using the standard constructor
template <class T>
struct factory_impl : public factory
{
	inline factory_impl(const std::string& _created_type_name, bool _is_singleton = false, const std::string& _object_options = "") :
				factory(_created_type_name, _is_singleton, _object_options) {}
	inline base_ptr create_object_impl() { return base_ptr(new T()); }
	std::string get_type_name() const { return std::string("factory_impl<")+get_created_type_name()+">"; }
};

/// implementation of factory for objects of type T using a constructor with one argument of type CA
template <class T, typename CA>
struct factory_impl_1 : public factory
{
	CA ca;
	inline factory_impl_1(CA _ca, const std::string& _created_type_name, bool is_singleton = false, const std::string& _object_options = "") :
		factory(_created_type_name, is_singleton, _object_options), ca(_ca) {}
	inline base_ptr create_object_impl() { return base_ptr(new T(ca)); }
	std::string get_type_name() const { return std::string("factory_impl_1<")+get_created_type_name()+">"; }
};

/// implementation of factory for objects of type T using a constructor with two arguments of types CA1 and CA2
template <class T, typename CA1, typename CA2>
struct factory_impl_2 : public factory
{
	CA1 ca1;
	CA2 ca2;
	inline factory_impl_2(CA1 _ca1, CA2 _ca2, const std::string& _created_type_name, bool is_singleton = false, const std::string& _object_options = "") :
		factory(_created_type_name, is_singleton, _object_options), ca1(_ca1), ca2(_ca2) {}
	inline base_ptr create_object_impl() { return base_ptr(new T(ca2)); }
	std::string get_type_name() const { return std::string("factory_impl_2<")+get_created_type_name()+">"; }
};

extern std::string CGV_API guess_created_type_name(const char* item_text);
extern void CGV_API register_factory_object(base_ptr fo, const char* item_text, char shortcut);

/// convenience class to register a factory of the given class type
template <class T>
struct factory_registration
{
	//! this registers an instance of the default factory implementation
	/*! parameters:
	    - \c _created_type_name ... name of the type of the instances created by the factory
	    - \c _options ... semicolon separated options used to register the factory. For gui
		                  integration these can include assignments to "menu_path" and "shortcut"
	    - \c _is_singleton ... whether the factory can create only one instance
		- \c _object_options ... options used to register created instances
	*/
	factory_registration(const std::string& _created_type_name, const std::string& _options = "", bool _is_singleton = false, const std::string& _object_options = "") {
		register_object(new factory_impl<T>(_created_type_name, _is_singleton, _object_options), _options);
	}
	//! this constructor is only provided for downward compatibility and should not be used anymore.
	/*! Item text and shortcut can be specified in the option string via
	    "menu_path=\"geometry/sphere\";shortcut='Q'". */
	factory_registration(const char* item_text, char shortcut, bool is_singleton = false) {
		register_factory_object(new factory_impl<T>(guess_created_type_name(item_text), is_singleton), item_text, shortcut);
	}
};

/// convenience class to register a factory of the given class type that uses a constructor with one argument of type CA
template <class T, typename CA>
struct factory_registration_1
{
	//! this registers an instance of a standard factory implementation
	/*! parameters:
        - \c _ca ... argument passed to the constructor of the created instances
		- \c _created_type_name ... name of the type of the instances created by the factory
	    - \c _options ... semicolon separated options used to register the factory. For gui integration these can include assignments to "menu_path" and "shortcut"
	    - \c _is_singleton ... whether the factory can create only one instance
		- \c _object_options ... options used to register created instances
	*/
	factory_registration_1(const std::string& _created_type_name, const CA& _ca, const std::string& _options = "", bool _is_singleton = false, const std::string& _object_options = "") {
		register_object(new factory_impl_1<T,CA>(_ca, _created_type_name, _is_singleton, _object_options), _options);
	}
	//! this constructor is only provided for downward compatibility and should not be used anymore.
	/*! Item text and shortcut can be specified in the option string via
	    "menu_path=\"geometry/sphere\";shortcut='Q'". */
	factory_registration_1(const char* item_text, char shortcut, const CA& _ca, bool is_singleton = false) {
		register_factory_object(new factory_impl_1<T,CA>(_ca, guess_created_type_name(item_text), is_singleton), item_text, shortcut);
	}
};

/// convenience class to register a factory of the given class type that uses a constructor with one argument of type CA
template <class T, typename CA1, typename CA2>
struct factory_registration_2
{
	//! this registers an instance of a standard factory implementation
	/*! parameters:
        - \c _ca1, \c _ca2 ... arguments passed to the constructor of the created instances
		- \c _created_type_name ... name of the type of the instances created by the factory
	    - \c _options ... semicolon separated options used to register the factory. For gui integration these can include assignments to "menu_path" and "shortcut"
	    - \c _is_singleton ... whether the factory can create only one instance
		- \c _object_options ... options used to register created instances
	*/
	factory_registration_2(const std::string& _created_type_name, const CA1& _ca1, const CA2& _ca2, const std::string& _options = "", bool _is_singleton = false, const std::string& _object_options = "") {
		register_object(new factory_impl_2<T,CA1,CA2>(_ca1, _ca2, _created_type_name, _is_singleton, _object_options), _options);
	}
};


//@}


/**@name test registration */
//@{
/// structure used to register a test function
class CGV_API test : public base
{
public:
	/// static counter for all tests
	static int nr_failed;
protected:
	/// name of test function
	std::string test_name;
	/// pointer to test function
	bool (*test_func)();
public:
	/// constructor for a test structure
	test(const std::string& _test_name, bool (*_test_func)());
	/// implementation of the type name function of the base class
	std::string get_type_name() const;
	/// access to name of test function
	std::string get_test_name() const;
	/// execute test and return whether this was successful
	bool exec_test() const;
};

/// use this macro in a test function to check whether expression V is true
#define TEST_ASSERT(V) \
	if (!(V)) { \
		std::cerr << "\n" << __FILE__ << "(" << __LINE__ << ") : error: test failure" << std::endl; \
		++cgv::base::test::nr_failed; \
	}

/// use this macro in a test function to check whether the two expression V and Q are equal
#define TEST_ASSERT_EQ(V,Q) \
	if ((V) != (Q)) { \
		std::cerr << "\n" << __FILE__ << "(" << __LINE__ << ") : error: test failure " << (V) << "!=" << (Q) << std::endl; \
		++cgv::base::test::nr_failed; \
	}

/// declare an instance of test_registration as static variable in order to register a test function in a test plugin
struct CGV_API test_registration
{
	/// the constructor creates a test structure and registeres the test
	test_registration(const std::string& _test_name, bool (*_test_func)());
};
//@}




/**@name resource file registration */
//@{
/// information registered with each resource file
struct CGV_API resource_file_info
{
	/// at which location the resource file starts within the executable or dll
	unsigned int file_offset;
	/// length of the resource file in bytes
	unsigned int file_length;
	/// pointer to
	const char* file_data;
	/// name of
	std::string source_file;
	/// construct resource file info
	resource_file_info(
		unsigned int _file_offset = 0,
		unsigned int _file_length = 0,
		const char* _file_data = 0,
		const std::string& _source_file = "");
};

/// return a reference to a mapping of resource file names to resource file infos
extern CGV_API std::map<std::string, resource_file_info>& ref_resource_file_map();

/// register a resource file
extern CGV_API void register_resource_file(const std::string& file_path, unsigned int file_offset, unsigned int file_length, const char* file_data, const std::string& source_file = "");

/// convenience class to register a resource file
struct CGV_API resource_file_registration
{
	/// builds a resource file info and registers it with the register_resource_file function
	resource_file_registration(const char* file_data);
};

/// register a resource string
extern CGV_API void register_resource_string(const std::string& string_name, const char* string_data);

/// convenience class to register a resource string
struct CGV_API resource_string_registration
{
	/// builds a resource file info and registers it with the register_resource_file function
	resource_string_registration(const std::string& string_name, const char* string_data);
};

//@}


/// interface for objects that process unknown command line arguments
struct argument_handler
{
	/// this function is called on registered objects with the list of unknown command line parameters
	virtual void handle_args(std::vector<std::string>& args) = 0;
};

/// enumerate type for all command types supported in configuration files
enum CommandType
{
	CT_UNKNOWN,    // command is not known to framework
	CT_EMPTY,      // command specification was empty
	CT_COMMENT,    // a comment was given starting with '/'
	CT_SHOW,       // a show command was specified, currently only show all is supported
	CT_PERSISTENT, // the persistent command means that all successive value set commands in a config file should be updated during execution when the user changes one of them
	CT_INITIAL,    // reverts a persistent command
	CT_PLUGIN,     // loads a plugin
	CT_CONFIG,     // executes a config file
	CT_GUI,        // loads a gui description file
	CT_NAME,       // sets a value of a registered object of the given name
	CT_TYPE        // sets a value of a registered object of the given type
};

/// a structure to store an analized command
struct command_info
{
	/// the command type
	CommandType command_type;
	/// the parameters, one file name parameter for PLUGIN, CONFIG, GUI and two parameters (name/type, declarations) for NAME or TYPE commands
	std::vector<cgv::utils::token> parameters;
};

/// parse a command and optionally store result in the command info, returns the command type
extern CommandType CGV_API analyze_command(const cgv::utils::token& cmd, bool eliminate_quotes = true, command_info* info_ptr = 0);

/// process a command given by a command info structure, return whether command was processed correctly
extern bool CGV_API process_command(const command_info& info);

/**@name processing of commands*/
//@{

//! process a command given as string.
/*! Return whether the command was processed correctly. If eliminate_quotes is
    set to true, quotes around the command arguments are eliminated. This feature
	is used for commands specified on the command line, where spaces in the command
	arguments would split one command into pieces. Quotes are used then to protect the
	command from splitting.

	The following commands are supported:
	- show all                   ... print out information on all registered objects
	- plugin:file_name           ... read a plugin
	- config:file_name           ... read a config file
	- gui:file_name              ... read a gui description file
	- name(xxx):assignment list  ... find registered object by name xxx and process assignments on them
	- type(yyy):assignment list  ... find registered object by type yyy and process assignments on them

	The assigment list in the name and type commands are of the form:

	member_name_1=value_1;member_name_2=value_2;...
*/
extern bool CGV_API process_command(const std::string& cmd, bool eliminate_quotes = true);

/// show information about all registered members
extern void CGV_API show_all();
//@}

/**@name processing of command line arguments*/
//@{
//! set the file name of the current program.
/*! simply pass argv[0] in the main procedure. This is done automatically in the
    process_command_line_args function.*/
extern CGV_API void register_prog_name(const char* prog_name);
/// return a refence to the name of the started executable
extern CGV_API std::string& ref_prog_name();
/// return a refence to the path prefix of the started executable, this can be prepended for example to dll names
extern CGV_API std::string& ref_prog_path_prefix();
/// process the command line arguments: extract program name and load all plugins
extern CGV_API void process_command_line_args(int argc, char** argv);
//@}

/**@name configuration and gui files*/
//@{

//! abstract interface for observers of config files.
/*! The typically used implementation is found in the cgv_gui library.*/
struct config_file_observer
{
	/// to be implemented method that adds permanent registration for a list of property assignments
	virtual void multi_observe(base_ptr bp, const std::string& property_assignments, size_t off) = 0;
};

//! abstract interface for a config file driver that handles permanent registration and gui config files.
/*! The typically used implementation is found in the cgv_gui library.*/
struct config_file_driver
{
public:
	/// create or find a config_file_observer from the given file name and the read content of the config file
	virtual config_file_observer* find_config_file_observer(const std::string& file_name, const std::string& content) = 0;
	/// process a gui file
	virtual bool process_gui_file(const std::string& file_name) = 0;
};

/// method to register a config_file_driver
extern CGV_API void register_config_file_driver(config_file_driver* cfd);

/// in case permanent registration is active, look for a registered object by name
extern named_ptr CGV_API find_object_by_name(const std::string& name);
/// in case permanent registration is active, look for a registered object by type name
extern base_ptr CGV_API find_object_by_type(const std::string& type_name);
/// interpret a config file
extern bool CGV_API process_config_file(const std::string& file_name);
/// interpret a gui file
extern bool CGV_API process_gui_file(const std::string& file_name);
//@}

/**@name loading of plugins*/
//@{
//! load a plugin or dll and return a handle to the plugin, or 0 if loading was not successful.
/*! During plugin loading the registration is always disabled in order to avoid deadlocks
    that can arise when a registered object triggers loading of another dll.*/
extern CGV_API void* load_plugin(const std::string& file_name);
/// return a reference to the currently loaded plugin
extern CGV_API std::string& ref_plugin_name();
/// unload the plugin with the given handle
extern CGV_API bool unload_plugin(void* handle);
//@}


	}
}

#include <cgv/config/lib_end.h>
