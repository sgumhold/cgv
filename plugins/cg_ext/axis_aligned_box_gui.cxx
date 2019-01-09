#include <cgv/base/base_generator.h>
#include <cgv/gui/gui_creator.h>
#include <cgv/gui/event.h>
#include <cgv/gui/provider.h>
#include <cgv/utils/scan.h>
#include <cgv/type/info/type_id.h>
#include <cgv/media/axis_aligned_box.h>
#include <limits>

using namespace cgv::gui;
using namespace cgv::signal;
using namespace cgv::media;

template <typename T>
struct ensure_order_functor : public cgv::gui::control<T>::value_change_signal_type::functor_type, public cgv::signal::tacker
{
	cgv::gui::provider* p;
	cgv::base::base_ptr b;
	bool is_min;
	int i, n;
	T* my_ptr;
	T* other_ptr;
	T  min_size;

	ensure_order_functor(cgv::gui::provider* _p, cgv::base::base_ptr _b, bool _is_min, int _i, int _n, double _min_size, void* _my_ptr, void* _other_ptr)
		: p(_p), b(_b), is_min(_is_min), i(_i), n(_n), min_size(T(_min_size)), my_ptr(reinterpret_cast<T*>(_my_ptr)), other_ptr(reinterpret_cast<T*>(_other_ptr)) 
	{
	}
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &(*b); p2 = my_ptr; }
	functor_base* clone() const                               { return new ensure_order_functor(*this); }
	void operator() (control<T>& c) const
	{
		if (ref_current_modifiers() & EM_CTRL) {
			if (is_min) {
				// if min not became larger
				if (*my_ptr <= c.get_old_value()) {
					T diff = c.get_old_value() - *my_ptr;
					T max_value = c.template get<T>("max");
					if (*other_ptr > max_value - diff)
						*other_ptr = max_value;
					else
						*other_ptr += diff;
					if (ref_current_modifiers() & EM_SHIFT) {
						for (int j = 0; j < n; ++j) {
							if (j == i)
								continue;
							T min_value = c.template get<T>("min");
							T* my_ptr_j = my_ptr - i + j;
							if (*my_ptr_j < min_value + diff)
								*my_ptr_j = min_value;
							else
								*my_ptr_j -= diff;
							T* other_ptr_j = other_ptr - i + j;
							if (*other_ptr_j > max_value - diff)
								*other_ptr_j = max_value;
							else
								*other_ptr_j += diff;
							p->update_member(my_ptr_j);
							p->update_member(other_ptr_j);
							if (b) {
								b->on_set(my_ptr_j);
								b->on_set(other_ptr_j);
							}
						}
					}
				}
				else {
					T diff = *my_ptr - c.get_old_value();
					if (*my_ptr > *other_ptr - diff - min_size) {
						T over_shoot = *my_ptr - (*other_ptr - diff - min_size);
						*my_ptr -= over_shoot / 2;
						*other_ptr -= diff - (over_shoot - over_shoot / 2);
					}
					else
						*other_ptr -= diff;
					if (ref_current_modifiers() & EM_SHIFT) {
						for (int j = 0; j < n; ++j) {
							if (j == i)
								continue;
							T* my_ptr_j = my_ptr - i + j;
							T* other_ptr_j = other_ptr - i + j;
							if (*my_ptr_j + diff > *other_ptr_j - diff - min_size) {
								T over_shoot = *my_ptr_j + diff - (*other_ptr_j - diff - min_size);
								*my_ptr_j += diff - over_shoot / 2;
								*other_ptr_j -= diff - (over_shoot - over_shoot / 2);
							}
							else {
								*my_ptr_j  += diff;
								*other_ptr_j -= diff;
							}
							p->update_member(my_ptr_j);
							p->update_member(other_ptr_j);
							if (b) {
								b->on_set(my_ptr_j);
								b->on_set(other_ptr_j);
							}
						}
					}
				}
			}
			else {
				// if max not became smaller
				if (*my_ptr >= c.get_old_value()) {
					T diff = *my_ptr - c.get_old_value();
					T min_value = c.template get<T>("min");
					if (*other_ptr < min_value + diff)
						*other_ptr = min_value;
					else
						*other_ptr -= diff;
					if (ref_current_modifiers() & EM_SHIFT) {
						for (int j = 0; j < n; ++j) {
							if (j == i)
								continue;
							T max_value = c.template get<T>("max");
							T* my_ptr_j = my_ptr - i + j;
							if (*my_ptr_j > max_value - diff)
								*my_ptr_j = max_value;
							else
								*my_ptr_j += diff;
							T* other_ptr_j = other_ptr - i + j;
							if (*other_ptr_j < min_value + diff)
								*other_ptr_j = min_value;
							else
								*other_ptr_j -= diff;
							p->update_member(my_ptr_j);
							p->update_member(other_ptr_j);
							if (b) {
								b->on_set(my_ptr_j);
								b->on_set(other_ptr_j);
							}
						}
					}
				}
				else {
					T diff = c.get_old_value() - *my_ptr;
					if (*my_ptr < *other_ptr + diff + min_size) {
						T over_shoot = (*other_ptr + diff + min_size) - *my_ptr;
						*my_ptr += over_shoot / 2;
						*other_ptr += diff - (over_shoot - over_shoot / 2);
					}
					else
						*other_ptr += diff;
					if (ref_current_modifiers() & EM_SHIFT) {
						for (int j = 0; j < n; ++j) {
							if (j == i)
								continue;
							T* my_ptr_j = my_ptr - i + j;
							T* other_ptr_j = other_ptr - i + j;
							if (*my_ptr_j - min_size - diff < *other_ptr_j + diff) {
								T over_shoot = *other_ptr_j + 2 * diff + min_size - *my_ptr_j;
								*my_ptr_j -= diff - over_shoot / 2;
								*other_ptr_j += diff - (over_shoot - over_shoot / 2);
							}
							else {
								*my_ptr_j -= diff;
								*other_ptr_j += diff;
							}
							p->update_member(my_ptr_j);
							p->update_member(other_ptr_j);
							if (b) {
								b->on_set(my_ptr_j);
								b->on_set(other_ptr_j);
							}
						}
					}
				}

			}
			p->update_member(other_ptr);
			if (b) {
				b->on_set(my_ptr);
				b->on_set(other_ptr);
			}
		}
		else if (ref_current_modifiers() & EM_SHIFT) {
			//std::cout << c.get_old_value() << "->" << *my_ptr << " vs " << *other_ptr << std::endl;
			if (is_min) {
				// if min not became larger
				if (*my_ptr <= c.get_old_value()) {
					// just drag max by same amount
					*other_ptr -= c.get_old_value() - *my_ptr;
					p->update_member(other_ptr);
				}
				// if min became bigger
				else {
					T size = *other_ptr - c.get_old_value();
					T max_value = c.template get<T>("max");
					T max_min_value = max_value - size;
					if (*my_ptr > max_min_value) {
						*other_ptr = max_value;
						*my_ptr = max_min_value;
						p->update_member(other_ptr);
					}
					else {
						*other_ptr = *my_ptr + size;
						p->update_member(other_ptr);
					}
				}
			}
			else {
				// if max not became smaller
				if (*my_ptr >= c.get_old_value()) {
					// just drag min by same amount
					*other_ptr += *my_ptr - c.get_old_value();
					p->update_member(other_ptr);
				}
				// if max became smaller
				else {
					T size = c.get_old_value() - *other_ptr;
					T min_value = c.template get<T>("min");
					T min_max_value = min_value + size;
					if (*my_ptr < min_max_value) {
						*other_ptr = min_value;
						*my_ptr = min_max_value;
						p->update_member(other_ptr);
					}
					else {
						*other_ptr = *my_ptr - size;
						p->update_member(other_ptr);
					}
				}
			}
			if (b) {
				b->on_set(my_ptr);
				b->on_set(other_ptr);
			}
		}
		else {
			// in case of minimum
			if (is_min) {
				// if min became larger
				if (*my_ptr > c.get_old_value()) {
					if (*my_ptr + min_size > *other_ptr)
						*my_ptr = *other_ptr - min_size;
				}
			}
			else {
				if (*my_ptr < c.get_old_value()) {
					if (*other_ptr + min_size > *my_ptr)
						*my_ptr = *other_ptr + min_size;
				}
			}
			if (b)
				b->on_set(my_ptr);
		}
	}
};


