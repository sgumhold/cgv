#include <cgv/base/base_generator.h>
#include <cgv/gui/gui_creator.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event.h>
#include <cgv/utils/scan.h>
#include <cgv/type/info/type_id.h>
#include <cgv/math/fvec.h>
#include <cgv/math/quaternion.h>
#include <cgv/math/vec.h>
#include <limits>

using namespace cgv::gui;
using namespace cgv::signal;
using namespace cgv::math;

struct call_on_set_functor : public cgv::gui::control<int>::value_change_signal_type::functor_type, public cgv::signal::tacker
{
	cgv::base::base_ptr b;
	unsigned dim;
	unsigned i;
	size_t element_size;
	void* member_ptr;
	call_on_set_functor(cgv::base::base_ptr _b, void* _member_ptr, unsigned _dim, unsigned _i, size_t _element_size) 
		: b(_b), member_ptr(_member_ptr), dim(_dim), i(_i), element_size(_element_size) {}
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &(*b); p2 = member_ptr; }
	void operator() (control<int>&) const		              
	{
		b->on_set(member_ptr);
		if ((ref_current_modifiers() & EM_SHIFT) != 0) {
			char* base_ptr = static_cast<char*>(member_ptr)-i*element_size;
			for (unsigned j = 0; j < dim; ++j) {
				if (j != i) {
					memcpy(base_ptr + j*element_size, member_ptr, element_size);
					b->on_set(base_ptr + j*element_size);
				}
			}
		}
	}
	functor_base* clone() const                               
	{
		return new call_on_set_functor(*this); 
	}
};

template <typename T>
struct ensure_normalized_functor : public cgv::gui::control<T>::value_change_signal_type::functor_type, public cgv::signal::tacker
{
	cgv::gui::provider* p;
	cgv::base::base_ptr b;
	T* member_ptr;
	int i;
	unsigned n;
	ensure_normalized_functor(cgv::gui::provider* _p, cgv::base::base_ptr _b, T* _member_ptr, int _i, unsigned _n) : p(_p), b(_b), member_ptr(_member_ptr), i(_i), n(_n) {}
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &(*b); p2 = member_ptr; }
	functor_base* clone() const                               { return new ensure_normalized_functor<T>(*this); }
	void operator() (control<T>&) const
	{ 
		unsigned j;
		T r2_old = 0;
		for (j=0; j<n; ++j)
			if (i != j)
				r2_old += member_ptr[j]*member_ptr[j];
	
		if (r2_old < std::numeric_limits<T>::epsilon()) {
			for (j=0; j<n; ++j)
				if (i != j)
					member_ptr[j] = 1;
			r2_old = (T)n-1;
		}
		T r2_new = 1-member_ptr[i]*member_ptr[i];
		T scale = sqrt(r2_new/r2_old);
		for (j=0; j<n; ++j)
			if (i != j)
				member_ptr[j] *= scale;

		for (j=0; j<n; ++j)
			if (j != i)
				p->update_member(member_ptr+j);

		if (b)
			b->on_set(member_ptr); 
	}
};

template <typename T>
struct ensure_ascending_functor : public cgv::gui::control<T>::value_change_signal_type::functor_type, public cgv::signal::tacker
{
	cgv::gui::provider* p;
	cgv::base::base_ptr b;
	T* member_ptr;
	int i;
	unsigned n;
	T min_size;
	ensure_ascending_functor(cgv::gui::provider* _p, cgv::base::base_ptr _b, T* _member_ptr, double _min_size, int _i, unsigned _n) : p(_p), b(_b), member_ptr(_member_ptr), min_size(T(_min_size)), i(_i), n(_n) {}
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &(*b); p2 = member_ptr; }
	functor_base* clone() const { return new ensure_ascending_functor<T>(*this); }
	void operator() (control<T>& c) const
	{
		if ((ref_current_modifiers() & EM_SHIFT) != 0) {
			// check if value decreased
			if (member_ptr[i] < c.get_old_value()) {
				T diff = c.get_old_value() - member_ptr[i];
				T min_value = c.template get<T>("min");
				if (i > 0) {
					if (member_ptr[0] < min_value + diff) {
						T new_diff = member_ptr[0] - min_value;
						member_ptr[i] += diff - new_diff;
						diff = new_diff;
					}
				}
				if (b)
					b->on_set(member_ptr + i);
				for (unsigned j = 0; j < n; ++j)
					if (j != i) {
						member_ptr[j] -= diff;
						p->update_member(member_ptr + j);
						if (b)
							b->on_set(member_ptr + j);
					}
			}
			else {
				T diff = member_ptr[i] - c.get_old_value();
				T max_value = c.template get<T>("max");
				if (i < (int)n-1) {
					if (member_ptr[n-1] > max_value - diff) {
						T new_diff = max_value - member_ptr[n-1];
						member_ptr[i] -= diff - new_diff;
						diff = new_diff;
					}
				}
				if (b)
					b->on_set(member_ptr + i);
				for (unsigned j = 0; j < n; ++j)
					if (j != i) {
						member_ptr[j] += diff;
						p->update_member(member_ptr + j);
						if (b)
							b->on_set(member_ptr + j);
					}
			}
		}
		else {
			if (i > 0) {
				if (member_ptr[i - 1] + min_size > member_ptr[i])
					member_ptr[i] = member_ptr[i - 1] + min_size;
			}
			if (i + 1 < int(n)) {
				if (member_ptr[i] + min_size > member_ptr[i + 1])
					member_ptr[i] = member_ptr[i + 1] - min_size;
			}
			if (b)
				b->on_set(member_ptr+i);
		}
	}
};


