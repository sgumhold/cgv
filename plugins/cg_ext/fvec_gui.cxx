#include <cgv/gui/gui_creator.h>
#include <cgv/gui/provider.h>
#include <cgv/math/fvec.h>
#include <limits>

using namespace cgv::gui;
using namespace cgv::signal;
using namespace cgv::math;

std::string get_component_name(unsigned i, unsigned N)
{
	static const char* names = "xyzw";
	if (N < 5)
		return std::string(" ")+std::string(1,names[i]);
	else
		return std::string("[") + cgv::utils::to_string(i) + "]";
}

template <typename T, cgv::type::uint32_type N>
struct fvec_gui_creator : public cgv::gui::gui_creator
{
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool*)
	{
		if (value_type != cgv::type::info::type_name<fvec<T,N> >::get_name())
			return false;
		if (!gui_type.empty() && gui_type != "vector")
			return false;
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
		fvec<T,N>& v = *((fvec<T,N>*)value_ptr);
		for (unsigned i=0; i<N; ++i)
			if (b)
				p->add_member_control(b, label+get_component_name(i,N), v(i), "value_slider", options);
			else
				p->add_control(label+get_component_name(i,N), v(i), "value_slider", options);
		return true;
	}
};

template <typename T>
void dir_gui_cb(provider* p, T& dir, unsigned i, unsigned n)
{
	unsigned j;
	T* D = &dir;
	T r2_old = 0;
	for (j=0; j<n; ++j)
		if (i != j)
			r2_old += D[j]*D[j];
	
	if (r2_old < std::numeric_limits<T>::epsilon()) {
		for (j=0; j<n; ++j)
			if (i != j)
				D[j] = 1;
		r2_old = (T)n-1;
	}
	T r2_new = 1-D[i]*D[i];
	T scale = sqrt(r2_new/r2_old);
	for (j=0; j<n; ++j)
		if (i != j)
			D[j] *= scale;

	for (j=0; j<n; ++j)
		if (j != i && p->find_control(D[j]))
			p->find_control(D[j])->update();

	cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
	if (b)
		b->on_set(D);
}


template <typename T, cgv::type::uint32_type N>
struct fvec_dir_gui_creator : public cgv::gui::gui_creator
{
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool*)
	{
		if (value_type != cgv::type::info::type_name<fvec<T,N> >::get_name())
			return false;
		if (!gui_type.empty() && gui_type != "direction")
			return false;
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
		fvec<T,N>& v = *((fvec<T,N>*)value_ptr);
		for (unsigned i=0; i<N; ++i)
			connect_copy(p->add_control(label+get_component_name(i,N), v(i), "value_slider", std::string("step=0.01;min=-1;max=1;ticks=true")+options)->value_change,
				rebind(dir_gui_cb<T>, p, cgv::signal::_r((T&)v), i, N));
		return true;
	}
};


cgv::gui::gui_creator_registration<fvec_gui_creator<float,2> > flt2vector_gc_reg("fvec2f_gui_creator");
cgv::gui::gui_creator_registration<fvec_gui_creator<float,3> > flt3vector_gc_reg("fvec3f_gui_creator");
cgv::gui::gui_creator_registration<fvec_gui_creator<float,4> > flt4vector_gc_reg("fvec4f_gui_creator");
cgv::gui::gui_creator_registration<fvec_gui_creator<double,2> > dbl2vector_gc_reg("fvec2d_gui_creator");
cgv::gui::gui_creator_registration<fvec_gui_creator<double,3> > dbl3vector_gc_reg("fvec3d_gui_creator");
cgv::gui::gui_creator_registration<fvec_gui_creator<double,4> > dbl4vector_gc_reg("fvec4d_gui_creator");

cgv::gui::gui_creator_registration<fvec_dir_gui_creator<float,2> > flt2direction_gc_reg("fvec2f_dir_gui_creator");
cgv::gui::gui_creator_registration<fvec_dir_gui_creator<float,3> > flt3direction_gc_reg("fvec3f_dir_gui_creator");
cgv::gui::gui_creator_registration<fvec_dir_gui_creator<float,4> > flt4direction_gc_reg("fvec4f_dir_gui_creator");
cgv::gui::gui_creator_registration<fvec_dir_gui_creator<double,2> > dbl2direction_gc_reg("fvec2d_dir_gui_creator");
cgv::gui::gui_creator_registration<fvec_dir_gui_creator<double,3> > dbl3direction_gc_reg("fvec3d_dir_gui_creator");
cgv::gui::gui_creator_registration<fvec_dir_gui_creator<double,4> > dbl4direction_gc_reg("fvec4d_dir_gui_creator");



