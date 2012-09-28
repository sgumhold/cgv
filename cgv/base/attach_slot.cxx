#include "attach_slot.h"

namespace cgv {
	namespace base {

/// init the user data
attach_slot::attach_slot()
{
	attachment_user_data = 0;
}


/// attach object to group
void attach_slot::attach(base_ptr attachment_object, void* user_data)
{
	attachment_user_data = user_data;
	attachment = attachment_object;
}

void attach_slot::attach(base_ptr attachment_object, int user_data)
{
	attach(attachment_object, (void*&)user_data);
}

/// return current attachment
base_ptr attach_slot::get_attachment() const
{
	return attachment;
}

/// return current attachment
void* attach_slot::get_attachement_data() const
{
	return attachment_user_data;
}

//! function to attach an object to an object of type attach_slot. 
bool attach(base_ptr slot_object, base_ptr attachment_object, void* user_data)
{
	attach_slot* as = slot_object->get_interface<attach_slot>();
	if (!as)
		return false;
	as->attach(attachment_object, user_data);
	return true;
}

bool attach(base_ptr slot_object, base_ptr attachment_object, int user_data)
{
	return attach(slot_object, attachment_object, (void*&) user_data);
}

//! query the attachment of an attach_slot object.
base_ptr get_attachment(base_ptr slot_object)
{
	attach_slot* as = slot_object->get_interface<attach_slot>();
	if (!as)
		return base_ptr();
	return as->get_attachment();
}

//! query the user data of the attachment of an attach_slot object.
void* get_attachment_data(base_ptr slot_object)
{
	attach_slot* as = slot_object->get_interface<attach_slot>();
	if (!as)
		return 0;
	return as->get_attachement_data();
}

int get_attachment_data_int(base_ptr slot_object)
{
	void* ud = get_attachment_data(slot_object);
	return (int&) ud;
}

	}
}
