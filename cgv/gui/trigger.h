#pragma once

#include <cgv/base/base.h>
#include <cgv/base/register.h>
#include <cgv/signal/signal.h>

#include <cgv/gui/lib_begin.h>

namespace cgv {
	namespace gui {

/// a trigger provides callbacks after a programmable time delay
class CGV_API trigger
{
protected:
	double delay;
	bool recur;
public:
	/// default construction
	trigger();
	/// return whether a trigger is scheduled
	bool is_scheduled() const;
	/// schedule a trigger event in delay seconds, return whether this was successful
	bool schedule_one_shot(double delay);
	/// start a recuring trigger with the given delay in seconds, return whether this was successful
	bool schedule_recuring(double delay);
	/// return the delay used for scheduling the last time
	double get_delay() const;
	/// return whether this is a recuring trigger
	bool is_recuring() const;
	/// stop a recuring trigger
	void stop();
	/// return the current time
	double get_current_time() const;
	/// the shoot signal is called when the trigger is pulled and takes the current time and delay as argument
	signal::signal<double,double> shoot;
};

/// return the global trigger used for animation, which runs by default with 60 Hz
extern CGV_API trigger& get_animation_trigger();

class CGV_API trigger_server : public cgv::base::base, public cgv::base::server
{
public:
	virtual bool is_scheduled(const trigger*) = 0;
	virtual bool schedule_one_shot(trigger*, double delay) = 0;
	virtual bool schedule_recuring(trigger*, double delay) = 0;
	virtual bool stop(trigger*) = 0;
	virtual double get_current_time() const = 0;
};

/// ref counted pointer to trigger server
typedef data::ref_ptr<trigger_server> trigger_server_ptr;

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<trigger_server>;
#endif

/// returns the currently registered trigger server
extern CGV_API trigger_server_ptr get_trigger_server();
/// registeres a new trigger server, call this in the on_register method of the server implementation
extern CGV_API void register_trigger_server(trigger_server_ptr);

	}
}

#include <cgv/config/lib_end.h>