/*



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

std::string get_component_name(unsigned i, unsigned N)
{
	static const char* names = "xyzw";
	if (N < 5)
		return std::string(" ")+std::string(1,names[i]);
	else
		return std::string("[") + cgv::utils::to_string(i) + "]";
}

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
		// first check against prefixes
		if (value_type.size() > fvec_prefix.size()+2 && (value_type.substr(0,fvec_prefix.size()) == fvec_prefix)) {
			// split off into coordinate type and dimension
			std::string::size_type p0 = fvec_prefix.size()+1;
			std::string::size_type p1 = value_type.find_first_of(',', p0);
			if (p1 == std::string::npos)
				return false;

			// determine coordinate type
			std::string coordinate_type = value_type.substr(p0,p1-p0);
			cgv::type::info::TypeId type_id = cgv::type::info::get_type_id(coordinate_type);
			if (!cgv::type::info::is_number(type_id))
				return false;
			unsigned crd_type_size = cgv::type::info::get_type_size(type_id);
			unsigned char* crd_ptr = (unsigned char*) value_ptr;

			// determine dimension
			std::string dimension = value_type.substr(p1+1,value_type.size()-1-p1-p0);
			int dim;
			if (!cgv::utils::is_integer(dimension, dim))
				return false;

			// analyze gui type

		}
		else if (value_type.size() > vec_prefix.size()+2 && (value_type.substr(0,vec_prefix.size()) == vec_prefix)) {
		}
		else
			return false;
		if (value_type != cgv::type::info::type_name<fvec<T,N> >::get_name())
			return false;
		if (!gui_type.empty() && gui_type != "vector")
			return false;
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
		fvec<T,N>& v = *((fvec<T,N>*)value_ptr);
		for (unsigned i=0; i<N; ++i)
			if (b)
				p->add_member_control(b, label+get_component_name(i,N), v(i), "value_slider", options);
			else
				p->add_control(label+get_component_name(i,N), v(i), "value_slider", options);
		p->add_control_void(
		return true;
	}
};

template <typename T>
void dir_gui_cb(provider* p, T& dir, unsigned i, unsigned n)
{
	unsigned j;
	T* D = &dir;
	T r2_old = 0;
	for (j=0; j<n; ++j)
		if (i != j)
			r2_old += D[j]*D[j];
	
	if (r2_old < std::numeric_limits<T>::epsilon()) {
		for (j=0; j<n; ++j)
			if (i != j)
				D[j] = 1;
		r2_old = (T)n-1;
	}
	T r2_new = 1-D[i]*D[i];
	T scale = sqrt(r2_new/r2_old);
	for (j=0; j<n; ++j)
		if (i != j)
			D[j] *= scale;

	for (j=0; j<n; ++j)
		if (j != i && p->find_control(D[j]))
			p->find_control(D[j])->update();

	cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
	if (b)
		b->on_set(D);
}


template <typename T, cgv::type::uint32_type N>
struct fvec_dir_gui_creator : public cgv::gui::gui_creator
{
	/// attempt to create a gui and return whether this was successful
	bool create(provider* p, const std::string& label, 
		void* value_ptr, const std::string& value_type, 
		const std::string& gui_type, const std::string& options, bool*)
	{
		if (value_type != cgv::type::info::type_name<fvec<T,N> >::get_name())
			return false;
		if (!gui_type.empty() && gui_type != "direction")
			return false;
		cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
		fvec<T,N>& v = *((fvec<T,N>*)value_ptr);
		for (unsigned i=0; i<N; ++i)
			connect_copy(p->add_control(label+get_component_name(i,N), v(i), "value_slider", std::string("step=0.01;min=-1;max=1;ticks=true;")+options)->value_change,
				rebind(dir_gui_cb<T>, p, cgv::signal::_r((T&)v), i, N));
		return true;
	}
};

cgv::gui::gui_creator_registration<fvec_gui_creator<float,2> > flt2vector_gc_reg("fvec2f_gui_creator");
cgv::gui::gui_creator_registration<fvec_gui_creator<float,3> > flt3vector_gc_reg("fvec3f_gui_creator");
cgv::gui::gui_creator_registration<fvec_gui_creator<float,4> > flt4vector_gc_reg("fvec4f_gui_creator");
cgv::gui::gui_creator_registration<fvec_gui_creator<double,2> > dbl2vector_gc_reg("fvec2d_gui_creator");
cgv::gui::gui_creator_registration<fvec_gui_creator<double,3> > dbl3vector_gc_reg("fvec3d_gui_creator");
cgv::gui::gui_creator_registration<fvec_gui_creator<double,4> > dbl4vector_gc_reg("fvec4d_gui_creator");

cgv::gui::gui_creator_registration<fvec_dir_gui_creator<float,2> > flt2direction_gc_reg("fvec2f_dir_gui_creator");
cgv::gui::gui_creator_registration<fvec_dir_gui_creator<float,3> > flt3direction_gc_reg("fvec3f_dir_gui_creator");
cgv::gui::gui_creator_registration<fvec_dir_gui_creator<float,4> > flt4direction_gc_reg("fvec4f_dir_gui_creator");
cgv::gui::gui_creator_registration<fvec_dir_gui_creator<double,2> > dbl2direction_gc_reg("fvec2d_dir_gui_creator");
cgv::gui::gui_creator_registration<fvec_dir_gui_creator<double,3> > dbl3direction_gc_reg("fvec3d_dir_gui_creator");
cgv::gui::gui_creator_registration<fvec_dir_gui_creator<double,4> > dbl4direction_gc_reg("fvec4d_dir_gui_creator");*/