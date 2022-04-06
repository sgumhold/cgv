#include "volume_rendering.h"

#include <cgv/defines/quote.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv/reflect/reflect_enum.h>
#include <cgv/signal/rebind.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace reflect {
	}
}

volume_viewer::volume_viewer() : application_plugin("Volume Viewer")
{
	// setup volume bounding box as unit cube centered around origin
	volume_bounding_box = box3(vec3(-0.5f), vec3(0.5f));

	// configure texture format, filtering and wrapping (no context necessary)
	volume_tex = cgv::render::texture("flt32[R]");
	volume_tex.set_min_filter(cgv::render::TF_LINEAR);
	volume_tex.set_mag_filter(cgv::render::TF_LINEAR);
	volume_tex.set_wrap_s(cgv::render::TW_CLAMP_TO_BORDER);
	volume_tex.set_wrap_t(cgv::render::TW_CLAMP_TO_BORDER);
	volume_tex.set_wrap_r(cgv::render::TW_CLAMP_TO_BORDER);
	volume_tex.set_border_color(0.0f, 0.0f, 0.0f, 0.0f);

	vstyle.enable_depth_test = false;

	show_box = true;
	
	vres = uvec3(128);

	view_ptr = nullptr;

	// instantiate a color map editor as an overlay for this viewer
	transfer_function_editor_ptr = register_overlay<cgv::glutil::color_map_editor>("TF Editor");
	transfer_function_editor_ptr->gui_options.show_heading = false;
	// enable support for editing opacity values
	transfer_function_editor_ptr->set_opacity_support(true);
}

void volume_viewer::stream_stats(std::ostream& os)
{
	os << "volume_viewer: resolution=" << vres[0] << "x" << vres[1] << "x" << vres[2] << std::endl;
}

bool volume_viewer::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return
		rh.reflect_member("show_box", show_box);
}

void volume_viewer::stream_help(std::ostream& os) 
{
	os << "volume_viewer: toggle <B>ox\n, toggle <T>ransfer function editor, ctrl+click in transfer function editor to add points, alt+click to remove";
}

bool volume_viewer::handle_event(cgv::gui::event& e) 
{
	if (e.get_kind() != cgv::gui::EID_KEY)
		return false;
	auto& ke = static_cast<cgv::gui::key_event&>(e);
	if (ke.get_action() == cgv::gui::KA_RELEASE)
		return false;
	switch (ke.get_key()) {
	case 'B': show_box = !show_box; on_set(&show_box); return true;
	case 'T':
		if(transfer_function_editor_ptr) {
			transfer_function_editor_ptr->set_visibility(!transfer_function_editor_ptr->is_visible());
			post_redraw();
		}
		return true;
	default: break;
	}
	return false;
}

void volume_viewer::on_set(void* member_ptr) 
{
	update_member(member_ptr);
	post_redraw();
}

