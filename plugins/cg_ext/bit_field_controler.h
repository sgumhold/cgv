#pragma once

#include <cgv/base/base.h>
#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

class bit_field_controler : public cgv::base::base, public cgv::gui::control_provider<bool>, public cgv::signal::tacker
{
protected:
	int& value;
	std::string bit_defs;
	std::vector<std::string> bit_names;
	std::vector<int> bit_values;
	cgv::base::base_ptr b;
	void value_change_callback(cgv::gui::control<bool>&);
public:
	bit_field_controler(int& _value, const std::string& _bit_defs, cgv::base::base_ptr _b = 0);
	~bit_field_controler();
	/// overload to set single bit, where user_data specifies bit index
	void set_value(const bool& new_value, void* user_data);
	/// overload to get the value
	const bool get_value(void* user_data) const;
	/// the default implementation compares ptr to &get_value().
	bool controls(const void* ptr, void* user_data) const;
	/// 
	void add_bit_controls(cgv::gui::provider* p, const std::string& gui_type, const std::string& options, const std::string& align);
};
	}
}