#pragma once

#include <vector>
#include <cgv/type/info/type_name.h>
#include <cgv/utils/convert.h>
#include <cgv/utils/token.h>
#include <cgv/type/traits/method_pointer.h>
#include <cgv/type/info/type_id.h>

#include "self_reflection_tag.h"
#include "reflection_traits_info.h"

#include "lib_begin.h"

/// the cgv namespace
namespace cgv {
	/// in this namespace reflection of types is implemented
	namespace reflect {

/** abstract interface to call a method of a given instance. For simplicity all parameters 
    are passed as const pointers. In case the parameter should also be written by the method,
	a const_cast needs to be used. */
struct method_interface
{
	virtual void call_void(void* instance, 
						   const std::vector<abst_reflection_traits*>& param_value_traits,
						   const std::vector<const void*>& param_value_ptrs,
						   const std::vector<std::string>& param_type_names,
						   const abst_reflection_traits* result_traits = 0,
						   void* result_value_ptr = 0,
						   const std::string& result_type = "") = 0;
};

template <int i, int n, typename M>
struct method_parameter_traits_helper
{
	static void fill_vector(std::vector<abst_reflection_traits*>& param_value_traits)
	{
		param_value_traits.push_back(reflection_traits_info<typename cgv::type::traits::method_pointer_argument_list<M,i>::type>::traits_type().clone());
		if (i+1 < n)
			method_parameter_traits_helper<i+1,n,M>::fill_vector(param_value_traits);
	}
};

template <int i, typename M>
struct method_parameter_traits_helper<i,i,M>
{
	static void fill_vector(std::vector<abst_reflection_traits*>& param_value_traits)
	{
	}
};

/// forward declaration of method_interface_impl which is implemented in <cgv/reflect/method_interface_impl.ph>, include the generated header <cgv/reflect/method_interface_impl.h>
template <typename M>
struct method_interface_impl;

/** the self reflection handler is passed to the virtual self_reflect() method
    of cgv::base::base. It is used to process type information by describing 
	member variables and function (methods) to the handler with its templated
	methods reflect_member() and reflect_method() or the non templated versions
	with the suffix _void. */
class CGV_API reflection_handler
{
#ifndef REFLECT_TRAITS_WITH_DECLTYPE
	template <typename B, typename RB>
	bool reflect_base_impl(B& base_ref, const RB&);
	template <typename T, unsigned n, typename RT>
	bool reflect_const_array_impl(const std::string& member_name, T (&member_ref)[n], const RT&);
	template <typename T, typename RT>
	bool reflect_vector_impl(const std::string& member_name, std::vector<T>& member_ref, const RT&);
#endif
public:
	template <typename T>
	friend bool reflect_enum(reflection_handler& rh, const std::string& name, T& instance, const std::string& declarations);
	template <typename T>
	friend bool reflect_string(reflection_handler& rh, const std::string& name, T& instance);
	friend struct detail;
	/**@ basic types with helper functions */
	//@{
	/// different values for group traversal strategy
	enum GroupTraversal { GT_TERMINATE = -3, GT_SKIP = -2, GT_COMPLETE = -1 };
	/// return the group traversals as a string
	static std::string group_traversal_name(GroupTraversal gt);
	/// different support group types
	enum GroupKind { GK_NO_GROUP, GK_BASE_CLASS, GK_STRUCTURE, GK_VECTOR, GK_ARRAY, GK_POINTER };
	/// return the group kind as a string
	static const char* group_kind_name(GroupKind gk);
	/// check whether a group kind is of array or vector kind
	static bool is_array_kind(GroupKind gk);
	//@}
protected:
	/**@name internal helper functions*/
	//@{
	/// type independent part of the reflect_group method that starts the group traversal
	GroupTraversal process_structural_group_begin(GroupKind gk, const std::string& member_name, GroupTraversal gt);
	/// updates the nesting info at the end of group and always returns true
	bool group_end(GroupKind gk);
	/// implementation of reflection with internal or external self_reflect function
	template <typename T, typename RT, typename D>
	bool self_reflect_member(const std::string& member_name, T& member_ref, const RT&, const D&, bool hard_cast)
	{
		RT rt;
		switch (process_structural_group_begin(GK_STRUCTURE, member_name, GroupTraversal(reflect_group_begin(GK_STRUCTURE, member_name, &member_ref, &rt)))) {
		case GT_TERMINATE : return false;
		case GT_COMPLETE : 
			if (hard_cast) 
				return static_cast<D&>(member_ref).D::self_reflect(*this) && group_end(GK_STRUCTURE);
			else {
				D& d = static_cast<D&>(member_ref);
				return d.self_reflect(*this) && group_end(GK_STRUCTURE);   
			}
		default: return true;
		}		
	}
	/// type independent functionality of array reflection
	int reflect_array_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, abst_reflection_traits* rt, unsigned grp_size);
	//@}

