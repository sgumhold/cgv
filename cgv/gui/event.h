#pragma once

#include <iostream>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// define the ids of the most common events
enum EventId 
{
	EID_NONE, //!< undefined %event id
	EID_KEY,  //!< id for key %event
	EID_MOUSE //!< id for mouse %event
};

/// flags
enum EventFlags
{
	EF_NONE = 0,
	EF_MULTI = 1, //!< whether event is tagged with id of the device that generated the event
	EF_DND = 2    //!< whether mouse has a drag and drop target attached
};

/// define constants for event modifiers
enum EventModifier 
{
	EM_SHIFT = 1, //!< shift modifier
	EM_ALT   = 2, //!< alt modifier
	EM_CTRL  = 4, //!< ctrl modifier
	EM_META  = 8  //!< meta modifier (windows or mac key)
};

/// define constants for toggle keys
enum EventToggleKeys
{
	ETK_CAPS_LOCK = 1,      //!< caps lock
	ETK_NUM_LOCK   = 2,     //!< num lock
	ETK_SCROLL_LOCK  = 4    //!< scroll lock
};

/*! most simple event class that holds an id, modifiers, toggle keys and the event time.
    All other event classes are derived from this. */
class CGV_API event
{
protected:
	/// store which kind of event we have
	unsigned char kind;
	/// store event flags
	unsigned char flags;
	/// store the active modifiers
	unsigned char modifiers;
	/// store the active toggle keys
	unsigned char toggle_keys;
	/// store the time of the event
	double time;
public:
	/// construct %event from its kind
	event(unsigned int _kind = EID_NONE, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
	/// virtual destructor for events
	virtual ~event();
	/// write to stream
	virtual void stream_out(std::ostream& os) const;
	/// read from stream
	virtual void stream_in(std::istream& is);
	/// set the kind of the %event
	void set_kind(unsigned char _kind);
	/// return, what kind of %event this is, typically a value from the #EventId enum
	unsigned get_kind() const;
	/// set the event flags
	void set_flags(unsigned char _flags);
	/// return the event flags
	unsigned get_flags() const;
	/// set the modifiers
	void set_modifiers(unsigned char _modifiers);
	/// return the active modifiers as values from #EventModifier combined with a logical or-operation
	unsigned char get_modifiers() const;
	/// set the state of the toggle keys
	void set_toggle_keys(unsigned char _toggle_keys);
	/// return the state of the toggle keys as values from #EventToggleKeys combined with a logical or-operation
	unsigned char get_toggle_keys() const;
	/// set the time of the %event
	void set_time(const double& _time);
	/// return the time of the %event in seconds
	double get_time() const;
};

/// convert a modifier combination into a readable string ending on a '+' sign if not empty, i.e. "Shift+Ctrl+"
extern CGV_API std::string get_modifier_string(unsigned char modifiers);

/// convert a toggle key combination into a readable string separated by '+' signs, i.e. "CapsLock+NumLock"
extern CGV_API std::string get_toggle_keys_string(unsigned char toggle_keys);

/// read modifiers in string format from a stream and set the passed reference to EventModifier s ored together. 
extern CGV_API unsigned char stream_in_modifiers(std::istream& is);

/// read toggle keys in string format from a stream and set the passed reference to EventToggleKeys ored together. 
extern CGV_API void stream_in_toggle_keys(std::istream& is, unsigned char& toggle_keys);

extern CGV_API unsigned char& ref_current_modifiers();

	}
}

#include <cgv/config/lib_end.h>