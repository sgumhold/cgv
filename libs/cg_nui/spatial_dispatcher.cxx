#include "spatial_dispatcher.h"
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/event_handler.h>
#include <libs/cg_vr/vr_events.h>

namespace cgv {
	namespace nui {

		void spatial_dispatcher::update_geometric_info_recursive(cgv::base::base_ptr root_ptr, cgv::base::base_ptr object_ptr, geometric_info& gi, bool recurse) const
		{
			auto* focusable_ptr = object_ptr->get_interface<focusable>();
			if (focusable_ptr) {
				if (gi.check_intersection) {
					auto* p = object_ptr->get_interface<pointable>();
					if (p) {
						vec3 hit_normal(0.0f);
						float hit_param;
						size_t primitive_index = 0;
						if (p->compute_intersection(gi.inter_info.ray_origin, gi.inter_info.ray_direction, hit_param, hit_normal, primitive_index)) {
							if (hit_param > 0 && hit_param < max_pointing_distance && hit_param < gi.inter_info.ray_param) {
								gi.inter_info.ray_param = hit_param;
								gi.inter_info.hit_normal = hit_normal;
								gi.inter_info.primitive_index = primitive_index;
								gi.inter_foc_info = { object_ptr, root_ptr, default_focus_info.config };
							}
						}
					}
				}
				if (gi.check_proximity) {
					auto* g = object_ptr->get_interface<grabable>();
					if (g) {
						vec3 closest_point;
						vec3 closest_normal(0.0f);
						size_t primitive_index = 0;
						if (g->compute_closest_point(gi.prox_info.query_point, closest_point, closest_normal, primitive_index)) {
							float distance = (closest_point - gi.prox_info.query_point).length();
							if (distance < max_grabbing_distance && distance < gi.prox_info.closest_distance) {
								gi.prox_info.closest_distance = distance;
								gi.prox_info.hit_point = closest_point;
								gi.prox_info.hit_normal = closest_normal;
								gi.prox_info.primitive_index = primitive_index;
								gi.prox_foc_info = { object_ptr, root_ptr, default_focus_info.config };
							}
						}
					}
				}
			}
			if (!recurse)
				return;
			// next check for group and children thereof
			auto grp_ptr = object_ptr->cast<cgv::base::group>();
			if (grp_ptr) {
				for (unsigned ci = 0; ci < grp_ptr->get_nr_children(); ++ci)
					update_geometric_info_recursive(root_ptr, grp_ptr->get_child(ci), gi, recurse);
			}
		}
		spatial_dispatcher::spatial_dispatcher()
		{
		}
		bool spatial_dispatcher::dispatch_spatial(const focus_attachment& foc_att, const cgv::gui::event& e, const hid_identifier& hid_id, refocus_info& rfi, bool* handle_called_ptr)
		{
			// next check mouse and pose events for proximity and intersection
			geometric_info gi;
			if (e.get_kind() == cgv::gui::EID_MOUSE) {
				const auto& me = reinterpret_cast<const cgv::gui::mouse_event&>(e);
				if (me.get_action() == cgv::gui::MA_ENTER || me.get_action() == cgv::gui::MA_MOVE || me.get_action() == cgv::gui::MA_DRAG) {
					gi.check_intersection = rfi.foc_info_ptr->config.spatial.pointing;
					gi.inter_info.ray_origin = cgv::render::context::get_model_point(me.get_x(), me.get_y(), 0.0, MVPW);
					gi.inter_info.ray_direction = normalize(cgv::render::context::get_model_point(me.get_x(), me.get_y(), 0.1, MVPW) - gi.inter_info.ray_origin);
					gi.check_proximity = false;
				}
				else {
					if (*rfi.dis_info_ptr_ptr && (*rfi.dis_info_ptr_ptr)->hid_id == hid_id) {
						if (handle_called_ptr)
							*handle_called_ptr = true;
						return dispath_with_focus_update(rfi.foc_info_ptr->root, rfi.foc_info_ptr->object, e, **rfi.dis_info_ptr_ptr, rfi);
					}
					return false;
				}
			}
			else
				if (e.get_kind() == cgv::gui::EID_POSE && ((e.get_flags() & cgv::gui::EF_VR) != 0)) {
					const auto& vrpe = reinterpret_cast<const cgv::gui::vr_pose_event&>(e);
					int ci = vrpe.get_trackable_index();
					if (ci >= 0 && ci <= 1) {
						gi.check_intersection = rfi.foc_info_ptr->config.spatial.pointing && ctrl_infos[ci].pointing;
						vrpe.get_state().controller[ci].put_ray(gi.inter_info.ray_origin, gi.inter_info.ray_direction);
						gi.check_proximity = rfi.foc_info_ptr->config.spatial.proximity && ctrl_infos[ci].grabbing;
						gi.prox_info.query_point = gi.inter_info.ray_origin + max_grabbing_distance*gi.inter_info.ray_direction;
						gi.inter_info.ray_origin += min_pointing_distance * gi.inter_info.ray_direction;
						if (!gi.check_proximity && !gi.check_intersection)
							return false;
					}
				}
				else {
					if (*rfi.dis_info_ptr_ptr && (*rfi.dis_info_ptr_ptr)->hid_id == hid_id) {
						if (handle_called_ptr)
							*handle_called_ptr = true;
						return dispath_with_focus_update(rfi.foc_info_ptr->root, rfi.foc_info_ptr->object, e, **rfi.dis_info_ptr_ptr, rfi);
					}
					return false;
				}
			if (rfi.foc_info_ptr->config.spatial.only_focus)
				update_geometric_info_recursive(rfi.foc_info_ptr->root, rfi.foc_info_ptr->object, gi, false);
			else
				// traverse all hierarchies to find closest object and first intersection
				for (auto root_ptr : objects)
					update_geometric_info_recursive(root_ptr, root_ptr, gi);

			// remove checks where spatial analysis failed
			if (rfi.foc_info_ptr->config.refocus.spatial) {
				if (gi.prox_info.closest_distance == std::numeric_limits<float>::max())
					gi.check_proximity = false;
				if (gi.inter_info.ray_param == std::numeric_limits<float>::max())
					gi.check_intersection = false;
			}
			// in case of proximity and intersection select only one of both based on distance threshold
			if (gi.check_proximity && gi.check_intersection) {
				if (gi.prox_info.closest_distance < intersection_bias * gi.inter_info.ray_param)
					gi.check_intersection = false;
				else
					gi.check_proximity = false;
			}
			focus_info* foc_info_ptr = 0;
			dispatch_info dummy = { hid_id, dispatch_mode::none };
			dispatch_info* di_ptr = &dummy;
			refocus_action rfa = refocus_action::none;
			proximity_dispatch_info pdi;
			intersection_dispatch_info idi;
			// if we are left with proximity handle event
			if (gi.check_proximity) {
				pdi.hid_id = hid_id;
				pdi.mode = dispatch_mode::proximity;
				pdi.query_point = gi.prox_info.query_point;
				pdi.hit_point = gi.prox_info.hit_point;
				pdi.primitive_index = gi.prox_info.primitive_index;
				// transfer normal if it was computed before
				if (gi.prox_info.hit_normal.length() > 0.01f)
					pdi.hit_normal = gi.prox_info.hit_normal;
				else {
					// in case of not availability try to estimate from vector from closest to reference point
					pdi.hit_normal = gi.prox_info.hit_point - gi.prox_info.query_point;
					// invert normal in case of inside points marked with negative distance
					if (gi.prox_info.closest_distance < 0)
						pdi.hit_normal = -pdi.hit_normal;
					float length = pdi.hit_normal.length();
					if (length > 0.0001f)
						pdi.hit_normal.normalize();
					else
						// what can fail if they are very close
						pdi.hit_normal = vec3(0.0f);
				}
				di_ptr = &pdi;
				if (rfi.report_ptr) {
					rfi.report_ptr->mode = dispatch_mode::proximity;
					if (rfi.report_ptr->di_ptr_ptr)
						*rfi.report_ptr->di_ptr_ptr = new proximity_dispatch_info(pdi);
				}
				foc_info_ptr = &gi.prox_foc_info;
				rfa = refocus_action::proximity;
			}
			// otherwise we are left with intersecion and process pointing request
			else if (gi.check_intersection) {
				idi.hid_id = hid_id;
				idi.mode = dispatch_mode::pointing;
				idi.ray_origin = gi.inter_info.ray_origin;
				idi.ray_direction = gi.inter_info.ray_direction;
				idi.primitive_index = gi.inter_info.primitive_index;
				idi.ray_param = gi.inter_info.ray_param;
				idi.hit_point = gi.inter_info.ray_origin + gi.inter_info.ray_param * gi.inter_info.ray_direction;
				idi.hit_normal = gi.inter_info.hit_normal;
				di_ptr = &idi;
				if (rfi.report_ptr) {
					rfi.report_ptr->mode = dispatch_mode::pointing;
					if (rfi.report_ptr->di_ptr_ptr)
						*rfi.report_ptr->di_ptr_ptr = new intersection_dispatch_info(idi);
				}
				foc_info_ptr = &gi.inter_foc_info;
				rfa = refocus_action::intersection;
			}
			bool focus_kept = false;
			// in case of no spatial refocusing, just call handle directly
			if (rfi.foc_info_ptr && !rfi.foc_info_ptr->config.refocus.spatial) {
				if (handle_called_ptr)
					*handle_called_ptr = true;
				return dispath_with_focus_update(rfi.foc_info_ptr->root, rfi.foc_info_ptr->object, e, *di_ptr, rfi);
			}
			// if we had object in focus before
			if (rfi.foc_info_ptr && rfi.foc_info_ptr->object) {
				// and if this keeps focus, nothing to be done here
				if (foc_info_ptr && (foc_info_ptr->object == rfi.foc_info_ptr->object))
					focus_kept = true;
				else {
					focus_kept = false;
					if (rfi.foc_info_ptr) {
						if (foc_info_ptr && foc_info_ptr->object) {
							if (!rfi.foc_info_ptr->config.refocus.transfer)
								focus_kept = true;
						}
						else
							if (!rfi.foc_info_ptr->config.refocus.deattach)
								focus_kept = true;
					}
					if (!focus_kept)
						detach_focus(rfa, foc_att, *rfi.foc_info_ptr, e, *di_ptr, rfi);
				}
			}
			// in case of a new object in focus, attach focus
			if (foc_info_ptr && foc_info_ptr->object) {
				if (!focus_kept) {
					// attach focus to object
					focus_attachment new_foc_att;
					new_foc_att.level = focus_level::hid;
					new_foc_att.hid_id = hid_id;
					attach_focus(rfa, new_foc_att, *foc_info_ptr, e, *di_ptr, rfi);
				}
			}
			// dispatch with extended dispatch info
			if (rfi.foc_info_ptr) {
				if (handle_called_ptr)
					*handle_called_ptr = true;
				return dispath_with_focus_update(rfi.foc_info_ptr->root, rfi.foc_info_ptr->object, e, *di_ptr, rfi);
			}
			return false;
		}
	}
}
