#ifdef WIN32
#pragma warning (disable:4996)
#endif

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <cgv/utils/file.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/math/random.h>
#include "ph_processor.h"
#include "expression_processor.h"

using namespace cgv::utils;

#define reflect_version_1 true
#define reflect_version_2 false

namespace cgv {
	namespace media {
		namespace text {
			namespace ppp {

ph_processor::ph_processor(const std::string& _additional_include_path, bool _search_recursive, char _special) : 
			additional_include_path(_additional_include_path), search_recursive(_search_recursive), special(_special)
{
	exit_code = 0;
	nr_functions = 0;
	found_error = false;
	generate_output = false;
	es = &std::cerr;
	os = 0;
	content = 0;
	content_is_external = false;
	inserted_shader_file_names = 0;
#ifdef _DEBUG
	debug_reflection = false;
#else
	debug_reflection = false;
#endif
	reflect_info = 0;
}

ph_processor::~ph_processor()
{
	close();
}

void ph_processor::configure_insert_to_shader(std::vector<std::string>* _inserted_shader_file_names)
{
	inserted_shader_file_names = _inserted_shader_file_names;
}

void ph_processor::close()
{
	if (!content_is_external && content) {
		delete content;
		content = 0;
		content_is_external = false;
	}
}

bool ph_processor::parse_string(const std::string& text)
{
	close();
	content = &text;
	content_is_external = true;
	return parse();
}

bool ph_processor::parse_file(const std::string& _file_name)
{
	// read the file
	close();
	std::string* file_content = new std::string();
	content_is_external = false;
	if (!cgv::utils::file::read(_file_name, *file_content, true)) {
		if (es) {
			(*es) << "could not read input file " << _file_name.c_str() << std::endl;
		}
		return false;
	}

#ifndef WIN32
	//correct linefeeds
	std::string* new_file_content = new std::string();
	bool carriage_return_found = false;
	for (unsigned int i=0; i<file_content->size(); ++i) {
		
		if(carriage_return_found&& (*file_content)[i] == 10){

			//linefeed found 
			//printf("§\n");
			continue;

		}//printf("%c",(*file_content)[i]);

		carriage_return_found = (*file_content)[i] == 13;

		if(carriage_return_found) (*new_file_content) += 10;

		else (*new_file_content) += (*file_content)[i];

	}
	delete file_content;
	file_content = new_file_content;
	
#endif

	content = file_content;
	file_name = _file_name;
	return parse();
}

bool ph_processor::process_without_output()
{
	generate_output = false;
	bool result = process(0, (unsigned int) commands.size());
/*
    if (debug_reflection) {
		ensure_reflect_info();
		std::cout << "reflect_info = " << ref_variable("reflect_info") << std::endl;
	}
*/
	return result;
}

void ph_processor::swap_output(ph_processor& pp)
{
	std::swap(generate_output, pp.generate_output);
	std::swap(os, pp.os);
}

bool ph_processor::process_to_string(std::string& output)
{
	std::stringstream str_os;
	os = &str_os;
	generate_output = true;

	bool result = process(0, (unsigned int) commands.size());
/*
	if (debug_reflection) {
		ensure_reflect_info();
		std::cout << "reflect_info = " << ref_variable("reflect_info") << std::endl;
	}
*/
	output = str_os.str();
	return result;
}

int ph_processor::process_to_file(const std::string& _file_name, long long last_write_time)
{
	/*
	if (last_write_time == 0) {
		std::ofstream fos(_file_name.c_str());
		if (fos.fail()) {
			if (es) 
				(*es) << "could not open output file " << _file_name.c_str() << " for writing" << std::endl;
			return false;
		}
		os = &fos;
		generate_output = true;
		bool result = process(0, (unsigned int) commands.size());
		fos.close();
		return result ? 1 : 0;
	}
	else {
	*/
		std::string output;
		if (!process_to_string(output))
			return 0;
		bool changed = true;
		bool do_write = true;
		if (cgv::utils::file::exists(_file_name)) {
			std::string content;
			if (cgv::utils::file::read(_file_name,content,true)) {
				if (output == content) {
					changed = false;
					do_write = cgv::utils::file::get_last_write_time(_file_name) < last_write_time;
				}
			}
		}
		if (do_write) {
			if (!cgv::utils::file::write(_file_name,output.c_str(),output.size(),true)) {
				std::cerr << "could not write to file " << _file_name << std::endl;
				return 0;
			}
			else 
				if (changed)
					return 2;
				else
					return 3;
		}
		return 1;
	//}
}

void ph_processor::set_error_stream(std::ostream& error_stream)
{
	es = &error_stream;
}

unsigned int ph_processor::get_line_number(const token& loc) const
{
	// determine line number
	unsigned int li;
	for (li = 0; li < lines.size(); ++li)
		if (lines[li].begin > loc.begin)
			break;
	return li;
}
void ph_processor::error(const std::string& text, const token& loc, 
									unsigned int error_number)
{
	found_error = true;
	// determine line number
	unsigned int li = get_line_number(loc);
	unsigned prev_line = li;
	if (li > 0)
		--prev_line;
	const char* lb = loc.begin;
	if (!lb)
		lb = lines[li].begin;
	if (es) {
		(*es) << file_name.c_str() << "(" << li << ") : error " 
			<< error_number << ": " << text.c_str() << "\n   "
			<< to_string(token(lines[prev_line].begin, lb)).c_str() << " \\_ ";
		if (loc.begin) {
			token l(loc.begin,loc.begin);
			while (l.end < loc.end && !is_space(*l.end))
				++l.end;
			(*es) << to_string(l).c_str() << " _/" << std::endl;
		}
	}
}

std::string ph_processor::my_getenv(const char* name)
{
	if (ref_variable(name).is_str())
		return ref_variable(name).get_str();

	const char* var = getenv(name);
	if (!var)
		return std::string();
	return std::string(var);
}

void my_append(std::string& p, const std::string& o)
{
	if (o.empty())
		return;
	if (p.empty())
		p = o;
	else {
		p += ";";
		p += o;
	}
}

std::string ph_processor::find_include_file(const std::string& fn, bool local) const
{
	token path_token(0,0);

	std::string include  = my_getenv("INCLUDE");
	std::string cgv_dir  = my_getenv("CGV_DIR");
	std::string cgv_root = my_getenv("CGV_ROOT");
	std::string include_paths = additional_include_path;
	if (local)
		include_paths = ref_variable("input_dir").get_str();
	my_append(include_paths, cgv_dir);
	my_append(include_paths, cgv_root);
	my_append(include_paths, include);
	std::vector<token> path_list;
	split_to_tokens(include_paths,path_list,"",false,"\"'","\"'",";");
	if (cgv::utils::file::exists(fn)) 
		return fn;
	for (unsigned int j=0; j<path_list.size(); ++j) {
		std::string file_path = to_string(path_list[j])+"/"+fn;
		if (!cgv::utils::file::exists(file_path) && search_recursive) {
			std::string path = to_string(path_list[j]);
			file_path = cgv::utils::file::find_in_paths(fn,path);
		}
		if (!file_path.empty() && cgv::utils::file::exists(file_path)) {
			return file_path;
		}
	}
	return "";
}

bool ph_processor::process_include(unsigned int i, bool is_cinclude, bool insert)
{
	std::string fn = to_string(commands[i]);
	if (commands[i].expressions.size() > 0) {
		variant v0;
		commands[i].expressions[0].evaluate(v0, this);
		if (exit_code != 0)
			return true;
		if (!check_evaluation_error(i,0))
			return false;
		fn = v0.get_str();
	}
	std::string file_path = find_include_file(fn, commands[i].expressions.size() == 0 && commands[i].begin[-1] == '"');
	if (file_path.empty()) {
		error(std::string("could not open file ")+fn, commands[i]);
		return false;
	}

	ph_processor* fp = new ph_processor(additional_include_path, false);
	bool success = false;
	if (success = fp->parse_file(file_path)) {
		if (insert) {
			std::string output;
			success = fp->process_to_string(output);
			if (inserted_shader_file_names) {
				(*os) << "#line 0 " << 1000+inserted_shader_file_names->size() << std::endl;
				(*os) << output.c_str() << std::endl;
				(*os) << "#line " << (int) get_line_number(commands[i])-1 << " 0" << std::endl;
				inserted_shader_file_names->push_back(file_path);
			}
			else
				(*os) << output.c_str() << std::endl;
		}
		else {
			success = fp->process_without_output();
		}
	}
	if (is_cinclude && generate_output) {
		token path_token(commands[i]);
		std::size_t pos = to_string(path_token).find_last_of('.');
		if (pos != std::string::npos)
			path_token.end = path_token.begin+pos;
		(*os) << "#include " << commands[i].get_open_parenthesis()
			   << to_string(path_token).c_str() << ".h"
			   << commands[i].get_close_parenthesis();
	}
	if (fp->exit_code != 0)
		exit_code = fp->exit_code;
	if (fp->nr_functions == 0)
		delete fp;
	return success;
}

bool ph_processor::check_evaluation_error(unsigned int i, unsigned int ei)
{
	if (commands[i].expressions[ei].found_error) {
		if (!commands[i].expressions[ei].issued_error) {
			error(std::string("evaluate expression: ")+
				   commands[i].expressions[ei].get_last_error(), commands[i].expressions[ei].get_last_error_token());
		}
		return false;
	}
	return true;
}


void bite_common_prefix(token& t0, token& t1, token& p0, token& p1)
{
	p0 = token(t0.begin,t0.begin);
	p1 = token(t1.begin,t1.begin);
	while (!t0.empty() && !t1.empty()) {
		if (*t0.begin != *t1.begin)
			break;
		p0.end = ++t0.begin;
		p1.end = ++t1.begin;
	}
}

bool bite_digits(token& t, int& n)
{
	bool result = false;
	n = 0;
	while (!t.empty() && is_digit(*t.begin)) {
		n = 10*n+ (*t.begin - '0');
		++t.begin;
		result = true;
	}
	return result;
}

bool ph_processor::process_list(unsigned int i)
{
	variant v0;
	commands[i].expressions[0].evaluate(v0, this);
	if (exit_code != 0)
		return true;
	if (!check_evaluation_error(i,0))
		return false;
	variant v1;
	commands[i].expressions[1].evaluate(v1, this);
	if (exit_code != 0)
		return true;
	if (!check_evaluation_error(i,1))
		return false;
	variant v2;
	commands[i].expressions[2].evaluate(v2, this);
	if (exit_code != 0)
		return true;
	if (!check_evaluation_error(i,2))
		return false;
	if (!generate_output)
		return true;

	std::string s0 = v0.get_str();
	token t0(s0);
	std::string s1 = v1.get_str();
	std::string s2 = v2.get_str();
	token t2(s2);
	std::vector<token> toks;
	const char* ptr_base = 0;
	do {
		token p0, p2;
//		std::cout << "tokens = " << to_string(t0).c_str() << " <-> " 
//			<< to_string(t2).c_str() << std::endl;

		bite_common_prefix(t0,t2,p0,p2);
		if (t0.empty() || !is_digit(*t0.begin) || 
			 t2.empty() || !is_digit(*t2.begin) ) {
				 if (!p0.empty() && is_digit(p0.end[-1])) {
					 t0.begin = --p0.end;
					 t2.begin = --p2.end;
				 }
				 else if (!(t0.empty() && t2.empty())) {
					 error(std::string("error in list [")+s0+";"+s1+";"+s2+"]\n   can only iterate between two numbers", commands[i]);
					 return false;
				 }
		}
		toks.push_back(p0);
		if (t0.empty() || t2.empty()) {
			if (t0.empty() && t2.empty())
				break;
			else {
				error(std::string("error in list [")+s0+";"+s1+";"+s2+"]\n   1st and 3rd expression split into different number of terms", commands[i]);
				return false;
			}
		}
//		std::cout << "bite off prefix " << p0.str().c_str() << " = " 
//			<< p2.str().c_str() << std::endl;
		int n0, n2;
		if (!bite_digits(t0, n0) || !bite_digits(t2, n2)) {
			error(std::string("error in list [")+s0+";"+s1+";"+s2+"]\n   can only iterate between two numbers (2)", commands[i]);
			return false;
		}
		toks.push_back(token(ptr_base+n0,ptr_base+n2));
//		std::cout << "bite off numbers " << n0 << " and " << n2 << std::endl;
	} while (!(t0.empty() && t2.empty()));

	if (toks.empty())
		return true;

	int n = 1;
	if (toks.size() > 1) {
		n = (int)((toks[1].end-ptr_base) - (toks[1].begin-ptr_base));
		n += 1;
	}
	unsigned int j;
	for (j = 3; j < toks.size(); j += 2) {
		int nn = (int)((toks[j].end-ptr_base) - (toks[j].begin-ptr_base));
		nn += 1;
		if (nn != n) {
			error(std::string("error in list [")+s0+";"+s1+";"+s2+"]\n   several number ranges of different size not allowed", commands[i]);
			return false;
		}
	}
	for (int k = 0; k < n; ++k) {
		if (k > 0)
			(*os) << s1.c_str();
		for (j = 0; j < toks.size(); j += 2) {		
			(*os) << to_string(toks[j]).c_str();
			if (j+1 < toks.size())
				(*os) << (int)(toks[j+1].begin-ptr_base)+k;
		}
	}
	return true;
}

void ph_processor::ensure_reflect_info()
{
	if (!reflect_info) {
		reflect_info = &ref_variable("reflect_info");
		*reflect_info = variant(variant::map_type());
		if (reflect_version_2)
			reflect_info->ref_element("elements") = variant(variant::list_type());
		if (reflect_version_1) {
			reflect_info->ref_element("typedefs") = variant(variant::list_type());
			reflect_info->ref_element("enums") = variant(variant::list_type());
			reflect_info->ref_element("compounds") = variant(variant::list_type());
			reflect_info->ref_element("variables") = variant(variant::list_type());
			reflect_info->ref_element("functions") = variant(variant::list_type());
		}
		reflect_info->ref_element("includes") = variant(variant::list_type());
	}
}

variant ph_processor::construct_namespace_list() const
{
	variant list = variant(variant::list_type());
	if (!compound_stack.empty())
		return list;
	for (unsigned int i=0; i<namespace_stack.size(); ++i)
		list.append_to_list(namespace_stack[i]);
	return list;
}

std::string ph_processor::construct_namespace_sequence() const
{
	std::string seq;
	if (!compound_stack.empty())
		return seq;
	for (unsigned int i=0; i<namespace_stack.size(); ++i)
		seq += namespace_stack[i]+"::";
	return seq;
}


bool ph_processor::reflect(const token& code, const token& comment)
{
	// split code fragment into tokens
	tokenizer t(code);
	t.set_sep("()[];");
	std::vector<token> toks;
	t.skip_whitespaces();
	t.reverse_skip_whitespaces();
	bite_all(t,toks);

	// check that at least one token is present
	if (toks.empty()) {
		error("no code to be reflected here", code);
		return false;
	}

	// check for closing code parenthesis
	if (to_string(toks[0]) == "}") {

		// these are only valid if the parenthesis stack is not empty
		if (rp_stack.empty()) {
			error("reflect end of something without starting something", code);
			return false;
		}

		// either close the top most namespace
		if (rp_stack.back() == RP_NAMESPACE) {
			if (namespace_stack.empty()) {
				error("reflect end of namespace without a namespace started", code);
				return false;
			}
			if (debug_reflection)
				std::cout << "reflect end of namespace " << namespace_stack.back().c_str() << std::endl;
			namespace_stack.pop_back();
		}
		// or the top most compound definition
		else {
			if (compound_stack.empty()) {
				error("reflect end of compound without a compound started", code);
				return false;
			}
			if (debug_reflection)
				std::cout << "reflect end of compound " << compound_stack.back()->get_element(2).get_str().c_str() << std::endl;
			compound_stack.pop_back();
			compound_access_type_stack.pop_back();
		}
		rp_stack.pop_back();
		return true;
	}
	// check for access type declaration
	std::string new_access_type;
	if (to_string(toks[0]) == "private" || to_string(toks[0]) == "private:")
		new_access_type = "private";
	else if (to_string(toks[0]) == "protected" || to_string(toks[0]) == "protected:")
		new_access_type = "protected";
	else if (to_string(toks[0]) == "public" || to_string(toks[0]) == "public:")
		new_access_type = "public";
	if (!new_access_type.empty()) {
		if (rp_stack.empty() || rp_stack.back() != RP_COMPOUND) {
			error("access type definition only valid inside compound declaration", toks[0]);
			return false;
		}
		if (compound_access_type_stack.empty()) {
			error("compound_access_type_stack empty", toks[0]);
			return false;
		}
		compound_access_type_stack.back() = new_access_type;
		return true;
	}

	// in all other cases there must be at least two tokens
	if (toks.size() < 2) {
		error("code splits only into one token and cannot be reflected", code);
		return false;
	}

	if (to_string(toks[0]) == "typedef") {
		// parse typedef declaration
		if (toks.size() < 3) {
			error("typedef with only two tokens cannot be reflected", code);
			return false;
		}
		token name_tok = toks.back();
		token type_tok(toks[1].begin, toks[toks.size()-2].end);

		// construct reflection info
		variant type_def = variant(variant::map_type());
		if (reflect_version_2)
			type_def.ref_element("reflect").set_str("typedef");
		type_def.ref_element("namespaces") = construct_namespace_list();
		type_def.ref_element("namespace_prefix") = construct_namespace_sequence();
		type_def.ref_element("name") = variant(to_string(name_tok));
		type_def.ref_element("type") = variant(to_string(type_tok));
		type_def.ref_element("comment") = variant(to_string(comment));

		// store reflection info
		ensure_reflect_info();
		if (reflect_version_2) {
			if (compound_stack.size() > 0)
				compound_stack.back()->ref_element("elements").append_to_list(type_def);
			else
				reflect_info->ref_element("elements").append_to_list(type_def);
		}
		if (reflect_version_1) {
			if (compound_stack.size() > 0)
				compound_stack.back()->ref_element("typedefs").append_to_list(type_def);
			else
				reflect_info->ref_element("typedefs").append_to_list(type_def);
		}
		// generate debug info
		if (debug_reflection)
			std::cout << "reflect typedef " << type_def << std::endl;
		return true;
	}
	else if (to_string(toks[0]) == "#include") {
		// parse include declaration
		if (toks.size() < 2) {
			error("cannot reflect include without file name", code);
			return false;
		}
		// store reflection info
		ensure_reflect_info();
		reflect_info->ref_element("includes").append_to_list(variant(to_string(toks[1])));
		// generate debug info
		if (debug_reflection)
			std::cout << "reflect include " << to_string(toks[1]).c_str() << std::endl;
		return true;
	}
	else if (to_string(toks[0]) == "enum") {
		// parse enum declaration and generate list of enum entries
		variant enum_entries = variant(variant::list_type());
		// skip CGV_API macro
		bool contains_cgv_api = false;
		int idx = 1;
		if (to_string(toks[1]) == "/*CGV_API*/") {
			contains_cgv_api = true;
			idx = 2;
		}
		// extract token of class name
		token name_tok = toks[idx];
		tokenizer t(token(name_tok.end, commands.back().end));
		t.set_sep(",={}");

		int v = 0;
		bool define_name = false;
		while (!t.skip_ws_check_empty()) {
			token tok = t.bite();
			if (to_string(tok) == "{") {
				define_name = true;
				continue;
			}
			if (to_string(tok) == "}")
				break;
			if (to_string(tok) == ",") {
				define_name = true;
				continue;
			}
			if (to_string(tok) == "=") {
				define_name = false;
				continue;
			}
			if (define_name) {
				variant enum_entry = variant(variant::map_type());
				enum_entry.ref_element("name").set_str(to_string(tok));
				enum_entry.ref_element("value").set_int(v);
				enum_entries.append_to_list(enum_entry);
				++v;
			}
			else {
				int w;
				if (!is_integer(to_string(tok), w)) {
					error("enum value expected", tok);
					return false;
				}
				enum_entries.ref_element(enum_entries.get_size()-1).ref_element(1).set_int(w);
				v = w+1;
			}
		}

		// prepate enum_info
		variant enum_info = variant(variant::map_type());
		if (reflect_version_2)
			enum_info.ref_element("reflect").set_str("enum");
		enum_info.ref_element("namespaces") = construct_namespace_list();
		enum_info.ref_element("namespace_prefix") = construct_namespace_sequence();
		enum_info.ref_element("name").set_str(to_string(name_tok));
		enum_info.ref_element("entries") = enum_entries;
		enum_info.ref_element("comment").set_str(to_string(comment));
		enum_info.ref_element("contains_CGV_API").set_bool(contains_cgv_api);

		// store reflection info
		ensure_reflect_info();
		if (reflect_version_1) {
			if (compound_stack.size() > 0)
				compound_stack.back()->ref_element("enums").append_to_list(enum_info);
			else
				reflect_info->ref_element("enums").append_to_list(enum_info);
		}
		if (reflect_version_2) {
			if (compound_stack.size() > 0)
				compound_stack.back()->ref_element("elements").append_to_list(enum_info);
			else
				reflect_info->ref_element("elements").append_to_list(enum_info);
		}
		// generate debug info
		if (debug_reflection)
			std::cout << "reflect enum " << enum_info << std::endl;
		return true;
	}
	else if (to_string(toks[0]) == "struct" || to_string(toks[0]) == "class"  || to_string(toks[0]) == "union") {
		// parse code and generate base list
		variant base_list = variant(variant::list_type());
		// skip CGV_API macro
		bool contains_cgv_api = false;
		int idx = 1;
		if (to_string(toks[1]) == "CGV_API") {
			contains_cgv_api = true;
			idx = 2;
		}
		// extract token of class name
		token name_tok = tokenizer(toks[idx]).set_sep(":").bite();
		// tokenize remaining text again to find first colon
		tokenizer tmp(token(name_tok.end, toks.back().end));
		tmp.set_sep(":").set_ws("").bite();
		token base_tok;
		if (!tmp.empty()) {
			if (to_string(tmp.bite()) == ":" ||
				 (!tmp.empty() && to_string(tmp.bite()) == ":")) {
					 if (!tmp.empty())
						 base_tok = token(tmp.bite().begin,toks.back().end);
			}
		}
		if (!base_tok.empty()) {
			tmp = tokenizer(base_tok);
			std::vector<token> base_toks;
			bite_all(tmp.set_sep(",").set_ws("").set_skip("<",">"),base_toks);
			if (base_toks.size() % 2 == 0 || 
				 (base_toks.size()>0 && to_string(base_toks[0]) == ",") ) {
				error("found invalid base list", base_tok);
				return false;
			}
			std::string access_type = "public";
			if (to_string(toks[0]) == "class")
				access_type = "private";
			for (unsigned int i=0; i<base_toks.size(); i+=2) {
				bool is_virtual = false;
				std::vector<token> toks;
				tokenizer ttt(base_toks[i]);
				bite_all(ttt,toks);
				if (toks.size() == 0) {
					error("found invalid base definition", base_toks[i]);
					return false;
				}
				unsigned int j = 0;
				for (unsigned int k=0; k<2; ++k, ++j) {
					if (toks.size() > j) {
						if (to_string(toks[j]) == "virtual")
							is_virtual = true;
						else if (to_string(toks[j]) == "private")
							access_type = "private";
						else if (to_string(toks[j]) == "protected")
							access_type = "protected";
						else if (to_string(toks[j]) == "public")
							access_type = "public";
						else
							break;
					}
				}
				token base_name_tok(toks[j].begin, toks.back().end);
				variant base = variant(variant::map_type());
				base.ref_element("type_name").set_str(to_string(base_name_tok));
				base.ref_element("is_virtual").set_bool(is_virtual);
				base.ref_element("access_type").set_str(access_type);
				base_list.append_to_list(base);
			}
		}

		// prepate compound reflection info
		variant compound = variant(variant::map_type());
		if (reflect_version_2)
			compound.ref_element("reflect").set_str("compound");
		compound.ref_element("namespaces") = construct_namespace_list();
		compound.ref_element("namespace_prefix") = construct_namespace_sequence();
		if (reflect_version_1) {
			compound.ref_element("type_name").set_str(to_string(name_tok));
			compound.ref_element("typedefs").set_list();
			compound.ref_element("enums").set_list();
			compound.ref_element("compounds").set_list();
			compound.ref_element("members").set_list();
			compound.ref_element("methods").set_list();
		}
		if (reflect_version_2) {
			compound.ref_element("name").set_str(to_string(name_tok));
			compound.ref_element("elements").set_list();
		}
		compound.ref_element("bases") = base_list;
		compound.ref_element("kind").set_str(to_string(toks[0]));
		compound.ref_element("comment").set_str(to_string(comment));
		compound.ref_element("contains_CGV_API").set_bool(contains_cgv_api);

		// store reflection info
		ensure_reflect_info();
		if (reflect_version_1) {
			if (compound_stack.size() > 0) {
				compound_stack.back()->ref_element("compounds").append_to_list(compound);
				compound_stack.push_back(&compound_stack.back()->ref_element("compounds").ref_element(compound_stack.back()->ref_element("compounds").get_size()-1));
			}
			else {
				reflect_info->ref_element("compounds").append_to_list(compound);
				compound_stack.push_back(&reflect_info->ref_element("compounds").ref_element(reflect_info->get_element("compounds").get_size()-1));
			}
		}
		if (reflect_version_2) {
			if (compound_stack.size() > 0) {
				compound_stack.back()->ref_element("elements").append_to_list(compound);
				compound_stack.push_back(&compound_stack.back()->ref_element("elements").ref_element(compound_stack.back()->ref_element("elements").get_size()-1));
			}
			else {
				reflect_info->ref_element("elements").append_to_list(compound);
				compound_stack.push_back(&reflect_info->ref_element("elements").ref_element(reflect_info->get_element("elements").get_size()-1));
			}
		}
		// generate debug info
		if (debug_reflection)
			std::cout << "reflect compound " << compound << std::endl;

		// start definition of compound and define default access type
		compound_access_type_stack.push_back("public");
		if (to_string(toks[0]) == "class")
			compound_access_type_stack.back() = "private";
		rp_stack.push_back(RP_COMPOUND);

		return true;
	}
	else if (to_string(toks[0]) == "namespace") {
		token name_tok = toks[1];
		if (debug_reflection)
			std::cout << "reflect start namespace " << to_string(name_tok).c_str() << " // " 
						 << to_string(comment).c_str() << std::endl;
		namespace_stack.push_back(to_string(name_tok));
		rp_stack.push_back(RP_NAMESPACE);
		return true;
	}
	else {
		// remaining declarations must be variable / member, function / method, constructor or operator

		// only variable or member declarations supported yet
		if (to_string(toks.back()) == "const" || to_string(toks.back()) == ")") {
			error("method reflection not supported yet", toks.back());
			return false;
		}
		if (to_string(toks.back()) == "]") {
			error("array reflection not supported yet", toks.back());
			return false;
		}

		// parse into static, type and name
		int idx = 0;
		bool is_static = false;
		if (to_string(toks[0]) == "static") {
			is_static = true;
			idx = 1;
		}
		token type_tok(toks[idx].begin, toks[toks.size()-2].end);
		token name_tok = toks.back();

		if (reflect_version_1) {
			if (!compound_stack.empty()) {
				// construct member info
				variant member_info = variant(variant::map_type());
				member_info.ref_element("name").set_str(to_string(name_tok));
				member_info.ref_element("type").set_str(to_string(type_tok));
				member_info.ref_element("access_type").set_str(compound_access_type_stack.back());
				member_info.ref_element("is_static").set_bool(is_static);
				member_info.ref_element("comment").set_str(to_string(comment));
				
				// store member info
				compound_stack.back()->ref_element("members").append_to_list(member_info);

				// generate debug info
				if (debug_reflection)
					std::cout << "reflect member " << member_info << std::endl;
			}
			else {
				// construct variable info
				variant variable_info = variant(variant::map_type());
				variable_info.ref_element("namespaces") = construct_namespace_list();
				variable_info.ref_element("namespace_prefix") = construct_namespace_sequence();
				variable_info.ref_element("name").set_str(to_string(name_tok));
				variable_info.ref_element("type").set_str(to_string(type_tok));
				variable_info.ref_element("comment").set_str(to_string(comment));

				// store variable info
				ensure_reflect_info();
				reflect_info->ref_element("variables").append_to_list(variable_info);

				// generate debug info
				if (debug_reflection)
					std::cout << "reflect variable " << variable_info << std::endl;
			}
		}
		if (reflect_version_2) {
			// construct variable info
			variant variable_info = variant(variant::map_type());
			variable_info.ref_element("reflect").set_str("variable");
			variable_info.ref_element("name").set_str(to_string(name_tok));
			variable_info.ref_element("type").set_str(to_string(type_tok));
			if (compound_stack.empty()) {
				variable_info.ref_element("namespaces") = construct_namespace_list();
				variable_info.ref_element("namespace_prefix") = construct_namespace_sequence();
			}
			else
				variable_info.ref_element("access_type").set_str(compound_access_type_stack.back());
			variable_info.ref_element("is_static").set_bool(is_static);
			variable_info.ref_element("comment").set_str(to_string(comment));
			if (!compound_stack.empty()) {
				compound_stack.back()->ref_element("elements").append_to_list(variable_info);
			}
			else {
				ensure_reflect_info();
				reflect_info->ref_element("elements").append_to_list(variable_info);
			}
			// generate debug info
			if (debug_reflection)
				std::cout << "reflect variable " << variable_info << std::endl;
		}
		return true;
	}
	error("could not handle reflection", code);
	return false;
}


using namespace std;
using namespace cgv::media::text;
using namespace cgv::utils::file;

void remove_comments(string& text)
{
	string new_text;
	unsigned i;
	for (i=0; i<text.length(); ++i) {
		if (text[i] == '/') {
			if (i+1 < text.length()) {
				if (text[i+1] == '/') {
					i += 2;
					while (i<text.length()) {
						if (text[i] == '\n') {
							break;
						}
						++i;
					}
				}
				else if (text[i+1] == '*') {
					i += 3;
					while (i<text.length()) {
						if (text[i-1] == '*' && text[i] == '/') {
							break;
						}
						++i;
					}
					++i;
				}
			}
		}
		if (i < text.length())
			new_text += text[i];
	}
	text = new_text;
}

void remove_strings(string& text)
{
	string new_text;
	unsigned i;
	for (i=0; i<text.length(); ++i) {
		bool was_string = false;
		if (text[i] == '"') {
			++i;
			while (i<text.length()) {
				if (text[i] == '\\')
					++i;
				else if (text[i] == '"') {
					was_string = true;
					break;
				}
				++i;
			}
		}
		if (!was_string)
			new_text += text[i];
	}
	text = new_text;
}

void filter_out_preprocessor(string& text)
{
	string new_text;
	vector<token> toks;
	tokenizer(text).set_ws("").set_sep("#").bite_all(toks);
	for (unsigned i=0; i<toks.size(); ++i) {
		if (toks[i] == "#") {
			const char* p = toks[i].begin;
			const char* e = toks.back().end;
			while (p < e && *p != '\n')
				++p;
			if (!new_text.empty())
				new_text += '\n';
			new_text += string(toks[i].begin, p-toks[i].begin);
			while (i+1 < toks.size() && toks[i+1].begin < p)
				++i;
		}
	}
	text = new_text;
}

void filter_out_local_includes(string& text)
{
	string new_text;
	vector<line> lines;
	split_to_lines(text, lines, true);
	unsigned i;
	for (i=0; i<lines.size(); ++i) {
		string l = to_upper(to_string(token(skip_spaces(lines[i].begin+1,lines[i].end), lines[i].end)));
		unsigned n = l.length();
		bool keep = true;
		if (l.substr(0,7) == "INCLUDE") {
			bool is_local = true;
			if (l.find_first_of('"') == string::npos) {
				unsigned p0 = l.find_first_of('<');
				unsigned p1 = l.find_first_of('>', p0);
				if (p0 != string::npos && p1 != string::npos)
					is_local = false;
			}
			if (is_local)
				keep = false;
		}
		if (keep) {
			if (!new_text.empty())
				new_text += '\n';
			new_text += to_string(lines[i]);
		}
	}
	text = new_text;
}

enum LineType {
	LT_IF,
	LT_ELIF,
	LT_ELSE,
	LT_ENDIF,
	LT_INCLUDE,
	LT_DEFINE,
	LT_OTHER
};


void filter_out_empty_blocks(string& text)
{
	string new_text;
	vector<line> lines;
	split_to_lines(text, lines, true);
	vector<LineType> line_types;
	line_types.resize(lines.size());
	unsigned i,j;
	for (i=0; i<lines.size(); ++i) {
		string l = to_upper(to_string(token(skip_spaces(lines[i].begin+1,lines[i].end), lines[i].end)));
		unsigned n = l.length();
		if (l.substr(0,2) == "IF")
			line_types[i] = LT_IF;
		else if (l.substr(0,4) == "ELIF")
			line_types[i] = LT_ELIF;
		else if (l.substr(0,4) == "ELSE")
			line_types[i] = LT_ELSE;
		else if (l.substr(0,5) == "ENDIF")
			line_types[i] = LT_ENDIF;
		else if (l.substr(0,7) == "INCLUDE")
			line_types[i] = LT_INCLUDE;
		else if (l.substr(0,6) == "DEFINE")
			line_types[i] = LT_DEFINE;
		else 
			line_types[i] = LT_OTHER;
	}
	j = i = 0;
	bool empty = true;
	unsigned d = 0;
	while (i<line_types.size()) {
		switch (line_types[i]) {
		case LT_IF: 
			++d; 
			break;
		case LT_ELIF:
		case LT_ELSE:
			break;
		case LT_ENDIF: 
			--d; 
			break;
		case LT_INCLUDE:
		case LT_DEFINE:
			 empty = false;
			 break;
		case LT_OTHER:
			break;
		}
		if (d == 0) {
			if (!empty) {
				while (j <= i) {
					if (line_types[j] != LT_OTHER) {
						if (!new_text.empty())
							new_text += '\n';
						new_text += to_string(lines[j]);
					}
					++j;
				}
				empty = true;
			}
			else
				j = i+1;
		}
		++i;
	}
	text = new_text;
}


bool scan_includes(const std::string& fn, std::string& s)
{
	std::string content;
	if (!cgv::utils::file::read(fn, content, true))
		return false;
	remove_comments(content);
	remove_strings(content);
	filter_out_preprocessor(content);
	filter_out_local_includes(content);
	filter_out_empty_blocks(content);
/*
	std::vector<line> lines;
	cgv::media::text::split_to_lines(content, lines, true);
	unsigned i;
	for (i=0; i<lines.size(); ++i) {
		lines[i].begin = skip_spaces(lines[i].begin,lines[i].end);
		std::string l = to_string(lines[i]);
		if (l.substr(0, 8) == "#include") {
			if (l.find_first_of('"') == std::string::npos) {
				unsigned p0 = l.find_first_of('<');
				unsigned p1 = l.find_first_of('>', p0);
				if (p0 != std::string::npos && p1 != std::string::npos) {
					l = l.substr(p0+1, p1-p0-1);
					replace(l, '\\', '/');
					M[l] = variant(true);
				}
			}
		}
	}
	*/
	if (!content.empty()) {
		if (!s.empty())
			s = s + '\n';
		s += "// ";
		s += fn;
		s += "\n";
		s = s+content;
	}
	return true;
}

bool ph_processor::process(unsigned int i, unsigned int j)
{
	while (i<j) {
		switch (commands[i].ct) {
		case CT_TEXT :
		case CT_IMPLICIT_TEXT :
			if (generate_output) {
				(*os) << to_string(commands[i]).c_str();
			}
			break;
		case CT_DEFINE :
		case CT_SKIP :
			{
				variant v;
				commands[i].expressions[0].evaluate(v, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
			}
			break;
		case CT_READ :
			{
				variant file_name, str, ascii;
				commands[i].expressions[0].evaluate(file_name, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				commands[i].expressions[2].evaluate(ascii, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,2))
					return false;
				if (!cgv::utils::file::exists(file_name.get_str())) {
					error(std::string("could not find file '")+file_name.get_str()+"'", commands[i]);
					return false;
				}
				std::string content;
				if (!cgv::utils::file::read(file_name.get_str(), content, ascii.get_bool())) {
					error(std::string("could not read file '")+file_name.get_str()+"'", commands[i]);
					return false;
				}
				commands[i].expressions[1].evaluate(str, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,1))
					return false;
				if (!(str.is_name() || str.is_str())) {
					error(std::string("file '")+file_name.get_str()+"' can only be read into a variable given by name or string ", commands[i]);
					return false;
				}
				ref_variable(str.is_name() ? str.get_name() : str.get_str()) = variant(content);
			}
			break;
		case CT_WRITE :
			{
				variant file_name, str, ascii;
				commands[i].expressions[0].evaluate(file_name, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				commands[i].expressions[1].evaluate(str, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,1))
					return false;
				commands[i].expressions[2].evaluate(ascii, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,2))
					return false;
				if (!(str.is_name() || str.is_str())) {
					error(std::string("file '")+file_name.get_str()+"' can only be read into a variable given by name or string ", commands[i]);
					return false;
				}
				std::string content = ref_variable(str.is_name() ? str.get_name() : str.get_str()).get_str();
				if (!cgv::utils::file::write(file_name.get_str(), &content[0], content.size(), ascii.get_bool())) {
					error(std::string("could not write to file '")+file_name.get_str()+"'", commands[i]);
					return false;
				}
			}
			break;
		case CT_RAND :
			{
				variant minval, maxval, str, seed;
				commands[i].expressions[0].evaluate(minval, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				commands[i].expressions[1].evaluate(maxval, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,1))
					return false;
				commands[i].expressions[2].evaluate(str, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,2))
					return false;
				if (!(str.is_name() || str.is_str())) {
					error(std::string("rand can only be generated into a variable given by name or string"), commands[i]);
					return false;
				}
				if (commands[i].expressions.size() == 4) {
					commands[i].expressions[3].evaluate(seed, this);
					if (exit_code != 0)
						return true;
					if (!check_evaluation_error(i,3))
						return false;
				}
				static cgv::math::random r;//(13);
				if (commands[i].expressions.size() == 4) {
					r.set_seed(seed.get_int());
				}
				if (minval.is_double() || maxval.is_double()) {
					double d;
					r.uniform(minval.get_double(),maxval.get_double(), d);
					ref_variable(str.is_name() ? str.get_name() : str.get_str()) = variant(d);
				}
				else {
					int i;
					r.uniform(minval.get_int(),maxval.get_int(), i);
					ref_variable(str.is_name() ? str.get_name() : str.get_str()) = 
						variant(i);
				}
			}
			break;
		case CT_NAMESPACE :
			{
				variant v;
				commands[i].expressions[0].evaluate(v, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				std::string var_name;
				if (v.is_name())
					var_name = v.get_name();
				else if (v.is_str())
					var_name = v.get_str();
				else {
					error("namespace argument must evaluate to name or string", commands[i]);
					return false;
				}
				variant* ns = find_variable(var_name, true);
				if (!ns) {
					ns = &ref_variable(var_name, true);
					*ns = variant(variant::map_type());
				}
				else if (ns->get_value_type() != MAP_VALUE) {
					error("namespace cannot overwrite previously defined variable", commands[i]);
					return false;
				}
				push_namespace(ns);
				if (!process(i+1, commands[i].block_end))
					return false;
				pop_namespace();
				i = commands[i].block_end;
			}
			--i;
			break;
		case CT_FOR : 
			{
				variant v;
				commands[i].expressions[0].evaluate(v, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				do {
					commands[i].expressions[1].evaluate(v, this);
					if (exit_code != 0)
						return true;
					if (!check_evaluation_error(i,1))
						return false;
					if (!v.get_bool())
						break;
					if (!process(i+1, commands[i].block_end))
						return false;
					commands[i].expressions[2].evaluate(v, this);
					if (exit_code != 0)
						return true;
					if (!check_evaluation_error(i,2))
						return false;
				} while (true);
				i = commands[i].block_end;
			}
			--i;
			break;
		case CT_IF : 
			{
				variant v;
				commands[i].expressions[0].evaluate(v, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if (v.get_bool()) {
					if (!process(i+1, commands[i].block_end))
						return false;
					i = commands[i].block_end;
					while (i < j && commands[i].ct == CT_ELIF)
						i = commands[i].block_end;
					if (i < j && commands[i].ct == CT_ELSE)
						i = commands[i].block_end;
				}
				else {
					i = commands[i].block_end;
					bool success = false;
					while (i < j && commands[i].ct == CT_ELIF) {
						if (!success) {
							commands[i].expressions[0].evaluate(v, this);
							if (exit_code != 0)
								return true;
							if (!check_evaluation_error(i,0))
								return false;
							if (v.get_bool()) {
								success = true;
								if (!process(i+1, commands[i].block_end))
									return false;
							}
						}
						i = commands[i].block_end;
					}
					if (i<j && commands[i].ct == CT_ELSE) {
						if (!success) {
							if (!process(i+1, commands[i].block_end))
								return false;
						}
						i = commands[i].block_end;
					}
				}
			}
			--i;
			break;
		case CT_EVAL : 
		case CT_STRING : 
			{
				variant v;
				commands[i].expressions[0].evaluate(v, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if (generate_output) {
					if (commands[i].ct == CT_STRING)
						(*os) << "\"" << v.get_str().c_str() << "\"";
					else
						(*os) << v.get_str().c_str();
				}
			}
			break;
		case CT_LIST : 
			if (!process_list(i))
				return false;
			if (exit_code != 0)
				return true;
			break;
		case CT_EXCLUDE : 
		case CT_INCLUDE : 
			if (!process_include(i))
				return false;
			if (exit_code != 0)
				return true;
			break;
		case CT_CINCLUDE : 
			if (!process_include(i, true))
				return false;
			if (exit_code != 0)
				return true;
			break;
		case CT_INSERT : 
			if (!process_include(i, false, true))
				return false;
			if (exit_code != 0)
				return true;
			break;
		case CT_ELIF : 
			error("elif without if", commands[i]);
			return false;
			break;
		case CT_ELSE : 
			error("else without if", commands[i]);
			return false;
			break;
		case CT_REFLECT_NEXT_LINE :
			{
				tokenizer t(token(commands[i].end,&(*content->rbegin())+1));
				if (t.empty()) {
					error("next line reflection command without a line to follow", commands[i]);
					return false;
				}
				token comment_tok = t.set_sep("\n").set_ws("").bite();
				if (to_string(comment_tok) == "\n")
					comment_tok = token();
				else {
					if (t.empty()) {
						error("next line reflection command without a line to follow", commands[i]);
						return false;
					}
					t.bite();
				}
				if (t.empty()) {
					error("next line reflection command without a line to follow", commands[i]);
					return false;
				}
				t.set_sep(";{");
				if (!reflect(t.bite(), comment_tok))
					return false;
			}
			break;
		case CT_REFLECT_PREV_LINE :
			{
				tokenizer t(token(commands[i].end,&(*content->rbegin())+1));
				token comment_tok;
				if (!t.empty()) {
					comment_tok = t.set_sep("\n").set_ws("").bite();
					if (to_string(comment_tok) == "\n")
						comment_tok = token();
				}
				t = tokenizer(token(&(*content->begin()), commands[i].begin-2));
				t.set_sep("\n").set_ws("");
				if (t.empty()) {
					error("previous line reflection command without a line to preceed", commands[i]);
					return false;
				}
				if (!reflect(t.reverse_bite(), comment_tok))
					return false;
			}
			break;
		case CT_COUT:
			{
				variant v;
				commands[i].expressions[0].evaluate(v, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				std::cout << v.get_value();
				std::cout.flush();
				break;
			}
		case CT_CIN:
			{
				variant v;
				commands[i].expressions[0].evaluate(v, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if (!v.is_reference()) {
					error("cin target expression must evaluate to reference", commands[i]);
					return false;
				}
				char buffer[1000];
				std::cin.getline(buffer, 1000);
				std::string s(buffer);
				int i;
				if (is_integer(s, i))
					v.ref_value().set_int(i);
				else
					v.ref_value().set_str(s);
				break;
			}
		case CT_ERROR:
		case CT_WARNING:
			{
				variant v1, v2;
				commands[i].expressions[0].evaluate(v1, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				commands[i].expressions[1].evaluate(v2, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,1))
					return false;
				std::cout << file_name.c_str() << "(" << get_line_number(commands[i]) << ") : ";
				if (commands[i].ct == CT_ERROR) 
					std::cout << "error ";
				else
					std::cout << "warning ";
				std::cout << v1.get_int() << ": " << v2.get_str().c_str() << std::endl;
				break;
			}
		case CT_SYSTEM :
			{
				variant v1;
				commands[i].expressions[0].evaluate(v1, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				variant v2;
				commands[i].expressions[1].evaluate(v2, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,1))
					return false;
				if (!v2.is_reference()) {
					error("system result expression must evaluate to reference", commands[i]);
					return false;
				}
				v2.ref_value().set_int(system(v1.get_str().c_str()));
				break;
			}
		case CT_EXIT :
			{
				variant v1;
				commands[i].expressions[0].evaluate(v1, this);
				if (exit_code != 0)
					return true;
				check_evaluation_error(i,0);
				exit_code = v1.get_int();
				return true;
			}
		case CT_FUNC:
			{
				variant v1;
				commands[i].expressions[0].evaluate(v1, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if(!(v1.is_str() || v1.is_name() || v1.is_reference())){
					error("first parameter of func() has to be a name, string or reference", commands[i]);
					return false;
				}
				if (commands[i].expressions.size() == 2 && !commands[i].expressions[1].is_func_decl()) {
					error(commands[i].expressions[1].get_last_error(), commands[i].expressions[1].get_last_error_token());
					return false;
				}
				++nr_functions;
				if (v1.is_reference())
					v1.ref_value() = variant(func_type(i+1, commands[i].block_end, this, get_current_namespace()->get_environment()));
				else
					ref_variable(v1.is_str() ? v1.get_str() : v1.get_name()) = variant(func_type(i+1, commands[i].block_end, this, get_current_namespace()->get_environment()));
				i = commands[i].block_end-1;
				break;
			}
		case CT_DIR:
			{
				variant v1;
				commands[i].expressions[0].evaluate(v1, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if(!v1.is_str()){
					error("first parameter of dir() has to be a directory as string", commands[i]);
					return false;
				}
				variant v2;
				commands[i].expressions[1].evaluate(v2, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if(!v2.is_str()){
					error("second parameter of dir() has to be a filter string", commands[i]);
					return false;
				}
				variant v3;
				commands[i].expressions[2].evaluate(v3, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if(!v3.is_int()){
					error("third parameter of dir() has to be a int", commands[i]);
					return false;
				}
				variant v4;
				commands[i].expressions[3].evaluate(v4, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if(!v4.is_list()){
					error("fourth parameter of dir() has to be a list", commands[i]);
					return false;
				}

				std::string directory=v1.get_str();
				void* fh=cgv::utils::file::find_first(directory+"/*");
				if(fh==0){
					error("path \""+directory+"\" does not exist", commands[i]);
				}

				std::string filter=v2.get_str();
				bool recursive=v3.get_int()>0;

				
				if(!(directory[directory.size()-1]=='/'))
					directory=directory+"/";

				std::string actual_directory="";//TODO

				bool rel_path=true;
				if(actual_directory==directory)
					rel_path=false;

				bool success=recursive_dir(directory,actual_directory,filter,rel_path,v4,recursive);
				if(!success) return false;

				break;
			}
		case CT_TRANSFORM :
			{
				variant v1;
				commands[i].expressions[0].evaluate(v1, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if(!v1.is_str()){
					error("first parameter of transform() must evaluate to a file name as string", commands[i]);
					return false;
				}
				variant v2;
				commands[i].expressions[1].evaluate(v2, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if(!v2.is_str()){
					error("second parameter of transform() must evaluate to a file name as string", commands[i]);
					return false;
				}
				std::string inp = find_include_file(v1.get_str(), true);
				if (inp.empty()) {
					error("could not find input file to transform() ", commands[i]);
					return false;
				}
				ph_processor* fp = new ph_processor(additional_include_path, false);
				bool success = false;
				if (success = fp->parse_file(inp)) {
					std::string output;
					success = fp->process_to_file(v2.get_str(), cgv::utils::file::get_last_write_time(inp)) != 0;
				}
				if (fp->exit_code != 0)
					exit_code = fp->exit_code;
				delete fp;
				if (!success)
					return false;
				break;
			}
		case CT_SCAN_INCLUDES:
			{
				variant v1;
				commands[i].expressions[0].evaluate(v1, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if(!v1.is_str()){
					error("first parameter of scan_includes() must evaluate to a file name as string", commands[i]);
					return false;
				}
				variant v2;
				commands[i].expressions[1].evaluate(v2, this);
				if (exit_code != 0)
					return true;
				if (!check_evaluation_error(i,0))
					return false;
				if(!v2.is_name()){
					error("second parameter of scan_includes() must evaluate to a variable name", commands[i]);
					return false;
				}
				ppp::variant& v_str = ref_variable(v2.get_name());
				if (!v_str.is_str()){
					error("second parameter of scan_includes() must evaluate to the name of variable of STRING type", commands[i]);
					return false;
				}
				std::string inp = find_include_file(v1.get_str(), true);
				if (inp.empty()) {
					error("could not find input file to scan_includes() ", commands[i]);
					return false;
				}
				scan_includes(inp, v_str.ref_str());
			}
			break;
		default:
			error(std::string("token ")+
				   commands[i].get_keyword()+
					" not expected", commands[i]);
			return false;
			break;
		}
		++i;
	}
	return true;
}

bool ph_processor::recursive_dir(
			std::string rel_directory,
			std::string actual_directory,
			std::string filter,
			bool rel_path,
			variant& desc_list,
			bool recursive)
{
	bool skip_files=false;
	bool list_dirs =false;
	void* fh;
	if (filter == ".") {
//		std::cout << "directories in :" << (rel_directory+"*").c_str() << std::endl;
		fh=cgv::utils::file::find_first(rel_directory+"*");
		skip_files= true;
		list_dirs = true;
	}
	else {
		std::string mask=rel_directory+filter;
		fh=cgv::utils::file::find_first(mask);
		if(fh==0 && recursive) {
			fh=cgv::utils::file::find_first(rel_directory+"*");
			skip_files=true;
		}
	}
	if(fh==0) 
		return true;
	bool not_finished;
	do {
		not_finished = false;
		do {
			std::string fn=cgv::utils::file::find_name(fh);
			if(fn=="."||fn==".."||fn==".svn")
				continue;
			bool is_dir=cgv::utils::file::find_directory(fh);
			if( (!is_dir && !skip_files) || (is_dir && list_dirs) ){
				std::vector<variant> file_desc;
				//filename
				file_desc.push_back(new variant(fn));
				//relative path
				file_desc.push_back(new variant(rel_directory+fn));
				//absolute path
				if(rel_path)
					file_desc.push_back(new variant(actual_directory+fn));
				else
					file_desc.push_back(new variant(rel_directory+fn));
				//is directory?
				file_desc.push_back(new variant(is_dir));
				//size of file
				file_desc.push_back(new variant((int)cgv::utils::file::find_size(fh)));
				//list of sub dirs
				std::vector<variant> sub_dirs;
				std::vector<token> toks;
				tokenizer(actual_directory).set_ws("/\\").bite_all(toks);
				for (unsigned int i=0; i<toks.size(); ++i)
					sub_dirs.push_back(to_string(toks[i]));
				file_desc.push_back(new variant(sub_dirs));
				
				desc_list.append_to_list(new variant(file_desc));
			}

			if(recursive&&is_dir){
				if(!recursive_dir(rel_directory+fn+"/",
										actual_directory+fn+"/",
										filter,
										rel_path,desc_list,recursive))
					return false;
			}
		}
		while((fh=cgv::utils::file::find_next(fh))!=0);
		if (!skip_files && recursive) {
			fh=cgv::utils::file::find_first(rel_directory+"*");
			skip_files=true;
			if (fh)
				not_finished = true;
		}
	} while (not_finished);
	return true;
}

bool ph_processor::parse()
{
	unsigned nr_skip = 0;
	if (content->size() > 1) {
		if (content->at(0) == '@' && content->at(1) == '=') {
			if (content->size() == 2 || is_space(content->at(2)))
				special = 0;
			else
				special = content->at(2);
			nr_skip = 3;
		}
	}
	if (special == 0)
		return parse_without_special(nr_skip);

	found_error = false;
	bool block_begin_follows = false;
	lines.clear();
	commands.clear();
	// iterate all lines and @-tokens on each line
	std::stack<unsigned int> block_stack;
	split_to_lines(*content, lines, false);

	tokenizer toker(*content);
	toker.begin += nr_skip;
	toker.set_sep(std::string(1,special)).set_skip("'\"","'\"").set_ws("");
	while (!toker.empty()) {
		// construct command token
		command_token ct;
		// in inter woven mode each token not starting with @ is a text token
		token tok = toker.bite();
		std::string s = to_string(tok);
		// either as a text token
		if (*tok.begin != special)
			ct = command_token(tok);
		// if equal to @, first check if a ppp comment is following
		else if (!toker.empty() && *toker.begin == '/') {
			// skip rest of current line
			while (!toker.empty()) {
				++toker.begin;
				if (toker.begin[-1] == '\n')
					break;
			}
			continue;
		}
		// finally split off command token needs to be split off from next token
		else {
			// if special character repeats, interpret first as escape character
			if (toker.begin < toker.end && *toker.begin == special) {
				++toker.begin;
				ct = command_token(tok);
			}
			else if (!ct.split_off_from(toker)) {
				error(ct.get_last_error(), ct.get_last_error_token());
				continue;
			}
		}
		// eliminate empty tokens before else and elif commands
		if (ct.remove_preceeding_empty_text_token() && !commands.empty() &&
			 commands.back().is_empty())
			 commands.pop_back();
		// handle the beginning of blocks
		if (block_begin_follows) {
			if (ct.ct == CT_BEGIN) {
				block_begin_follows = false;
				continue;
			}
			if (ct.is_empty())
				continue;
			error(std::string("expected ")+special+'{', ct);
			block_begin_follows = false;
		}
		// handle the end of blocks
		if (ct.ct == CT_END) {
			if (block_stack.empty()) {
				error(std::string()+special+" unexpected", ct);
				continue;
			}
			commands[block_stack.top()].block_end = (unsigned int) commands.size();
			block_stack.pop();
			continue;
		}
		// handle blocks that are to follow
		block_begin_follows = ct.block_follows();
		if (block_begin_follows)
			block_stack.push((unsigned int) commands.size());
		commands.push_back(ct);
	}
	if (!block_stack.empty()) {
		error("block begin not matched", commands[block_stack.top()]);
	}
	//std::cout << (unsigned int) lines.size() << " lines and " << commands.size() << " attributed line tokens" << std::endl;
	return !found_error;
}
bool ph_processor::parse_without_special(unsigned nr_skip)
{
	found_error = false;
	bool block_begin_follows = false;
	lines.clear();
	commands.clear();

	// iterate all lines and @-tokens on each line
	std::stack<unsigned int> block_stack;
	split_to_lines(*content, lines, false);

	token T(*content);
	T.begin += nr_skip;
	do {
		T.skip(" \t\n");
		if (T.empty())
			break;
		// construct command token
		command_token ct;
		// if equal to @, first check if a ppp comment is following
		if (*T.begin == '/') {
			// skip rest of current line
			while (!T.empty()) {
				++T.begin;
				if (T.begin[-1] == '\n')
					break;
			}
			continue;
		}
		// finally split off command token needs to be split off from next token
		else {
			const char* old_begin = T.begin;
			bool success = ct.split_off_from(T);
			if (!success && ct.get_last_error() != "could not determine command") {
				error(ct.get_last_error(), ct.get_last_error_token());
				return false;
			}
			if (success && (ct.ct == CT_EVAL || ct.ct == CT_STRING || ct.ct == CT_LIST)) {
				T.begin = old_begin;
				success = false;
			}
			if (!success) {
				// try to interpret as expression
				token old_T = T, exp_tok;
				if (tokenizer(T).set_sep(";{}").set_ws("").set_skip("'\"","'\"").balanced_bite(exp_tok, "([", ")]", true)) {
					T.begin = exp_tok.end;
					if (!T.empty() && T[0] == ';')
						++T.begin;

					expression_processor ep;
					if (!ep.parse(exp_tok)) {
						error(ep.get_last_error(), ep.get_last_error_token());
						return false;
					}
					if (!ep.validate(true)) {
						error(std::string("could not validate expression: ")+ep.get_last_error(), 
							ep.get_last_error_token().empty() ? exp_tok : ep.get_last_error_token());
						return false;
					}
					ct.ct = CT_DEFINE;
					ct.parenthesis_index = 0;
					ct.expressions.clear();
					ct.begin = exp_tok.begin;
					ct.end = exp_tok.end;
					ct.expressions.push_back(ep);
				}
				else {
					error("could not extract expression with balanced parantheses", old_T);
					return false;
				}
			}
		}
		// handle the beginning of blocks
		if (block_begin_follows) {
			if (ct.ct == CT_BEGIN) {
				block_begin_follows = false;
				continue;
			}
			if (ct.is_empty())
				continue;
			error("expected {", ct);
			block_begin_follows = false;
		}
		// handle the end of blocks
		if (ct.ct == CT_END) {
			if (block_stack.empty()) {
				error("} unexpected", ct);
				continue;
			}
			commands[block_stack.top()].block_end = (unsigned int) commands.size();
			block_stack.pop();
			continue;
		}
		// handle blocks that are to follow
		block_begin_follows = ct.block_follows();
		if (block_begin_follows)
			block_stack.push((unsigned int) commands.size());
		commands.push_back(ct);
	} while (!T.empty());
	if (!block_stack.empty()) {
		error("block begin not matched", commands[block_stack.top()]);
	}
	//std::cout << (unsigned int) lines.size() << " lines and " << commands.size() << " attributed line tokens" << std::endl;
	return !found_error;
}

			}
		}
	}
}
