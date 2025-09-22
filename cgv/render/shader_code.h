#pragma once

#include <cgv/render/context.h>
#include <set>

#include "lib_begin.h"

namespace cgv {
	namespace render {

/** a globally unique shader config is registered by default when the cgv library
    is used. Currently it has one member only defining the search path for shader
	files. The shader path is initialized to the environment variable CGV_SHADER_PATH
	or empty otherwise. It can also be set with a command line argument of the form
	
	type(shader_config):shader_path='@(INPUT_PATH)'

	or in a config file as

	type(shader_config):shader_path='D:/my_shaders'

	To set the shader path at runtime, query the shader_config with the
	get_shader_config() function.
*/
struct CGV_API shader_config : public cgv::base::base
{
	/// the path used to find shaders with the cgv::utils::file::find_in_paths function
	std::string shader_path;
	/// whether to keep track of file names
	bool trace_file_names;
	/// whether to output full paths of read shaders
	bool show_file_paths;
	/// mapping of shader index to file name
	std::vector<std::string> shader_file_names;
	/// mapping of shader index to inserted files name
	std::vector<std::string> inserted_shader_file_names;
	/// construct config without file name tracing
	shader_config();
	/// return "shader_config"
	std::string get_type_name() const;
	/// reflect the shader_path member
	bool self_reflect(cgv::reflect::reflection_handler& srh);
};

/// type of ref counted pointer to shader configuration
typedef cgv::data::ref_ptr<shader_config> shader_config_ptr;

/// return a pointer to the current shader configuration
extern CGV_API shader_config_ptr get_shader_config();

/// @brief Stores preprocessor options used for conditionally compiling shader programs.
/// 
/// Options include standard preprocessor defines and text replacement snippets.
/// 
/// Preprocessor macros will be handled as follows:
///		- Existing macros in the source file that are defined in this class will have their values replaced.
///		- Existing macros in the source file that are not defined in this class will be left untouched.
///		- Macros not present in the source file but defined in this class will be added to the source before compilation.
/// 
/// Snippets will be handled as follows:
///		- Snippets are text replacements similar to macros but offer more flexibility. A snippet replacement text supports
///		  multi-line strings without the need to escape newlines.
///		- Snippet replacement locations in shader code are marked using special comments of form:
///			//$cgv::<identifier>
///		  where <identifier> is a user-defined name.
///		- The comment marking a snippet location is replaced with the content of a snippet whose id matches the given id.
///		- If at least one snippet is given, the additional define <CGV_USE_SNIPPETS> is set internally before compilation.
///		  To prevent ill-formed shader code due to potential missing definitions before snippet replacement, i.e.before pre-processing,
///		  affected parts of code can be disabled by enclosing them in a define guard like so:
///			#ifdef CGV_USE_SNIPPETS
///			...code relying on snippet content
///			#endif
class shader_compile_options {
public:
	using string_map = std::map<std::string, std::string>;
	
	/// Return true if no options are set.
	bool empty() const {
		return defines.empty() && snippets.empty();
	}

	/// Return const reference to defined macros.
	const string_map& get_macros() const {
		return defines;
	}

	/// Return const reference to defined snippets.
	const string_map& get_snippets() const {
		return snippets;
	}

	/// Define a macro as identifier and no replacement text.
	void define_macro(const std::string& identifier) {
		defines[identifier] = "";
	}

	/// @brief Define identifier as a macro with value as replacement text.
	/// 
	/// If value is of arithmetic type, it is converted to a string.
	/// If value is of boolean type, it is converted to 1 if true and 0 if false.
	/// If value is of enum type, it is first converted into its underlying arithmetic type.
	/// 
	/// An existing macro with the same identifier is overwritten with the new value.
	/// 
	/// @tparam T The value type.
	/// @param identifier The macro name.
	/// @param value The macro replacement value.
	template<typename T, typename std::enable_if<std::is_arithmetic_v<T>, bool>::type = true>
	void define_macro(const std::string& identifier, const T& value) {
		defines[identifier] = std::to_string(value);
	}

	template<typename T, typename std::enable_if<std::is_enum_v<T>, bool>::type = true>
	void define_macro(const std::string& identifier, const T& value) {
		defines[identifier] = std::to_string(static_cast<std::underlying_type_t<T>>(value));
	}