void volume_viewer::create_volume(cgv::render::context& ctx)
{
	// destruct previous texture
	volume_tex.destruct(ctx);

	// calculate voxel size
	float voxel_size = 1.0f / vres.x();

	// generate volume data
	std::vector<float> vol_data(vres[0] * vres[1] * vres[2], 0.0f);

	std::mt19937 rng(42);
	std::uniform_real_distribution<float> distr(0.0f, 1.0f);

	const vec3& a = volume_bounding_box.ref_min_pnt();
	const vec3& b = volume_bounding_box.ref_max_pnt();

	// generate a single large sphere in the center of the volume
	splat_sphere(vol_data, voxel_size, 0.5f*(a + b), 0.5f, 0.75f);

	// add and subtract volumes of an increasing amount of randomly placed spheres of decreasing size
	splat_spheres(vol_data, voxel_size, rng, 5, 0.2f, 0.5f);
	splat_spheres(vol_data, voxel_size, rng, 5, 0.2f, -0.5f);

	splat_spheres(vol_data, voxel_size, rng, 50, 0.1f, 0.25f);
	splat_spheres(vol_data, voxel_size, rng, 50, 0.1f, -0.25f);

	splat_spheres(vol_data, voxel_size, rng, 100, 0.05f, 0.1f);
	splat_spheres(vol_data, voxel_size, rng, 100, 0.05f, -0.1f);

	splat_spheres(vol_data, voxel_size, rng, 200, 0.025f, 0.1f);
	splat_spheres(vol_data, voxel_size, rng, 200, 0.025f, -0.1f);

	// make sure the volume values are in the range [0,1]
	for(size_t i = 0; i < vol_data.size(); ++i)
		vol_data[i] = cgv::math::clamp(vol_data[i], 0.0f, 1.0f);

	// transfer volume data into volume texture
	//and compute mipmaps
	cgv::data::data_format vol_df(vres[0], vres[1], vres[2], cgv::type::info::TypeId::TI_FLT32, cgv::data::ComponentFormat::CF_R);
	cgv::data::const_data_view vol_dv(&vol_df, vol_data.data());
	volume_tex.create(ctx, vol_dv, 0);

	// set the volume bounding box to later scale the rendering accordingly
	volume_bounding_box.ref_min_pnt() = volume_bounding_box.ref_min_pnt();
	volume_bounding_box.ref_max_pnt() = volume_bounding_box.ref_max_pnt();
}

// splates n spheres of given radius into the volume, by adding the contribution to the covered voxel cells
void volume_viewer::splat_spheres(std::vector<float>& vol_data, float voxel_size, std::mt19937& rng, size_t n, float radius, float contribution) {
	std::uniform_real_distribution<float> distr(0.0f, 1.0f);

	const vec3& a = volume_bounding_box.ref_min_pnt();
	const vec3& b = volume_bounding_box.ref_max_pnt();

	for(size_t i = 0; i < n; ++i) {
		vec3 pos;
		pos.x() = cgv::math::lerp(a.x(), b.x(), distr(rng));
		pos.y() = cgv::math::lerp(a.y(), b.y(), distr(rng));
		pos.z() = cgv::math::lerp(a.z(), b.z(), distr(rng));
		splat_sphere(vol_data, voxel_size, pos, radius, contribution);
	}
}

// spalts a single sphere of given radius into the volume by adding the contribution value to the voxel cells
void volume_viewer::splat_sphere(std::vector<float>& vol_data, float voxel_size, const vec3& pos, float radius, float contribution) {

	// compute the spheres bounding box
	box3 box(pos - radius, pos + radius);
	box.ref_max_pnt() -= 0.005f * voxel_size;

	// get voxel indices of bounding box minimum and maximum
	ivec3 sidx((box.get_min_pnt() - volume_bounding_box.ref_min_pnt()) / voxel_size);
	ivec3 eidx((box.get_max_pnt() - volume_bounding_box.ref_min_pnt()) / voxel_size);

	const ivec3 res = static_cast<ivec3>(vres);

	// make sure to stay inside the volume
	sidx = cgv::math::clamp(sidx, ivec3(0), res - 1);
	eidx = cgv::math::clamp(eidx, ivec3(0), res - 1);

	// for each covered voxel...
	for(int z = sidx.z(); z <= eidx.z(); ++z) {
		for(int y = sidx.y(); y <= eidx.y(); ++y) {
			for(int x = sidx.x(); x <= eidx.x(); ++x) {
				// ...get its center location in world space
				vec3 voxel_pos(
					static_cast<float>(x),
					static_cast<float>(y),
					static_cast<float>(z)
				);
				voxel_pos *= voxel_size;
				voxel_pos += volume_bounding_box.ref_min_pnt() + 0.5f*voxel_size;

				// calculate the distance to the sphere center
				float dist = length(voxel_pos - pos);
				if(dist < radius) {
					// add contribution to voxel if its center is inside the sphere
					vol_data[x + vres.x()*y + vres.x()*vres.y()*z] += contribution;
				}
			}
		}
	}
}

void volume_viewer::clear(cgv::render::context& ctx)
{
	cgv::render::ref_volume_renderer(ctx, -1);
	cgv::render::ref_box_wire_renderer(ctx, -1);
}

