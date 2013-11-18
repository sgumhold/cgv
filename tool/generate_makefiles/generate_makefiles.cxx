#include <stdlib.h>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
	std::string prog_name(argv[0]);
	prog_name = prog_name.substr(0, prog_name.find_last_of(".")+1)+"bat";
	std::string command = prog_name;
	for (int i=1; i<argc;++i) {
		command += " ";
		command += argv[i];
	}
	return system(command.c_str());
}