	void define_macro(const std::string& identifier, bool value) {
		defines[identifier] = value ? "1" : "0";
	}

	void define_macro(const std::string& identifier, const std::string& value) {
		defines[identifier] = value;
	}

	/// @brief Conditionally define identifier as a macro with value as replacement text.
	/// 
	/// The macro is only defined if value != default. If value is default and the macro
	/// is already defined it will be removed (undefined).
	/// 
	/// If value is of arithmetic type, it is converted to a string.
	/// If value is of boolean type, it is converted to 1 if true and 0 if false.
	/// If value is of enum type, it is first converted into its underlying arithmetic type.
	/// 
	/// An existing macro with the same identifier is overwritten with the new value.
	/// 
	/// @tparam T the value type.
	/// @param identifier The macro name.
	/// @param value The macro replacement value.
	/// @param default The default value used for comparison.
	template<typename T, typename std::enable_if<std::is_arithmetic_v<T>, bool>::type = true>
	void define_macro_if_not_default(const std::string& identifier, const T& value, const T& default) {
		if(value != default)
			defines[identifier] = std::to_string(value);
		else
			defines.erase(identifier);
	}

	template<typename T, typename std::enable_if<std::is_enum_v<T>, bool>::type = true>
	void define_macro_if_not_default(const std::string& identifier, const T& value, const T& default) {
		if(value != default)
			defines[identifier] = std::to_string(static_cast<std::underlying_type_t<T>>(value));
		else
			defines.erase(identifier);
	}

	void define_macro_if_not_default(const std::string& identifier, bool value, bool default) {
		if(value != default)
			defines[identifier] = value ? "1" : "0";
		else
			defines.erase(identifier);
	}

	void define_macro_if_not_default(const std::string& identifier, const std::string& value, const std::string& default) {
		if(value != default)
			defines[identifier] = value;
		else
			defines.erase(identifier);
	}

	/// @brief Conditionally define identifier as a macro with replacement text 1.
	/// 
	/// Will only define the macro if predicate is true. If predicate is false and
	/// the macro is already defined it will be removed (undefined).
	/// 
	/// @param predicate The condition.
	/// @param identifier The macro name.
	void define_macro_if_true(bool predicate, const std::string& identifier) {
		if(predicate)
			define_macro(identifier, "1");
		else
			undefine_macro(identifier);
	}

	/// @brief Remove (undefine) the macro with the given identifier.
	/// @param identifier The macro name.
	void undefine_macro(const std::string& identifier) {
		defines.erase(identifier);
	}

	/// @brief Define a text replacement snippet.
	/// @param identifier The snippet name.
	/// @param replacement_text The snippet content.
	void define_snippet(const std::string& identifier, const std::string& replacement_text) {
		snippets[identifier] = replacement_text;
	}

	/// @brief Remove (undefine) the snippet with the given identifier.
	/// @param identifier The snippet name.
	void undefine_snippet(const std::string& identifier) {
		snippets.erase(identifier);
	}

	/// @brief Extend options by content of other shader_compile_options via merging.
	/// 
	/// If overwrite is true, existing defines and snippets are overwritten with the content from other.
	/// 
	/// @param other The other options.
	/// @param overwrite If true, existing options are overwritten with other.
	void extend(const shader_compile_options& other, bool overwrite) {
		if(overwrite) {
			for(const auto& define : other.defines)
				defines[define.first] = define.second;

			for(const auto& snippet : other.snippets)
				snippets[snippet.first] = snippet.second;
		} else {
			defines.insert(other.defines.begin(), other.defines.end());
			snippets.insert(other.snippets.begin(), other.snippets.end());
		}
	}

	/// Compare two shader_compile_options for equality.
	bool operator==(const shader_compile_options& other) const {
		return defines == other.defines && snippets == other.snippets;
	}

