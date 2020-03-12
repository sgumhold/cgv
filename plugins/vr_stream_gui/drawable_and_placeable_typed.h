#pragma once

#include "drawable_and_placeable.h"

namespace vr {
namespace room {
	class drawable_and_placeable_typed : public cgv::render::drawable_and_placeable {
	  public:
		// Render priority increases with position
		enum class roomobject_render_type : uint8_t {
			INVALID,
			TRANSLUCENT, // translucent because winapi TRANSPARENT is bleeding everywhere
			SOLID,
			OCCLUSION_ONLY
		};

		roomobject_render_type get_render_type() const { return render_type; }
		void set_render_type(const roomobject_render_type type) { render_type = type; }

		static roomobject_render_type render_type_from_string(const std::string &type)
		{
			if (type == "solid")
				return roomobject_render_type::SOLID;
			else if (type == "transparent")
				return roomobject_render_type::TRANSLUCENT;
			else if (type == "occlusion")
				return roomobject_render_type::OCCLUSION_ONLY;
			else
				return roomobject_render_type::INVALID;
		}

	  protected:
		roomobject_render_type render_type = roomobject_render_type::SOLID;
	};
} // namespace room
} // namespace vr
