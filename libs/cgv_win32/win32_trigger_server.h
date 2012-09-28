#pragma once

#include <cgv/gui/trigger.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

class CGV_API win32_trigger_server : public trigger_server
{
public:
	win32_trigger_server();
	bool is_scheduled(const trigger*);
	void schedule_one_shot(trigger*, double delay);
	void schedule_recuring(trigger*, double delay);
	void stop(trigger*);
	double get_current_time() const;
};

	}
}

#include <cgv/config/lib_end.h>