	/**@name management of nesting information*/
	//@{
	/// for each nesting during traversal a nesting info is pushed back to the end of the nesting info stack
	struct nesting_info
	{
		/// group kind of nesting level
		GroupKind group_kind;
		/// pointer to type name for base class groups and member name otherwise
		const std::string* name;
		/// current element index for vector and array groups
		unsigned idx;
		///
		nesting_info(GroupKind _group_kind = GK_NO_GROUP, const std::string* _name = 0, unsigned _idx = 0) : group_kind(_group_kind), name(_name), idx(_idx) {};
	};
	/// stack of nesting_info used during the reflection process
	std::vector<nesting_info> nesting_info_stack;
	//@}

	/**@name interface to be implemented by derived reflection handlers*/
	//@{
	//! abstract interface to start reflection of a group of members.
	/*! The return value gives information about further traversal of the group:
	    - GT_TERMINATE ... terminate traversal completely
		- GT_SKIP      ... skip traversal of group
	    - GT_COMPLETE  ... traverse group completely
		- index i >= 0 ... traverse only the i-th member of the group.
		Depending on the group kind, the remaining parameters are given as follows:
		- GK_BASE_CLASS ... group_name and grp_size are undefined, group_ptr is pointer to instance of base class, rt reflects base class
		- GK_STRUCTURE  ... group_name is instance name, group_ptr is pointer to instance, rt reflects type of structure, grp_size is undefined, 
		- GK_VECTOR ... group_name is vector name, group_ptr is pointer to vector data structure, rt reflects element type, grp_size is size of vector
		- GK_ARRAY ... group_name is array name, group_ptr is pointer to first array element, rt reflects element type, grp_size is size of array.
		- GK_POINTER ... group_name is pointer name, group_ptr is pointer to first array element, rt reflects type to which pointer poits, grp_size is undefined. */
	virtual int reflect_group_begin(GroupKind group_kind, const std::string& group_name, void* group_ptr, abst_reflection_traits* rt, unsigned grp_size = -1);
	//! abstract interface to terminate reflection of a group of members
	/*! This function should only be called if group is traversed completely. */
	virtual void reflect_group_end(GroupKind group_kind);
	/** abstract interface to reflect a member variable, where the member type is specified
	    as a string. Returns whether to continue the reflection. */
	virtual bool reflect_member_void(const std::string& member_name, void* member_ptr, abst_reflection_traits* rt) = 0;
	/** abstract interface to reflect a method, where return and parameter types are specified
	    as strings. Returns whether to continue the reflection. */
	virtual bool reflect_method_void(const std::string& method_name, method_interface* mi_ptr,
									 abst_reflection_traits* return_traits, const std::vector<abst_reflection_traits*>& param_value_traits) = 0;
	//@}

public:
	/// declare virtual destructor
	virtual ~reflection_handler();

