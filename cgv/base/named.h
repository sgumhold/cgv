#pragma once

#include "base.h"

#include "lib_begin.h"

namespace cgv {
	namespace base {

class CGV_API named;

/// ref counted pointer to a node
typedef data::ref_ptr<named, true> named_ptr;

/// ref counted pointer to a const node
typedef data::ref_ptr<const named, true> const_named_ptr;

/** base class for all gui types */
class CGV_API named : public base
{
protected:
	/// declare named_ptr to be a friend class
	friend class data::ref_ptr<named,true>;
	/// store the name as a string
	std::string name;
public:
	/// construct from name
	named(const std::string& name = "");
	/// return the parent node
	const std::string& get_name() const;
	/// set a new parent node
	void set_name(const std::string& _name);
	/// cast upward to named
	named_ptr get_named();
	/// cast upward to const named
	const_named_ptr get_named_const();
	/// overload to return the type name of this object
	std::string get_type_name() const;
};

template <> struct cast_helper<named>
{
	inline static named_ptr cast(base* b) { return b->get_named(); }
};

template <> struct cast_const_helper<named>
{
	inline static const_named_ptr cast_const(const base* b) { return b->get_named_const(); }
};

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<named>;
#endif

	}
}

#include <cgv/config/lib_end.h>
