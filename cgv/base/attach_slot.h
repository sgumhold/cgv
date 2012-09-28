#pragma once

#include "base.h"

#include "lib_begin.h"

namespace cgv {
	namespace base {

/** interface for an attachment slot that can store a base pointer */
class CGV_API attach_slot
{
protected:
	/// allow to attach some object to group
	base_ptr attachment;
	/// allow to add user data to the attachment
	void* attachment_user_data;
public:
	/// init the user data
	attach_slot();
	/// attach object to attachment slot
	void attach(base_ptr attachment_object, void* user_data = 0);
	/// attach object to attachment slot
	void attach(base_ptr attachment_object, int user_data);
	/// return current attachment
	base_ptr get_attachment() const;
	/// return current attachment
	void* get_attachement_data() const;
	/// return current attachment
	int get_attachement_data_int() const;
};

//! function to attach an object to an object of type attach_slot. 
/*! Return whether the slot_object implements the attach_slot interface
    and could retreive the attachment. */
extern CGV_API bool attach(base_ptr slot_object, base_ptr attachment_object, void* user_data = 0);
extern CGV_API bool attach(base_ptr slot_object, base_ptr attachment_object, int user_data);

//! query the attachment of an attach_slot object.
/*! If the slot_object is not derived from the attach_slot interface,
    return an empty base_ptr. */
extern CGV_API base_ptr get_attachment(base_ptr slot_object);

//! query the user data of the attachment of an attach_slot object.
/*! If the slot_object is not derived from the attach_slot interface,
    return the null pointer. */
extern CGV_API void* get_attachment_data(base_ptr slot_object);
extern CGV_API int get_attachment_data_int(base_ptr slot_object);

	}
}

#include <cgv/config/lib_end.h>