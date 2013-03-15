#include <cgv/base/base_generator.h>
#include <cgv/gui/gui_creator.h>
#include <cgv/gui/provider.h>
#include <cgv/utils/scan.h>
#include <cgv/type/info/type_id.h>
#include <cgv/math/fvec.h>
#include <cgv/math/vec.h>
#include <limits>

using namespace cgv::gui;
using namespace cgv::signal;
using namespace cgv::math;

struct call_on_set_functor : public cgv::gui::abst_control::functor_type
{
	cgv::base::base_ptr b;
	void* member_ptr;
	call_on_set_functor(cgv::base::base_ptr _b, void* _member_ptr) : b(_b), member_ptr(_member_ptr) {}
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &(*b); p2 = member_ptr; }
	functor_base* clone() const                               { return new call_on_set_functor(*this); }
	void operator() (const control_ptr& cp) const             { b->on_set(member_ptr); }
};

template <typename T>
struct ensure_normalized_functor : public cgv::gui::abst_control::functor_type
{
	cgv::gui::provider* p;
	cgv::base::base_ptr b;
	T* member_ptr;
	int i;
	unsigned n;
	ensure_normalized_functor(cgv::gui::provider* _p, cgv::base::base_ptr _b, T* _member_ptr, int _i, unsigned _n) : p(_p), b(_b), member_ptr(_member_ptr), i(_i), n(_n) {}
	void put_pointers(const void* &p1, const void* &p2) const { p1 = &(*b); p2 = member_ptr; }
	functor_base* clone() const                               { return new ensure_normalized_functor(*this); }
	void operator() (const control_ptr& cp) const
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


struct vec_gui_creator : public cgv::gui::gui_creator
{
	static std::string fvec_prefix;
	static std::string vec_prefix;

	vec_gui_creator()
	{
		// determine prefixes of fvec and vec template types
		if (fvec_prefix.empty()) {
			fvec_prefix = cgv::type::info::type_name<fvec<int,2> >::get_name();
			fvec_prefix = fvec_prefix.substr(0, fvec_prefix.find_first_of('<'));
			vec_prefix = cgv::type::info::type_name<vec<int> >::get_name();
			vec_prefix = vec_prefix.substr(0, vec_prefix.find_first_of('<'));
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
		if (value_type.size() > fvec_prefix.size()+2 && (value_type.substr(0,fvec_prefix.size()) == fvec_prefix)) {
			// split off into coordinate type and dimension
			std::string::size_type p0 = fvec_prefix.size()+1;
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
		else if (value_type.size() > vec_prefix.size()+2 && (value_type.substr(0,vec_prefix.size()) == vec_prefix)) {
			// determine coordinate type
			std::string::size_type p0 = vec_prefix.size()+1;
			coordinate_type = value_type.substr(p0,value_type.size()-1-p0);
			type_id = cgv::type::info::get_type_id(coordinate_type);
			if (!cgv::type::info::is_number(type_id))
				return false;
			crd_ptr = &(*static_cast<cgv::math::vec<unsigned char>*>(value_ptr))(0);

			// determine dimension
			dim = (int)(static_cast<cgv::math::vec<unsigned char>*>(value_ptr)->size());
		}
		else
			return false;

		// reconstruct coordinate type name for which controls are registered
		coordinate_type = cgv::type::info::get_type_name(type_id);
		unsigned crd_type_size = cgv::type::info::get_type_size(type_id);

		// analyze gui type
		bool ensure_normalized;
		if (gui_type.empty() || gui_type == "vector")
			ensure_normalized = false;
		else if (gui_type == "direction") {
			if (type_id == cgv::type::info::TI_FLT32 || type_id == cgv::type::info::TI_FLT64)					
				ensure_normalized = true; // normalization only makes sense for floating point types
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

		std::string proposed_child_options;
		if (ensure_normalized)
			proposed_child_options = "min=-1;max=1;ticks=true";

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
			cgv::gui::control_ptr cp = p->add_control_void(lab_i, member_ptr, 0, coordinate_type, child_gui_type, child_options, child_align, 0);
			if (cp) {
				cgv::gui::abst_control::functor_type* f = 0;
				if (ensure_normalized) {
					if (type_id == cgv::type::info::TI_FLT32)
						f = new ensure_normalized_functor<float>(p, b, reinterpret_cast<float*>(crd_ptr), i, dim);
					else
						f = new ensure_normalized_functor<double>(p, b, reinterpret_cast<double*>(crd_ptr), i, dim);
				}
				else if (b)
					f = new call_on_set_functor(b, member_ptr);
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

cgv::gui::gui_creator_registration<vec_gui_creator> flt2vector_gc_reg("vec_gui_creator");