struct axis_aligned_box_gui_creator : public cgv::gui::gui_creator
{
	static std::string name_prefix;

	bool is_axis_aligned_box(const std::string value_type, cgv::type::info::TypeId& type_id, int& dim) const
	{
#if _MSC_VER >= 1400
		// ensure that value type is axis_aligned_box<T,dim>
		if (value_type.size() <= name_prefix.size() + 2 || (value_type.substr(0, name_prefix.size()) != name_prefix))
			return false;

		// find and ensure location of coordinate type and dimension
		std::string::size_type p0 = name_prefix.size() + 1;
		std::string::size_type p1 = value_type.find_first_of(',', p0);
		if (p1 == std::string::npos)
			return false;

		// extract coordinate type and ensure that it is a number type from type id
		std::string coordinate_type = value_type.substr(p0, p1 - p0);
		type_id = cgv::type::info::get_type_id(coordinate_type);
		if (!cgv::type::info::is_number(type_id))
			return false;

		// determine dimension and ensure that it is an integer type
		std::string dimension = value_type.substr(p1 + 1, value_type.size() - 2 - p1);
		if (!cgv::utils::is_integer(dimension, dim))
			return false;
		return true;
#else
		type_id = cgv::type::info::type_id<float>::get_id();
		if (value_type == cgv::type::info::type_name<axis_aligned_box<float, 2> >::get_name()) {
			dim = 2;
			return true;
		}
		if (value_type == cgv::type::info::type_name<axis_aligned_box<float, 3> >::get_name()) {
			dim = 3;
			return true;
		}
		if (value_type == cgv::type::info::type_name<axis_aligned_box<float, 4> >::get_name()) {
			dim = 4;
			return true;
		}
		type_id = cgv::type::info::type_id<double>::get_id();
		if (value_type == cgv::type::info::type_name<axis_aligned_box<double, 2> >::get_name()) {
			dim = 2;
			return true;
		}
		if (value_type == cgv::type::info::type_name<axis_aligned_box<double, 3> >::get_name()) {
			dim = 3;
			return true;
		}
		if (value_type == cgv::type::info::type_name<axis_aligned_box<double, 4> >::get_name()) {
			dim = 4;
			return true;
		}
		type_id = cgv::type::info::TI_INT32;
		if (value_type == cgv::type::info::type_name<axis_aligned_box<cgv::type::int32_type, 2> >::get_name()) {
			dim = 2;
			return true;
		}
		if (value_type == cgv::type::info::type_name<axis_aligned_box<cgv::type::int32_type, 3> >::get_name()) {
			dim = 3;
			return true;
		}
		if (value_type == cgv::type::info::type_name<axis_aligned_box<cgv::type::int32_type, 4> >::get_name()) {
			dim = 4;
			return true;
		}
		type_id = cgv::type::info::TI_UINT32;
		if (value_type == cgv::type::info::type_name<axis_aligned_box<cgv::type::uint32_type, 2> >::get_name()) {
			dim = 2;
			return true;
		}
		if (value_type == cgv::type::info::type_name<axis_aligned_box<cgv::type::uint32_type, 3> >::get_name()) {
			dim = 3;
			return true;
		}
		if (value_type == cgv::type::info::type_name<axis_aligned_box<cgv::type::uint32_type, 4> >::get_name()) {
			dim = 4;
			return true;
		}
		return false;
#endif
	}
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
		cgv::type::info::TypeId type_id;
		int dim;
		if (!is_axis_aligned_box(value_type, type_id, dim))
			return false;

