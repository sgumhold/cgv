#pragma once

#include "hids.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		enum class dispatch_mode : uint8_t
		{
			none,        // used for dispatch info passed to fcousable::focus_change caused by focusable::wants_to_grab_focus
			focus,		 // dispatched based on object in focus 
			structural,  // dispatched based on traversal of object hierarchies
			proximity,   // proximity based dispatching
			pointing     // pointing based dispatching
		};

		struct CGV_API dispatch_info
		{
			hid_identifier hid_id;
			dispatch_mode mode;
			/// construct
			dispatch_info(hid_identifier _hid_id = hid_identifier(), dispatch_mode _mode = dispatch_mode::none);
			/// virtual destructor necessary for deletion
			virtual ~dispatch_info();
			/// copy from dispatch info of same type
			virtual void copy(const dispatch_info& dis_info);
			/// allocate a copy of this dispatch info on the heap
			virtual dispatch_info* clone() const;
		};
	}
}

#include <cgv/config/lib_end.h>