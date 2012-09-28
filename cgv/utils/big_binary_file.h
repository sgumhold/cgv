#pragma once

#include <string>
#include "lib_begin.h"

namespace cgv {
	namespace utils {
/**
* class to handle files with very large sizes (>=2GB)
* hiding the ugly win32 api calls
* 
* linux-support by using 64bit pointer, LFS required in kernel
*/
class CGV_API big_binary_file
{
public:
	enum MODE {READ = 1, WRITE = 2, READ_WRITE = 3};

private:
	MODE access_mode;
	std::string filename;
	bool fileopened;
	void* file;
public:
	
	///assosiates the new instance to the file filename
	big_binary_file(const std::string &filename = "");
	
	///close the file (if it is still opened)
	virtual ~big_binary_file();
	
	///open a file in read or write mode
	bool open(MODE m, const std::string& file_name = "");
	
	///close the file
	void close();
	
	///return true if the file is opened
	bool is_open();

	///return the size of the file in bytes
	long long size();
		
	///read num bytes from the file into the targetbuffer
	bool read(unsigned char *targetbuffer,unsigned long num, unsigned long *numread=NULL);

	///write num bytes to the file from the sourcebuffer (file must be opened with write access first)
	bool write(const unsigned char *sourcebuffer,unsigned long num, unsigned long *numwrote=NULL);

	/// read a typedef value
	template <typename T>
	bool read(T& v) { return read((unsigned char*)(&v),sizeof(T)); }
	/// read an array of typed values
	template <typename T>
	bool read_array(T* a, unsigned int n) { return read((unsigned char*)a,sizeof(T)*n); }

	/// write a typedef value
	template <typename T>
	bool write(const T& v) { return write((const unsigned char*)(&v),sizeof(T)); }
	/// write an array of typed values
	template <typename T>
	bool write_array(const T* a, unsigned int n) { return write((const unsigned char*)a,sizeof(T)*n); }

	//set the file pointer to index (position in bytes from the beginning of the file)
	bool seek(long long index);
	
	///return the position of the file pointer in bytes 
	long long position();
};

	}
}

#include <cgv/config/lib_end.h>