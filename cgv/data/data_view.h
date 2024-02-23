#pragma once

#include <cgv/type/info/type_ptr.h>
#include <cgv/type/func/transfer_const.h>
#include <cgv/type/func/drop_pointer.h>
#include <cgv/data/data_format.h>

#include "lib_begin.h"

namespace cgv {
	namespace data {

/** base class of both implementations of the data view managing the
    component format, the dimension of the view and the step sizes */
class CGV_API data_view_base
{
protected:
	const data_format* format;
	/// whether to own the data format
	bool owns_format;
	unsigned int dim;
	size_t step_sizes[4];
	/// constructor used to construct sub views onto the data view
	data_view_base(const data_format* _format, unsigned int _dim, const size_t* _step_sizes);
public:
	/** construct the base of a data view from the given format, 
		 such that the step sizes and dimension are set to view
		 the complete data set defined in the format. */
	data_view_base(const data_format* _format = 0);
	/// delete format if it is owned
	virtual ~data_view_base();
	/// whether to manage the data format pointer
	void manage_format(bool enable = true);
	/// return the component format
	const data_format* get_format() const;
	/// set a new data format
	void set_format(const data_format* _format);
	/// return the dimension of the data view, which is less or equal to the dimension of the data format
	unsigned int get_dim() const;
	/// return the step size in bytes in the i-th dimension
	size_t get_step_size(unsigned int dim) const;
};

/** template class implementing the part of the view that depends on whether
    the pointer is const or not and manages the data pointer itself. */
template <class D, typename P = unsigned char*>
class CGV_API data_view_impl : public data_view_base
{
protected:
	/// data pointer of type unsigned char or const unsigned char
	P data_ptr;
	/// constructor used to construct sub views onto the data view
	data_view_impl(const data_format* _format, P _data_ptr, unsigned _dim, const size_t* _step_sizes);
public:
	/// construct a data view from the given format, viewing the complete data set
	data_view_impl(const data_format* _format = 0, typename cgv::type::func::transfer_const<P,void*>::type _data_ptr = 0);
	/// return whether the data pointer is a null pointer
	bool empty() const;
	/// return a data pointer to type S
	template <typename S>
	typename cgv::type::func::transfer_const<P,S*>::type get_ptr() const { 
		return (typename cgv::type::func::transfer_const<P,S*>::type)(data_ptr); 
	}
	/// return a pointer to type S for i-th data entry
	template <typename S>
	typename cgv::type::func::transfer_const<P, S*>::type get_ptr(size_t i) const {
		return (typename cgv::type::func::transfer_const<P, S*>::type)(data_ptr + i*step_sizes[0]);
	}
	/// return a pointer to type S for (i,j)-th data entry
	template <typename S>
	typename cgv::type::func::transfer_const<P, S*>::type get_ptr(size_t i, size_t j) const {
		return (typename cgv::type::func::transfer_const<P, S*>::type)(data_ptr + i*step_sizes[0] + j*step_sizes[1]);
	}
	/// return a pointer to type S for (i,j,k)-th data entry
	template <typename S>
	typename cgv::type::func::transfer_const<P, S*>::type get_ptr(size_t i, size_t j, size_t k) const {
		return (typename cgv::type::func::transfer_const<P, S*>::type)(data_ptr + i*step_sizes[0] + j*step_sizes[1] + k*step_sizes[2]);
	}
	/// return a pointer to type S for (i,j,k,l)-th data entry
	template <typename S>
	typename cgv::type::func::transfer_const<P, S*>::type get_ptr(size_t i, size_t j, size_t k, size_t l) const {
		return (typename cgv::type::func::transfer_const<P, S*>::type)(data_ptr + i*step_sizes[0] + j*step_sizes[1] + k*step_sizes[2] + l*step_sizes[3]);
	}
	/// constant access to the ci-th component
	template <typename S>
	S get(unsigned ci) const {
		return format->get<S>(ci, data_ptr);
	}
	/// constant access to the ci-th component of i-th data entry
	template <typename S> S get(unsigned ci, size_t i) const {
		return format->get<S>(ci, get_ptr<cgv::type::func::drop_pointer<P>::type>(i));
	}
	/// constant access to the ci-th component of (i,j)-th data entry
	template <typename S> S get(unsigned ci, size_t i, size_t j) const {
		return format->get<S>(ci, get_ptr<cgv::type::func::drop_pointer<P>::type>(i, j));
	}
	/// constant access to the ci-th component of (i,j,k)-th data entry
	template <typename S> S get(unsigned ci, size_t i, size_t j, size_t k) const {
		return format->get<S>(ci, get_ptr<cgv::type::func::drop_pointer<P>::type>(i,j,k));
	}
	/// constant access to the ci-th component of (i,j,k,l)-th data entry
	template <typename S> S get(unsigned ci, size_t i, size_t j, size_t k, size_t l) const {
		return format->get<S>(ci, get_ptr<cgv::type::func::drop_pointer<P>::type>(i, j, k, l));
	}
	/// access to i-th data entry
	D operator () (size_t i) const;
	/// access to entry at (i,j)
	D operator () (size_t i, size_t j) const;
	/// access to entry at (i,j,k)
	D operator () (size_t i, size_t j, size_t k) const;
	/// access to entry at (i,j,k,l)
	D operator () (size_t i, size_t j, size_t k, size_t l) const;

