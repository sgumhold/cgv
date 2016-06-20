#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cgv/utils/scan.h>
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>

using namespace cgv::utils;
using namespace cgv::utils::file;

bool ensure_output_dir(const char* dst)
{
	std::string path = get_path(dst);
	if (path.empty())
		return true;
	if (cgv::utils::dir::exists(path))
		return true;

	switch (path.at(path.size()-1)) {
		case '/' :
		case '\\' :
		case ':' :
		case '.' :
			return false;
	}

	std::string dir_name = cgv::utils::file::get_file_name(path);
	std::string new_path = cgv::utils::file::get_path(path);
	if (!ensure_output_dir(new_path.c_str()))
		return false;
	if (cgv::utils::dir::mkdir(path)) {
		std::cout << "created directory " << path.c_str() << std::endl;
		return true;
	}
	return false;
}

int dump_to_cxx(const char* src, const char* dst)
{
	if (!ensure_output_dir(dst)) {
		std::cerr << "error: could not create output directory" << src << std::endl;
		return -1;
	}
	size_t size = 0;
	char* data = cgv::utils::file::read(src,false,&size);
	if (!data) {
		std::cerr << "error: could not open resource file " << src << std::endl;
		return -1;
	}
	std::ofstream fout(dst);
	if (fout.fail()) {
		std::cerr << "error: could not open resource source file " << dst << std::endl;
		delete [] data;
		return -1;
	}
	std::string fn(cgv::utils::file::get_file_name(src));
	fout << "#include <cgv/base/register.h>\n\n//" << src << " ==> " << fn.c_str() << "\nstatic char resource_file[] = { " << fn.size();
	unsigned int i;
	for (i=0; i<fn.size(); ++i) {
		fout << "," << (int)fn[i];
	}
	fout << ",\n   -1,-1,-1,-1, ";
	fout << (int)(char)(size&255) << "," << (int)(char)((size>>8)&255) << "," << (int)(char)((size>>16)&255) << "," << (int)(char)((size>>24)&255);
	for (i=0; i<size; ++i) {
		fout << ',';
		if (i % 20 == 0)
			fout << "\n   ";
		fout << (int) data[i];
	}
	fout << "\n};\n\ncgv::base::resource_file_registration resource_file_";
	for (i=0; i<fn.size(); ++i) {
		switch (fn[i]) {
			case '.':
			case '\\':
			case ':' :
			case '/' :
				fout << '_';
				break;
			default:
				fout << fn[i];
		}
	}

	fout << "(resource_file);\n";
	return 0;
}

int dump_to_h(const char* src, const char* dst)
{
	if (!ensure_output_dir(dst)) {
		std::cerr << "error: could not create output directory" << src << std::endl;
		return -1;
	}
	size_t size = 0;
	std::string content;
	if (!cgv::utils::file::read(src,content,true)) {
		std::cerr << "error: could not open text file " << src << std::endl;
		return -1;
	}
	std::ofstream fout(dst);
	if (fout.fail()) {
		std::cerr << "error: could not open resource header file " << dst << std::endl;
		return -1;
	}
	std::string fn(cgv::utils::file::get_file_name(src));
	fout << "const char* ";
	unsigned i;
	for (i=0; i<fn.size(); ++i) {
		if (fn[i] == '.' || fn[i] == ' ')
			fout << '_';
		else
			fout << fn[i];
	}
	fout << " =\n\t\"";
	for (i=0; i<content.size(); ++i) {
		switch (content[i]) {
		case '\n' : fout << "\\n\"\n\t\""; break;
		case '\t' : fout << "\\t"; break;
		case '\f' : fout << "\\f"; break;
		case '\r' : fout << "\\r"; break;
		case '\v' : fout << "\\v"; break;
		case '\b' : fout << "\\b"; break;
		case '\a' : fout << "\\a"; break;
		case '"'  : fout << "\\\""; break;
		case '\\'  : fout << "\\\\"; break;
		default : fout << content[i]; break;
		}
	}
	fout << "\";\n";
	return 0;
}

int main(int argc, char** argv)
{
	if (argc == 3) {
		if (to_upper(cgv::utils::file::get_extension(argv[1]) == "txt"))
			return dump_to_h(argv[1],argv[2]);
		else
			return dump_to_cxx(argv[1],argv[2]);
	}
	else {
		std::cerr << "usage: res_prep resource_file source_file" << std::endl;
	}
	return -1;
}
