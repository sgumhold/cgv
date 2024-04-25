#pragma once

#include "view.h"

#include <cgv/signal/signal.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// %gui and %type independent %base class of all controls
class CGV_API abst_control : public abst_view
{
public:
	/// construct from name
	abst_control(const std::string& name);
	/// add default implementation passing the query to the controls() method
	bool shows(const void* ptr) const;
	/// return whether the control controls the value pointed to by ptr
	virtual bool controls(const void* ptr) const = 0;
	/// attach a functor to the value change signal
	virtual void attach_to_value_change(cgv::signal::functor_base* func) = 0;
	/// attach a functor to the value change signal
	virtual void attach_to_check_value(cgv::signal::functor_base* bool_func) = 0;
};

/// ref counted pointer to abst %control
typedef data::ref_ptr<abst_control> control_ptr;

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<abst_control>;
#endif

/// %type independent %base class of %control %provider interface
struct abst_control_provider
{
	//! overload to check if ptr points to the controlled value
	/*! The method is used when searching controls with find_control.
	    The default implementation compares ptr to get_value_void(). */
	virtual bool controls(const void* ptr, void* user_data) const = 0;
};

/*! reimplement the %control %provider for a customized control. 
    A %control %provider can be used as argument to the constructor
	of the control class. The provider class also implements an
	overloaded version of the add_control method that takes a 
	%control %provider as second argument instead of the reference
	to a value. */
template <typename T>
struct control_provider : public abst_control_provider
{
	/// overload to set the value
	virtual void set_value(const T& value, void* user_data) = 0;
	/// overload to get the value
	virtual const T get_value(void* user_data) const = 0;
	/// the default implementation compares ptr to &get_value().
	virtual bool controls(const void* ptr, void* user_data) const { return false; }
};

/*! gui independent control of a value that has the type of the template argument. 
    The value can either be specified as a reference or through the control_provider
	that implements a get and a set method. 
	
	Before the control changes a value, it
	emits the signal check_value to test if the new value is valid. The check_value
	signal has a boolean return value. All attached callbacks must return true in order
	for the check to be successful. The attached callbacks can also change the new value
	to a valid value. The check_value callbacks should use the get_new_value(), get_value()
	and set_new_value() methods of the control to update the new value and to access the
	current value that has not been changed yet.
	
	If the validity check is successful, the value is changed to the new value and the
	value_change signal is emitted. The callbacks attached to this signal can not only
	query the current value with get_value() but also the old value with get_old_value().
	Take care that the get_old_value() cannot be used in the callbacks attached to 
	check_value and the get_new_value() method cannot be used in the callbacks attached
	to the value_change signal.
*/
template <typename T>
class control : public abst_control
{
private:
	T* value_ptr;
	control_provider<T>* cp;
	T  new_value;
protected:
	// protected function for the setter
	void set_value(const T& v) { 
		if (cp)
			cp->set_value(v, value_ptr); 
		else {
			*value_ptr = v;
			update_views();
		}
	}
	// return pointer to value or 0 if not available
	const T* get_value_ptr() const { return cp ? 0 : value_ptr; }
public:
	/// type of the value check signal
	typedef cgv::signal::bool_signal<control<T>&> value_check_signal_type;
	/// type of the value change signal
	typedef cgv::signal::signal<control<T>&> value_change_signal_type;
	/// construct abstract element from reference to value
	control(const std::string& _name, T& _value) : abst_control(_name), value_ptr(&_value), new_value(_value), cp(0) {
		attach_to_reference(value_ptr);
	}
	/// construct abstract element from control_provider
	control(const std::string& _name, T* _value) : abst_control(_name) {
		value_ptr = _value;
		new_value = *_value;
		cp = 0;
		attach_to_reference(value_ptr);
	}
	//! this constructor allows contruction from control_provider with user data or if the pointer
	//! to the control_provider is null, interpret the pointer to the user data as the value pointer
	//! and act as the previous constructor.
	control(const std::string& _name, abst_control_provider* _cp, void* user_data) : abst_control(_name), cp(0) {
		if (_cp) {
			cp = static_cast<control_provider<T>*>(_cp);
			(void*&)value_ptr = user_data;
			new_value = cp->get_value(user_data);
		}
		else {
			value_ptr = (T*)user_data;
			new_value = *value_ptr;
			attach_to_reference(value_ptr);
		}
	}
	//! this signal is sent when the user triggered a change of value in order to check whether the new value is valid. 
	/*! Use get_new_value() and set_new_value() to get and correct the new value. Return true if the new 
	    value is ok or could be corrected, false otherwise. */
	cgv::signal::bool_signal<control<T>&> check_value;
	//! this signal is sent after the user triggered a change of value and the check_value succeeded. 
	/*! You can access the old value with get_old_value() method. */
	cgv::signal::signal<control<T>&> value_change;
	/// return a reference to the current value
	const T get_value() const { return cp ? cp->get_value(value_ptr) : *value_ptr; }
	/// return the new value to the callbacks attached to the check_value signal
	const T& get_new_value() const { return new_value; }
	/// set a different new value from the callbacks attached to the check_value signal
	void set_new_value(const T& nv) { new_value = nv; }
	/// return the old value to the callbacks attached to the change_value signal
	const T& get_old_value() const { return new_value; }
	/// set new value only if check_value signal succeeds and send value_change signal. Return true if value has been changed.
	bool check_and_set_value(const T& nv) {
		set_new_value(nv);
		if (check_value(*this)) {
			T tmp_value = get_value();
			set_value(this->get_new_value());
			set_new_value(tmp_value);
			value_change(*this);
			return true;
		}
		return false;
	}
	/// check whether the value represented by this element is pointing to the passed pointer
	bool controls(const void* ptr) const { return cp ? cp->controls(ptr,value_ptr) : (value_ptr == ptr); }
	/// attach a functor to the value change signal
	void attach_to_value_change(cgv::signal::functor_base* func) {
		value_change.connect_abst(func);
	}
	/// attach a functor to the value change signal
	void attach_to_check_value(cgv::signal::functor_base* bool_func) {
		check_value.connect_abst(bool_func);
	}
};

	}
}

#include <cgv/config/lib_end.h>