	/// Compare two shader_compile_options for inequality.
	bool operator!=(const shader_compile_options& other) const {
		return !(*this == other);
	}

private:
	/// Maps pre-processor define identifiers to replacement texts
	string_map defines;
	/// Maps snippet identifiers to replacement texts
	string_map snippets;
};

/** a shader code object holds a code fragment of a geometry
    vertex or fragment shader and can be added to a shader 
	program. */
class CGV_API shader_code : public render_component
{
public:
	///create shader a shader code object
	shader_code();
	/// calls the destruct method
	~shader_code();
	/// decode a string if it is base64 encoded
	static void decode_if_base64(std::string& content);
	/// @brief Find the full path to a shader by its file name.
	/// 
	/// Check if the file exists. If not, check if a resource file of this file_name has
	/// been registered. If not, search it in the shader_file_name_map that caches paths
	/// for file names based on a recursive search through the paths given in the shader_path
	/// of the shader_config. This avoids excessive queries to the file system. The
	/// shader_path is accessed by the get_shader_config() method. This path is initialized
	/// to the environment variable CGV_SHADER_PATH or empty if that is not defined. If
	/// search_exhaustive is true and the file has not been found yet, search recursively
	/// in the shader_path.
	/// 
	/// @param file_name The shader file_name to search.
	/// @param search_exhaustive If true, a full recursive search through the given shader_path will be performed if all other attempts have failed.
	/// @return The full path of the shader file_name.
	static std::string find_file(const std::string& file_name, bool search_exhaustive = false);
	/** format given last error in a way that developer environments can locate errors in the source file */
	static std::string get_last_error(const std::string& file_name, const std::string& last_error);
	/// read shader code from file and return string with content or empty string if read failed
	static std::string read_code_file(const std::string &file_name, std::string* _last_error = 0);
	/// retreive shader code either by reading the file from disk or from the cache if enabled
	static std::string retrieve_code(const std::string& file_name, bool use_cache, std::string* _last_error);
	/** detect the shader type from the extension of the given
		 file_name, i.e.
		 - glvs ... ST_VERTEX
		 - glgs ... ST_GEOMETRY
		 - glfs ... ST_FRAGMENT
		 - glcs ... ST_COMPUTE
	*/
	static ShaderType detect_shader_type(const std::string& file_name);
	/// destruct shader code
	void destruct(const context& ctx);
	/** read shader code from file that is searched for with find_file.
	    If the shader type defaults to ST_DETECT, the detect_shader_type()
		 method is applied to the file name.*/
	bool read_code(const context& ctx, const std::string &file_name, ShaderType st = ST_DETECT, const shader_compile_options& options = {});
	/// set shader code from string
	bool set_code(const context& ctx, const std::string &source, ShaderType st);
	/// return the shader type of this code
	ShaderType get_shader_type() const;
	///compile attached source; returns true if successful
	bool compile(const context& ctx);
	/** read shader code with read_code and compile. If show_error is true
	    print error messages formated with the get_last_error method in case
		 an error arose. */
	bool read_and_compile(const context& ctx, const std::string& file_name, ShaderType st = ST_DETECT, const shader_compile_options& options = {}, bool show_error = true);
	/// return whether shader has been compiled successfully
	bool is_compiled() const;

protected:
	/// map that caches full shader file paths indexed by the shader file name
	static std::map<std::string, std::string> shader_file_name_map;
	/// whether the shader file name map is initialized
	static bool shader_file_name_map_initialized;
	/// map that caches shader file contents indexed by their file name
	static std::map<std::string, std::string> code_cache;

	/// store the shader type
	ShaderType st;

private:
	/// search for include directives in the given source code, replace them by the included file contents and return the full source code as well as the set of all included files
	static std::string resolve_includes(const std::string& source, bool use_cache, std::set<std::string>& included_file_names, std::string* _last_error = 0);
	/// search for include directives in the given source code, replace them by the included file contents and return the full source code
	static std::string resolve_includes(const std::string& source, bool use_cache, std::string* _last_error = 0);
	/// search for version directives (including those in special comments, starting with //? or //!) and replace them with a single statement of the maximum required version; extension directives are moved after the top-most version directive
	static void resolve_version_and_extensions(std::string& source);
	/// set shader code defines and snippets
	static void set_defines_and_snippets(std::string& source, const shader_compile_options& options);
	/// set shader code vertex attribute locations (a hotfix for AMD driver behaviour on vertex shaders)
	static void set_vertex_attrib_locations(std::string& source);
};


	}
}

#include <cgv/config/lib_end.h>
