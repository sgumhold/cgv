#include "fltk_trigger_server.h"
#include <fltk/run.h>
#include <map>


std::map<void*,double> time_of_schedule;

void timeout_handler(void* _trig)
{
	cgv::gui::trigger* trig = (cgv::gui::trigger*) _trig;
	double t = fltk::get_time_secs();
	std::map<void*,double>::iterator i = time_of_schedule.find(_trig);
	if (i == time_of_schedule.end())
		trig->shoot(t, trig->get_delay());
	else
		trig->shoot(t, t-i->second);
	if (trig->is_recuring()) {
		::fltk::repeat_timeout((float)trig->get_delay(), timeout_handler, trig);
		double time = ::fltk::get_time_secs();
		time_of_schedule[trig] = time;
	}
	else
		if (i != time_of_schedule.end())
			time_of_schedule.erase(i);
}

void fltk_trigger_server::on_register()
{
	cgv::gui::register_trigger_server(cgv::gui::trigger_server_ptr(this));
}

std::string fltk_trigger_server::get_type_name() const
{
	return "fltk_trigger_server";
}

bool fltk_trigger_server::is_scheduled(const cgv::gui::trigger* t)
{
	return ::fltk::has_timeout(timeout_handler, const_cast<cgv::gui::trigger*>(t));
}

bool fltk_trigger_server::schedule_one_shot(cgv::gui::trigger* t, double delay)
{

	double time = ::fltk::get_time_secs();
	time_of_schedule[t] = time;
	::fltk::add_timeout((float)delay,timeout_handler,t);
	return true;
}

bool fltk_trigger_server::schedule_recuring(cgv::gui::trigger* t, double delay)
{
	double time = ::fltk::get_time_secs();
	time_of_schedule[t] = time;
	::fltk::add_timeout((float)delay,timeout_handler,t);
	return true;
}

bool fltk_trigger_server::stop(cgv::gui::trigger* t)
{
	::fltk::remove_timeout(timeout_handler,t);
	std::map<void*,double>::iterator i = time_of_schedule.find(t);
	if (i != time_of_schedule.end()) {
		time_of_schedule.erase(i); 
		return true;
	}
	return false;
}

double fltk_trigger_server::get_current_time() const
{
	return ::fltk::get_time_secs();
}

cgv::base::object_registration<fltk_trigger_server> trig_serv("");