		// reconstruct coordinate type name for which controls are registered
		std::string coordinate_type = cgv::type::info::get_type_name(type_id);
		unsigned char* crd_ptr = (unsigned char*)value_ptr;
		unsigned crd_type_size = cgv::type::info::get_type_size(type_id);

		// extract from options
		std::string main_label;                     cgv::base::has_property(options, "main_label", main_label); // none,first,heading
		bool order_by_coords = false;               cgv::base::has_property(options, "order_by_coords", order_by_coords);
		std::string components("xyzw");             cgv::base::has_property(options, "components", components);
		bool long_label = false;                    cgv::base::has_property(options, "long_label", long_label);
		std::string child_gui_type("value_slider"); cgv::base::has_property(options, "gui_type", child_gui_type);
		std::string child_options;                  cgv::base::has_property(options, "options", child_options);
		double min_size = 0.0;                      cgv::base::has_property(options, "min_size", min_size);
		std::string child_align_row, child_align_col, child_align_end;
		if (cgv::base::has_property(options, "align", child_align_row))
			child_align_end = child_align_col = child_align_row;
		else
			child_align_end = child_align_col = child_align_row = "\n";
		if (cgv::base::has_property(options, "align_row", child_align_row))
			child_align_end = child_align_row;
		cgv::base::has_property(options, "align_col", child_align_col);
		cgv::base::has_property(options, "align_end", child_align_end);

