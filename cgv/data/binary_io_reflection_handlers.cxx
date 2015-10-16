#include "binary_io_reflection_handlers.h"

using namespace cgv::reflect;

namespace cgv {
	namespace data {


bool binary_reflection_handler::reflect_header()
{
	in_header = true;
	bool res = reflect_member("version", version) &&
			   reflect_member("content", file_content);
	in_header = false;
	return res;
}

/// this should return true
bool binary_read_reflection_handler::is_creative() const
{
	return true;
}

binary_reflection_handler::binary_reflection_handler(const std::string& _content, unsigned _ver) : io_reflection_handler(_content, _ver)
{
}

///
void binary_reflection_handler::close() 
{
	fclose(fp);
}

bool binary_read_reflection_handler::read_reflect_header(const std::string& _content, unsigned _ver)
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

binary_read_reflection_handler::binary_read_reflection_handler(const std::string& file_name, const std::string& _content, unsigned _ver) :
	binary_reflection_handler(_content, _ver)
{
	fp = fopen(file_name.c_str(), "rb");
	if (!fp)
		last_error = RE_FILE_OPEN_ERROR;
	else
		read_reflect_header(_content, _ver);
}

///
binary_read_reflection_handler::binary_read_reflection_handler(FILE* _fp, const std::string& _content, unsigned _ver) :
	binary_reflection_handler(_content, _ver)
{
	fp = _fp;
	read_reflect_header(_content, _ver);
}
///
bool binary_read_reflection_handler::reflect_member_void(const std::string& member_name, 
							void* member_ptr, abst_reflection_traits* rt)
{
	switch (rt->get_type_id()) {
	case cgv::type::info::TI_STRING :
		{
			std::string& str = *((std::string*)member_ptr);
			cgv::type::uint32_type s;
			if (fread(&s, sizeof(cgv::type::uint32_type), 1, fp) != 1) {
				last_error = RE_FILE_READ_ERROR;
				return false;
			}
			str.resize(s);
			if (fread(&str[0], sizeof(char), s, fp) != s) {
				last_error = RE_FILE_READ_ERROR;
				return false;
			}
			break;
		}
	case cgv::type::info::TI_WSTRING :
		{
			std::wstring& str = *((std::wstring*)member_ptr);
			cgv::type::uint32_type s;
			if (fread(&s, sizeof(cgv::type::uint32_type), 1, fp) != 1) {
				last_error = RE_FILE_READ_ERROR;
				return false;
			}
			str.resize(s);
			if (fread(&str[0], sizeof(cgv::type::wchar_type), s, fp) != s) {
				last_error = RE_FILE_READ_ERROR;
				return false;
			}
			break;
		}
	default:
		if (fread(member_ptr, rt->size(), 1, fp) != 1) {
			last_error = RE_FILE_READ_ERROR;
			return false;
		}
		break;
	}
	return true;
}

binary_write_reflection_handler::binary_write_reflection_handler(const std::string& file_name, const std::string& _content, unsigned _ver) :
	binary_reflection_handler(_content, _ver)
{
	fp = fopen(file_name.c_str(), "wb");
	if (!fp)
		last_error = RE_FILE_OPEN_ERROR;
	else
		reflect_header();
}

binary_write_reflection_handler::binary_write_reflection_handler(FILE* _fp, const std::string& _content, unsigned _ver) :
	binary_reflection_handler(_content, _ver)
{
	fp = _fp;
	reflect_header();
}

///
bool binary_write_reflection_handler::reflect_member_void(const std::string& member_name, 
							void* member_ptr, abst_reflection_traits* rt)
{
	switch (rt->get_type_id()) {
	case cgv::type::info::TI_STRING :
		{
			const std::string& str = *((std::string*)member_ptr);
			cgv::type::uint32_type s = (cgv::type::uint32_type)str.size();
			if (fwrite(&s, sizeof(cgv::type::uint32_type), 1, fp) != 1) {
				last_error = RE_FILE_WRITE_ERROR;
				return false;
			}
			if (fwrite(&str[0], sizeof(char), s, fp) != s) {
				last_error = RE_FILE_WRITE_ERROR;
				return false;
			}
			break;
		}
	case cgv::type::info::TI_WSTRING :
		{
			const std::wstring& str = *((std::wstring*)member_ptr);
			cgv::type::uint32_type s = (cgv::type::uint32_type)str.size();
			if (fwrite(&s, sizeof(cgv::type::uint32_type), 1, fp) != 1) {
				last_error = RE_FILE_WRITE_ERROR;
				return false;
			}
			if (fwrite(&str[0], sizeof(cgv::type::wchar_type), s, fp) != s) {
				last_error = RE_FILE_WRITE_ERROR;
				return false;
			}
			break;
		}
	default:
		if (fwrite(member_ptr, rt->size(), 1, fp) != 1) {
			last_error = RE_FILE_WRITE_ERROR;
			return false;
		}
		break;
	}
	return true;
}

	}
}

