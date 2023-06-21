#include <cgv/data/data_view.h>
#include <iostream> 
#include <memory.h>
#include <map>
#include <cgv/data/ref_ptr.h>

namespace cgv {
	namespace data {
		/*
bool validate_delete(const void* ptr)
{
	static std::map<const void*, int> del_map;
	if (del_map.find(ptr) != del_map.end()) {
		++del_map[ptr];
//		std::cerr << "ERROR:  multiple (" << del_map[ptr] << ")  delete on " << ptr << std::endl;
		return false;
	}
	else {
		del_map[ptr] = 1;
//		std::cerr << "DELETE: " << ptr << std::endl;
		return true;
	}
}
*/
/// constructor used to construct sub views onto the data view
data_view_base::data_view_base(
		const data_format* _format, 
		unsigned int _dim, 
		const unsigned int* _step_sizes) : format(_format)
{
	dim = _dim;
	owns_format = false;
	std::fill(step_sizes+dim,step_sizes+4,0);
	std::copy(_step_sizes, _step_sizes+dim, step_sizes);
}

/** construct the base of a data view from the given format, 
	 such that the step sizes and dimension are set to view
	 the complete data set defined in the format. */
data_view_base::data_view_base(const data_format* _format)
	: format(_format)
{
	owns_format = false;
	if (!_format)
		return;
	dim = _format->get_nr_dimensions();
	std::fill(step_sizes+dim,step_sizes+4,0);
	if (dim > 0) {
		step_sizes[dim-1] = _format->align(format->get_entry_size(), 
													  _format->get_alignment(0));
	}
	for (unsigned int i=1; i < dim; ++i)
		step_sizes[dim-1-i] = format->align(
			_format->get_resolution(i-1)*step_sizes[dim-i], 
			_format->get_alignment(i));
}

/// whether to manage the data format pointer
void data_view_base::manage_format(bool enable)
{
	owns_format = true;
}

/// delete format if it is owned
data_view_base::~data_view_base()
{
	if (owns_format && format)
		delete format;
}


/// return the component format
const data_format* data_view_base::get_format() const
{
	return format;
}
/// set a new data format
void data_view_base::set_format(const data_format* _format)
{
	format = _format;
}

/// return the dimension of the data view, which is less or equal to the dimension of the data format
unsigned int data_view_base::get_dim() const 
{ 
	return dim; 
}
/// return the step size in bytes in the i-th dimension
unsigned int data_view_base::get_step_size(unsigned int dim) const 
{ 
	return step_sizes[dim];
}


/// constructor used to construct sub views onto the data view
template <class D, typename P>
data_view_impl<D,P>::data_view_impl(const data_format* _format, 
			P _data_ptr, unsigned int _dim, const unsigned int* _step_sizes)
			: data_view_base(_format, _dim, _step_sizes), data_ptr(_data_ptr)
{
}

/// construct a data view from the given format, viewing the complete data set
template <class D, typename P>
data_view_impl<D,P>::data_view_impl(const data_format* _format, 
					typename cgv::type::func::transfer_const<P,void*>::type _data_ptr)
					: data_view_base(_format), data_ptr(static_cast<P>(_data_ptr))
{
}

/// return whether the data pointer is a null pointer
template <class D, typename P>
bool data_view_impl<D,P>::empty() const 
{
	return data_ptr == 0 || format == 0;
}
/// access to i-th data entry
template <class D, typename P>
D data_view_impl<D,P>::operator () (unsigned int i) const
{
	if (dim == 0) {
		std::cerr << "1d operator access to 0d data ptr" << std::endl;
		return D();
	}
	return D(format, data_ptr+i*step_sizes[0], 
			   (unsigned int) (dim-1), step_sizes+1);
}
/// access to entry at (i,j)
template <class D, typename P>
D data_view_impl<D,P>::operator () (unsigned int i, unsigned int j) const
{
	if (dim < 2) {
		std::cerr << "2d operator access to " << dim << "d data ptr" << std::endl;
		return D();
	}
	return D(format, data_ptr+i*step_sizes[0]+j*step_sizes[1],
			   (unsigned int) (dim-2),step_sizes+2);
}
/// access to entry at (i,j,k)
template <class D, typename P>
D data_view_impl<D,P>::operator () (unsigned int i, unsigned int j, unsigned int k) const
{
	if (dim < 3) {
		std::cerr << "3d operator access to " << dim << "d data ptr" << std::endl;
		return D();
	}
	return D(format, data_ptr+i*step_sizes[0]+j*step_sizes[1]+k*step_sizes[2],
			   (unsigned int) (dim-3),step_sizes+3);
}
/// access to entry at (i,j,k,l)
template <class D, typename P>
D data_view_impl<D,P>::operator () (unsigned int i, unsigned int j, unsigned int k, unsigned int l) const
{
	if (dim < 4) {
		std::cerr << "4d operator access to " << dim << "d data ptr" << std::endl;
		return D();
	}
	return D(format, data_ptr+i*step_sizes[0]+j*step_sizes[1]+k*step_sizes[2]+l*step_sizes[3],
				(unsigned int) (dim-4),step_sizes+4);
}

/** permute the order of the indices, where the permutation argument "kji" implies that after 
    the permutation the operator (i,j,k) returns the same as the operator (k,j,i) before the
	call to permute. The permutation string must have at least two entries. If it has n entries
	it must contain each of the first n letters of "ijkl" exactly once, i.e. "ik" would be invalid,
	whereas "ikj" is a valid permutation. */
template <class D, typename P>
D data_view_impl<D,P>::permute(const std::string& permutation) const
{
	unsigned int n = (unsigned int) permutation.size();
	if (n < 2 || n > 4) {
		std::cerr << "permutation '" << permutation.c_str() << "' has invalid length " << n << std::endl;
		return D();
	}
	unsigned int new_step_sizes[4];
	unsigned int i;
	bool used[4] = { false, false, false, false };
	for (i=0; i<4; ++i)
		new_step_sizes[i] = step_sizes[i];
	for (i=0; i<n; ++i) {
		int idx = permutation[i]-'i';
		if (idx < 0 || idx > 3) {
			std::cerr << "invalid permutation entry '" << permutation[i] << "', only 'ijkl' allowed" << std::endl;
			return D();
		}
		used[idx] = true;
		new_step_sizes[i] = step_sizes[idx];
	}
	for (i=0; i<get_dim(); ++i)
		if (!used[i]) {
			std::cerr << "invalid permutation of length " << n << " without reference to '" << ('i'+i) << "'" << std::endl;
			return D();
		}
	return D(get_format(), get_ptr<unsigned char>(), get_dim(), 
			   new_step_sizes);
}

/// use base class for construction and don't manage data pointer 
data_view::data_view(const data_format* _format, unsigned char* _data_ptr, 
							unsigned int _dim, const unsigned int* _step_sizes) 
	: data_view_impl<data_view, unsigned char*>(_format, _data_ptr, _dim, _step_sizes),
	  owns_ptr(false)
{
}

/// construct an empty data view without format and with empty data pointer*/
data_view::data_view() : owns_ptr(false) 
{
}
/// destruct view and delete data pointer if it is owned by the view
data_view::~data_view()
{
	if (owns_ptr && data_ptr) {
		delete [] data_ptr; 
		data_ptr = 0;
	}
}
/** construct a data view from the given format. Allocate a new data
    pointer with the new [] operator of type (unsigned char) and own
	 the pointer. The data_view will view the complete data set as defined
	 in the format. */
data_view::data_view(const data_format* _format) 
	: data_view_impl<data_view, unsigned char*>(_format),
	  owns_ptr(false)
{
	if (_format) {
		data_ptr = new unsigned char[_format->get_nr_bytes()];
		owns_ptr = true;
	}
}
/** construct a data view from the given format, viewing the complete 
    data set. The passed pointer will not be owned by the view. */
data_view::data_view(const data_format* _format, void* _data_ptr)
	: data_view_impl<data_view, unsigned char*>(_format, _data_ptr),
	  owns_ptr(false)
{
}
/** construct a data view from the given format, viewing the complete 
    data set. The passed pointer will be owned by the view if the
	 manage_ptr flag is true. In this case the pointer is deleted on
	 destruction with the delete [] operator of type (unsigned char*). */
data_view::data_view(const data_format* _format, unsigned char* _data_ptr, 
							bool manage_ptr) 
	: data_view_impl<data_view, unsigned char*>(_format, _data_ptr),
	  owns_ptr(manage_ptr)
{
}
/** the assignment operator takes over the data format and data pointers
    in case they are managed by the source data view */
data_view& data_view::operator = (const data_view& dv)
{
	if (owns_format && format != dv.format)
		delete format;
	format = dv.format;
	owns_format = dv.owns_format;
	const_cast<data_view&>(dv).owns_format = false;

	dim = dv.dim;
	for (int i=0; i<4; ++i)
		step_sizes[i] = dv.step_sizes[i];

	if (owns_ptr && data_ptr && data_ptr != dv.data_ptr)
		delete [] data_ptr; 

	data_ptr = dv.data_ptr;
	owns_ptr = dv.owns_ptr;
	const_cast<data_view&>(dv).owns_ptr = false;
	return *this;
}

const_data_view& const_data_view::operator = (const const_data_view& dv)
{
	format = dv.format;
	owns_format = false;
	dim = dv.dim;
	for (int i=0; i<4; ++i)
		step_sizes[i] = dv.step_sizes[i];
	data_ptr = dv.data_ptr;
	return *this;
}

/** set a different data pointer that will be deleted with the 
    delete [] operator of type (unsigned char*) on destruction 
	 if the manage_ptr flag is true */
void data_view::set_ptr(unsigned char* ptr, bool manage_ptr)
{
	if (owns_ptr && data_ptr && data_ptr != ptr)
		delete [] data_ptr;
	data_ptr = ptr;
	owns_ptr = manage_ptr && ptr != 0;
}
/** set a different data pointer that is not owned by the data view
    and will not be deleted on destruction. */
void data_view::set_ptr(void* ptr)
{
	if (owns_ptr && data_ptr && data_ptr != ptr)
		delete [] data_ptr;
	data_ptr = static_cast<unsigned char*>(ptr);
	owns_ptr = false;
}

/// use base class for construction
const_data_view::const_data_view(const data_format* _format, const unsigned char* _data_ptr, 
					 unsigned int _dim, const unsigned int* _step_sizes)
	: data_view_impl<const_data_view, const unsigned char*>(_format, _data_ptr, _dim, _step_sizes)
{
}

/// construct an empty data view without format and with empty data pointer*/
const_data_view::const_data_view()
{
}

/** construct a data view from the given format, viewing the complete 
	 data set pointed to by the passed data pointer */
const_data_view::const_data_view(const data_format* _format, const void* _data_ptr)
	: data_view_impl<const_data_view, const unsigned char*>(_format, _data_ptr)
{
}

/// copy construct from a non const data view
const_data_view::const_data_view(const data_view& dv) 
	: data_view_impl<const_data_view, const unsigned char*>(
		dv.get_format(), dv.get_ptr<const unsigned char>(), dv.get_dim(),
		dv.step_sizes)
{
}

/// set a different data pointer
void const_data_view::set_ptr(const void* ptr)
{
	data_ptr = static_cast<const unsigned char*>(ptr);
}

/// reflect image at horizontal axis
void data_view::reflect_horizontally()
{
	unsigned int delta = get_step_size(0);
	char* data_ptr = get_ptr<char>();
	char* buffer = new char[delta];
	unsigned H = get_format()->get_height();
	for (unsigned y = 0; y < H/2; ++y) {
		memcpy(buffer, data_ptr + y*delta, delta);
		memcpy(data_ptr + y*delta, data_ptr + (H-y-1)*delta, delta);
		memcpy(data_ptr + (H-y-1)*delta, buffer, delta);
	}
	delete [] buffer;
}
/// combine multiple n-dimensional data views with the same format into a (n+1)-dimensional data view by appending them
bool data_view::compose(data_view& composed_dv, const std::vector<data_view>& dvs)
{
	if(dvs.size() > 0) {
		const data_format* src_df_ptr = dvs[0].format;

		unsigned n_dims = src_df_ptr->get_nr_dimensions();
		const component_format cf = src_df_ptr->get_component_format();
		
		data_format* composed_df = new data_format(*src_df_ptr);

		if(n_dims < 1 || n_dims > 3) {
			std::cerr << "cannot compose data views with " << n_dims << " dimension" << std::endl;
			return false;
		}
		
		switch(n_dims) {
		case 1: composed_df->set_height       ((unsigned)dvs.size()); break;
		case 2: composed_df->set_depth        ((unsigned)dvs.size()); break;
		case 3: composed_df->set_nr_time_steps((unsigned)dvs.size()); break;
		}

		if(composed_dv.empty()) {
			new(&composed_dv) data_view(composed_df);
		} else {
			std::cerr << "cannot compose into a non empty data view" << std::endl;
			return false;
		}

		unsigned char* dst_ptr = composed_dv.get_ptr<unsigned char>();
		unsigned wrong_format_count = 0;

		size_t bytes_per_slice = composed_df->get_nr_bytes() / dvs.size();

		for(size_t i = 0; i < dvs.size(); ++i) {
			const data_view& dv = dvs[i];
			const data_format* df_ptr = dv.get_format();
			unsigned char* src_ptr = dv.get_ptr<unsigned char>();
			size_t n_bytes = df_ptr->get_nr_bytes();

			if(*src_df_ptr != *df_ptr || n_bytes != bytes_per_slice) {
				++wrong_format_count;
				continue;
			}

			memcpy(dst_ptr, src_ptr, n_bytes);
			dst_ptr += n_bytes;
		}
		
		if(wrong_format_count > 0) {
			std::cerr << "skipped " << wrong_format_count << " data views with unmatching formats while composing" << std::endl;
			return false;
		}
		return true;
	}

	return false;
}

bool data_view::combine_components(data_view& dv, const std::vector<data_view>::iterator first, const std::vector<data_view>::iterator last) {

	unsigned n_components = (unsigned)std::distance(first, last);
	if(n_components < 2 || n_components > 4) {
		std::cerr << "cannot combine channels of less than 2 or more than 4 data views" << std::endl;
		return false;
	}
	
	const data_format* src_df_ptr = first->format;

	const component_format src_cf = src_df_ptr->get_component_format();
	if(src_cf.get_nr_components() > 1) {
		std::cerr << "cannot combine components of data views with more than one component" << std::endl;
		return false;
	}

	std::vector<data_view>::iterator it = first;
	for(unsigned i = 1; i < n_components; ++i) {
		const data_format* df_ptr = it->format;

		if(*src_df_ptr != *df_ptr) {
			std::cerr << "cannot combine channels of data views with different formats" << std::endl;
			return false;
		}
		++it;
	}

	unsigned n_dims = src_df_ptr->get_nr_dimensions();
	cgv::type::info::TypeId component_type = src_df_ptr->get_component_type();

	ComponentFormat dst_component_format = CF_R;
	switch(n_components) {
	case 2: dst_component_format = CF_RG;  break;
	case 3: dst_component_format = CF_RGB;  break;
	case 4: dst_component_format = CF_RGBA; break;
	}

	data_format* dst_df_ptr = nullptr;
	unsigned w, h, d, t;
	w = h = d = t = 1;

	unsigned mask_h = 0;
	unsigned mask_d = 0;
	unsigned mask_t = 0;

	switch(n_dims) {
	case 1:
		w = src_df_ptr->get_width();
		dst_df_ptr = new data_format(w, component_type, dst_component_format);
		break;
	case 2:
		w = src_df_ptr->get_width();
		h = src_df_ptr->get_height();
		mask_h = 1;
		dst_df_ptr = new data_format(w, h, component_type, dst_component_format);
		break;
	case 3:
		w = src_df_ptr->get_width();
		h = src_df_ptr->get_height();
		d = src_df_ptr->get_depth();
		mask_h = 1;
		mask_d = 1;
		dst_df_ptr = new data_format(w, h, d, component_type, dst_component_format);
		break;
	case 4:
		w = src_df_ptr->get_width();
		h = src_df_ptr->get_height();
		d = src_df_ptr->get_depth();
		t = src_df_ptr->get_nr_time_steps();
		mask_h = 1;
		mask_d = 1;
		mask_t = 1;
		dst_df_ptr = new data_format(w, h, d, t, component_type, dst_component_format);
		break;
	}

	if(dv.empty()) {
		new(&dv) data_view(dst_df_ptr);
	} else {
		std::cerr << "cannot combine channels into a non empty data view" << std::endl;
		return false;
	}

	//component_type
	unsigned component_size = cgv::type::info::get_type_size(component_type);

	for(unsigned i = 0; i < t; ++i) {
		for(unsigned z = 0; z < d; ++z) {
			for(unsigned y = 0; y < h; ++y) {
				for(unsigned x = 0; x < w; ++x) {

					it = first;
					for(unsigned j = 0; j < n_components; ++j) {
						const data_view& src_dv = (*it);
						unsigned char* src_ptr = src_dv.get_ptr<unsigned char>(x, mask_h*y, mask_d*z, mask_t*i);
						unsigned char* dst_ptr = dv.get_ptr<unsigned char>(x, mask_h*y, mask_d*z, mask_t*i);

						dst_ptr += j * component_size;

						memcpy(dst_ptr, src_ptr, component_size);
						++it;
					}

				}
			}
		}
	}

	return true;
}

template class data_view_impl<data_view,unsigned char*>;
template class data_view_impl<const_data_view,const unsigned char*>;

	}
}
