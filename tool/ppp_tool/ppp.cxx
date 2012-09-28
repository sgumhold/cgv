#include <stdlib.h>
#include <iostream>
#include <libs/ppp/ph_processor.h>
#include <libs/ppp/command_token.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/scan.h>
#include <cgv/utils/file.h>

using namespace cgv::media::text::ppp;
using namespace cgv::utils::file;
using namespace cgv::utils;

#ifdef WIN32
#pragma warning(disable:4996)
#endif

#ifdef _DEBUG
void test_command_token(const char* t)
{
	const std::string s(t);
	token tok(&s[0], &s[0]+s.size());
	
	command_token ct;
	std::cout << "\ncommand token('" << t << "')\n==> ";
	if (!ct.split_off_from(tok)) {
		std::cout << "error:"
			       << "\n   " << ct.get_last_error().c_str()
					 << "\n...rest token = '" << to_string(tok).c_str() << "'" << std::endl;
		return;
	}
	std::cout << "\n   token=" << to_string(ct).c_str()
				 << "\n   type=" << ct.get_keyword() 
				 << "\n   block follows=" << (ct.block_follows() ? "true" : "false")
		       << "\n   nr_expressions=" << (unsigned int) ct.expressions.size() 
				 << "\n   parenthesis_index=" << ct.parenthesis_index
				 << "\n...rest token = '" << to_string(tok).c_str() << "'" << std::endl;
}

void test_processor(const char* t)
{
	std::string s(t);
	std::cout << "\n\n\nfile_procesor applied to\n" << t << "\n\n";
	ph_processor fp;
	if (!fp.parse_string(s)) {
		std::cout << "\n\nresults in parse error" << std::endl;
		return;
	}
	std::string result;
	if (!fp.process_to_string(result)) {
		std::cout << "\n\nresults in process error" << std::endl;
		return;
	}
	std::cout << "\n\nresults in success with output:\n\n" << result.c_str() << std::endl;
}
#endif

void cgv_type_reflect_tests();

/*
static std::string my_getenv(const char* name)
{
	const char* var = getenv(name);
	if (!var) {
		if (ref_variable(name).is_str())
			return ref_variable(name).get_str();
		return std::string();
	}
	return std::string(var);
}
*/

char parse_definitions(int defc, char** defv)
{
	for (int i=0; i<defc; ++i) {
		std::string def(defv[i]+1);
		if (def == "script")
			continue;
		std::vector<token> toks;
		tokenizer(def).set_ws("=").bite_all(toks);
		if (toks.size() != 2) {
			std::cerr << "error in definition: <" << def.c_str() << ">\n"
				<< "  is not of the syntax <name=value>" << std::endl;
		}
		else {
			std::string name = to_string(toks[0]);
			std::string value = to_string(toks[1]);
			if (value[0] == '"' || value[0] == '\'')
				ref_variable(name) = variant(value.substr(1,value.size()-2));
			else {
				int ivalue;
				if (is_integer(value,ivalue))
					ref_variable(name) = variant(ivalue);
				else {
					if (value == "true")
						ref_variable(name) = variant(true);
					else {
						if (value == "false")
							ref_variable(name) = variant(false);
						else
							ref_variable(name) = variant(value);
					}
				}
			}
		}
	}
	char special = '@';
	variant* v = find_variable("SPECIAL");
	if (v) {
		if (v->is_int())
			special = (char) (v->get_int());
		else if (v->is_str())
			special = v->get_str()[0];
	}
	return special;
}

void init_input_file(const std::string& fn, const char* truncation_path_list = 0)
{
	if (truncation_path_list) {
		std::string include_paths(ph_processor::my_getenv(truncation_path_list));
		std::vector<token> toks;
		bite_all(tokenizer(include_paths).set_ws(";"), toks);
		bool found_prefix = false;
		for (unsigned int j=0; j<toks.size(); ++j) {
			std::string fp(fn);
			if (shorten_path(fp, to_string(toks[j]))) {
				ref_variable("input_file").set_str(std::string("<")+fn+">");
				found_prefix = true;
				break;
			}
		}
		if (!found_prefix)
			ref_variable("input_file").set_str(std::string("\"")+get_file_name(fn)+"\"");
	}
	else
		ref_variable("input_file").set_str(fn);
	ref_variable("input_path").set_str(fn);
	ref_variable("input_dir").set_str(cgv::utils::file::get_path(fn));
}

