#pragma once

#include <cgv/gui/trigger.h>

#include "lib_begin.h"


/// implements a trigger server with fltk
class CGV_API fltk_trigger_server : public cgv::gui::trigger_server
{
public:
	void on_register();
	std::string get_type_name() const;
	bool is_scheduled(const cgv::gui::trigger*);
	bool schedule_one_shot(cgv::gui::trigger*, double delay);
	bool schedule_recuring(cgv::gui::trigger*, double delay);
	bool stop(cgv::gui::trigger*);
	double get_current_time() const;
};

#include <cgv/config/lib_end.h>