	/** permute the order of the indices, where the permutation argument "kji" implies that after 
	    the permutation the operator (i,j,k) returns the same as the operator (k,j,i) before the
		call to permute. The permutation string must have at least two entries. If it has n entries
		it must contain each of the first n letters of "ijkl" exactly once, i.e. "ik" would be invalid,
		whereas "ikj" is a valid permutation. */
	D permute(const std::string& permutation) const;
	/// transpose is equivalent to permute("ji")
	D transpose() const { return permute("ji"); }

	/// return a pointer that points to the n-th next location if index i is increase by n
	template <typename S>
	typename cgv::type::func::transfer_const<P,S*>::type 
		step_i(S* ptr, std::ptrdiff_t n=1) const { return static_cast<typename cgv::type::func::transfer_const<P,S*>::type>(static_cast<P>(ptr)+n*step_sizes[0]); }
	/// return a pointer that points to the n-th next location if index j is increase by n
	template <typename S>
	typename cgv::type::func::transfer_const<P,S*>::type 
		step_j(S* ptr, std::ptrdiff_t n=1) const { return static_cast<typename cgv::type::func::transfer_const<P,S*>::type>(static_cast<P>(ptr)+n*step_sizes[1]); }
	/// return a pointer that points to the n-th next location if index k is increase by n
	template <typename S>
	typename cgv::type::func::transfer_const<P,S*>::type 
		step_k(S* ptr, std::ptrdiff_t n=1) const { return static_cast<typename cgv::type::func::transfer_const<P,S*>::type>(static_cast<P>(ptr)+n*step_sizes[2]); }
	/// return a pointer that points to the n-th next location if index l is increase by n
	template <typename S>
	typename cgv::type::func::transfer_const<P,S*>::type 
		step_l(S* ptr, std::ptrdiff_t n=1) const { return static_cast<typename cgv::type::func::transfer_const<P,S*>::type>(static_cast<P>(ptr)+n*step_sizes[3]); }
};

class CGV_API const_data_view;

/** the data view gives access to a data array of one, two, three or four
    dimensions. Each data entry can consist of several components as defined
	 in the referenced component format. It allows to permute the dimensions, 
	 construct views of lower dimension with the ()-operators defined in the
	 data_view_impl and to access the data components with the get- and
	 set-method of data_view_impl.
	 It keeps a flag that tells whether the data pointer belongs to the 
	 data view and therefore needs to be deleted on destruction.
*/
class CGV_API data_view : public data_view_impl<data_view, unsigned char*>
{
protected:
	/// declare base as friend
	friend class data_view_impl<data_view, unsigned char*>;
	friend class const_data_view;
	/// a flag telling whether the data ptr is owned by the view
	bool owns_ptr;
	/// use base class for construction and don't manage data pointer 
	data_view(const data_format* _format, unsigned char* _data_ptr, 
				 unsigned int _dim, const size_t* _step_sizes);
public:
	/// construct an empty data view without format and with empty data pointer*/
	data_view();
	/// destruct view and delete data pointer if it is owned by the view
	~data_view();
	/** construct a data view from the given format. Allocate a new data
	    pointer with the new [] operator of type (unsigned char) and own
		 the pointer. The data_view will view the complete data set as defined
		 in the format. */
	data_view(const data_format* _format);
	/** construct a data view from the given format, viewing the complete 
	    data set. The passed pointer will not be owned by the view. */
	data_view(const data_format* _format, void* _data_ptr);
	/** construct a data view from the given format, viewing the complete 
	    data set. The passed pointer will be owned by the view if the
		 manage_ptr flag is true. In this case the pointer is deleted on
		 destruction with the delete [] operator of type (unsigned char*). */
	data_view(const data_format* _format, unsigned char* _data_ptr, bool manage_ptr);
	/** construct a data view as a copy of the given data view. Copy the
		data and own the pointer. The data_view will view the complete data
		set as defined in the format. */
	data_view(const data_view& other) : data_view(other.format) {
		const cgv::type::uint8_type* src_ptr = other.get_ptr<cgv::type::uint8_type>();
		cgv::type::uint8_type* dst_ptr = get_ptr<cgv::type::uint8_type>();
		memcpy(dst_ptr, src_ptr, other.format->get_nr_bytes());
	}
	/** the assignment operator takes over the data format and data pointers
	    in case they are managed by the source data view */
	data_view& operator = (const data_view& dv);
	/** return a copy of this data_view and tha data via the copy constructor */
	data_view copy() {
		return data_view(*this);
	}
	/** set a different data pointer that will be deleted with the 
	    delete [] operator of type (unsigned char*) on destruction 
		 if the manage_ptr flag is true */
	void set_ptr(unsigned char* ptr, bool manage_ptr);
	/** set a different data pointer that is not owned by the data view
	    and will not be deleted on destruction. */
	void set_ptr(void* ptr);
	/// write access to the i-th component, return whether write was successful
	template <typename T>
	bool set(int ci, const T& v) {
		return format->set(ci, data_ptr, v);
	}
	/// reflect 2D data view at horizontal axis
	void reflect_horizontally();
	/// combine multiple n-dimensional data views with the same format into a (n+1)-dimensional data view by appending them
	static bool compose(data_view& composed_dv, const std::vector<data_view>& dvs);
	/// combine n data views each with one component channel into a single data view with n component channels, the format of the input data views needs to match
	static bool combine_components(data_view& dv, const std::vector<data_view>::iterator first, const std::vector<data_view>::iterator last);
};

/** The const_data_view has the functionality of the data_view but 
    uses a const pointer and therefore does not allow to manage the
	 pointer nor to set the data.
*/
class CGV_API const_data_view : public data_view_impl<const_data_view, const unsigned char*>
{
protected:
	/// declare base as friend
	friend class data_view_impl<const_data_view, const unsigned char*>;
	/// use base class for construction
	const_data_view(const data_format* _format, const unsigned char* _data_ptr, 
						 unsigned _dim, const size_t* _step_sizes);
public:
	/// construct an empty data view without format and with empty data pointer*/
	const_data_view();
	/// copy construct from a non const data view
	const_data_view(const data_view& dv);
	/** construct a data view from the given format, viewing the complete 
		 data set pointed to by the passed data pointer */
	const_data_view(const data_format* _format, const void* _data_ptr);
	/// assignment of const_data_view never gains ownership of format or data
	const_data_view& operator = (const const_data_view& dv);
	/// set a different data pointer
	void set_ptr(const void* ptr);
};

	}
}

#include <cgv/config/lib_end.h>