	/**@name interface used by reflect and self_reflect functions*/
	//@{
	/// give information on whether reflection_handler creates object (defaults to false)
	virtual bool is_creative() const;
	/** call this to reflect a member by member name and reference to the member. The
	    member type is deduced from the reference via templates. The method uses reflect_member_impl 
		to dispath types with implementation of a self_reflect method and types without. For polymorphic
		objects with a polymorphic self_reflect() method the parameter \c hard_cast steers whether the
		concrete implementation T::self_reflect() is used or the overloaded function member_ref.self_reflect().
		This is important for self reflection of base classes in polymorphic objects where hard_cast is set
		to true. In most other cases one can use the default argument false. */
	template <typename T>
	bool reflect_member(const std::string& member_name, T& member_ref, bool hard_cast = false);
	/** call this to reflect a method by method name and reference to the member. The
	    method type is deduced from the reference via templates. This only works, if you
		additionally include the header <cgv/reflect/method_interface_impl.h>, where the 
		template code is located. This header is not included automatically, because of 
		its length. 
		
		To give names to the parameters, append them to the name enclosed in parenthesis, 
		i.e. "sin(x)" or "dot_prod(p,q)". */
	template <typename M>
	bool reflect_method(const std::string& method_name, M m);
	/// reflect a base class with its members
	template <typename B>
	bool reflect_base(B& base_ref);
	/// reflect a member of constant size array type
	template <typename T, unsigned n>
	bool reflect_member(const std::string& member_name, T (&member_ref)[n]);
	/// reflect a member of vector type
	template <typename T>
	bool reflect_member(const std::string& member_name, std::vector<T>& member_ref);
	//! reflect a member of pointer type.
	/*! This is only a minimal implementation that allows pointers to memory allocated with new.
	    No reference counting or pointers into memory ranges allocated differently are supported.
		This will be later on. */
	template <typename T>
	bool reflect_member(const std::string& member_name, T*& member_ref);
	/// reflect a dynamic array  member of vector type
	template <typename T, typename S>
	bool reflect_array(const std::string& member_name, T*& member_ref, S& size);
};


struct detail {
#ifndef REFLECT_TRAITS_WITH_DECLTYPE
		template <typename T, ReflectionTraitsKind K> 
		struct reflect_member_impl
		{
			template <bool has_external, typename RT>
			struct reflect_impl {
				static bool reflect(reflection_handler* rh, const std::string& member_name, T& member_ref, const RT&, bool hard_cast) {
					RT rt;
					return rh->reflect_member_void(member_name, &member_ref, &rt);
				}
			};
			template <typename RT>
			struct reflect_impl<true,RT> {
				static bool reflect(reflection_handler* rh, const std::string& member_name, T& member_ref, const RT&, bool hard_cast) {
					RT rt;
					return rh->self_reflect_member(member_name, member_ref, rt, static_cast<typename RT::external_self_reflect_type&>(member_ref), hard_cast);
				}
			};
			template <typename RT>
			static bool reflect_RT(reflection_handler* rh, const std::string& member_name, T& member_ref, const RT& rt, bool hard_cast) {
				return reflect_impl<RT::has_external,RT>::reflect(rh, member_name, member_ref, rt, hard_cast);	
			}
			static bool reflect(reflection_handler* rh, const std::string& member_name, T& member_ref, bool hard_cast) {
				return reflect_RT(rh, member_name, member_ref, get_reflection_traits(member_ref), hard_cast);
			}
		};

		template <typename T> struct reflect_member_impl<T,RTK_STD_TYPE> 
		{
			static bool reflect(reflection_handler* rh, const std::string& member_name, T& member_ref, bool hard_cast) {
				typename reflection_traits_info<T>::traits_type rt;
				return rh->reflect_member_void(member_name, &member_ref, &rt);
			}
		};
#else
	template <typename T, ReflectionTraitsKind K> 
	struct reflect_member_impl
	{
		static bool reflect(reflection_handler* rh, const std::string& member_name, T& member_ref, bool hard_cast) {
			typename reflection_traits_info<T>::traits_type rt;
			return rh->reflect_member_void(member_name, &member_ref, &rt);
		}
	};

