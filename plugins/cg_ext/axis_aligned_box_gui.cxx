#include <cgv/base/base_generator.h>
#include <cgv/gui/gui_creator.h>
#include <cgv/gui/provider.h>
#include <cgv/utils/scan.h>
#include <cgv/type/info/type_id.h>
#include <cgv/media/axis_aligned_box.h>
#include <limits>

using namespace cgv::gui;
using namespace cgv::signal;
using namespace cgv::media;

template <typename T, bool is_min>
struct ensure_order_functor : public cgv::gui::control<T>::value_change_signal_type::functor_type, public cgv::signal::tacker
{
	cgv::gui::provider* p;
	cgv::base::base_ptr b;
	T* my_ptr;
	T* other_ptr;
	ensure_order_functor(cgv::gui::provider* _p, cgv::base::base_ptr _b, T* _my_ptr, T* _other_ptr) : p(_p), b(_b), my_ptr(_my_ptr), other_ptr(_other_ptr) {}
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &(*b); p2 = my_ptr; }
	functor_base* clone() const                               { return new ensure_order_functor(*this); }
	void operator() (control<T>& c) const
	{ 
		// in case of minimum
		if (is_min) {
//			cgv::data::ref_ptr<control<T> > c = cp.up_cast<control<T> >();
			// if the control is known
//			if (c) {
				// add difference to maximum in order to keep interval width
				*other_ptr += *my_ptr - c.get_old_value();
				p->update_member(other_ptr);
				if (b) {
					b->on_set(my_ptr);
					b->on_set(other_ptr);
				}
/*			}
			else {
				// if no control available ensure that maximum is at least as large as minimum
				if (*my_ptr > *other_ptr) {
					*other_ptr = *my_ptr;
					if (b) {
						b->on_set(my_ptr);
						b->on_set(other_ptr);
					}
				}
				else
					if (b)
						b->on_set(my_ptr);
			}*/
		}
		// in case of maximum
		else {
			// just ensure that minimum is no larger than maximum
			if (*my_ptr < *other_ptr) {
				*other_ptr = *my_ptr;
				if (b) {
					b->on_set(my_ptr);
					b->on_set(other_ptr);
				}
			}
			else
				if (b)
					b->on_set(my_ptr);
		}
	}
};


struct axis_aligned_box_gui_creator : public cgv::gui::gui_creator
{
	static std::string name_prefix;

