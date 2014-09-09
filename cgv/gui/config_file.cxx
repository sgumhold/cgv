#include "view.h"
#include "gui_driver.h"

#include <cgv/utils/scan.h>
#include <cgv/utils/file.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/base/register.h>
#include <cgv/reflect/find_reflection_handler.h>
#include <cgv/reflect/reflect_enum.h>
#include <algorithm>

using namespace cgv::base;
using namespace cgv::reflect;
using namespace cgv::utils;

struct config_file_view
{
	bool out_of_date;
	unsigned pos, len;
	cgv::gui::view_ptr view;
	bool operator < (const config_file_view& cfv) const { return pos < cfv.pos; }
};

class gui_config_file_observer : public cgv::base::config_file_observer
{
protected:
	std::string file_name;
	std::string content;
	std::vector<config_file_view> views;
	unsigned nr_out_of_date;
public:
	gui_config_file_observer(const std::string& fn, const std::string& c) : file_name(fn), content(c)
	{
		nr_out_of_date = 0;
	}
	void add_view(const config_file_view& cfv)
	{
		views.push_back(cfv);
		std::sort(views.begin(), views.end());
	}
	void inc_out_of_date_count() { 
		++nr_out_of_date;
		update_config_file();
	}
	void update_view(cgv::gui::view_ptr v)
	{
		for (unsigned i=0; i<views.size(); ++i)
			if (views[i].view == v) {
				if (!views[i].out_of_date) {
					views[i].out_of_date = true;
					inc_out_of_date_count();
				}
			}
	}
	void update_config_file()
	{
		std::string new_content;
		unsigned off = 0;
		int delta_pos = 0;
		for (unsigned i=0; i<views.size(); ++i) {
			std::string new_value((const char*)views[i].view->get_user_data());
			unsigned new_len = new_value.length();
			new_content += content.substr(off, views[i].pos - off);
			new_content += new_value;
			off = views[i].pos+views[i].len;
			views[i].pos += delta_pos;
			delta_pos += (int)new_len - (int)(views[i].len);
			views[i].len = new_len;
			views[i].out_of_date = false;
		}
		new_content += content.substr(off);
		cgv::utils::file::write(file_name, new_content.c_str(), new_content.size(), true);
		content = new_content;
	}
	void multi_observe(base_ptr bp, const std::string& property_assignments, unsigned off);
};

template <typename T>
class config_view : public cgv::gui::view<T>
{
	gui_config_file_observer* observer;
public:
	config_view(const std::string& _name, const T& _value, gui_config_file_observer* _obs) : cgv::gui::view<T>(_name, _value), observer(_obs) {}
	void update()
	{
		observer->update_view(cgv::gui::view_ptr(this));
	}
	void* get_user_data() const
	{
		static std::string last_value;
		last_value = to_string(this->get_value());
		return const_cast<char*>(last_value.c_str());
	}
};

template<> class config_view<std::string> : public cgv::gui::view<std::string>
{
	gui_config_file_observer* observer;
public:
	config_view(const std::string& _name, const std::string& _value, gui_config_file_observer* _obs) : view<std::string>(_name, _value), observer(_obs) {}
	void update()
	{
		observer->update_view(cgv::gui::view_ptr(this));
	}
	void* get_user_data() const
	{
		static std::string last_value;
		last_value = get_value();
		for (unsigned i=0; i<last_value.size(); ++i)
			if (last_value[i] == '"' || last_value[i] == '\\') {
				last_value.insert(i,1,'\\');
				++i;
			}
		last_value = std::string("\"")+last_value+"\"";
		return const_cast<char*>(last_value.c_str());
	}
};

template<> class config_view<bool> : public cgv::gui::view<bool>
{
	gui_config_file_observer* observer;
public:
	config_view(const std::string& _name, const bool& _value, gui_config_file_observer* _obs) : view<bool>(_name, _value), observer(_obs) {}
	void update()
	{
		observer->update_view(cgv::gui::view_ptr(this));
	}
	void* get_user_data() const
	{
		static std::string last_value;
		last_value = get_value() ? "true" : "false";
		return const_cast<char*>(last_value.c_str());
	}
};

class enum_config_view : public cgv::gui::view<cgv::type::int32_type>
{
	abst_enum_reflection_traits* enum_traits;
	gui_config_file_observer* observer;
public:
	enum_config_view(const std::string& _name, void *member_ptr, cgv::reflect::abst_enum_reflection_traits* aert, gui_config_file_observer* _obs) : view<cgv::type::int32_type>(_name, *((cgv::type::int32_type*)member_ptr)), enum_traits(aert), observer(_obs) {}
	void update()
	{
		observer->update_view(cgv::gui::view_ptr(this));
	}
	void* get_user_data() const
	{
		static std::string last_value;
		last_value = enum_traits->get_enum_name(get_value());
		return const_cast<char*>(last_value.c_str());
	}
};

