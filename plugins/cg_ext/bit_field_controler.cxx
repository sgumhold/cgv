#include "bit_field_controler.h"
#include <cgv/utils/scan_enum.h>
#include <cgv/base/base_generator.h>

namespace cgv {
	namespace gui {

bit_field_controler::bit_field_controler(int& _value, const std::string& _bit_defs, cgv::base::base_ptr _b) : value(_value), bit_defs(_bit_defs), b(_b)
{
	std::vector<cgv::utils::token> toks;
	cgv::utils::parse_enum_declarations(bit_defs, toks, bit_values);
	for (unsigned i = 0; i < toks.size(); ++i) {
		bit_names.push_back(to_string(toks[i]));
//		std::cout << bit_names[i] << "=" << bit_values[i] << std::endl;
	}
}

bit_field_controler::~bit_field_controler()
{
}

void bit_field_controler::value_change_callback(cgv::gui::control<bool>&)
{
	if (b)
		b->on_set(&value);
}

/// overload to set single bit, where user_data specifies bit index
void bit_field_controler::set_value(const bool& new_value, void* user_data) 
{
	if (new_value)
		value |= bit_values[reinterpret_cast<size_t>(user_data)];
	else
		value &= ~bit_values[reinterpret_cast<size_t>(user_data)];
}

/// overload to get the value
const bool bit_field_controler::get_value(void* user_data) const 
{
	return (value & bit_values[reinterpret_cast<size_t>(user_data)]) != 0;
}

/// the default implementation compares ptr to &get_value().
bool bit_field_controler::controls(const void* ptr, void* user_data) const 
{
	return ptr == &value; 
}

/// 
void bit_field_controler::add_bit_controls(cgv::gui::provider* p, const std::string& gui_type, const std::string& options, const std::string& align)
{
	for (size_t i = 0; i < bit_names.size(); ++i) {
		connect(p->add_control(bit_names[i], this, gui_type, options, align, reinterpret_cast<void*>(i))->value_change, this, &bit_field_controler::value_change_callback);
	}
}

typedef cgv::data::ref_ptr<bit_field_controler> bit_field_controler_ptr;

struct bit_field_controler_gui_creator : public cgv::gui::gui_creator
{
	/// attempt to create a gui and return whether this was successful
	bool create(cgv::gui::provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool*)
	{
		if (gui_type != "bit_field_control")
			return false;
//		if (value_type != cgv::type::info::type_name<cgv::type::DummyEnum>::get_name())
//			return false;
		int& value = *reinterpret_cast<int*>(value_ptr);
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
		std::string bit_defs;
		if (!cgv::base::has_property(options, "enums", bit_defs)) {
			std::cerr << "WARNING: bit_field_controler needs definition of bit fields in option called 'enums'" << std::endl;
			return false;
		}
		std::string align = "\n";
		std::string _options;
		std::string _gui_type("check");
		int heading_size = 0;
		cgv::base::has_property(options, "heading_size", heading_size);
		cgv::base::has_property(options, "align", align);
		cgv::base::has_property(options, "options", _options);
		cgv::base::has_property(options, "gui_type", _gui_type);
		bit_field_controler_ptr bfp(new bit_field_controler(value, bit_defs, b));
		if (heading_size > 0)
			p->add_decorator(label, "heading", "level=3");
		bfp->add_bit_controls(p, _gui_type, _options, align);
		p->get_parent_group()->add_managed_objects(bfp);
		return true;
	}
};

#include "lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<bit_field_controler_gui_creator> bfc_gc_reg("bit_field_gui_creator");
	
	}
}