	template <typename T> struct reflect_member_impl<T,RTK_EXTERNAL_SELF_REFLECT> 
	{
		static bool reflect(reflection_handler* rh, const std::string& member_name, T& member_ref, bool hard_cast) {
			typename reflection_traits_info<T>::traits_type rt;
			return rh->self_reflect_member(member_name, member_ref, rt, static_cast<typename reflection_traits_info<T>::traits_type::external_self_reflect_type&>(member_ref), hard_cast);
		}
	};
#endif
	template <typename T> struct reflect_member_impl<T,RTK_SELF_REFLECT> 
	{
		static bool reflect(reflection_handler* rh, const std::string& member_name, T& member_ref, bool hard_cast) {
			typename reflection_traits_info<T>::traits_type rt;
			return rh->self_reflect_member(member_name, member_ref, rt, member_ref, hard_cast);
		}
	};

	template <typename M, typename R>
	struct reflect_method_impl {
		static bool reflect(reflection_handler* rh, const std::string& method_name, M m)
		{
			static std::vector<abst_reflection_traits*> param_value_traits;
			static method_interface_impl<M> mi(m);
			if (param_value_traits.size() != cgv::type::traits::method_pointer<M>::nr_arguments)
				method_parameter_traits_helper<0,cgv::type::traits::method_pointer<M>::nr_arguments,M>::fill_vector(param_value_traits);
			typename reflection_traits_info<R>::traits_type rt;
			return rh->reflect_method_void(method_name, &mi, &rt, param_value_traits);
		}
	};
	template <typename M>
	struct reflect_method_impl<M,void> {
		static bool reflect(reflection_handler* rh, const std::string& method_name, M m)
		{
			static std::vector<abst_reflection_traits*> param_value_traits;
			static method_interface_impl<M> mi(m);
			if (param_value_traits.size() != cgv::type::traits::method_pointer<M>::nr_arguments)
				method_parameter_traits_helper<0,cgv::type::traits::method_pointer<M>::nr_arguments,M>::fill_vector(param_value_traits);
			return rh->reflect_method_void(method_name, &mi, 0, param_value_traits);
		}
	};

#ifndef REFLECT_TRAITS_WITH_DECLTYPE
	template <bool use_get, typename B>
	struct reflect_base_dispatch {         static bool reflect(reflection_handler* rh, B& base_ref) { 
		return rh->reflect_base_impl(base_ref, typename reflection_traits_info<B>::traits_type()); } };
	template <typename B>
	struct reflect_base_dispatch<true,B> { static bool reflect(reflection_handler* rh, B& base_ref) { 
		return rh->reflect_base_impl(base_ref, get_reflection_traits(base_ref)); } };

	template <bool use_get, typename T, unsigned n>
	struct reflect_const_array_dispatch {         static bool reflect(reflection_handler* rh, const std::string& member_name, T (&member_ref)[n]) { 
		return rh->reflect_const_array_impl(member_name, member_ref, typename reflection_traits_info<T>::traits_type()); } };
	template <typename T, unsigned n>
	struct reflect_const_array_dispatch<true,T,n> {         static bool reflect(reflection_handler* rh, const std::string& member_name, T (&member_ref)[n]) { 
		return rh->reflect_const_array_impl(member_name, member_ref, get_reflection_traits(T())); } };

