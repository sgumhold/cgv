#include <cgv/gui/layout_spacings.h>
#include <map>
#include <iostream>

namespace cgv {
	namespace gui {

		// some presets
		// compact layout without spacings at all
		static const layout_spacings spacing_compact = {"compact", {0, 0}, {0, 0}};
		// small layout with a little spacing
		static const layout_spacings spacing_small = {"small", {1, 1}, {1, 1}};
		// normal layout with nice spacings
		static const layout_spacings spacing_normal = {"normal", {2, 2}, {2, 2}};
		// broad spacing
		static const layout_spacings spacing_broad = {"broad", {5, 5}, {5, 5}};
		// huge spacing
		static const layout_spacings spacing_huge = {"huge", {15, 15}, {15, 15}};


		// the list of presets
		std::map<std::string,layout_spacings>& ref_spacings()
		{
			static std::map<std::string, layout_spacings> spacings;
			static bool presets_added = false;
			if (!presets_added) {
				spacings[spacing_compact.name] = spacing_compact;
				spacings[spacing_small.name] = spacing_small;
				spacings[spacing_normal.name] = spacing_normal;
				spacings[spacing_broad.name] = spacing_broad;
				spacings[spacing_huge.name] = spacing_huge;
				presets_added = true;
			}
			return spacings;
		}

		// get a spacing 
		const layout_spacings& get_layout_spacings(std::string name) 
		{
			if (ref_spacings().find(name) != ref_spacings().end())
				return ref_spacings()[name];
			return ref_spacings()["compact"];
		}


		void add_layout_spacings(const layout_spacings& new_spacings) 
		{
			ref_spacings()[new_spacings.name] = new_spacings;
		}


		void remove_layout_spacings(const std::string& name)
		{
			ref_spacings().erase(name);
		}

	}
}