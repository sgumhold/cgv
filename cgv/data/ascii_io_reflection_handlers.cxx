#include "ascii_io_reflection_handlers.h"
#include <cgv/utils/scan.h>

using namespace cgv::reflect;
namespace cgv {
	namespace reflect {

		enum_reflection_traits<cgv::data::NamingConvention> get_reflection_traits(const cgv::data::NamingConvention&) {
			return cgv::reflect::enum_reflection_traits<cgv::data::NamingConvention>("NC_NONE, NC_SHORT, NC_LONG");
		}
	}
}

namespace cgv {
	namespace data {


std::string ascii_reflection_handler::extend_name(const std::string& name, bool assign)
{
	NamingConvention nc = naming_convention;
	if (in_header)
		nc = NC_SHORT;
	if (nc == NC_NONE)
		return "";

	std::string res;
	for (unsigned i=0; i<nesting_info_stack.size(); ++i) {
		res += *nesting_info_stack[i].name;
		switch (nesting_info_stack[i].group_kind) {
		case GK_BASE_CLASS :
		case GK_STRUCTURE  : 
			res += "."; 
			break;
		case GK_VECTOR :
		case GK_ARRAY :
			res += "[";
			res += cgv::utils::to_string(nesting_info_stack[i].idx) + "]";
			break;
		}
	}
	res += name;

	size_t p = res.find_last_of(".");
	if (p != std::string::npos) {
		res = res.substr(p+1);
		for (size_t j=0; j<nr_idents*tab_size; ++j)
			res = std::string(" ") + res;
	}
	if (assign)
		res += "=";
	return res;
}

/// this should return true
bool ascii_read_reflection_handler::is_creative() const
{
	return true;
}

bool ascii_reflection_handler::reflect_header()
{
	in_header = true;
	bool res = reflect_member("version", version) &&
			reflect_member("content", file_content) &&
			reflect_member("tab_size", tab_size) &&
			reflect_member("naming_convention", naming_convention);
	in_header = false;
	return res;
}

ascii_reflection_handler::ascii_reflection_handler(const std::string& _content, unsigned _ver, unsigned _tab) : io_reflection_handler(_content, _ver)
{
	nr_idents = 0;
	tab_size = _tab;
}

/// 
int ascii_reflection_handler::reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, abst_reflection_traits* rt, unsigned grp_size)
{
	++nr_idents;
	return GT_COMPLETE;
}
/// 
void ascii_reflection_handler::reflect_group_end(GroupKind group_kind)
{
	if (group_kind != reflection_handler::GK_BASE_CLASS)
		--nr_idents;
}

bool ascii_read_reflection_handler::read_reflect_header(const std::string& _content, unsigned _ver)
{
	reflect_header();
	if (failed())
		return false;
	if (_content != file_content)
		last_error = RE_CONTENT_MISMATCH;
	else if (_ver != version)
		last_error = RE_VERSION_MISMATCH;
	else return true;
	return false;
}

ascii_read_reflection_handler::ascii_read_reflection_handler(const std::string& file_name, const std::string& _content, unsigned _ver, NamingConvention _nc, unsigned _tab) :
	ascii_reflection_handler(_content, _ver, _tab), is(file_is)
{
#if defined (WIN32) && !defined(__MINGW32__)
	file_is.open(cgv::utils::str2wstr(file_name).c_str());
#else
	file_is.open(file_name.c_str());
#endif
	if (file_is.fail())
		last_error = RE_FILE_OPEN_ERROR;
	else
		read_reflect_header(_content, _ver);
}

///
ascii_read_reflection_handler::ascii_read_reflection_handler(std::istream& _is, const std::string& _content, unsigned _ver, NamingConvention _nc, unsigned _tab) :
	ascii_reflection_handler(_content, _ver, _tab), is(_is)
{
	read_reflect_header(_content, _ver);
}
///
void ascii_read_reflection_handler::close() 
{
	if (&is == &file_is)
		file_is.close(); 
}

int ascii_read_reflection_handler::reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, abst_reflection_traits* rt, unsigned grp_size)
{
	if (group_kind != reflection_handler::GK_BASE_CLASS) {
		char buffer[10000];
		is.getline(buffer, 10000);
		if (is.fail()) {
			last_error = RE_FILE_READ_ERROR;
			return false;
		}
		++nr_idents;
	}
	return GT_COMPLETE;
}
///
bool ascii_read_reflection_handler::reflect_member_void(const std::string& member_name, 
							void* member_ptr, abst_reflection_traits* rt)
{
	char buffer[10000];
	is.getline(buffer, 10000);
	if (is.fail()) {
		last_error = RE_FILE_READ_ERROR;
		return false;
	}
	if (rt->get_type_id() == cgv::type::info::TI_STRING) {
		std::string val_str(buffer + extend_name(member_name).size()+1);
		*((std::string*)member_ptr) = cgv::utils::interpret_special(val_str.substr(0, val_str.size()-1));
	}
	else if (rt->has_string_conversions()) {
		std::string val_str(buffer + extend_name(member_name).size());
		rt->set_from_string(member_ptr, val_str);
	}
	return true;
}

ascii_write_reflection_handler::ascii_write_reflection_handler(const std::string& file_name, const std::string& _content, unsigned _ver, NamingConvention _nc, unsigned _tab) :
	ascii_reflection_handler(_content, _ver, _tab), os(file_os)
{
	naming_convention = _nc;
#if defined(WIN32) && !defined(__MINGW32__)
	file_os.open(cgv::utils::str2wstr(file_name).c_str());
#else
	file_os.open(file_name.c_str());
#endif
	if (file_os.fail())
		last_error = RE_FILE_OPEN_ERROR;
	else
		reflect_header();
}
ascii_write_reflection_handler::ascii_write_reflection_handler(std::ostream& _os, const std::string& _content, unsigned _ver, NamingConvention _nc, unsigned _tab) :
	ascii_reflection_handler(_content, _ver, _tab), os(_os)
{
	naming_convention = _nc;
	reflect_header();
}
///
bool ascii_write_reflection_handler::failed() const { return os.fail(); }
///
void ascii_write_reflection_handler::close()
{
	if(&os == &file_os)
		file_os.close(); 
}
/// 
int ascii_write_reflection_handler::reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, abst_reflection_traits* rt, unsigned grp_size)
{
	if (group_kind != reflection_handler::GK_BASE_CLASS) {
		os << extend_name(group_name, false) << std::endl;
		if (os.fail()) {
			last_error = RE_FILE_WRITE_ERROR;
			return GT_TERMINATE;
		}
		++nr_idents;
	}
	return GT_COMPLETE;
}
///
bool ascii_write_reflection_handler::reflect_member_void(const std::string& member_name, 
							void* member_ptr, abst_reflection_traits* rt)
{
	os << extend_name(member_name);
	if (rt->get_type_id() == cgv::type::info::TI_STRING)
		os << '"' << cgv::utils::escape_special(*((const std::string*)member_ptr)) << '"';
	else if (rt->has_string_conversions()) {
		std::string s;
		rt->get_to_string(member_ptr, s);
		os << s;
	}
	os << std::endl;
	if (os.fail()) {
		last_error = RE_FILE_WRITE_ERROR;
		return false;
	}
	return true;
}

	}
}