void gui_config_file_observer::multi_observe(base_ptr bp, const std::string& property_assignments, unsigned off)
{
	// split into single assignments
	std::vector<token> toks;
	bite_all(tokenizer(property_assignments).set_skip("'\"","'\"","\\\\").set_ws(";"),toks);
	// for each assignment
	for (unsigned int i=0; i<toks.size(); ++i) {
		std::vector<token> sides;
		bite_all(tokenizer(toks[i]).set_skip("'\"","'\"","\\\\").set_ws("="),sides);
		if (sides.size() != 2) {
			std::cerr << "property assignment >" << to_string(toks[i]).c_str() << "< does not match pattern lhs=rhs" << std::endl;
			continue;
		}
		std::string lhs(to_string(sides[0]));
		find_reflection_handler rsrh(lhs);
		bp->self_reflect(rsrh);
		config_file_view cfv;
		void* member_ptr = 0;
		if (rsrh.found_target()) {
			member_ptr = rsrh.get_member_ptr();
			cfv.len = sides[1].get_length();
			cfv.pos = off+sides[1].begin-&property_assignments[0];
			cfv.out_of_date = false;
			switch (rsrh.get_reflection_traits()->get_type_id()) {
			case cgv::type::info::TI_BOOL: cfv.view = new config_view<bool>(lhs,*((bool*)member_ptr),this); break;
			case cgv::type::info::TI_INT8 : cfv.view = new config_view<cgv::type::int8_type>(lhs,*((cgv::type::int8_type*)member_ptr),this); break;
			case cgv::type::info::TI_UINT8 : cfv.view = new config_view<cgv::type::uint8_type>(lhs,*((cgv::type::uint8_type*)member_ptr),this); break;
			case cgv::type::info::TI_INT16 : cfv.view = new config_view<cgv::type::int16_type>(lhs,*((cgv::type::int16_type*)member_ptr),this); break;
			case cgv::type::info::TI_UINT16 : cfv.view = new config_view<cgv::type::uint16_type>(lhs,*((cgv::type::uint16_type*)member_ptr),this); break;
			case cgv::type::info::TI_INT32 : cfv.view = new config_view<cgv::type::int32_type>(lhs,*((cgv::type::int32_type*)member_ptr),this); break;
			case cgv::type::info::TI_UINT32 : cfv.view = new config_view<cgv::type::uint32_type>(lhs,*((cgv::type::uint32_type*)member_ptr),this); break;
			case cgv::type::info::TI_INT64 : cfv.view = new config_view<cgv::type::int64_type>(lhs,*((cgv::type::int64_type*)member_ptr),this); break;
			case cgv::type::info::TI_UINT64 : cfv.view = new config_view<cgv::type::uint64_type>(lhs,*((cgv::type::uint64_type*)member_ptr),this); break;
			case cgv::type::info::TI_FLT32 : cfv.view = new config_view<cgv::type::flt32_type>(lhs,*((cgv::type::flt32_type*)member_ptr),this); break;
			case cgv::type::info::TI_FLT64 : cfv.view = new config_view<cgv::type::flt64_type>(lhs,*((cgv::type::flt64_type*)member_ptr),this); break;
			case cgv::type::info::TI_STRING : cfv.view = new config_view<std::string>(lhs,*((std::string*)member_ptr),this); break;
			default:
				{
					cgv::reflect::abst_enum_reflection_traits* aert = dynamic_cast<cgv::reflect::abst_enum_reflection_traits*>(rsrh.get_reflection_traits());
					if (aert) {
						cfv.view = new enum_config_view(lhs, member_ptr, dynamic_cast<cgv::reflect::abst_enum_reflection_traits*>(aert->clone()), this);
					}
					else {
						std::cerr << "could not create config view for permanent attribute " << lhs << std::endl;
						exit(0);
					}
					break;
				}
			}
			cfv.view->attach_to_reference(member_ptr);
			this->add_view(cfv);
		}
		else
			std::cout << "did not find reference to member " << lhs << std::endl;

		std::string rhs(to_string(sides[1]));
		if (rhs == "?") {
			if (member_ptr) {
				cfv.out_of_date = true;
				this->inc_out_of_date_count();
			}
		}
		else if (rhs[0] == '"' || rhs[0] == '\'') {
			unsigned int n = (unsigned int) (rhs.size()-1);
			char open = rhs[0];
			if (rhs[n] == rhs[0])
				--n;
			rhs = rhs.substr(1, n);
			for (unsigned i=1; i<rhs.size(); ++i) {
				if (rhs[i-1] == '\\' && (rhs[i] == '\\' || rhs[i] == open))
					rhs.erase(i-1,1);
			}
			bp->set_void(lhs,"string",&rhs);
		}
		else if (rhs == "true" || rhs == "false") {
			bool value = rhs == "true";
			bp->set_void(lhs, "bool", &value);
		}
		else if (is_digit(rhs[0]) || rhs[0] == '.' || rhs[0] == '+' || rhs[0] == '-') {
			int int_value;
			if (is_integer(rhs,int_value)) {
				bp->set_void(lhs, "int32", &int_value);
			}
			else {
				double value = atof(rhs.c_str());
				bp->set_void(lhs, "flt64", &value);
			}
		}
		else
			bp->set_void(lhs,"string",&rhs);
	}
}


class gui_config_file_driver : public cgv::base::config_file_driver
{
protected:
	static std::map<std::string,config_file_observer*>& ref_config_file_observer_map()
	{
		static std::map<std::string,config_file_observer*> cfom;
		return cfom;
	}
public:
	config_file_observer* find_config_file_observer(const std::string& file_name, const std::string& content)
	{
		config_file_observer* cfo = 0;
		if (ref_config_file_observer_map().find(file_name) == ref_config_file_observer_map().end()) {
			cfo = new gui_config_file_observer(file_name, content);
			ref_config_file_observer_map()[file_name] = cfo;
		}
		else
			cfo = ref_config_file_observer_map()[file_name];
		return cfo;
	}
	/// process a gui file
	bool process_gui_file(const std::string& file_name)
	{
		cgv::gui::gui_driver_ptr d = cgv::gui::get_gui_driver();
		if (d)
			return d->process_gui_file(file_name);
		return false;
	}
};

struct cfg_reg_type
{
	cfg_reg_type(const char* dummy)
	{
		register_config_file_driver(new gui_config_file_driver());
	}
};

#include "lib_begin.h"

extern CGV_API cfg_reg_type cfg_reg;

cfg_reg_type cfg_reg("");