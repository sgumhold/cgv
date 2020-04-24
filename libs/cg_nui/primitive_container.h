#pragma once

#include <cgv/render/drawable.h>
#include <cgv_gl/renderer.h>
#include "contact_info.h"
#include "bounding_box_cache.h"
#include "nui_interactable.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		enum PrimitiveType
		{
			PT_SPHERE,
			PT_SURFEL,
			PT_RECTANGLE,
			PT_BOX,
			PT_ARROW,
			PT_OTHER
		};

		class CGV_API nui_node;

		class CGV_API primitive_container : public cgv::render::drawable, public bounding_box_cache, public nui_interactable
		{
		protected:
			/// primtive type
			PrimitiveType type;
			/// primitive centers
			std::vector<vec3> center_positions;
			/// primtive colors
			std::vector<rgba> colors;
			/// whether to use colors
			bool use_colors;
			/// orientations
			std::vector<quat> orientations;
			/// whether to use orientation
			bool use_orientations;
			/// scaling information
			std::vector<float> uniform_scales;
			std::vector<vec3> scales;
			///
			nui_node* parent;
			virtual void prepare_render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr = 0) const;
			virtual bool render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr = 0) const;
		public:
			primitive_container(nui_node* _parent, PrimitiveType _type, bool _use_colors, bool _use_orientations, ScalingMode _scaling_mode, InteractionCapabilities ic = IC_ALL);
			virtual ~primitive_container();
			nui_node* get_parent() const;
			/// return primitive type
			virtual std::string get_primitive_type() const = 0;
			uint32_t get_nr_primitives() const { return center_positions.size(); }
			box3 get_bounding_box(uint32_t i) const;
			virtual box3 get_oriented_bounding_box(uint32_t i) const = 0;
			/// return whether a closer point has been found
			virtual bool compute_closest_point(contact_info& info, const vec3& pos) = 0;
			/// return whether a closer point has been found; last parameter is weight for trading between position and normal distances for closest oriented point query; default implementation defers call to computer_closest_point
			virtual bool compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight = 0.5f);
			virtual bool compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction) = 0;
			virtual int compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points = true) = 0;
			virtual const cgv::render::render_style* get_render_style() const = 0;
			/// access to primitive placements
			bool has_orientations() const { return use_orientations; }
			quat get_orientation(uint32_t i);
			bool set_orientation(uint32_t i, const quat& q);
			vec3 get_position(uint32_t i) const;
			void set_position(uint32_t i, const vec3& p);
			
			float get_uniform_scale(uint32_t i) const;
			bool set_uniform_scale(uint32_t i, float u);
			vec3 get_scale(uint32_t i) const;
			bool set_scale(uint32_t i, const vec3& s);
		};
	}
}

#include <cgv/config/lib_end.h>