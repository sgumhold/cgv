#pragma once

#include <cgv/render/drawable.h>
#include <cgv_gl/renderer.h>
#include "contact_info.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class bounding_box_cache : public cgv::render::render_types
		{
		protected:
			///
			mutable box3 box;
			/// 
			mutable bool box_outofdate;
		public:
			bounding_box_cache();
			void check_box_update(const box3& old_box, const box3& new_box) const;
			virtual uint32_t get_nr_primitives() const = 0;
			virtual box3 get_bounding_box(uint32_t i) const = 0;
			const box3& compute_bounding_box() const;
		};


		enum PrimitiveType
		{
			PT_SPHERE,
			PT_SURFEL,
			PT_RECTANGLE,
			PT_BOX,
			PT_ARROW,
			PT_OTHER
		};

		enum ScalingMode
		{
			SM_NONE,
			SM_UNIFORM,
			SM_NON_UNIFORM
		};

		enum InteractionCapabilities
		{
			IC_NONE = 0,
			IC_APPROACH = 1,
			IC_TRANSLATE = 2,
			IC_ROTATE = 4,
			IC_UNIFORM_SCALE = 8,
			IC_SCALE = 16,
			IC_THROW = 32,
			IC_ALL = 63
		};

		class CGV_API nui_node;

		class CGV_API primitive_container : public cgv::render::drawable, public bounding_box_cache
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
			/// mode of using scales
			ScalingMode scaling_mode;
			///
			nui_node* parent;
			///
			InteractionCapabilities interaction_capabilities;
			virtual void prepare_render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr = 0) const;
			virtual bool render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr = 0) const;
			void primitive_container::consider_closest_point(uint32_t i, contact_info& info, float distance, const vec3& p, const vec3& n, const vec3& tc);
		public:
			primitive_container(nui_node* _parent, PrimitiveType _type, bool _use_colors, bool _use_orientations, ScalingMode _scaling_mode, InteractionCapabilities ic = IC_ALL);
			virtual ~primitive_container();
			nui_node* get_parent() const;
			/// return primitive type
			virtual std::string get_primitive_type() const = 0;
			uint32_t get_nr_primitives() const { return center_positions.size(); }
			virtual void compute_closest_point(contact_info& info, const vec3& pos) = 0;
			/// last parameter is weight for trading between position and normal distances for closest oriented point query; default implementation defers call to computer_closest_point
			virtual void compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight = 0.5f);
			virtual void compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction) = 0;
			virtual void compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points = true) = 0;
			virtual const cgv::render::render_style* get_render_style() const = 0;
			// interaction capabilities
			InteractionCapabilities get_interaction_capabilities() const { return interaction_capabilities; }
			void set_interaction_capabilities(InteractionCapabilities ic);
			bool approachable() const { return (interaction_capabilities & IC_APPROACH) != 0; }
			bool translatable() const { return (interaction_capabilities & IC_TRANSLATE) != 0; }
			bool rotatable() const { return use_orientations && ((interaction_capabilities & IC_ROTATE) != 0); }
			bool scalable() const { return (scaling_mode == SM_NON_UNIFORM) && ((interaction_capabilities & IC_SCALE) != 0); }
			bool uniform_scalable() const { return (scaling_mode == SM_UNIFORM) && ((interaction_capabilities & IC_UNIFORM_SCALE) != 0); }
			bool throwable() const { return (interaction_capabilities & IC_THROW) != 0; }
			/// access to primitive placements
			bool has_orientations() const { return use_orientations; }
			quat get_orientation(uint32_t i);
			bool set_orientation(uint32_t i, const quat& q);
			vec3 get_position(uint32_t i) const;
			void set_position(uint32_t i, const vec3& p);
			
			ScalingMode get_scaling_mode() const { return scaling_mode; };
			float get_uniform_scale(uint32_t i) const;
			bool set_uniform_scale(uint32_t i, float u);
			vec3 get_scale(uint32_t i) const;
			bool set_scale(uint32_t i, const vec3& s);
		};
	}
}

#include <cgv/config/lib_end.h>