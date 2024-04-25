#include "label_drawable.h"
#include <cgv/math/pose.h>

namespace cgv {
	namespace nui {

uint32_t label_drawable::add_label(const std::string& text, const rgba& bg_clr, int _border_x, int _border_y, int _width, int _height) 
{
	uint32_t li = lm.add_label(text, bg_clr, _border_x, _border_y, _width, _height);
	label_positions.push_back(vec3(0.0f));
	label_orientations.push_back(quat());
	label_extents.push_back(vec2(1.0f));
	label_texture_ranges.push_back(vec4(0.0f));
	label_coord_systems.push_back(coordinate_system::lab);
	label_visibilities.push_back(1);
	return li;
}
void label_drawable::update_label_text(uint32_t li, const std::string& text) 
{
	lm.update_label_text(li, text); 
}
void label_drawable::update_label_size(uint32_t li, int w, int h)
{
	lm.update_label_size(li, w, h); 
}
void label_drawable::update_label_background_color(uint32_t li, const rgba& bgclr)
{
	lm.update_label_background_color(li, bgclr); 
}
void label_drawable::fix_label_size(uint32_t li) 
{
	lm.fix_label_size(li); 
}
void label_drawable::place_label(uint32_t li, const vec3& pos, const quat& ori, coordinate_system coord_system, label_alignment align, float scale)
{
	label_extents[li] = vec2(scale * pixel_scale * lm.get_label(li).get_width(), scale * pixel_scale * lm.get_label(li).get_height());
	static vec2 offsets[5] = { vec2(0.0f,0.0f), vec2(0.5f,0.0f), vec2(-0.5f,0.0f), vec2(0.0f,0.5f), vec2(0.0f,-0.5f) };
	label_positions[li] = pos + ori.get_rotated(vec3(offsets[static_cast<int>(align)] * label_extents[li], 0.0f));
	label_orientations[li] = ori;
	label_coord_systems[li] = coord_system;
}
void label_drawable::hide_label(uint32_t li)
{
	label_visibilities[li] = 0; 
}
void label_drawable::show_label(uint32_t li) 
{
	label_visibilities[li] = 1; 
}
void label_drawable::set_label_border_color(const rgba& border_color)
{
	rrs.default_border_color = border_color;
}
void label_drawable::set_label_border_width(float border_width)
{
	rrs.percentual_border_width = border_width;
}
label_drawable::label_drawable() : lm(false)
{		
	std::fill(valid, valid + 5, false);

	rrs.map_color_to_material = cgv::render::CM_COLOR_AND_OPACITY;
	rrs.border_mode = cgv::render::RBM_MIN;
	rrs.texture_mode = cgv::render::RTM_RED_MIX_COLOR_AND_SECONDARY_COLOR;
	rrs.illumination_mode = cgv::render::IM_OFF;
}
bool label_drawable::init(cgv::render::context& ctx)
{
	cgv::render::ref_rectangle_renderer(ctx, 1);
	aam.init(ctx);
	lm.init(ctx);
	cgv::media::font::font_ptr f = cgv::media::font::default_font(true);
	ctx.enable_font_face(f->get_font_face(cgv::media::font::FFA_BOLD), 36.0f);
	lm.set_font_face(f->get_font_face(cgv::media::font::FFA_BOLD));
	lm.set_font_size(36.0f);
	lm.set_text_color(rgba(0, 0, 0, 1));
	return true;
}
void label_drawable::set_coordinate_systems(const vr::vr_kit_state* state_ptr, const mat34* table_pose_ptr)
{
	mat34 ID; ID.identity();
	pose[0] = pose[1] = ID;
	valid[0] = valid[1] = true;
	valid[2] = valid[3] = valid[4] = false;
	if (table_pose_ptr)
		pose[static_cast<int>(coordinate_system::table)] = *table_pose_ptr;
	else
		cgv::math::pose_position(pose[static_cast<int>(coordinate_system::table)]) = vec3(0.0f, 0.7f, 0.0f);

	if (state_ptr) {
		// extract poses from tracked vr devices
		valid[static_cast<int>(coordinate_system::head)] = state_ptr->hmd.status == vr::VRS_TRACKED;
		valid[static_cast<int>(coordinate_system::left_controller)] = state_ptr->controller[0].status == vr::VRS_TRACKED;
		valid[static_cast<int>(coordinate_system::right_controller)] = state_ptr->controller[1].status == vr::VRS_TRACKED;
		if (valid[static_cast<int>(coordinate_system::head)])
			pose[static_cast<int>(coordinate_system::head)] = reinterpret_cast<const mat34&>(state_ptr->hmd.pose[0]);
		if (valid[static_cast<int>(coordinate_system::left_controller)])
			pose[static_cast<int>(coordinate_system::left_controller)] = reinterpret_cast<const mat34&>(state_ptr->controller[0].pose[0]);
		if (valid[static_cast<int>(coordinate_system::right_controller)])
			pose[static_cast<int>(coordinate_system::right_controller)] = reinterpret_cast<const mat34&>(state_ptr->controller[1].pose[0]);
	}
}
void label_drawable::init_frame(cgv::render::context& ctx)
{
}
void label_drawable::clear(cgv::render::context& ctx)
{
	aam.destruct(ctx);
	lm.destruct(ctx);
	cgv::render::ref_rectangle_renderer(ctx, -1);
}
void label_drawable::draw(cgv::render::context& ctx)
{
	bool repack = lm.is_packing_outofdate();
	lm.ensure_texture_uptodate(ctx);
	if (repack) {
		for (uint32_t li = 0; li < label_texture_ranges.size(); ++li)
			label_texture_ranges[li] = lm.get_texcoord_range(li);
	}
}
void label_drawable::finish_frame(cgv::render::context& ctx)
{
	// compute label poses in lab coordinate system
	std::vector<vec3> P;
	std::vector<quat> Q;
	std::vector<vec2> E;
	std::vector<vec4> T;
	std::vector<rgba> C;
	// set poses of visible labels in valid coordinate systems
	for (uint32_t li = 0; li < label_coord_systems.size(); ++li) {
		if (label_visibilities[li] == 0 || !valid[static_cast<int>(label_coord_systems[li])])
			continue;
		mat34 label_pose = cgv::math::pose_construct(label_orientations[li], label_positions[li]);
		cgv::math::pose_transform(pose[static_cast<int>(label_coord_systems[li])], label_pose);
		P.push_back(cgv::math::pose_position(label_pose));
		mat3 L = cgv::math::pose_orientation(label_pose);
		float s = L.frobenius_norm() / sqrt(3.0f);
		Q.push_back(quat((1 / s) * L));
		E.push_back(s * label_extents[li]);
		T.push_back(label_texture_ranges[li]);
		C.push_back(lm.get_label(li).background_color);
	}
	// draw labels
	if (!P.empty()) {
		GLboolean blend = glIsEnabled(GL_BLEND); glEnable(GL_BLEND);
		GLenum blend_src, blend_dst, depth;
		glGetIntegerv(GL_BLEND_DST, reinterpret_cast<GLint*>(&blend_dst));
		glGetIntegerv(GL_BLEND_SRC, reinterpret_cast<GLint*>(&blend_src));
		glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&depth));

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		auto& rr = cgv::render::ref_rectangle_renderer(ctx);
		rr.set_render_style(rrs);
		rr.enable_attribute_array_manager(ctx, aam);
		rr.set_position_array(ctx, P);
		rr.set_rotation_array(ctx, Q);
		rr.set_extent_array(ctx, E);
		rr.set_texcoord_array(ctx, T);
		rr.set_color_array(ctx, C);
		rr.set_secondary_color(ctx, lm.get_text_color());
		lm.get_texture()->enable(ctx);
		rr.render(ctx, 0, P.size());
		lm.get_texture()->disable(ctx);
		rr.disable_attribute_array_manager(ctx, aam);

		if (!blend)
			glDisable(GL_BLEND);
		glBlendFunc(blend_src, blend_dst);
	}
}
bool label_drawable::is_coordsystem_valid(coordinate_system cs) const 
{ 
	return valid[static_cast<int>(cs)];
}
const mat34& label_drawable::get_coordsystem(coordinate_system cs) const 
{
	return pose[static_cast<int>(cs)]; 
}
	}
}