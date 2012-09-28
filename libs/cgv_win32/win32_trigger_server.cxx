#include "win32_trigger_server.h"
#include <windows.h>

namespace cgv {
	namespace gui {


std::map<trigger*, UINT_PTR>& ref_trigger_map()
{
	static std::map<trigger*, UINT_PTR> trigger_map;
	return trigger_map;
}

std::map<UINT_PTR, trigger*>& ref_id_map()
{
	static std::map<UINT_PTR, trigger*> id_map;
	return id_map;
}

win32_trigger_server::win32_trigger_server()
{
}

bool win32_trigger_server::is_scheduled(const trigger* t)
{
	return (ref_trigger_map().find(const_cast<trigger*>(t)) != ref_trigger_map().end());
}

void win32_trigger_server::schedule_one_shot(trigger* t, double delay)
{
	schedule_recuring(t,delay);
}

VOID CALLBACK TimerFunc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (ref_id_map().find(idEvent) == ref_id_map().end())
		return;
	trigger* t = ref_id_map()[idEvent];
	t->shoot(dwTime*0.001,t->get_delay());
	if (!t->is_recuring()) {
		KillTimer(NULL, idEvent);
		ref_trigger_map().erase(ref_trigger_map().find(t));
		ref_id_map().erase(ref_id_map().find(idEvent));
	}
}

void win32_trigger_server::schedule_recuring(trigger* t, double delay)
{
	if (ref_trigger_map().find(t) == ref_trigger_map().end()) {
		UINT_PTR id = SetTimer(NULL, 0, (UINT)(delay*1000), TimerFunc);
		ref_trigger_map()[t] = id;
		ref_id_map()[id] = t;
	}
	else
		SetTimer(NULL, ref_trigger_map()[t], (UINT)(delay*1000), TimerFunc);
}

void win32_trigger_server::stop(trigger* t)
{
	if (ref_trigger_map().find(t) != ref_trigger_map().end()) {
		UINT_PTR id = ref_trigger_map()[t];
		KillTimer(NULL, id);
		ref_trigger_map().erase(ref_trigger_map().find(t));
		ref_id_map().erase(ref_id_map().find(id));
	}
}

double win32_trigger_server::get_current_time() const
{
	return GetTickCount()*0.001;
}

	}
}