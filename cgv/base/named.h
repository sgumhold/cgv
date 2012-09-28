#pragma once

#include <cgv/data/ref_ptr.h>
#include "base.h"
#include "lib_begin.h"

namespace cgv {
	namespace base {

class CGV_API named;

/// ref counted pointer to a node
typedef data::ref_ptr<named,true> named_ptr;

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
	data::ref_ptr<named,true> get_named();
	/// overload to return the type name of this object
	std::string get_type_name() const;
};

template <> 
struct cast_helper<named>
{ 
	inline static data::ref_ptr<named,true> cast(base* b) 
	{ 
		return b->get_named();
	} 
};

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<named>;
#endif

	}
}

#include <cgv/config/lib_end.h>