struct vec_gui_creator : public cgv::gui::gui_creator
{
	static std::string fvec_prefix;
	static std::string vec_prefix;
	static std::string quat_prefix;

	vec_gui_creator()
	{
		// determine prefixes of fvec and vec template types
		if (fvec_prefix.empty()) {
			fvec_prefix = cgv::type::info::type_name<fvec<int,2> >::get_name();
			fvec_prefix = fvec_prefix.substr(0, fvec_prefix.find_first_of('<'));
			vec_prefix = cgv::type::info::type_name<vec<int> >::get_name();
			vec_prefix = vec_prefix.substr(0, vec_prefix.find_first_of('<'));
			quat_prefix = cgv::type::info::type_name<quaternion<float> >::get_name();
			quat_prefix = quat_prefix.substr(0, quat_prefix.find_first_of('<'));
		}
	}
	bool is_vec_type(void* value_ptr, const std::string& value_type, cgv::type::info::TypeId& type_id, int& dim, unsigned char*& crd_ptr) const
	{
#if _MSC_VER >= 1400
		// first check against prefixes
		if (value_type.size() > fvec_prefix.size() + 2 && (value_type.substr(0, fvec_prefix.size()) == fvec_prefix)) {
			// split off into coordinate type and dimension
			std::string::size_type p0 = fvec_prefix.size() + 1;
			std::string::size_type p1 = value_type.find_first_of(',', p0);
			if (p1 == std::string::npos)
				return false;

			// determine coordinate type
			std::string coordinate_type = value_type.substr(p0, p1 - p0);
			type_id = cgv::type::info::get_type_id(coordinate_type);
			if (!cgv::type::info::is_number(type_id))
				return false;
			crd_ptr = (unsigned char*)value_ptr;

			// determine dimension
			std::string dimension = value_type.substr(p1 + 1, value_type.size() - 2 - p1);
			if (!cgv::utils::is_integer(dimension, dim))
				return false;
		}
		else if (value_type.size() > vec_prefix.size() + 2 && (value_type.substr(0, vec_prefix.size()) == vec_prefix)) {
			// determine coordinate type
			std::string::size_type p0 = vec_prefix.size() + 1;
			std::string coordinate_type = value_type.substr(p0, value_type.size() - 1 - p0);
			type_id = cgv::type::info::get_type_id(coordinate_type);
			if (!cgv::type::info::is_number(type_id))
				return false;
			crd_ptr = &(*static_cast<cgv::math::vec<unsigned char>*>(value_ptr))(0);

			// determine dimension
			dim = (int)(static_cast<cgv::math::vec<unsigned char>*>(value_ptr)->size());
		}
		else if (value_type.size() > quat_prefix.size() + 2 && (value_type.substr(0, quat_prefix.size()) == quat_prefix)) {
			// determine coordinate type
			std::string::size_type p0 = quat_prefix.size() + 1;
			std::string coordinate_type = value_type.substr(p0, value_type.size() - 1 - p0);
			type_id = cgv::type::info::get_type_id(coordinate_type);
			if (!cgv::type::info::is_number(type_id))
				return false;
			crd_ptr = (unsigned char*)value_ptr;
			dim = 4;
		}
		else
			return false;
		return true;
#else
		type_id = cgv::type::info::type_id<float>::get_id();
		crd_ptr = reinterpret_cast<unsigned char*>(value_ptr);
		if (value_type == cgv::type::info::type_name<fvec<float, 2> >::get_name()) {
			dim = 2;
			return true;
		}
		if (value_type == cgv::type::info::type_name<fvec<float, 3> >::get_name()) {
			dim = 3;
			return true;
		}
		if (value_type == cgv::type::info::type_name<fvec<float, 4> >::get_name()) {
			dim = 4;
			return true;
		}
		if (value_type == cgv::type::info::type_name<quaternion<float> >::get_name()) {
			dim = 4;
			return true;
		}
		if (value_type == cgv::type::info::type_name<vec<float> >::get_name()) {
			dim = (int)reinterpret_cast<vec<float>*>(value_ptr)->size();
			crd_ptr = reinterpret_cast<unsigned char*>(&(*reinterpret_cast<vec<float>*>(value_ptr))(0));
			return true;
		}
		type_id = cgv::type::info::type_id<double>::get_id();
		if (value_type == cgv::type::info::type_name<fvec<double, 2> >::get_name()) {
			dim = 2;
			return true;
		}
		if (value_type == cgv::type::info::type_name<fvec<double, 3> >::get_name()) {
			dim = 3;
			return true;
		}
		if (value_type == cgv::type::info::type_name<fvec<double, 4> >::get_name()) {
			dim = 4;
			return true;
		}
		if (value_type == cgv::type::info::type_name<quaternion<double> >::get_name()) {
			dim = 4;
			return true;
		}
		if (value_type == cgv::type::info::type_name<vec<double> >::get_name()) {
			dim = (int)reinterpret_cast<vec<double>*>(value_ptr)->size();
			crd_ptr = reinterpret_cast<unsigned char*>(&(*reinterpret_cast<vec<double>*>(value_ptr))(0));
			return true;
		}
		type_id = cgv::type::info::type_id<cgv::type::int32_type>::get_id();
		if (value_type == cgv::type::info::type_name<fvec<cgv::type::int32_type, 2> >::get_name()) {
			dim = 2;
			return true;
		}
		if (value_type == cgv::type::info::type_name<fvec<cgv::type::int32_type, 3> >::get_name()) {
			dim = 3;
			return true;
		}
		if (value_type == cgv::type::info::type_name<fvec<cgv::type::int32_type, 4> >::get_name()) {
			dim = 4;
			return true;
		}
		if (value_type == cgv::type::info::type_name<vec<cgv::type::int32_type> >::get_name()) {
			dim = (int)reinterpret_cast<vec<cgv::type::int32_type>*>(value_ptr)->size();
			crd_ptr = reinterpret_cast<unsigned char*>(&(*reinterpret_cast<vec<cgv::type::int32_type>*>(value_ptr))(0));
			return true;
		}
		type_id = cgv::type::info::type_id<cgv::type::uint32_type>::get_id();
		if (value_type == cgv::type::info::type_name<fvec<cgv::type::uint32_type, 2> >::get_name()) {
			dim = 2;
			return true;
		}
		if (value_type == cgv::type::info::type_name<fvec<cgv::type::uint32_type, 3> >::get_name()) {
			dim = 3;
			return true;
		}
		if (value_type == cgv::type::info::type_name<fvec<cgv::type::uint32_type, 4> >::get_name()) {
			dim = 4;
			return true;
		}
		if (value_type == cgv::type::info::type_name<vec<cgv::type::uint32_type> >::get_name()) {
			dim = (int)reinterpret_cast<vec<cgv::type::uint32_type>*>(value_ptr)->size();
			crd_ptr = reinterpret_cast<unsigned char*>(&(*reinterpret_cast<vec<cgv::type::uint32_type>*>(value_ptr))(0));
			return true;
		}
		return false;
#endif
	}
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool*)
	{
		cgv::type::info::TypeId type_id;
		int dim;
		unsigned char* crd_ptr;
		if (!is_vec_type(value_ptr, value_type, type_id, dim, crd_ptr))
			return false;
		// reconstruct coordinate type name for which controls are registered
		std::string coordinate_type = cgv::type::info::get_type_name(type_id);
		unsigned crd_type_size = cgv::type::info::get_type_size(type_id);

		// analyze gui type
		bool ensure_normalized = false, ascending = false;
		if (gui_type.empty() || gui_type == "vector") {
		}
		else if (gui_type == "direction") {
			if (type_id == cgv::type::info::TI_FLT32 || type_id == cgv::type::info::TI_FLT64)
				ensure_normalized = true; // normalization only makes sense for floating point types
		}
		else if (gui_type == "ascending") {
			ascending = true;
		}
		else
			return false;

		// extract from options
		std::string child_gui_type;
		std::string child_options;
		std::string child_align;
		bool long_label;
		std::string main_label; // none,first,heading
		std::string components; 
		double min_size;

		std::string proposed_child_options;
		if (ensure_normalized)
			proposed_child_options = "min=-1;max=1;ticks=true";

		if (!cgv::base::has_property(options, "main_label", main_label, false))
			main_label = "heading";
		if (!cgv::base::has_property(options, "components", components, false))
			components = "xyzw";
		if (!cgv::base::has_property(options, "long_label", long_label, false))
			long_label = false;
		if (!cgv::base::has_property(options, "min_size", min_size, false))
			min_size = 0.0;

		if (!cgv::base::has_property(options, "gui_type", child_gui_type, false))
			child_gui_type = "value_slider";
		if (!cgv::base::has_property(options, "options", child_options, false))
			child_options = proposed_child_options;
		if (!cgv::base::has_property(options, "align", child_align, false))
			child_align = "\n";

		// construct gui
		if (main_label=="heading") {
			p->add_decorator(label, "heading", "level=3");
		}
		cgv::base::base_ptr b(dynamic_cast<cgv::base::base*>(p));
		for (int i=0; i<dim; ++i) {
			void* member_ptr = static_cast<void*>(crd_ptr + i*crd_type_size);
			std::string lab_i;
			if (main_label == "first") {
				if (i==0)
					lab_i = label;
			}
			else {
				if (dim > (int)components.size())
					lab_i = (long_label ? label+"|" : std::string()) + cgv::utils::to_string(i);
				else
					lab_i = (long_label ? label+"|" : std::string()) + components[i];
			}
			cgv::gui::control_ptr cp;
			if (child_gui_type != "view")
				cp = p->add_control_void(lab_i, member_ptr, 0, coordinate_type, child_gui_type, child_options, child_align, 0);
			if (cp) {
				cgv::signal::functor_base* f = 0;
				if (ensure_normalized) {
					if (type_id == cgv::type::info::TI_FLT32)
						f = new ensure_normalized_functor<float>(p, b, reinterpret_cast<float*>(crd_ptr), i, dim);
					else
						f = new ensure_normalized_functor<double>(p, b, reinterpret_cast<double*>(crd_ptr), i, dim);
				}
				else if (ascending) {
					switch (type_id) {
					case cgv::type::info::TI_INT8:   f = new ensure_ascending_functor<cgv::type::int8_type>  (p, b, reinterpret_cast<cgv::type::int8_type*>  (crd_ptr), min_size, i, dim); break;
					case cgv::type::info::TI_INT16:  f = new ensure_ascending_functor<cgv::type::int16_type> (p, b, reinterpret_cast<cgv::type::int16_type*> (crd_ptr), min_size, i, dim); break;
					case cgv::type::info::TI_INT32:  f = new ensure_ascending_functor<cgv::type::int32_type> (p, b, reinterpret_cast<cgv::type::int32_type*> (crd_ptr), min_size, i, dim); break;
					case cgv::type::info::TI_INT64:  f = new ensure_ascending_functor<cgv::type::int64_type> (p, b, reinterpret_cast<cgv::type::int64_type*> (crd_ptr), min_size, i, dim); break;
					case cgv::type::info::TI_UINT8 : f = new ensure_ascending_functor<cgv::type::uint8_type> (p, b, reinterpret_cast<cgv::type::uint8_type*> (crd_ptr), min_size, i, dim); break;
					case cgv::type::info::TI_UINT16: f = new ensure_ascending_functor<cgv::type::uint16_type>(p, b, reinterpret_cast<cgv::type::uint16_type*>(crd_ptr), min_size, i, dim); break;
					case cgv::type::info::TI_UINT32: f = new ensure_ascending_functor<cgv::type::uint32_type>(p, b, reinterpret_cast<cgv::type::uint32_type*>(crd_ptr), min_size, i, dim); break;
					case cgv::type::info::TI_UINT64: f = new ensure_ascending_functor<cgv::type::uint64_type>(p, b, reinterpret_cast<cgv::type::uint64_type*>(crd_ptr), min_size, i, dim); break;
					case cgv::type::info::TI_FLT32:  f = new ensure_ascending_functor<float>(p, b, reinterpret_cast<float*>(crd_ptr), min_size, i, dim); break;
					case cgv::type::info::TI_FLT64:  f = new ensure_ascending_functor<double>(p, b, reinterpret_cast<double*>(crd_ptr), min_size, i, dim); break;
					}
				}
				else if (b)
					f = new call_on_set_functor(b, member_ptr, dim, i, cgv::type::info::get_type_size(type_id));
				cp->attach_to_value_change(f);
			}
			else 
				p->add_view_void(lab_i, member_ptr, coordinate_type,  child_gui_type, child_options, child_align);
		}
		return true;
	}
};

std::string vec_gui_creator::fvec_prefix;
std::string vec_gui_creator::vec_prefix;
std::string vec_gui_creator::quat_prefix;

cgv::gui::gui_creator_registration<vec_gui_creator> flt2vector_gc_reg("vec_gui_creator");