int main(int argc, char** argv)
{
	// determine the number of arguments used to define variables
	int defc = 1;
	bool script_mode = false;
	while (argc > defc) {
		if (argv[defc][0] == '-' 
#ifdef WIN32 //because in linux directories starts with / 
			|| argv[defc][0] == '/'
#endif		
			) {
			if (std::string(argv[defc]).substr(1) == "script")
				script_mode = true;
			++defc;
		}
		else
			break;

	}
	// compute the number of remaining arguments
	int argc_remain = argc-defc;
	if (!(argc_remain == 2 || ((argc_remain > 2) && ((argc_remain%2)==1)) || (script_mode && argc_remain > 0) ) ) {
		std::cerr << "usages:\n"
			      << "   " << argv[0] << " -script [-name=value]* script-file [input-file]*\n" 
			      << "   " << argv[0] << " [-name=value]* input-file output-file\n" 
			      << "   " << argv[0] << " [-name=value]* input-file [template-file output-file]+\n" 
				  << " :" << std::endl;
		return -1;
	}
	int i;
	// determine the time stamp of the newest input file
	long long last_write_time = cgv::utils::file::get_last_write_time(argv[defc]);
	if (script_mode) {
		init_environment(argc, argv);
		char special = parse_definitions(defc-1, argv+1);
		init_input_file(argv[defc]);
		ref_variable("input_files").set_list();
		ref_variable("input_paths").set_list();
		ref_variable("input_dirs").set_list();
		for (i = 1; i < argc_remain; ++i) {
			ref_variable("input_files").ref_list().push_back(cgv::utils::file::get_file_name(argv[defc+i]));
			ref_variable("input_paths").ref_list().push_back(std::string(argv[defc+i]));
			ref_variable("input_dirs").ref_list().push_back(cgv::utils::file::get_path(argv[defc+i]));
		}
		ph_processor fp("",false,special);
		if (!fp.parse_file(argv[defc]))
			return -1;
		if (!fp.process_without_output())
			return -2;
		std::cout << "successfully processed script " << argv[defc] << std::endl;
		return 0;
	}
	if (argc_remain == 2) {
		init_environment(argc, argv);
		char special = parse_definitions(defc-1, argv+1);
		init_input_file(argv[defc]);
		ph_processor fp("",false,special);
		if (!fp.parse_file(argv[defc]))
			return -1;
		int result = fp.process_to_file(argv[defc+1], cgv::utils::file::get_last_write_time(argv[defc]));
		if (result == 0)
			return -2;
		if (fp.exit_code != 0)
			return fp.exit_code;
		switch (result) {
		case 1 :
			std::cout << "no change in " << argv[defc+1] << std::endl;
			break;
		case 2:
			std::cout << "successfully processed " << argv[defc] << " to " << argv[defc+1] << std::endl;
			break;
		case 3 : 
			std::cout << "updated time stamp of " << argv[defc+1] << std::endl;
			break;
		}
		return 0;
	}
	for (i = 1; i <= argc_remain/2; ++i) {
		long long new_last_write_time = cgv::utils::file::get_last_write_time(argv[2*i+defc-1]);
		if (new_last_write_time > last_write_time)
			last_write_time = new_last_write_time;
	}
	for (i = 1; i <= argc_remain/2; ++i) {
		clear_variables();
		init_environment(argc, argv);
		char special = parse_definitions(defc-1, argv+1);
		init_input_file(argv[defc], "INCLUDE");
		ph_processor fp("",false,special);
		if (!fp.parse_file(argv[defc]))
			return -1;
		if (!fp.process_without_output())
			return -1;
		if (fp.exit_code != 0)
			return fp.exit_code;

		ph_processor fp2("",false,fp.get_special());
		if (!fp2.parse_file(argv[2*i+defc-1]))
			return -2*i;

//		std::cout << ref_variable("reflect_info") << std::endl;

		int result = fp2.process_to_file(argv[2*i+defc], last_write_time);
		if (result == 0)
			return -2*i;
		if (fp2.exit_code != 0)
			return fp2.exit_code;
		switch (result) {
		case 1 :
			std::cout << "no change in " << argv[2*i+defc] << std::endl;
			break;
		case 2 :
			std::cout << "successfully transformed " << argv[defc] 
			          << " via template " << argv[2*i+defc-1] 
						 << " to " << argv[2*i+defc] << std::endl;
			break;
		case 3 : 
			std::cout << "updated time stamp of " << argv[2*i+defc] << std::endl;
			break;
		}
	}
	return 0;
}
