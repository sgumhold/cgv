#pragma once

#include <cgv/base/base.h>
#include <cgv/data/ref_ptr.h>

#include "lib_begin.h"

namespace cgv {
	namespace base {

class console;

/// pointer to console
typedef cgv::data::ref_ptr<console,true> console_ptr;

/** interface to the console of the application with implementation
    of the property interface */
class CGV_API console : public cgv::base::base
{
private:
	/// hide constructor for singleton protection
	console();
public:
	/// return console singleton
	static console_ptr get_console();
	/// return "console"
	std::string get_type_name() const;
	/// supported properties are x:int32;y:int32;w:int32;h:int32;show:bool;title:string
	std::string get_property_declarations();
	///
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	///
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
};


	}
}

#include <cgv/config/lib_end.h>