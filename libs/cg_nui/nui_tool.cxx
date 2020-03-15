#include <cgv/base/base.h>
#include "nui_tool.h"

namespace cgv {
	namespace nui {

std::string get_tool_type_string(ToolType type)
{
	switch (type) {
		case TT_MOUSE: return "mouse";
		case TT_KEYBOARD: return "keyboard";
		case TT_VR_CONTROLLER: return "vr_controller";
		case TT_GAMEPAD: return "gamepad";
		case TT_HAND: return "hand";
		case TT_BODY: return "body";
		default: return "unknown";
	}
}

nui_tool::nui_tool(const std::string& _name, ToolType _type, uint32_t _nr_modi)
	: cgv::base::named(_name), type(_type), nr_modi(_nr_modi)
{
	current_mode = 0;
}

	}
}