bool volume_viewer::init(cgv::render::context& ctx)
{
	cgv::render::ref_volume_renderer(ctx, 1);
	cgv::render::ref_box_wire_renderer(ctx, 1);

	// init the box wire render data object
	box_rd.init(ctx);
	// add the volume bounding box
	box_rd.add(volume_bounding_box.get_center(), volume_bounding_box.get_extent());

	// init a color map used as a transfer function
	transfer_function.init(ctx);
	// Preset 1
	//transfer_function.add_color_point(0.0f, rgb(1.0f));
	//transfer_function.add_opacity_point(0.0f, 0.0f);
	//transfer_function.add_opacity_point(1.0f, 1.0f);

	// Preset 2
	transfer_function.add_color_point(0.0f, rgb(0.0f, 0.0f, 1.0f));
	transfer_function.add_color_point(0.5f, rgb(1.0f, 0.0f, 0.0f));
	transfer_function.add_color_point(1.0f, rgb(1.0f, 1.0f, 0.0f));

	transfer_function.add_opacity_point(0.05f, 0.0f);
	transfer_function.add_opacity_point(0.1f, 0.1f);
	transfer_function.add_opacity_point(0.3f, 0.1f);
	transfer_function.add_opacity_point(0.35f, 0.0f);
	transfer_function.add_opacity_point(0.35f, 0.0f);
	transfer_function.add_opacity_point(0.45f, 0.0f);
	transfer_function.add_opacity_point(0.5f, 0.15f);
	transfer_function.add_opacity_point(0.55f, 0.15f);
	transfer_function.add_opacity_point(0.6f, 0.0f);
	transfer_function.add_opacity_point(0.8f, 0.0f);
	transfer_function.add_opacity_point(0.95f, 0.5f);

	// generate the texture containing the interpolated color map values
	transfer_function.generate_texture(ctx);

	create_volume(ctx);
	return true;
}

void volume_viewer::init_frame(cgv::render::context& ctx) {
	if(!view_ptr) {
		view_ptr = find_view_as_node();
		
		if(view_ptr) {
			// do one-time initialization
			// set the transfer function as the to-be-edited color map in the editor
			transfer_function_editor_ptr->set_color_map(&transfer_function);
		}
	}

	// check for changes in transfer function and update texture if necessary
	if(transfer_function_editor_ptr && transfer_function_editor_ptr->was_updated()) {
		transfer_function.generate_texture(ctx);
	}
}

void volume_viewer::draw(cgv::render::context& ctx) 
{
	// render the wireframe bounding box if enabled
	if(show_box)
		box_rd.render(ctx, cgv::render::ref_box_wire_renderer(ctx), cgv::render::box_wire_render_style());

	// render the volume
	auto& vr = cgv::render::ref_volume_renderer(ctx);
	vr.set_render_style(vstyle);
	vr.set_volume_texture(&volume_tex); // set volume texture as 3D scalar input data
	vr.set_transfer_function_texture(&transfer_function.ref_texture()); // get the texture from the transfer function color map to transform scalar volume values into RGBA colors
	// set the volume bounding box and enable transform to automatically place and size the volume to the defined bounds
	vr.set_bounding_box(volume_bounding_box);
	vr.transform_to_bounding_box(true);

	vr.render(ctx, 0, 0);
}

void volume_viewer::create_gui() 
{
	add_decorator("Gradient Viewer", "heading", "level=2");

	add_member_control(this, "Show Box", show_box, "check");
	
	if(begin_tree_node("Volume Rendering", vstyle, false)) {
		align("\a");
		add_gui("vstyle", vstyle);
		align("\b");
		end_tree_node(vstyle);
	}

	if(begin_tree_node("Transfer Function Editor", transfer_function_editor_ptr, false)) {
		align("\a");
		inline_object_gui(transfer_function_editor_ptr);
		align("\b");
		end_tree_node(transfer_function_editor_ptr);
	}
}

#include <cgv/base/register.h>

cgv::base::factory_registration<volume_viewer> volume_viewer_fac("New/Render/Volume Rendering");
