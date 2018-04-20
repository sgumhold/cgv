#include "trigger.h"
#include <iostream>

namespace cgv {
	namespace gui {

/// default construction
trigger::trigger()
{
	delay = 0;
	recur = false;
}
/// return whether a trigger is scheduled
bool trigger::is_scheduled() const
{
	trigger_server_ptr ts = get_trigger_server();
	if (ts)
		return ts->is_scheduled(this);
	return false;
}

/// schedule a trigger event in delay seconds
bool trigger::schedule_one_shot(double _delay)
{
	delay = _delay;
	recur = false;
	trigger_server_ptr ts = get_trigger_server();
	if (ts)
		return ts->schedule_one_shot(this, delay);
	else
		std::cerr << "no trigger server registered" << std::endl;
	return false;
}

/// start a recuring trigger with the given delay in seconds
bool trigger::schedule_recuring(double _delay)
{
	delay = _delay;
	recur = true;
	trigger_server_ptr ts = get_trigger_server();
	if (ts)
		return ts->schedule_recuring(this, delay);
	else
		std::cerr << "no trigger server registered" << std::endl;
	return false;
}
/// return the delay used for scheduling the last time
double trigger::get_delay() const
{
	return delay;
}

/// return whether this is a recuring trigger
bool trigger::is_recuring() const
{
	return recur; 
}

/// stop a recuring trigger
void trigger::stop()
{
	trigger_server_ptr ts = get_trigger_server();
	if (ts)
		ts->stop(this);
	else
		std::cerr << "no trigger server registered" << std::endl;
}

/// return the current time
double trigger::get_current_time()
{
	trigger_server_ptr ts = get_trigger_server();
	if (ts)
		return ts->get_current_time();
	else
		std::cerr << "no trigger server registered" << std::endl;
	return -1;
}

/// return the global trigger used for animation, which runs by default with 60 Hz
trigger& get_animation_trigger()
{
	static trigger t;
	static bool initialized = false;
	if (!initialized) {
		if (get_trigger_server()) {
			initialized = true;
			t.schedule_recuring(1.0/60);
		}
	}
	return t;
}

trigger_server_ptr& ref_trigger_server()
{
	static trigger_server_ptr ts;
	return ts;
}

trigger_server_ptr get_trigger_server()
{
	return ref_trigger_server();
}

void register_trigger_server(trigger_server_ptr _ts)
{
	ref_trigger_server() = _ts;
	get_animation_trigger();
}

	}
}
