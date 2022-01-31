#pragma once

#include "hids.h"
#include "focusable.h"

#include <cgv/base/node.h>
#include <cgv/signal/bool_signal.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/event.h>
#include <libs/vr/vr_kit.h>
#include <set>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		struct CGV_API focus_info
		{
			cgv::base::base_ptr object = 0;
			cgv::base::base_ptr root = 0;
			focus_configuration config;
			bool empty() const;
			void clear();
		};

		struct dispatch_report
		{
			cgv::base::base_ptr handling_object = 0;
			cgv::base::base_ptr focused_object = 0;
			dispatch_mode mode = dispatch_mode::none;
			dispatch_info** di_ptr_ptr = 0;
			refocus_action action;
		};

		struct CGV_API refocus_info
		{
			focus_attachment foc_att;
			focus_info* foc_info_ptr = 0;
			dispatch_info** dis_info_ptr_ptr = 0;
			dispatch_report* report_ptr = 0;
		};

		/**
		dispatcher:
			keeps list of objects to handle events
			manages non overlapping attachment of focus to hids
				per hid one unique focus attachment
				per kit one unique focus attachment that detaches attachments of contained hids
				per category one unique focus attachment potentially valid for category selection; detaches overlapping kit and hid attachments
				for all hids unique focus attachment that detaches all other attachments
			dispatch(event, hid, [report])
				finds focus info for hid
				check objects for grab of focus
				spatial analysis of event
				distributes event to focusable objects till handle function returns true
				update of focus
					on attach hid, detach all, detach category selection if overlaps, detach kit if overlaps, detach hid
					on attach kit, detach all, detach category selection if overlaps, detach kit if overlaps, detach hid that overlap
					on attach category, detach all, previous detach category selection always, detach kit that overlap, detach all hids that overlap
					on all, detach all */
		class CGV_API dispatcher
		{
		protected:
			std::set<cgv::base::base_ptr> objects;
			/// store dispatch information per hid
			std::map<hid_identifier, dispatch_info*> dis_info_hid_map;
			/// store focus information per hid
			std::map<hid_identifier, focus_info> focus_hid_map;
			/// store focus information per kit
			std::map<kit_identifier, focus_info> focus_kit_map;
			/// store list of category attachments
			std::vector<std::pair<focus_attachment, focus_info>> focus_category_attachments;
			/// store unique all hids attachment, which can by empty, what is marked by nullptr to object
			focus_info focus_all_attachment;
			/// default focus config
			focus_info default_focus_info;
			void update_dispatch_info(dispatch_info** dis_info_ptr_ptr, const dispatch_info& dis_info);
			bool dispath_with_focus_update(cgv::base::base_ptr root_ptr, cgv::base::base_ptr object_ptr, const cgv::gui::event& e, const dispatch_info& dis_info, refocus_info& rfi);
			bool dispatch_recursive(cgv::base::base_ptr root_ptr, cgv::base::base_ptr object_ptr, const cgv::gui::event& e, const dispatch_info& dis_info, refocus_info& rfi);
			void detach_focus(refocus_action rfa, focus_attachment foc_att, const focus_info& foc_info, const cgv::gui::event& e, const dispatch_info& dis_info, refocus_info& rfi);
			void attach_focus(refocus_action rfa, focus_attachment foc_att, const focus_info& foc_info, const cgv::gui::event& e, const dispatch_info& dis_info, refocus_info& rfi);
			bool wants_to_grab_recursive(cgv::base::base_ptr root_ptr, cgv::base::base_ptr object_ptr, const cgv::gui::event& e, const hid_identifier& hid_id, refocus_info& rfi);
			virtual bool dispatch_spatial(const focus_attachment& foc_att, const cgv::gui::event& e, const hid_identifier& hid_id, refocus_info& rfi, bool* handle_called_ptr);
			friend class focusable;
			void set_dispatcher_ptr_recursive(cgv::base::base_ptr obj);
			bool reconfigure(const focus_attachment& foc_att, cgv::base::base_ptr obj_ptr, focus_configuration& focus_config);
		public:
			dispatcher() = default;
			virtual ~dispatcher();
			virtual void add_object(cgv::base::base_ptr root);
			virtual void remove_object(cgv::base::base_ptr root);
			virtual bool dispatch(const cgv::gui::event& e, const hid_identifier& hid_id, dispatch_report* report_ptr);
			virtual bool dispatch(const cgv::gui::event& e, dispatch_report* report_ptr);
		};
	}
}

#include <cgv/config/lib_end.h>