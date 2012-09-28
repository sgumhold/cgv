#include <cgv/data/data_view.h>
#include <iostream> 
#include <memory.h>

namespace cgv {
	namespace data {

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
template <class D, typename T>
data_view_impl<D,T>::data_view_impl(const data_format* _format, 
			T* _data_ptr, unsigned int _dim, const unsigned int* _step_sizes)
			: data_view_base(_format, _dim, _step_sizes), data_ptr(_data_ptr)
{
}

/// construct a data view from the given format, viewing the complete data set
template <class D, typename T>
data_view_impl<D,T>::data_view_impl(const data_format* _format, 
					typename cgv::type::func::transfer_const<T,void>::type* _data_ptr)
					: data_view_base(_format), data_ptr(static_cast<T*>(_data_ptr))
{
}

/// return whether the data pointer is a null pointer
template <class D, typename T>
bool data_view_impl<D,T>::empty() const 
{
	return data_ptr == 0 || format == 0;
}
/// access to i-th data entry
template <class D, typename T>
D data_view_impl<D,T>::operator () (unsigned int i) const
{
	if (dim == 0) {
		std::cerr << "1d operator access to 0d data ptr" << std::endl;
		return D();
	}
	return D(format, data_ptr+i*step_sizes[0], 
			   (unsigned int) (dim-1), step_sizes+1);
}
/// access to entry at (i,j)
template <class D, typename T>
D data_view_impl<D,T>::operator () (unsigned int i, unsigned int j) const
{
	if (dim < 2) {
		std::cerr << "2d operator access to " << dim << "d data ptr" << std::endl;
		return D();
	}
	return D(format, data_ptr+i*step_sizes[0]+j*step_sizes[1],
			   (unsigned int) (dim-2),step_sizes+2);
}
/// access to entry at (i,j,k)
template <class D, typename T>
D data_view_impl<D,T>::operator () (unsigned int i, unsigned int j, unsigned int k) const
{
	if (dim < 3) {
		std::cerr << "3d operator access to " << dim << "d data ptr" << std::endl;
		return D();
	}
	return D(format, data_ptr+i*step_sizes[0]+j*step_sizes[1]+k*step_sizes[2],
			   (unsigned int) (dim-3),step_sizes+3);
}
/// access to entry at (i,j,k,l)
template <class D, typename T>
D data_view_impl<D,T>::operator () (unsigned int i, unsigned int j, unsigned int k, unsigned int l) const
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
template <class D, typename T>
D data_view_impl<D,T>::permute(const std::string& permutation) const
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
	: data_view_impl<data_view, unsigned char>(_format, _data_ptr, _dim, _step_sizes),
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
	: data_view_impl<data_view, unsigned char>(_format),
	  owns_ptr(false)
{
	if (_format) {
		data_ptr = new unsigned char[_format->get_size()*_format->get_entry_size()];
		owns_ptr = true;
	}
}
/** construct a data view from the given format, viewing the complete 
    data set. The passed pointer will not be owned by the view. */
data_view::data_view(const data_format* _format, void* _data_ptr)
	: data_view_impl<data_view, unsigned char>(_format, _data_ptr),
	  owns_ptr(false)
{
}
/** construct a data view from the given format, viewing the complete 
    data set. The passed pointer will be owned by the view if the
	 manage_ptr flag is true. In this case the pointer is deleted on
	 destruction with the delete [] operator of type (unsigned char*). */
data_view::data_view(const data_format* _format, unsigned char* _data_ptr, 
							bool manage_ptr) 
	: data_view_impl<data_view, unsigned char>(_format, _data_ptr),
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
	: data_view_impl<const_data_view, const unsigned char>(_format, _data_ptr, _dim, _step_sizes)
{
}

/// construct an empty data view without format and with empty data pointer*/
const_data_view::const_data_view()
{
}

/** construct a data view from the given format, viewing the complete 
	 data set pointed to by the passed data pointer */
const_data_view::const_data_view(const data_format* _format, const void* _data_ptr)
	: data_view_impl<const_data_view, const unsigned char>(_format, _data_ptr)
{
}

/// copy construct from a non const data view
const_data_view::const_data_view(const data_view& dv) 
	: data_view_impl<const_data_view, const unsigned char>(
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


template class data_view_impl<data_view,unsigned char>;
template class data_view_impl<const_data_view,const unsigned char>;

	}
}
