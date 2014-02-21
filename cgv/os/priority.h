#pragma once

#include "lib_begin.h"

namespace cgv {
	namespace os {

/// supports only three different priorities
enum ExecutionPriority {
	EP_IDLE,
	EP_NORMAL,
	EP_HIGH
};

/// return the execution priority of the current thread and process
ExecutionPriority CGV_API get_execution_priority();

/// set the execution priority of the current thread and process
bool CGV_API set_execution_priority(ExecutionPriority ep);

	}
}

#include <cgv/config/lib_end.h>