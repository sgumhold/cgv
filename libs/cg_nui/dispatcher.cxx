#include "dispatcher.h"
#include "grabbable.h"
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/event_handler.h>
#include <libs/cg_vr/vr_events.h>

namespace cgv {
	namespace nui {

		bool focus_info::empty() const
		{
			return object.empty();
		}
		void focus_info::clear()
		{
			object.clear();
			root.clear();
		}

		dispatcher::~dispatcher()
		{
			for (auto& obj : objects) {
				auto* foc = obj->get_interface<focusable>();
				if (foc)
					foc->set_dispatcher_ptr(0);
			}
		}
		void dispatcher::update_dispatch_info(dispatch_info** dis_info_ptr_ptr, const dispatch_info& dis_info)
		{
			if (*dis_info_ptr_ptr) {
				if ((*dis_info_ptr_ptr)->mode == dis_info.mode) {
					(*dis_info_ptr_ptr)->copy(dis_info);
					return;
				}
				delete* dis_info_ptr_ptr;
			}
			*dis_info_ptr_ptr = dis_info.clone();
		}

		void dispatcher::attach_focus(refocus_action rfa, focus_attachment foc_att, const focus_info& foc_info, const cgv::gui::event& e, const dispatch_info& dis_info, refocus_info& rfi)
		{
			// detach overlapping focus attachments
			switch (foc_att.level) {
			case focus_level::hid:
			{
				// first check, whether focusable acccepts focus attachment
				if (foc_info.object->get_interface<focusable>()->focus_change(focus_change_action::attach, rfa, { foc_att, foc_info.config }, e, dis_info)) {
					// detach conflicting hid attachments
					auto iter = focus_hid_map.find(foc_att.hid_id);
					if (iter != focus_hid_map.end()) {
						iter->second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
						focus_hid_map.erase(iter);
					}
					// detach conflicting kit attachments
					if (is_part_of_kit(foc_att.hid_id.category)) {
						kit_identifier kit_id = { get_kit_category(foc_att.hid_id.category), foc_att.hid_id.kit_ptr };
						auto iter = focus_kit_map.find(kit_id);
						if (iter != focus_kit_map.end()) {
							iter->second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
							focus_kit_map.erase(iter);
						}
					}
					// detach conflicting category selection attachments
					for (size_t i = 0; i < focus_category_attachments.size(); ) {
						auto jter = focus_category_attachments.begin() + i;
						if (jter->first.selection.contains(foc_att.hid_id.category)) {
							jter->second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
							focus_category_attachments.erase(jter);
						}
						else
							++i;
					}
					// detach all attachment
					if (!focus_all_attachment.empty()) {
						focus_all_attachment.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
						focus_all_attachment.clear();
					}
					// finally attach focus to hid
					rfi.foc_info_ptr = &(focus_hid_map[foc_att.hid_id] = foc_info);
				}
				break;
			}
			case focus_level::kit:
			{
				// first check, whether focusable acccepts focus attachment
				if (foc_info.object->get_interface<focusable>()->focus_change(focus_change_action::attach, rfa, { foc_att, foc_info.config }, e, dis_info)) {
					// detach conflicting hid attachments by iterating categories and device indices and copy dispatch info to these
					for (hid_category hid_cat : get_hid_categories(foc_att.kit_id.category)) {
						for (int16_t di = get_min_index(hid_cat); di <= get_max_index(hid_cat); ++di) {
							hid_identifier hid_id = { hid_cat, foc_att.kit_id.kit_ptr, di };
							hid_id.index = di;
							auto iter = focus_hid_map.find(hid_id);
							if (iter != focus_hid_map.end()) {
								iter->second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
								focus_hid_map.erase(iter);
							}
						}
					}
					// detach conflicting kit attachment
					auto iter = focus_kit_map.find(foc_att.kit_id);
					if (iter != focus_kit_map.end()) {
						iter->second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
						focus_kit_map.erase(iter);
					}
					// detach conflicting category selection attachments
					for (size_t i = 0; i < focus_category_attachments.size(); ) {
						auto jter = focus_category_attachments.begin() + i;
						if (jter->first.selection.overlaps(foc_att.kit_id.category)) {
							jter->second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
							focus_category_attachments.erase(jter);
						}
						else
							++i;
					}
					// detach all attachment
					if (!focus_all_attachment.empty()) {
						focus_all_attachment.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
						focus_all_attachment.clear();
					}
					// finally attach focus to kit
					rfi.foc_info_ptr = &(focus_kit_map[foc_att.kit_id] = foc_info);
				}
				break;
			}
			case focus_level::category:
			{
				// first check, whether focusable acccepts focus attachment
				if (foc_info.object->get_interface<focusable>()->focus_change(focus_change_action::attach, rfa, { foc_att, foc_info.config }, e, dis_info)) {
					for (hid_category hid_cat : foc_att.selection.get_categories()) {
						// detach conflicting hid attachments by iterating categories and device indices
						for (int16_t di = get_min_index(hid_cat); di < get_max_index(hid_cat); ++di) {
							hid_identifier hid_id = { hid_cat, foc_att.kit_id.kit_ptr, di };
							hid_id.index = di;
							auto iter = focus_hid_map.find(hid_id);
							if (iter != focus_hid_map.end()) {
								iter->second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
								focus_hid_map.erase(iter);
							}
						}
						// detach conflicting kit attachment by iterating all kit attachments
						std::vector<kit_identifier> to_be_removed;
						kit_category kit_cat = get_kit_category(hid_cat);
						for (auto iter = focus_kit_map.begin(); iter != focus_kit_map.end(); ++iter)
							if (kit_cat == iter->first.category)
								to_be_removed.emplace_back(iter->first);
						for (auto tbr : to_be_removed) {
							focus_kit_map[tbr].object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
							focus_kit_map.erase(tbr);
						}
					}
					// detach conflicting category selection attachments
					for (size_t i = 0; i < focus_category_attachments.size(); ) {
						auto jter = focus_category_attachments.begin() + i;
						if (jter->first.selection.overlaps(foc_att.selection)) {
							jter->second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
							focus_category_attachments.erase(jter);
						}
						else
							++i;
					}
					// detach all attachment
					if (!focus_all_attachment.empty()) {
						focus_all_attachment.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
						focus_all_attachment.clear();
					}
					// finally attach focus to category selection
					focus_category_attachments.push_back({ foc_att, foc_info });
					rfi.foc_info_ptr = &focus_category_attachments.back().second;
				}
				break;
			}
			case focus_level::all:
			{
				// first check, whether focusable acccepts focus attachment
				if (foc_info.object->get_interface<focusable>()->focus_change(focus_change_action::attach, rfa, { foc_att, foc_info.config }, e, dis_info)) {
					// detach all hid attachments
					for (auto hid2foc : focus_hid_map) {
						hid2foc.second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
					}
					focus_hid_map.clear();
					// detach all kit attachments
					for (auto kit2foc : focus_kit_map) {
						kit2foc.second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
					}
					focus_kit_map.clear();
					// detach all category selection attachments
					for (auto cat2foc : focus_category_attachments) {
						cat2foc.second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
					}
					focus_category_attachments.clear();
					// detach all attachment
					if (!focus_all_attachment.empty()) {
						focus_all_attachment.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, foc_info.config }, e, dis_info);
					}
					// finally attach focus to all
					rfi.foc_info_ptr = &(focus_all_attachment = foc_info);
				}
				break;
			}
			}
			rfi.foc_att = foc_att;
		}
		void dispatcher::detach_focus(refocus_action rfa, focus_attachment foc_att, const focus_info& fi, const cgv::gui::event& e, const dispatch_info& dis_info, refocus_info& rfi)
		{
			switch (foc_att.level) {
			case focus_level::hid:
			{
				// find hid attachment and detach if same object as in detach request
				auto iter = focus_hid_map.find(foc_att.hid_id);
				if (iter != focus_hid_map.end() && iter->second.object == fi.object) {
					iter->second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, iter->second.config }, e, dis_info);
					focus_hid_map.erase(iter);
					rfi.foc_info_ptr = &default_focus_info;
				}
				break;
			}
			case focus_level::kit:
			{
				// find kit attachment and detach if same object as in detach request
				auto iter = focus_kit_map.find(foc_att.kit_id);
				if (iter != focus_kit_map.end() && iter->second.object == fi.object) {
					iter->second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, iter->second.config }, e, dis_info);
					focus_kit_map.erase(iter);
					rfi.foc_info_ptr = &default_focus_info;
				}
				break;
			}
			case focus_level::category:
			{
				// find category attachments and detach if same object as in detach request
				for (size_t i = 0; i < focus_category_attachments.size(); ) {
					auto jter = focus_category_attachments.begin() + i;
					if (jter->first.selection.overlaps(foc_att.selection) && jter->second.object == fi.object) {
						jter->second.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, jter->second.config }, e, dis_info);
						focus_category_attachments.erase(jter);
						rfi.foc_info_ptr = &default_focus_info;
					}
					else
						++i;
				}
				break;
			}
			case focus_level::all:
			{
				// detach all attachment
				if (!focus_all_attachment.empty() && focus_all_attachment.object == fi.object) {
					focus_all_attachment.object->get_interface<focusable>()->focus_change(focus_change_action::detach, rfa, { foc_att, focus_all_attachment.config }, e, dis_info);
					focus_all_attachment.clear();
					rfi.foc_info_ptr = &default_focus_info;
				}
				break;
			}
			}
			rfi.foc_att.level = focus_level::none;
		}

		void dispatcher::add_object(cgv::base::base_ptr obj)
		{
			objects.insert(obj);
			set_dispatcher_ptr_recursive(obj);
		}
		void dispatcher::set_dispatcher_ptr_recursive(cgv::base::base_ptr obj)
		{
			auto* foc = obj->get_interface<focusable>();
			if (foc)
				foc->set_dispatcher_ptr(this);
			// next check for group and children thereof
			auto grp_ptr = obj->cast<cgv::base::group>();
			if (grp_ptr) {
				for (unsigned ci = 0; ci < grp_ptr->get_nr_children(); ++ci)
					set_dispatcher_ptr_recursive(grp_ptr->get_child(ci));
			}
		}
		void dispatcher::remove_object(cgv::base::base_ptr obj)
		{
			objects.erase(obj);
		}

		bool dispatcher::wants_to_grab_recursive(cgv::base::base_ptr root_ptr, cgv::base::base_ptr object_ptr, const cgv::gui::event& e, const hid_identifier& hid_id, refocus_info& rfi)
		{
			// first check object
			auto* focusable_ptr = object_ptr->get_interface<focusable>();
			if (focusable_ptr) {
				focus_demand fd;
				fd.attach = rfi.foc_att;
				if (rfi.foc_info_ptr)
					fd.config = rfi.foc_info_ptr->config;
				if (focusable_ptr->wants_to_grab_focus(e, hid_id, fd)) {
					dispatch_info dis_info(hid_id);
					attach_focus(refocus_action::grab, fd.attach, { object_ptr, root_ptr, fd.config }, e, dis_info, rfi);
					return true;
				}
			}
			// next check for group and children thereof
			auto grp_ptr = object_ptr->cast<cgv::base::group>();
			if (grp_ptr) {
				for (unsigned ci = 0; ci < grp_ptr->get_nr_children(); ++ci)
					if (wants_to_grab_recursive(root_ptr, grp_ptr->get_child(ci), e, hid_id, rfi))
						return true;
			}
			// otherwise focus is not grabbed
			return false;
		}
		bool dispatcher::dispath_with_focus_update(cgv::base::base_ptr root_ptr, cgv::base::base_ptr object_ptr, const cgv::gui::event& e, const dispatch_info& dis_info, refocus_info& rfi)
		{
			// first check object with focusable interface
			auto* focusable_ptr = object_ptr->get_interface<focusable>();
			if (focusable_ptr) {
				focus_request fr;
				fr.demand.attach = rfi.foc_att;
				if (rfi.foc_info_ptr)
					fr.demand.config = rfi.foc_info_ptr->config;
				bool ret = focusable_ptr->handle(e, dis_info, fr);
				if (rfi.foc_info_ptr && rfi.foc_info_ptr->config.refocus.handle) {
					switch (fr.request) {
					case focus_change_action::detach:
						detach_focus(refocus_action::handle, fr.demand.attach, { object_ptr, root_ptr, fr.demand.config }, e, dis_info, rfi);
						break;
					case focus_change_action::attach:
						attach_focus(refocus_action::handle, fr.demand.attach, { object_ptr, root_ptr, fr.demand.config }, e, dis_info, rfi);
						break;
					case focus_change_action::reconfigure:
						rfi.foc_info_ptr->config = fr.demand.config;
						break;
					case focus_change_action::reattach:
						detach_focus(refocus_action::handle, rfi.foc_att, { object_ptr, root_ptr, fr.demand.config }, e, dis_info, rfi);
						attach_focus(refocus_action::handle, fr.demand.attach, { object_ptr, root_ptr, fr.demand.config }, e, dis_info, rfi);
						break;
					}
				}
				if (ret) {
					update_dispatch_info(rfi.dis_info_ptr_ptr, dis_info);
					return true;
				}
			}
			return false;
		}

		bool dispatcher::dispatch_recursive(cgv::base::base_ptr root_ptr, cgv::base::base_ptr object_ptr, const cgv::gui::event& e, const dispatch_info& dis_info, refocus_info& rfi)
		{
			if (object_ptr == rfi.foc_info_ptr->object) {
				if (rfi.foc_info_ptr->config.dispatch.focus_recursive)
					return false;
			}
			else {
				if (dispath_with_focus_update(root_ptr, object_ptr, e, dis_info, rfi))
					return true;
			}
			// next check object with event_handler interface
			if (rfi.foc_info_ptr->config.dispatch.event_handler) {
				auto* event_handler_ptr = object_ptr->get_interface<cgv::gui::event_handler>();
				if (event_handler_ptr) {
					if (event_handler_ptr->handle(const_cast<cgv::gui::event&>(e)))
						return true;
				}
			}
			// next check for group and children thereof
			auto grp_ptr = object_ptr->cast<cgv::base::group>();
			if (grp_ptr) {
				for (unsigned ci = 0; ci < grp_ptr->get_nr_children(); ++ci)
					if (dispatch_recursive(root_ptr, grp_ptr->get_child(ci), e, dis_info, rfi))
						return true;
			}
			// otherwise event not handled
			return false;
		}
		bool dispatcher::dispatch_spatial(const focus_attachment& foc_att, const cgv::gui::event& e, const hid_identifier& hid_id, refocus_info& rfi, bool* handle_called_ptr)
		{
			if (handle_called_ptr)
				*handle_called_ptr = false;
			return false;
		}

		bool dispatcher::reconfigure(const focus_attachment& foc_att, cgv::base::base_ptr obj_ptr, focus_configuration& focus_config)
		{
			switch (foc_att.level) {
			case focus_level::hid:
			{
				auto iter = focus_hid_map.find(foc_att.hid_id);
				if (iter == focus_hid_map.end())
					return false;
				if (iter->second.object != obj_ptr)
					return false;
				iter->second.config = focus_config;
				return true;
			}
			case focus_level::kit:
			{
				auto iter = focus_kit_map.find(foc_att.kit_id);
				if (iter == focus_kit_map.end())
					return false;
				if (iter->second.object != obj_ptr)
					return false;
				iter->second.config = focus_config;
				return true;
			}
			case focus_level::category:
			{
				for (auto fca : focus_category_attachments) {
					if (fca.second.object == obj_ptr) {
						fca.second.config = focus_config;
						return true;
					}
				}
				return false;
			}
			case focus_level::all:
			{
				if (focus_all_attachment.empty())
					return false;
				if (focus_all_attachment.object != obj_ptr)
					return false;
				focus_all_attachment.config = focus_config;
				return true;
			}
			}
			return false;
		}

		bool dispatcher::dispatch(const cgv::gui::event& e, const hid_identifier& hid_id, dispatch_report* report_ptr)
		{
			// prepare refocus information
			refocus_info rfi;
			rfi.report_ptr = report_ptr;
			// extract hid_id specific information
			rfi.dis_info_ptr_ptr = &dis_info_hid_map[hid_id];
			
			// extract information from currently active focus info
			rfi.foc_info_ptr = &default_focus_info;
			// check for active all attachment
			if (!focus_all_attachment.empty()) {
				rfi.foc_info_ptr = &focus_all_attachment;
				rfi.foc_att.level = focus_level::all;
			}
			else {
				// check for active kit attachment
				kit_identifier kit_id = { get_kit_category(hid_id.category), hid_id.kit_ptr };
				auto iter = focus_kit_map.find(kit_id);
				if (iter != focus_kit_map.end()) {
					rfi.foc_info_ptr = &iter->second;
					rfi.foc_att.level = focus_level::kit;
					rfi.foc_att.kit_id = kit_id;
				}
				else {
					// check for active hid attachment
					auto jter = focus_hid_map.find(hid_id);
					if (jter != focus_hid_map.end()) {
						rfi.foc_info_ptr = &jter->second;
						rfi.foc_att.level = focus_level::hid;
						rfi.foc_att.hid_id = hid_id;
					}
				}
			}
			// next check whether object wants to grab focus and in this case change focus and send event
			if (rfi.foc_info_ptr->config.refocus.grab) {
				for (auto root_ptr : objects)
					if (wants_to_grab_recursive(root_ptr, root_ptr, e, hid_id, rfi))
						break;
			}
			// next we try to dispatch spatially check for spatial refocussing that can change root and object in focus
			bool handle_called = false;
			if (rfi.foc_info_ptr->config.dispatch.spatial)
				if (dispatch_spatial(rfi.foc_att, e, hid_id, rfi, &handle_called))
					return true;

			// secondly we dispatch object in focus if not yet done in dispatch_spatial
			if (!handle_called && rfi.foc_info_ptr->object) {
				dispatch_info dis_info(hid_id, dispatch_mode::focus);
				if (dispath_with_focus_update(rfi.foc_info_ptr->root, rfi.foc_info_ptr->object, e, dis_info, rfi))
					return true;
				if (rfi.foc_info_ptr->config.dispatch.focus_recursive) {
					auto grp_ptr = rfi.foc_info_ptr->object->cast<cgv::base::group>();
					if (grp_ptr) {
						for (unsigned ci = 0; ci < grp_ptr->get_nr_children(); ++ci)
							if (dispatch_recursive(rfi.foc_info_ptr->root, grp_ptr->get_child(ci), e, dis_info, rfi))
								return true;
					}
				}
			}
			// and finally traverse all structural hierarchies
			if (rfi.foc_info_ptr->config.dispatch.structural) {
				cgv::base::base_ptr root_in_focus;
				dispatch_info dis_info(hid_id, dispatch_mode::structural);
				if (rfi.foc_info_ptr->config.dispatch.current_root_first) {
					root_in_focus = rfi.foc_info_ptr->root.empty() ? *objects.begin() : rfi.foc_info_ptr->root;
					if (dispatch_recursive(root_in_focus, root_in_focus, e, dis_info, rfi))
						return true;
				}
				for (auto root_ptr : objects) {
					if (root_ptr == root_in_focus)
						continue;
					if (dispatch_recursive(root_ptr, root_ptr, e, dis_info, rfi))
						return true;
				}
			}
			//delete* rfi.dis_info_ptr_ptr;
			//rfi.dis_info_ptr_ptr = 0;
			update_dispatch_info(rfi.dis_info_ptr_ptr, { hid_id, dispatch_mode::none } );

			return false;
		}
		bool dispatcher::dispatch(const cgv::gui::event& e, dispatch_report* report_ptr)
		{
			if ((e.get_flags() & cgv::gui::EF_VR) != 0) {
				hid_identifier hid_id;
				switch (e.get_kind()) {
				case cgv::gui::EID_KEY:
					hid_id.category = hid_category::controller;
					hid_id.index = reinterpret_cast<const cgv::gui::vr_key_event&>(e).get_controller_index();
					hid_id.kit_ptr = reinterpret_cast<const cgv::gui::vr_key_event&>(e).get_device_id();
					break;
				case cgv::gui::EID_THROTTLE:
					hid_id.category = hid_category::controller;
					hid_id.index = reinterpret_cast<const cgv::gui::vr_throttle_event&>(e).get_controller_index();
					hid_id.kit_ptr = reinterpret_cast<const cgv::gui::vr_throttle_event&>(e).get_device_id();
					break;
				case cgv::gui::EID_STICK:
					hid_id.category = hid_category::controller;
					hid_id.index = reinterpret_cast<const cgv::gui::vr_stick_event&>(e).get_controller_index();
					hid_id.kit_ptr = reinterpret_cast<const cgv::gui::vr_stick_event&>(e).get_device_id();
					break;
				case cgv::gui::EID_POSE:
					hid_id.category = hid_category::controller;
					hid_id.index = reinterpret_cast<const cgv::gui::vr_pose_event&>(e).get_trackable_index();
					hid_id.kit_ptr = reinterpret_cast<const cgv::gui::vr_pose_event&>(e).get_device_id();
					break;
				}
				return dispatch(e, hid_id, report_ptr);
			}
			else {
				switch (e.get_kind()) {
				case cgv::gui::EID_KEY:
					return dispatch(e, { hid_category::keyboard }, report_ptr);
				case cgv::gui::EID_MOUSE:
					return dispatch(e, { hid_category::mouse }, report_ptr);
				}
			}
			return false;
		}
	}
}