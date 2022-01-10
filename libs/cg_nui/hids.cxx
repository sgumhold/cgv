#include "hids.h"
#include <map>

namespace cgv {
	namespace nui {

		std::string to_string(hid_category hc)
		{
			const char* names[] = {
				"keyboard",
				"mouse",
				"joystick",
				"gamepad",
				"hmd",
				"controller",
				"tracker"
			};
			return names[int(hc)];
		}
		std::string to_string(kit_category kc)
		{
			const char* names[] = {
				"none",
				"keyboard_mouse",
				"vr"
			};
			return names[int(kc)];
		}
		void consider_flag(bool f, const std::string& name, std::string& res, char sep)
		{
			if (!f)
				return;
			if (!res.empty())
				res += sep;
			res += name;
		}
		std::string to_string(hid_selection hs, char sep)
		{
			std::string res;
			consider_flag(hs.keyboard, "keyboard", res, sep);
			consider_flag(hs.mouse, "mouse", res, sep);
			consider_flag(hs.joystick, "joystick", res, sep);
			consider_flag(hs.gamepad, "gamepad", res, sep);
			consider_flag(hs.hmd, "hmd", res, sep);
			consider_flag(hs.controller, "controller", res, sep);
			consider_flag(hs.tracker, "tracker", res, sep);
			return res;
		}
		bool is_part_of_kit(hid_category hid_cat)
		{
			switch (hid_cat) {
			case hid_category::keyboard:
			case hid_category::mouse:
			case hid_category::hmd:
			case hid_category::controller:
			case hid_category::tracker:
				return true;
			}
			return false;
		}
		kit_category get_kit_category(hid_category hid_cat)
		{
			switch (hid_cat) {
			case hid_category::keyboard:
			case hid_category::mouse:
				return kit_category::keyboard_mouse;
			case hid_category::hmd:
			case hid_category::controller:
			case hid_category::tracker:
				return kit_category::vr;
			}
			return kit_category::none;
		}

		int get_min_index(hid_category hid_cat)
		{
			return hid_cat == hid_category::hmd ? -1 : 0;
		}
		int get_max_index(hid_category hid_cat)
		{
			switch (hid_cat) {
			case hid_category::keyboard: return 1;
			case hid_category::mouse: return 3;
			case hid_category::joystick: return 3;
			case hid_category::gamepad: return 1;
			case hid_category::hmd: return -1;
			case hid_category::controller: return 3;
			case hid_category::tracker: return 7;
			}
			return 8;
		}
		std::vector<hid_category> get_hid_categories(kit_category kit_cat)
		{
			switch (kit_cat) {
			case kit_category::keyboard_mouse:
				return { hid_category::keyboard, hid_category::mouse };
			case kit_category::vr:
				return { hid_category::hmd, hid_category::controller, hid_category::tracker };
			}
			return {};
		}

		bool hid_identifier::operator < (const hid_identifier& hid_id) const
		{
			return kit_ptr < hid_id.kit_ptr ||
				(kit_ptr == hid_id.kit_ptr &&
					(index < hid_id.index ||
						(index == hid_id.index && category < hid_id.category)));
		}
		bool hid_identifier::operator == (const hid_identifier& hid_id) const
		{
			return kit_ptr == hid_id.kit_ptr && index == hid_id.index && category == hid_id.category;
		}

		bool kit_identifier::operator < (const kit_identifier& kit_id) const
		{
			return kit_ptr < kit_id.kit_ptr || (kit_ptr == kit_id.kit_ptr && (category < kit_id.category));
		}
		bool kit_identifier::operator == (const kit_identifier& kit_id) const
		{
			return kit_ptr == kit_id.kit_ptr && category == kit_id.category;
		}

		std::map<hid_identifier, int>& ref_hid_unique_index_map()
		{
			static std::map<hid_identifier, int> hid2ui_map;
			return hid2ui_map;
		}
		std::vector<hid_identifier>& ref_hid_list()
		{
			static std::vector<hid_identifier> hid_list;
			return hid_list;
		}
		bool hid_selection::contains(hid_category hid_cat) const
		{
			switch (hid_cat) {
			case hid_category::keyboard: return keyboard;
			case hid_category::mouse: return mouse;
			case hid_category::joystick: return joystick;
			case hid_category::gamepad: return gamepad;
			case hid_category::hmd: return hmd;
			case hid_category::controller: return controller;
			case hid_category::tracker: return tracker;
			}
			return false;
		}
		bool hid_selection::overlaps(kit_category kit_cat) const
		{
			switch (kit_cat) {
			case kit_category::keyboard_mouse:
				return keyboard || mouse;
			case kit_category::vr:
				return hmd || controller || tracker;
			}
			return false;
		}
		bool hid_selection::overlaps(hid_selection sel) const
		{
			return
				(sel.keyboard && keyboard) ||
				(sel.mouse && mouse) ||
				(sel.joystick == joystick) ||
				(sel.gamepad == gamepad) ||
				(sel.hmd == hmd) ||
				(sel.controller == controller) ||
				(sel.tracker == tracker);
		}
		std::vector<hid_category> hid_selection::get_categories() const
		{
			std::vector<hid_category> hid_cats;
			if (keyboard)
				hid_cats.emplace_back(hid_category::keyboard);
			if (mouse)
				hid_cats.emplace_back(hid_category::mouse);
			if (joystick)
				hid_cats.emplace_back(hid_category::joystick);
			if (gamepad)
				hid_cats.emplace_back(hid_category::gamepad);
			if (hmd)
				hid_cats.emplace_back(hid_category::hmd);
			if (controller)
				hid_cats.emplace_back(hid_category::controller);
			if (tracker)
				hid_cats.emplace_back(hid_category::tracker);
			return hid_cats;
		}
		bool hid_selection::operator == (const hid_selection& hs) const
		{
			return
				hs.controller == controller &&
				hs.gamepad == gamepad &&
				hs.hmd == hmd &&
				hs.joystick == joystick &&
				hs.keyboard == keyboard &&
				hs.mouse == mouse &&
				hs.tracker == tracker;
		}
		int get_hid_unique_index(const hid_identifier& hid_id)
		{
			auto& M = ref_hid_unique_index_map();
			const auto& i = M.find(hid_id);
			if (i != M.end())
				return i->second;
			auto& L = ref_hid_list();
			int ret = M[hid_id] = int(L.size());
			L.emplace_back(hid_id);
			return ret;
		}
		std::ostream& operator << (std::ostream& os, const hid_identifier& hid_id)
		{
			return os << to_string(hid_id.category) << "[" << hid_id.kit_ptr << "|" << hid_id.index << "]";
		}
		std::ostream& operator << (std::ostream& os, const kit_identifier& kit_id)
		{
			return os << to_string(kit_id.category) << "[" << kit_id.kit_ptr << "]";
		}

	}
}