	axis_aligned_box_gui_creator()
	{
		// determine prefixes of fvec and vec template types
		if (name_prefix.empty()) {
			name_prefix = cgv::type::info::type_name<axis_aligned_box<float,2> >::get_name();
			name_prefix = name_prefix.substr(0, name_prefix.find_first_of('<'));
		}
	}
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool*)
	{
		std::string coordinate_type;
		cgv::type::info::TypeId type_id;
		unsigned char* crd_ptr;
		int dim;
		// first check against prefixes
		if (value_type.size() > name_prefix.size()+2 && (value_type.substr(0,name_prefix.size()) == name_prefix)) {
			// split off into coordinate type and dimension
			std::string::size_type p0 = name_prefix.size()+1;
			std::string::size_type p1 = value_type.find_first_of(',', p0);
			if (p1 == std::string::npos)
				return false;

			// determine coordinate type
			coordinate_type = value_type.substr(p0,p1-p0);
			type_id = cgv::type::info::get_type_id(coordinate_type);
			if (!cgv::type::info::is_number(type_id))
				return false;
			crd_ptr = (unsigned char*) value_ptr;

			// determine dimension
			std::string dimension = value_type.substr(p1+1,value_type.size()-2-p1);
			if (!cgv::utils::is_integer(dimension, dim))
				return false;
		}
		else
			return false;

		// reconstruct coordinate type name for which controls are registered
		coordinate_type = cgv::type::info::get_type_name(type_id);
		unsigned crd_type_size = cgv::type::info::get_type_size(type_id);

		// extract from options
		std::string child_gui_type;
		std::string child_options;
		std::string child_align;
		bool long_label;
		std::string main_label; // none,first,heading
		std::string components; 
		std::string proposed_child_options;

		if (!cgv::base::has_property(options, "main_label", main_label))
			main_label = "";
		if (!cgv::base::has_property(options, "components", components))
			components = "xyzw";
		if (!cgv::base::has_property(options, "long_label", long_label))
			long_label = false;

		if (!cgv::base::has_property(options, "gui_type", child_gui_type))
			child_gui_type = "value_slider";
		if (!cgv::base::has_property(options, "options", child_options))
			child_options = proposed_child_options;
		if (!cgv::base::has_property(options, "align", child_align))
			child_align = "\n";

		// construct gui
		if (main_label=="heading") {
			p->add_decorator(label, "heading", "level=3");
		}
		cgv::base::base_ptr b(dynamic_cast<cgv::base::base*>(p));
		for (int i=0; i<dim; ++i) {
			void* min_ptr = static_cast<void*>(crd_ptr + i*crd_type_size);
			void* max_ptr = static_cast<void*>(crd_ptr + (dim+i)*crd_type_size);
			std::string lab_prefix;
			std::string lab_suffix;
			if (main_label == "first") {
				if (i==0)
					lab_prefix = label;
			}
			else {
				if (long_label) 
					lab_prefix = label+"|";
				if (dim > (int)components.size())
					lab_suffix = cgv::utils::to_string(i);
				else
					lab_suffix = components[i];
			}

			cgv::gui::control_ptr cp_min = p->add_control_void(lab_prefix+"min_"+lab_suffix, min_ptr, 0, coordinate_type, child_gui_type, child_options, child_align, 0);
			cgv::gui::control_ptr cp_max = p->add_control_void(lab_prefix+"max_"+lab_suffix, max_ptr, 0, coordinate_type, child_gui_type, child_options, child_align, 0);
			if (cp_min && cp_max) {
				cgv::signal::functor_base* f_min = 0;
				cgv::signal::functor_base* f_max = 0;
				switch (type_id) {
				case cgv::type::info::TI_UINT32 : 
					f_min = new ensure_order_functor<cgv::type::uint32_type,true> (p, b, reinterpret_cast<cgv::type::uint32_type*>(min_ptr), reinterpret_cast<cgv::type::uint32_type*>(max_ptr));
					f_max = new ensure_order_functor<cgv::type::uint32_type,false>(p, b, reinterpret_cast<cgv::type::uint32_type*>(max_ptr), reinterpret_cast<cgv::type::uint32_type*>(min_ptr));
					break;
				case cgv::type::info::TI_INT32 : 
					f_min = new ensure_order_functor<cgv::type::int32_type,true> (p, b, reinterpret_cast<cgv::type::int32_type*>(min_ptr), reinterpret_cast<cgv::type::int32_type*>(max_ptr));
					f_max = new ensure_order_functor<cgv::type::int32_type,false>(p, b, reinterpret_cast<cgv::type::int32_type*>(max_ptr), reinterpret_cast<cgv::type::int32_type*>(min_ptr));
					break;
				case cgv::type::info::TI_FLT32 : 
					f_min = new ensure_order_functor<cgv::type::flt32_type,true> (p, b, reinterpret_cast<cgv::type::flt32_type*>(min_ptr), reinterpret_cast<cgv::type::flt32_type*>(max_ptr));
					f_max = new ensure_order_functor<cgv::type::flt32_type,false>(p, b, reinterpret_cast<cgv::type::flt32_type*>(max_ptr), reinterpret_cast<cgv::type::flt32_type*>(min_ptr));
					break;
				case cgv::type::info::TI_FLT64 : 
					f_min = new ensure_order_functor<double,true> (p, b, reinterpret_cast<double*>(min_ptr), reinterpret_cast<double*>(max_ptr));
					f_max = new ensure_order_functor<double,false>(p, b, reinterpret_cast<double*>(max_ptr), reinterpret_cast<double*>(min_ptr));
					break;
				default:
					break;
				}
				if (f_min && f_max) {
					cp_min->attach_to_value_change(f_min);
					cp_max->attach_to_value_change(f_max);
				}
			}
			else {
				p->add_view_void(lab_prefix+"min_"+lab_suffix, min_ptr, coordinate_type,  child_gui_type, child_options, child_align);
				p->add_view_void(lab_prefix+"max_"+lab_suffix, max_ptr, coordinate_type,  child_gui_type, child_options, child_align);
			}
		}
		return true;
	}
};

std::string axis_aligned_box_gui_creator::name_prefix;

cgv::gui::gui_creator_registration<axis_aligned_box_gui_creator> aab_gc_reg("axis_aligned_box_gui_creator");
