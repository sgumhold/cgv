#pragma once

#include <iostream>
#include <fstream>
#include "io_reflection_handler.h"

#include "lib_begin.h"

namespace cgv {
	namespace data {

		/// different naming conventions for member names
		enum NamingConvention { NC_NONE, NC_SHORT, NC_LONG };
	}
}

namespace cgv {
	namespace reflect {
		extern CGV_API enum_reflection_traits<cgv::data::NamingConvention> get_reflection_traits(const cgv::data::NamingConvention&);
	}
}

namespace cgv {
	namespace data {


/** read from ascii file */
class CGV_API ascii_reflection_handler : public io_reflection_handler
{
public:
protected:
	unsigned nr_idents;

	NamingConvention naming_convention;
	unsigned tab_size;

	std::string extend_name(const std::string& name, bool assign = true);
public:
	///
	bool reflect_header();
	///
	ascii_reflection_handler(const std::string& _content, unsigned _ver, unsigned _tab = 3);
	/// 
	int reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, cgv::reflect::abst_reflection_traits* rt, unsigned grp_size);
	/// 
	void reflect_group_end(GroupKind group_kind);
};

/** read from ascii file */
class CGV_API ascii_read_reflection_handler : public ascii_reflection_handler
{
protected:
	std::ifstream file_is;
	std::istream& is;

	bool read_reflect_header(const std::string& _content, unsigned _ver);
public:
	ascii_read_reflection_handler(const std::string& file_name, const std::string& _content, unsigned _ver, NamingConvention _nc = NC_SHORT, unsigned _tab = 3);
    ///
	ascii_read_reflection_handler(std::istream& _is, const std::string& _content, unsigned _ver, NamingConvention _nc = NC_SHORT, unsigned _tab = 3);
	/// this should return true
	bool is_creative() const;
	///
	void close();
	int reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, cgv::reflect::abst_reflection_traits* rt, unsigned grp_size);
	///
	bool reflect_member_void(const std::string& member_name, 
							 void* member_ptr, cgv::reflect::abst_reflection_traits* rt);
};


/** read from ascii file */
class CGV_API ascii_write_reflection_handler : public ascii_reflection_handler
{
protected:
	std::ofstream file_os;
	std::ostream& os;
public:
	/// construct from file_name by opening file in ascii mode
	ascii_write_reflection_handler(const std::string& file_name, const std::string& _content, unsigned _ver, NamingConvention _nc = NC_SHORT, unsigned _tab = 3);
	/// construct from std::ostream
	ascii_write_reflection_handler(std::ostream& _os, const std::string& _content, unsigned _ver, NamingConvention _nc = NC_SHORT, unsigned _tab = 3);
	///
	bool failed() const;
	///
	void close();
	/// 
	int reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, cgv::reflect::abst_reflection_traits* rt, unsigned grp_size);
	///
	bool reflect_member_void(const std::string& member_name, 
							 void* member_ptr, cgv::reflect::abst_reflection_traits* rt);
};


	}
}

#include <cgv/config/lib_end.h>