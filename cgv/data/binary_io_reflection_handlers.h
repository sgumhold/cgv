#pragma once

#include <stdio.h>
#include "io_reflection_handler.h"

#include "lib_begin.h"

namespace cgv {
	namespace data {


/** reflect to and from binary file */
class CGV_API binary_reflection_handler : public io_reflection_handler
{
protected:
	FILE* fp;	
public:
	///
	binary_reflection_handler(const std::string& _content, unsigned _ver);
	///
	bool reflect_header();
	///
	void close();
};

/** read from ascii file */
class CGV_API binary_read_reflection_handler : public binary_reflection_handler
{
protected:
	bool read_reflect_header(const std::string& _content, unsigned _ver);
public:
	binary_read_reflection_handler(const std::string& file_name, const std::string& _content, unsigned _ver);
    ///
	binary_read_reflection_handler(FILE* _fp, const std::string& _content, unsigned _ver);
	/// this should return true
	bool is_creative() const;
	///
	bool reflect_member_void(const std::string& member_name, 
							 void* member_ptr, cgv::reflect::abst_reflection_traits* rt);
};


/** read from ascii file */
class CGV_API binary_write_reflection_handler : public binary_reflection_handler
{
public:
	/// construct from file_name by opening file in ascii mode
	binary_write_reflection_handler(const std::string& file_name, const std::string& _content, unsigned _ver);
	/// construct from std::ostream
	binary_write_reflection_handler(FILE* _fp, const std::string& _content, unsigned _ver);
	///
	bool reflect_member_void(const std::string& member_name, 
							 void* member_ptr, cgv::reflect::abst_reflection_traits* rt);
};


	}
}

#include <cgv/config/lib_end.h>