		// construct gui
		if (main_label=="heading")
			p->add_decorator(label, "heading", "level=3");
		
		cgv::base::base_ptr b(dynamic_cast<cgv::base::base*>(p));
		int nr_cols = order_by_coords ? dim : 2;
		for (int k=0; k<2*dim; ++k) {
			int  i = order_by_coords ? k % 3 : k/2;
			bool is_min = order_by_coords ? (k/3 == 0) : ((k&1) == 0);

			void* my_ptr    = static_cast<void*>(crd_ptr + i*crd_type_size);
			void* other_ptr = static_cast<void*>(crd_ptr + (dim+i)*crd_type_size);

			if (!is_min)
				std::swap(my_ptr, other_ptr);
			const std::string& child_align = k == (2*dim-1) ? child_align_end : (k%nr_cols+1 < nr_cols ? child_align_col : child_align_row);

			std::string lab_suffix(is_min ? "min." : "max.");
			if (dim > (int)components.size())
				lab_suffix += cgv::utils::to_string(i);
			else
				lab_suffix += components[i];

			std::string lab_prefix;
			if (long_label || (main_label == "first" && (k%nr_cols == 0)))
				lab_prefix = label+ (label.empty()?"":".") + lab_prefix;
			if (!long_label && (main_label == "first") && (k%nr_cols != 0))
				lab_prefix = "";
			std::string ctrl_label = lab_prefix + lab_suffix;

			cgv::gui::control_ptr cp = p->add_control_void(ctrl_label, my_ptr, 0, coordinate_type, child_gui_type, child_options, child_align, 0);
			if (cp) {
				cgv::signal::functor_base* f = 0;
				switch (type_id) {
				case cgv::type::info::TI_UINT32: f = new ensure_order_functor<cgv::type::uint32_type>(p, b, is_min, i, dim, min_size, my_ptr, other_ptr); break;
				case cgv::type::info::TI_INT32 : f = new ensure_order_functor<cgv::type::int32_type> (p, b, is_min, i, dim, min_size, my_ptr, other_ptr); break;
				case cgv::type::info::TI_FLT32 : f = new ensure_order_functor<cgv::type::flt32_type> (p, b, is_min, i, dim, min_size, my_ptr, other_ptr); break;
				case cgv::type::info::TI_FLT64 : f = new ensure_order_functor<cgv::type::flt64_type> (p, b, is_min, i, dim, min_size, my_ptr, other_ptr); break;
				default:
					break;
				}
				if (f)
					cp->attach_to_value_change(f);
			}
			else
				p->add_view_void(ctrl_label, my_ptr, coordinate_type,  child_gui_type, child_options, child_align);
		}
		return true;
	}
};

std::string axis_aligned_box_gui_creator::name_prefix;

cgv::gui::gui_creator_registration<axis_aligned_box_gui_creator> aab_gc_reg("axis_aligned_box_gui_creator");