	template <bool use_get, typename T>
	struct reflect_vector_dispatch {         static bool reflect(reflection_handler* rh, const std::string& member_name, std::vector<T>& member_ref) { 
		return rh->reflect_vector_impl(member_name, member_ref, typename reflection_traits_info<T>::traits_type()); } };
	template <typename T>
	struct reflect_vector_dispatch<true,T> {         static bool reflect(reflection_handler* rh, const std::string& member_name, std::vector<T>& member_ref) { 
		return rh->reflect_vector_impl(member_name, member_ref, get_reflection_traits(T())); } };
#endif
};


template <typename T>
bool reflection_handler::reflect_member(const std::string& member_name, T& member_ref, bool hard_cast)
{
#ifdef REFLECT_TRAITS_WITH_DECLTYPE
	return detail::reflect_member_impl<T, reflection_traits_info<T>::kind>::reflect(this, member_name, member_ref, hard_cast);
#else
	return detail::reflect_member_impl<T, reflection_traits_info<T>::kind>::reflect(this, member_name, member_ref, hard_cast);
#endif
}

template <typename M>
bool reflection_handler::reflect_method(const std::string& method_name, M m)
{
	return detail::reflect_method_impl<M,typename cgv::type::traits::method_pointer<M>::return_type>::reflect(this, method_name, m);
}

#ifdef REFLECT_TRAITS_WITH_DECLTYPE
template <typename B>
bool reflection_handler::reflect_base(B& base_ref)
{
	typename reflection_traits_info<B>::traits_type rt;
#else
template <typename B, typename RB>
bool reflection_handler::reflect_base_impl(B& base_ref, const RB&)
{
	RB rt;
#endif
	switch (process_structural_group_begin(GK_BASE_CLASS, "", GroupTraversal(reflect_group_begin(GK_BASE_CLASS, "", &base_ref, &rt)))) {
	case GT_TERMINATE : return false;
	case GT_COMPLETE : return reflect_member("", base_ref, true) && group_end(GK_BASE_CLASS);
	default: return true;
	}
}
#ifndef REFLECT_TRAITS_WITH_DECLTYPE
template <typename B>
bool reflection_handler::reflect_base(B& base_ref)
{
	return detail::reflect_base_dispatch<reflection_traits_info<B>::use_get,B>::reflect(this,base_ref);
}
#endif





#ifdef REFLECT_TRAITS_WITH_DECLTYPE
template <typename T, unsigned n>
bool reflection_handler::reflect_member(const std::string& member_name, T (&member_ref)[n])
{
	typename reflection_traits_info<T>::traits_type rt;
#else
template <typename T, unsigned n, typename RT>
bool reflection_handler::reflect_const_array_impl(const std::string& member_name, T (&member_ref)[n], const RT&)
{
	RT rt;
#endif
	int grp_tra = reflect_array_begin(GK_ARRAY, member_name, member_ref, &rt, n);
	if (grp_tra == GT_TERMINATE || grp_tra == GT_SKIP)
		return grp_tra == GT_SKIP;
	bool res = true;
	if (grp_tra == GT_COMPLETE) {
		for (nesting_info_stack.back().idx=0; res && nesting_info_stack.back().idx<n; ++nesting_info_stack.back().idx)
			res = reflect_member("", member_ref[nesting_info_stack.back().idx]);
		group_end(GK_ARRAY);
	}
	else {
		res = reflect_member("", member_ref[grp_tra]);
		nesting_info_stack.pop_back();
	}
	return res;
}
#ifndef REFLECT_TRAITS_WITH_DECLTYPE
template <typename T, unsigned n>
bool reflection_handler::reflect_member(const std::string& member_name, T (&member_ref)[n])
{
	return detail::reflect_const_array_dispatch<reflection_traits_info<T>::use_get,T,n>::reflect(this,member_name,member_ref);
}
#endif

#ifdef REFLECT_TRAITS_WITH_DECLTYPE
template <typename T>
bool reflection_handler::reflect_member(const std::string& member_name, std::vector<T>& member_ref)
{
	typename reflection_traits_info<T>::traits_type rt;
#else
template <typename T, typename RT>
bool reflection_handler::reflect_vector_impl(const std::string& member_name, std::vector<T>& member_ref, const RT&)
{
	RT rt;
#endif
	int grp_tra = reflect_array_begin(GK_VECTOR, member_name, &member_ref, &rt, member_ref.size());
	if (grp_tra == GT_TERMINATE || grp_tra == GT_SKIP)
		return grp_tra == GT_SKIP;
	bool res = true;
	if (grp_tra == GT_COMPLETE) {
		unsigned size = member_ref.size();
		res = reflect_member("size", size);
		if (member_ref.size() != size)
			member_ref.resize(size);
		for (nesting_info_stack.back().idx=0; res && nesting_info_stack.back().idx<size; ++nesting_info_stack.back().idx)
			res = reflect_member("", member_ref[nesting_info_stack.back().idx]);
		group_end(GK_VECTOR);
	}
	else {
		res = reflect_member("", member_ref[grp_tra]);
		nesting_info_stack.pop_back();
	}
	return res;
}
#ifndef REFLECT_TRAITS_WITH_DECLTYPE
template <typename T>
bool reflection_handler::reflect_member(const std::string& member_name, std::vector<T>& member_ref)
{
	return detail::reflect_vector_dispatch<reflection_traits_info<T>::use_get,T>::reflect(this, member_name, member_ref);
}
#endif
//! reflect a member of pointer type.
/*! This is only a minimal implementation that allows pointers to memory allocated with new.
    No reference counting or pointers into memory ranges allocated differently are supported.
	This will be later on. */
template <typename T>
bool reflection_handler::reflect_member(const std::string& member_name, T*& member_ref)
{
	typename reflection_traits_info<T>::traits_type rt;
	GroupTraversal gt = reflect_group_begin(GK_POINTER, member_name, &member_ref, &rt);
	switch (gt) {
	case GT_TERMINATE : return false;
	case GT_SKIP : return true;
	case GT_COMPLETE : break;
	default: 
		std::cerr << "group traversal " << group_traversal_name(gt) << " not allowed on pointer group " << member_name << "!" << std::endl;
		return false;
	}
	nesting_info_stack.push_back(nesting_info(GK_POINTER, &member_name));
	unsigned pointer_type = (member_ref == 0 ? 0 : 1);
	unsigned pointer_type_tmp = pointer_type;
	bool res = reflect_member("pointer_type", pointer_type_tmp);
	if (res) {
		if (pointer_type_tmp != pointer_type) {
			if (pointer_type != 0)
				delete member_ref;
			if (pointer_type_tmp == 0)
				member_ref = 0;
			else
				member_ref = new T();
		}
		if (pointer_type_tmp != 0)
			res = reflect_member(*member_ref);
	}
	group_end(GK_POINTER);
	return res;
}
/// reflect a dynamic array  member of vector type
template <typename T, typename S>
bool reflection_handler::reflect_array(const std::string& member_name, T*& member_ref, S& size)
{
	typename reflection_traits_info<T>::traits_type rt;
	int grp_tra = reflect_array_begin(GK_ARRAY, member_name, member_ref, &rt, size);
	if (grp_tra == GT_TERMINATE || grp_tra == GT_SKIP)
		return grp_tra == GT_SKIP;
	bool res = true;
	if (grp_tra == GT_COMPLETE) {
		unsigned tmp_size = size;
		res = reflect_member("size", tmp_size);
		if (size != tmp_size) {
			T* tmp = member_ref;
			member_ref = new T[tmp_size];
			if (tmp) {
				for (unsigned i=0; i<tmp_size && i < size; ++i)
					member_ref[i] = tmp[i];
				delete [] tmp;
			}
			size = tmp_size;
		}
		for (nesting_info_stack.back().idx=0; res && nesting_info_stack.back().idx<size; ++nesting_info_stack.back().idx)
			res = reflect_member("", member_ref[nesting_info_stack.back().idx]);
		group_end(GK_ARRAY);
	}
	else {
		res = reflect_member("", member_ref[grp_tra]);
		nesting_info_stack.pop_back();
	}
	return res;
}

	}
}

#include <cgv/config/lib_end.h>
