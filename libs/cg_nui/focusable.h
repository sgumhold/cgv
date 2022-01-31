#pragma once

#include <cgv/base/base.h>
#include <cgv/gui/event.h>
#include "dispatch_info.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class CGV_API dispatcher;

		/// policy that defines how to update the focus during dispatching
		struct refocus_policy
		{
			bool deattach : 1;   // allow to deattach focus completely
			bool transfer : 1;   // allow to transfer focus to different object
			bool grab : 1;       // allow all objects to grab the focus
			bool handle : 1;     // process focus requests from handle function
			bool spatial : 1;    // allow focus change based on spatial analysis
		};
		/// provide conversion of refocus_policy to string
		extern CGV_API std::string to_string(refocus_policy rp, char sep = ',');

		/// refocus action performed during dispatch used to in dispatch report
		enum class refocus_action
		{
			none,
			grab,
			handle,
			proximity,
			intersection
		};

		/// policy defining which modes used for dispatching
		struct dispatch_policy
		{
			bool focus_recursive : 1;     // dispatch focused object and recursively all its children
			bool structural : 1;          // dispatch over all objects and their children
			bool current_root_first : 1;  // start structural dispatch with root of object in focus
			bool spatial : 1;             // use spatial dispatching
			bool event_handler : 1;       // whether to dispatch to objects implementing cgv::gui::event_handler interface
		};
		/// provide conversion of dispatch_policy to string
		extern CGV_API std::string to_string(dispatch_policy dp, char sep = ',');

		/// configuration of spatial analysis performed during dispatching
		struct spatial_analysis
		{
			bool only_focus : 1; // restrict spatial analysis to object in focus
			bool proximity : 1;  // perform proximity query
			bool pointing : 1;   // perform pointing query
		};
		/// provide conversion of spatial_analysis to string
		extern CGV_API std::string to_string(spatial_analysis sa, char sep = ',');

		/// focus level defines abstraction level for attachment of focus
		enum class focus_level : uint8_t
		{
			none,     // not attached at all
			hid,      // attach to hid
			kit,      // attach to all hids in same kit 
			category, // attach to all hids in a selection of categories
			all       // attach to all hids
		};
		/// provide conversion of focus_level to string
		extern CGV_API std::string to_string(focus_level fl);
		/// focus configuration defines how dispatch processed events 
		struct focus_configuration
		{
			refocus_policy   refocus = { true, true, true, true, true };
			dispatch_policy  dispatch = { true, true, true, true, false };
			spatial_analysis spatial = { false, true, true };
		};
		extern CGV_API std::ostream& operator << (std::ostream& os, const focus_configuration& foc_cfg);

		/// defines information on where to attach focus to
		struct CGV_API focus_attachment
		{
			focus_level level;
			union {
				hid_identifier hid_id;   // used if level == device
				kit_identifier kit_id;   // used if level == kit 
				hid_selection selection; // used if level == category
			};
			focus_attachment(focus_level _level = focus_level::none);
			focus_attachment(const hid_identifier& _hid_id);
			focus_attachment(const kit_identifier& _kit_id);
			focus_attachment(hid_selection _selection);
		};
		extern CGV_API std::ostream& operator << (std::ostream& os, const focus_attachment& foc_att);

		/// focus demand combines attachment and configuration
		struct focus_demand
		{
			focus_attachment attach;
			focus_configuration config;
		};
		/// different options of changing the focus
		enum class focus_change_action : uint8_t
		{
			none,
			detach,
			reconfigure,
			reattach,
			attach,
			index_change
		};
		/// focus request extends the demand by a flag that has to be actively set to ask for a focus change
		struct focus_request
		{
			focus_demand demand;
			focus_change_action request = focus_change_action::none;
		};

		/// interface for object that can receive input focus from the focus_manager
		class CGV_API focusable
		{
		protected:
			friend class dispatcher;
			dispatcher* disp_ptr = 0;
			void set_dispatcher_ptr(dispatcher* _disp_ptr);
			/// <summary>
			/// helper function to be used in handle() to reconfigure the focus on start of a drag operation 
			/// such that it is not lost during the operation. If drag_begin() is called, it always has to
			/// be terminated with a mathcing drag_end() that is called either again in handle() with a 
			/// focus_request as first argument or outside handle with a focus_attachment as first argument.
			/// The matching drag_end() call recovers the original focus configuration for the attachment.
			/// </summary>
			/// <param name="request">focus request passed to handle() function</param>
			/// <param name="pointing_not_grabbing">true if drag operation is based on pointing 
			/// at object and fals if drag operation is based on grabbing an object</param>
			/// <param name="original_config"></param>
			void drag_begin(focus_request& request, bool pointing_not_grabbing, focus_configuration& original_config) const;
			/// <summary>
			/// helper function to be used in handle() to recover original focus configuration at end of drag operation
			/// </summary>
			/// <param name="request">focus request passed to handle() function</param>
			/// <param name="original_config">focus config instance set in corresponding drag_begin() function</param>
			void drag_end(focus_request& request, focus_configuration& original_config) const;
			/// <summary>
			/// helper function to be used outside of handle() to recover original focus configuration 
			/// from corresponding drag_begin() function call.
			/// </summary>
			/// <param name="foc_att">attachment to which focus was attached</param>
			/// <param name="obj_ptr">object to which focus was attached</param>
			/// <param name="original_config">focus config instance set in corresponding drag_begin() function</param>
			/// <returns>whether focus attachment with specific object has been found and whether 
			/// original configuration could be recovered.</returns>
			bool drag_end(const focus_attachment& foc_att, cgv::base::base_ptr obj_ptr, focus_configuration& original_config) const;
		public:
			/// construct
			focusable();
			/// ask focusable what it wants to grab focus based on given event - in case of yes, the focus_demand should be filled
			virtual bool wants_to_grab_focus(const cgv::gui::event& e, const hid_identifier& hid_id, focus_demand& demand);
			/// <summary>
			/// inform focusable of focus change. Possible focus_change_actions are detach, attach or index_change.
			/// In case of attach and index_change, the focusable can refuse the attachment by returning false. 
			/// In case of index_change a refusal causes a focus detachment. For the detach focus_change_action 
			/// the return value is ignored.
			/// </summary>
			/// <param name="action">one out of detach, attach or index_change</param>
			/// <param name="rfa">cause for focus change</param>
			/// <param name="demand">in case of attach, detailed information on attachment and configuration</param>
			/// <param name="e">event causing focus change</param>
			/// <param name="dis_info">dispatch info</param>
			/// <returns>whether to accept attach or index_change - ignored for focus detach</returns>
			virtual bool focus_change(focus_change_action action, refocus_action rfa, const focus_demand& demand, const cgv::gui::event& e, const dispatch_info& dis_info);
			/// stream help information to the given output stream
			virtual void stream_help(std::ostream& os) = 0;
			//! ask active focusable to handle event providing dispatch info
			/*! return whether event was handled
				To request a focus change, fill the passed request struct and set the focus_change_request flag.*/
			virtual bool handle(const cgv::gui::event& e, const dispatch_info& dis_info, focus_request& request) = 0;
		};
	}
}

#include <cgv/config/lib_end.h>