#pragma once

#include <cgv/base/group.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include "bounding_box_cache.h"
#include "nui_interactable.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class nui_node;

		typedef cgv::data::ref_ptr<nui_node, true> nui_node_ptr;

		class CGV_API nui_node :
			public cgv::base::group, 
			public cgv::render::drawable,
			public cgv::gui::event_handler,
			public bounding_box_cache,
			public nui_interactable,
			public cgv::gui::provider
		{
		protected:
			/// rotation part of node transformation
			quat rotation;
			/// translation part of node transformation
			vec3 translation;
			/// scene extend and maybe scale of node transformation
			vec3 scale;
			///
			void correct_contact_info(contact_info::contact& C, const mat4& M, const mat4& inv_M, const vec3& pos);
		public:
			nui_node(const std::string& _name, ScalingMode _scaling_mode = SM_NONE);
			virtual ~nui_node();
			void stream_help(std::ostream& os);
			bool handle(cgv::gui::event& e);
			void integrate_child_node(nui_node_ptr child_node_ptr, bool init_drawable = true);
			void desintegrate_child_node(nui_node_ptr child_node_ptr, bool clear_drawable = true);
			uint32_t get_nr_primitives() const;
			box3 get_bounding_box(uint32_t i) const;
			box3 compute_and_transform_bounding_box() const;
			virtual bool compute_closest_point(contact_info& info, const vec3& pos);
			virtual bool compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight = 0.5f);
			virtual bool compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction);
			virtual int  compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points = true);

			/// access to model view transformation
			mat4 get_model_matrix() const;
			mat4 get_inverse_model_matrix() const;
			dmat4 get_node_to_world_transformation() const;
			dmat4 get_world_to_node_transformation() const;

			// access to transformation and extend
			const quat& get_rotation() const { return rotation; }
			void set_rotation(const quat& _rotation) { rotation = _rotation; }
			const vec3& get_translation() const { return translation; }
			const vec3& get_scale() const { return scale; }
			void set_translation(const vec3& _translation) { translation = _translation; }
			void set_scale(const vec3& _scale) { scale = _scale; }
			// call methods of containers
			void draw(cgv::render::context& ctx);
			void finish_draw(cgv::render::context& ctx);

			void create_gui();
		};
	}
}

#include <cgv/config/lib_end.h>