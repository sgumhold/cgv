#pragma once

#include <string>
#include <vector>
#include <fstream>
#include "variables.h"
#include "command_token.h"
#include <cgv/utils/advanced_scan.h>

#include "lib_begin.h"

namespace cgv {
	namespace ppp {
		/** the pre header processor parses a pre header file and converts it to a
			header file or uses the information from parsing to transform a template
			 file to a header file.

			 Support for type reflection:

			 @< ... generates type reflection info from the code on this line before the @< token
			 @> ... generates type reflection info starting with the code on the next line
					up to and not including the next ; or { symbol

			 the to be reflected code is parsed and the following code fragments detected:

			 #include                          ... adds an include to the list of include files
			 namespace name                    ... enters the given namespace
			 class / struct / union name       ... begins a compound definition
			 protected / private / public :    ... switches the member access type
			 type name                         ... declares a member
			 typedef type name                 ... declares a type inside or outside a class definition
			 type name(parameter list) [const] ... declares a method
			 }                                 ... leaves a namespace or ends a compound definition

			 the extracted information is stored in the variable reflect_info in the
			 following structure of nested lists:

			 reflect_info = {includes:[string],
							 typedefs:[typedef],
								  enums:[enum],
								  compounds:[compound],
								  variables:[variable],
								  functions:[function] }
			 typedef      = {namespaces:[string],
							 namespace_prefix:string,
								  name:string,
							 type:string,
								  comment:string}
			 enum         = {namespaces:[string],
							 namespace_prefix:string,
								  name:string,
							 entries:[enum_entries],
								  comment:string}
			 enum_entry   = {name:string,
								  value:int,
								  comment:string}
			 compound     = {namespaces:[string],
							 namespace_prefix:string,
								  type_name:string,
								  typedefs:[typedef],
								  enums:[enum],
								  compounds:[compound],
								  members:[member],
								  methods:[method]
							 bases:[base],
								  kind:string [class|struct|union],
								  comment:string}
			 base         = {type_name:string,
							 is_virtual:bool,
							 access_type:string [private,protected,public] }
			 member       = {name:string,
							 type:string,
								  access_type:string [private,protected,public],
								  is_static:bool,
								  comment:string}
			method       = {name:string,
							 type:string,  -> return type)
								  parameters:[parameter],
								  access_type:string [private,protected,public]
								  is_virtual:bool,
								  is_const:bool,
								  comment:string}
			variable     = {namespaces:[string],
							 namespace_prefix:string,
								  name:string,
							 type:string,
								  comment:string}
			function     = [namespaces:[string],
							 namespace_prefix:string,
								  name:string,
							 type:string,
								  parameters:[parameter],
								  comment:string]
			 parameter    = {name:string,
							 type:string,
								  has_default_value:bool,
								  default_value:string}
		*/

		class CGV_API ph_processor
		{
		protected:
			friend class expression_processor;
			bool found_error;
			unsigned nr_functions;
			char special;
			bool generate_output;
			bool content_is_external;
			std::ostream* os;
			std::ostream* es;
			const std::string* content;
			std::string file_name;
			std::vector<cgv::utils::line> lines;
			std::vector<command_token> commands;
			int recursion_depth;

			std::vector<std::string>* inserted_shader_file_names;

			unsigned int get_line_number(const token& loc) const;
			void error(const std::string& text, const token& loc, unsigned int error_number = 0);
			bool check_evaluation_error(unsigned int i, unsigned int ei);

			bool process(unsigned int i, unsigned int j);
			bool parse();
			bool parse_without_special(unsigned nr_skip = 0);
		public:
			static std::string my_getenv(const char* name);
			std::string find_include_file(const std::string& fn, bool local) const;
		protected:
			bool process_include(unsigned int i, bool is_cinclude = false, bool insert = false);
			bool process_list(unsigned int i);

			bool debug_reflection;
			variant* reflect_info;
			enum ReflectionParenthesis { RP_COMPOUND, RP_NAMESPACE };
			std::vector<ReflectionParenthesis> rp_stack;
			std::vector<std::string> namespace_stack;
			std::vector<variant*>    compound_stack;
			std::vector<std::string> compound_access_type_stack;
			variant construct_namespace_list() const;
			std::string construct_namespace_sequence() const;
			void ensure_reflect_info();
			bool reflect(const token& code, const token& comment);

			bool recursive_dir(std::string rel_directory,
				std::string actual_directory, std::string filter,
				bool rel_path, variant& desc_list, bool recursive);
			std::string additional_include_path;
			bool search_recursive;
		public:
			ph_processor(const std::string& _additional_include_path = "", bool _search_recursive = false, char _special = '@');
			~ph_processor();
			int exit_code;
			void swap_output(ph_processor& pp);
			char get_special() const { return special; }
			void configure_insert_to_shader(std::vector<std::string>* _inserted_shader_file_names);


			void set_error_stream(std::ostream& error_stream);

			bool parse_string(const std::string& text);
			bool parse_file(const std::string& file_name);

			bool process_to_string(std::string& output);
			int process_to_file(const std::string& _file_name, long long last_write_time = 0);
			bool process_without_output();
			/*
				enum ProcessingOptions {

				int process_input_to_string(const std::string& inp_file, std::string& output, int processing_options);
				int process_input_to_file(const std::string& inp_file, const std::string& out_file, int processing_options, );
				*/
			void close();
		};

	}
}
#include <cgv/config/lib_end.h>