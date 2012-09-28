#include <cgv/utils/dir.h>

#ifdef _WIN32
#include <cgv/utils/file.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>


#endif

namespace cgv {
	namespace utils {
		namespace dir {

bool exists(const std::string& dir_name)
{
#ifdef _WIN32
	return file::find_first(dir_name+"\\*.*") != 0;
#else
	struct stat fileInfo;
	int t = stat(dir_name.c_str(),&fileInfo);
	return !t;
#endif
}

bool chdir(const std::string& dir_name)
{
#ifdef _WIN32
	return _chdir(dir_name.c_str()) == 0;
#else
	return ::chdir(dir_name.c_str());
//	std::cerr << "Not Implemented\n" << std::endl;
#endif
	return false;
}

bool mkdir(const std::string& dir_name)
{
#ifdef _WIN32
	return _mkdir(dir_name.c_str()) == 0;
#else
	return ::mkdir(dir_name.c_str(),S_IRWXU|S_IRWXG|S_IRWXO);
//	std::cerr << "Not Implemented\n" << std::endl;
#endif
}

bool rmdir(const std::string& dir_name)
{
#ifdef _WIN32
	return _rmdir(dir_name.c_str()) == 0;
#else
	return ::rmdir(dir_name.c_str());
//	std::cerr << "Not Implemented\n" << std::endl;
#endif
	return false;
}


std::string current()
{
#ifdef _WIN32
	char buf[FILENAME_MAX];
	char * pbuf = _getcwd(buf,FILENAME_MAX);
	std::string s = (pbuf) ? buf : "";
#else
	char* r = getcwd(NULL,0);//buff,256);
	std::string s="";
	if (r == NULL) {
		std::cerr<< "failed"<<std::endl;
	}
	else {
		s = r;
		free(r);
	}
#endif
	return s;
}
		}
	}
}
