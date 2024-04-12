#include "volume_rendering.h"

#include <cgv/defines/quote.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv/reflect/reflect_enum.h>
#include <cgv/signal/rebind.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/utils/file.h>
#include <cgv/utils/big_binary_file.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

volume_viewer::volume_viewer() : application_plugin("Volume Viewer"), depth_tex("[D]")
{
	// setup volume bounding box as unit cube centered around origin
	volume_bounding_box = cgv::box3(cgv::vec3(-0.5f), cgv::vec3(0.5f));

	// configure texture format, filtering and wrapping (no context necessary)
	volume_tex = cgv::render::texture("flt32[R]");
	volume_tex.set_min_filter(cgv::render::TF_LINEAR);
	volume_tex.set_mag_filter(cgv::render::TF_LINEAR);
	volume_tex.set_wrap_s(cgv::render::TW_CLAMP_TO_BORDER);
	volume_tex.set_wrap_t(cgv::render::TW_CLAMP_TO_BORDER);
	volume_tex.set_wrap_r(cgv::render::TW_CLAMP_TO_BORDER);
	volume_tex.set_border_color(0.0f, 0.0f, 0.0f, 0.0f);

	// an extra depth texture is used to enable mixing of opaque geometry and the volume
	depth_tex.set_min_filter(cgv::render::TF_NEAREST);
	depth_tex.set_mag_filter(cgv::render::TF_NEAREST);

	vstyle.enable_depth_test = true;

	show_box = true;
	
#ifdef _DEBUG
	vres = cgv::uvec3(32);
#else
	vres = cgv::uvec3(128);
#endif
	vspacing = cgv::vec3(1.0f);

	view_ptr = nullptr;

	// instantiate a color map editor as an overlay for this viewer
	transfer_function_editor_ptr = register_overlay<cgv::app::color_map_editor>("Editor");
	// make the editor cover the whole width of the window
	transfer_function_editor_ptr->set_stretch(cgv::app::overlay::StretchOption::SO_HORIZONTAL);
	transfer_function_editor_ptr->gui_options.show_heading = false;
	// enable support for editing opacity values
	transfer_function_editor_ptr->set_opacity_support(true);
	// connect a callback function to handle changes of the transfer function
	transfer_function_editor_ptr->set_on_change_callback(std::bind(&volume_viewer::handle_transfer_function_change, this));
	
	// instantiate a color map legend to show the used transfer function
	transfer_function_legend_ptr = register_overlay<cgv::app::color_map_legend>("Legend");
	// place the legend in the top left corner
	transfer_function_legend_ptr->set_alignment(cgv::app::overlay::AlignmentOption::AO_START, cgv::app::overlay::AlignmentOption::AO_END);
	transfer_function_legend_ptr->set_title("Density");
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
	if(e.get_kind() == cgv::gui::EID_MOUSE) {
		auto& me = static_cast<cgv::gui::mouse_event&>(e);

		if(me.get_flags() & cgv::gui::EF_DND) {
			switch(me.get_action()) {
			case cgv::gui::MA_ENTER:
				return true;
			case cgv::gui::MA_DRAG:
				return true;
			case cgv::gui::MA_LEAVE:
				return true;
			case cgv::gui::MA_RELEASE:
				load_volume_from_file(me.get_dnd_text());
				return true;
			default: break;
			}
		}
	} else if(e.get_kind() == cgv::gui::EID_KEY) {
		auto& ke = static_cast<cgv::gui::key_event&>(e);
		if(ke.get_action() == cgv::gui::KA_RELEASE)
			return false;

		switch(ke.get_key()) {
		case 'B':
			show_box = !show_box;
			on_set(&show_box);
			return true;
		case 'T':
			if(transfer_function_editor_ptr) {
				transfer_function_editor_ptr->set_visibility(!transfer_function_editor_ptr->is_visible());
				post_redraw();
			}
			return true;
		default: break;
		}
	}

	return false;
}

void volume_viewer::on_set(void* member_ptr) 
{
	cgv::vec3& a = volume_bounding_box.ref_min_pnt();
	cgv::vec3& b = volume_bounding_box.ref_max_pnt();

	if (member_ptr == &a[0] ||
		member_ptr == &a[1] ||
		member_ptr == &a[2] ||
		member_ptr == &b[0] ||
		member_ptr == &b[1] ||
		member_ptr == &b[2]) {
		update_bounding_box();
	}

	if(member_ptr == &transfer_function_preset_idx)
		load_transfer_function_preset();

	update_member(member_ptr);
	post_redraw();
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
	load_transfer_function_preset();

	create_volume(ctx);
	return true;
}

void volume_viewer::init_frame(cgv::render::context& ctx) {
	if(!view_ptr) {
		view_ptr = find_view_as_node();
		
		if(view_ptr) {
			// do one-time initialization
			// set the transfer function as the to-be-edited color map in the editor
			if(transfer_function_editor_ptr)
				transfer_function_editor_ptr->set_color_map(&transfer_function);
			if(transfer_function_legend_ptr)
				transfer_function_legend_ptr->set_color_map(ctx, transfer_function);
		}
	}
	if (depth_tex.is_created() && (ctx.get_width() != depth_tex.get_width() || ctx.get_height() != depth_tex.get_height()))
		depth_tex.destruct(ctx);

	if (!depth_tex.is_created())
		depth_tex.create(ctx, cgv::render::TT_2D, ctx.get_width(), ctx.get_height());
}

void volume_viewer::draw(cgv::render::context& ctx)
{
	// default render style for the bounding box
	static const cgv::render::box_wire_render_style box_rs;

	// render the wireframe bounding box if enabled
	if (show_box)
		box_rd.render(ctx, cgv::render::ref_box_wire_renderer(ctx), box_rs);

}

void volume_viewer::after_finish(cgv::render::context & ctx)
{
	if (!view_ptr)
		return;

	// copy the contents of the depth buffer from the opaque geometry into the extra depth texture
	depth_tex.replace_from_buffer(ctx, 0, 0, 0, 0, ctx.get_width(), ctx.get_height());

	// render the volume
	auto& vr = cgv::render::ref_volume_renderer(ctx);
	vr.set_render_style(vstyle);
	vr.set_volume_texture(&volume_tex); // set volume texture as 3D scalar input data
	vr.set_transfer_function_texture(&transfer_function.ref_texture()); // get the texture from the transfer function color map to transform scalar volume values into RGBA colors
	// set the volume bounding box and enable transform to automatically place and size the volume to the defined bounds
	vr.set_bounding_box(volume_bounding_box);
	vr.set_depth_texture(&depth_tex);
	vr.transform_to_bounding_box(true);
	vr.render(ctx, 0, 0);
}

void volume_viewer::create_gui() 
{
	add_decorator("Volume Viewer", "heading", "level=2");

	add_member_control(this, "Show Box", show_box, "check");
	
	if(begin_tree_node("Volume Rendering", vstyle, true)) {
		align("\a");
		add_gui("vstyle", vstyle);
		align("\b");
		end_tree_node(vstyle);
	}

	if(begin_tree_node("Bounding Box", volume_bounding_box, false)) {
		align("/a");
		cgv::vec3& a = volume_bounding_box.ref_min_pnt();
		cgv::vec3& b = volume_bounding_box.ref_max_pnt();

		add_member_control(this, "Min X", a.x(), "value_slider", "min=-1;max=1;step=0.05;");
		add_member_control(this, "Y", a.y(), "value_slider", "min=-1;max=1;step=0.05;");
		add_member_control(this, "Z", a.z(), "value_slider", "min=-1;max=1;step=0.05;");

		add_member_control(this, "Max X", b.x(), "value_slider", "min=-1;max=1;step=0.05;");
		add_member_control(this, "Y", b.y(), "value_slider", "min=-1;max=1;step=0.05;");
		add_member_control(this, "Z", b.z(), "value_slider", "min=-1;max=1;step=0.05;");
		align("/b");
		end_tree_node(volume_bounding_box);
	}

	add_decorator("Scaling", "heading", "level=3");
	connect_copy(add_button("Fit to Resolution")->click, cgv::signal::rebind(this, &volume_viewer::fit_to_resolution));
	connect_copy(add_button("Fit to Spacing")->click, cgv::signal::rebind(this, &volume_viewer::fit_to_spacing));
	connect_copy(add_button("Fit to Both")->click, cgv::signal::rebind(this, &volume_viewer::fit_to_resolution_and_spacing));

	add_decorator("Transfer Function", "heading", "level=3");
	add_member_control(this, "Preset", transfer_function_preset_idx, "dropdown", "enums='#1 (White),#2,#3 (Aneurysm),#4 (Head)'");

	inline_object_gui(transfer_function_editor_ptr);
	
	inline_object_gui(transfer_function_legend_ptr);
}

void volume_viewer::handle_transfer_function_change() {

	if(auto ctx_ptr = get_context()) {
		auto& ctx = *ctx_ptr;
		if(transfer_function_editor_ptr) {
			transfer_function.generate_texture(ctx);
			if(transfer_function_legend_ptr)
				transfer_function_legend_ptr->set_color_map(ctx, transfer_function);
		}
	}
}

void volume_viewer::update_bounding_box() {

	box_rd.clear();
	box_rd.add(volume_bounding_box.get_center(), volume_bounding_box.get_extent());

	cgv::vec3& a = volume_bounding_box.ref_min_pnt();
	cgv::vec3& b = volume_bounding_box.ref_max_pnt();

	update_member(&a.x());
	update_member(&a.y());
	update_member(&a.z());
	update_member(&b.x());
	update_member(&b.y());
	update_member(&b.z());
	
	post_redraw();
}

void volume_viewer::load_transfer_function_preset() {

	unsigned idx = static_cast<unsigned>(transfer_function_preset_idx);
	idx = std::min(idx, 3u);

	transfer_function.clear();

	switch(idx) {
	case 0:
		// plain white with linear opacity ramp
		transfer_function.add_color_point(0.0f, cgv::rgb(1.0f));
		transfer_function.add_opacity_point(0.0f, 0.0f);
		transfer_function.add_opacity_point(1.0f, 1.0f);
		break;
	case 1:
		// blue -> red -> yellow, optimized for example volume
		transfer_function.add_color_point(0.0f, cgv::rgb(0.0f, 0.0f, 1.0f));
		transfer_function.add_color_point(0.5f, cgv::rgb(1.0f, 0.0f, 0.0f));
		transfer_function.add_color_point(1.0f, cgv::rgb(1.0f, 1.0f, 0.0f));

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
		break;
	case 2:
		// optimized for aneurysm.vox
		transfer_function.add_color_point(0.0f, cgv::rgb(1.0f, 1.0f, 1.0f));
		transfer_function.add_color_point(0.25f, cgv::rgb(0.95f, 1.0f, 0.8f));
		transfer_function.add_color_point(1.0f, cgv::rgb(1.0f, 0.4f, 0.333f));

		transfer_function.add_opacity_point(0.1f, 0.0f);
		transfer_function.add_opacity_point(1.0f, 1.0f);
		break;
	case 3:
		// optimized for head256.vox
		transfer_function.add_color_point(0.332f, cgv::rgb(0.5f, 0.8f, 0.85f));
		transfer_function.add_color_point(0.349f, cgv::rgb(0.85f, 0.5f, 0.85f));
		transfer_function.add_color_point(0.370f, cgv::rgb(0.9f, 0.85f, 0.8f));
		transfer_function.add_color_point(0.452f, cgv::rgb(0.9f, 0.85f, 0.8f));
		transfer_function.add_color_point(0.715f, cgv::rgb(0.9f, 0.85f, 0.8f));
		transfer_function.add_color_point(1.0f, cgv::rgb(1.0f, 0.0f, 0.0f));

		transfer_function.add_opacity_point(0.208f, 0.0f);
		transfer_function.add_opacity_point(0.22f, 0.17f);
		transfer_function.add_opacity_point(0.315f, 0.17f);
		transfer_function.add_opacity_point(0.326f, 0.0f);
		transfer_function.add_opacity_point(0.345f, 0.0f);
		transfer_function.add_opacity_point(0.348f, 0.23f);
		transfer_function.add_opacity_point(0.35f, 0.0f);
		transfer_function.add_opacity_point(0.374f, 0.0f);
		transfer_function.add_opacity_point(0.539f, 0.31f);
		transfer_function.add_opacity_point(0.633f, 0.31f);
		transfer_function.add_opacity_point(0.716f, 0.0f);
		transfer_function.add_opacity_point(0.8f, 1.0f);
		break;
	default: break;
	}
	
	if(auto ctx_ptr = get_context()) {
		// generate the texture containing the interpolated color map values
		transfer_function.generate_texture(*ctx_ptr);

		if(transfer_function_editor_ptr)
			transfer_function_editor_ptr->set_color_map(&transfer_function);
		if(transfer_function_legend_ptr)
			transfer_function_legend_ptr->set_color_map(*ctx_ptr, transfer_function);
	}
}

void volume_viewer::create_volume(cgv::render::context& ctx) {
	// destruct previous texture
	volume_tex.destruct(ctx);

	// calculate voxel size
	float voxel_size = 1.0f / vres.x();

	// generate volume data
	vol_data.clear();
	vol_data.resize(vres[0] * vres[1] * vres[2], 0.0f);

	std::mt19937 rng(42);
	std::uniform_real_distribution<float> distr(0.0f, 1.0f);

	const cgv::vec3& a = volume_bounding_box.ref_min_pnt();
	const cgv::vec3& b = volume_bounding_box.ref_max_pnt();

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
	// and compute mipmaps
	cgv::data::data_format vol_df(vres[0], vres[1], vres[2], cgv::type::info::TypeId::TI_FLT32, cgv::data::ComponentFormat::CF_R);
	cgv::data::const_data_view vol_dv(&vol_df, vol_data.data());
	volume_tex.create(ctx, vol_dv, 0);

	// set the volume bounding box to later scale the rendering accordingly
	volume_bounding_box.ref_min_pnt() = volume_bounding_box.ref_min_pnt();
	volume_bounding_box.ref_max_pnt() = volume_bounding_box.ref_max_pnt();

	// calculate a histogram
	create_histogram();
}

// splats n spheres of given radius into the volume, by adding the contribution to the covered voxel cells
void volume_viewer::splat_spheres(std::vector<float>& vol_data, float voxel_size, std::mt19937& rng, size_t n, float radius, float contribution) {
	std::uniform_real_distribution<float> distr(0.0f, 1.0f);

	const cgv::vec3& a = volume_bounding_box.ref_min_pnt();
	const cgv::vec3& b = volume_bounding_box.ref_max_pnt();

	for(size_t i = 0; i < n; ++i) {
		cgv::vec3 pos;
		pos.x() = cgv::math::lerp(a.x(), b.x(), distr(rng));
		pos.y() = cgv::math::lerp(a.y(), b.y(), distr(rng));
		pos.z() = cgv::math::lerp(a.z(), b.z(), distr(rng));
		splat_sphere(vol_data, voxel_size, pos, radius, contribution);
	}
}

// splats a single sphere of given radius into the volume by adding the contribution value to the voxel cells
void volume_viewer::splat_sphere(std::vector<float>& vol_data, float voxel_size, const cgv::vec3& pos, float radius, float contribution) {

	// compute the spheres bounding box
	cgv::box3 box(pos - radius, pos + radius);
	box.ref_max_pnt() -= 0.005f * voxel_size;

	// get voxel indices of bounding box minimum and maximum
	cgv::ivec3 sidx((box.get_min_pnt() - volume_bounding_box.ref_min_pnt()) / voxel_size);
	cgv::ivec3 eidx((box.get_max_pnt() - volume_bounding_box.ref_min_pnt()) / voxel_size);

	const cgv::ivec3 res = static_cast<cgv::ivec3>(vres);

	// make sure to stay inside the volume
	sidx = cgv::math::clamp(sidx, cgv::ivec3(0), res - 1);
	eidx = cgv::math::clamp(eidx, cgv::ivec3(0), res - 1);

	// for each covered voxel...
	for(int z = sidx.z(); z <= eidx.z(); ++z) {
		for(int y = sidx.y(); y <= eidx.y(); ++y) {
			for(int x = sidx.x(); x <= eidx.x(); ++x) {
				// ...get its center location in world space
				cgv::vec3 voxel_pos(
					static_cast<float>(x),
					static_cast<float>(y),
					static_cast<float>(z)
				);
				voxel_pos *= voxel_size;
				voxel_pos += volume_bounding_box.ref_min_pnt() + 0.5f*voxel_size;

				// calculate the distance to the sphere center
				float dist = length(voxel_pos - pos);
				// add contribution to voxel if its center is inside the sphere
				if(dist < radius) {
					// modulate contribution by distance to sphere center
					float dist_factor = 1.0f - (dist / radius);
					dist_factor = sqrt(dist_factor);
					vol_data[x + vres.x()*y + vres.x()*vres.y()*z] += contribution * dist_factor;
				}
			}
		}
	}
}

void volume_viewer::load_volume_from_file(const std::string& file_name) {

	std::string header_content;

	std::string hd_file_name = "";
	std::string vox_file_name = "";

	std::string extension = cgv::utils::file::get_extension(file_name);
	if(cgv::utils::to_upper(extension) == "HD") {
		hd_file_name = file_name;
		vox_file_name = file_name.substr(0, file_name.length() - 2) + "vox";
	} else if(cgv::utils::to_upper(extension) == "VOX") {
		hd_file_name = file_name.substr(0, file_name.length() - 3) + "hd";
		vox_file_name = file_name;
	}

	if(!cgv::utils::file::exists(hd_file_name) || !cgv::utils::file::exists(vox_file_name))
		return;

	std::cout << "Loading volume from: ";
	std::cout << vox_file_name << std::endl;

	if(!cgv::utils::file::read(hd_file_name, header_content, true)) {
		std::cout << "Error: failed to read header file." << std::endl;
		return;
	}

	cgv::ivec3 resolution(-1);
	cgv::vec3 spacing(1.0f);

	std::vector<cgv::utils::line> lines;
	cgv::utils::split_to_lines(header_content, lines);

	for(const auto& line : lines) {
		std::vector<cgv::utils::token> tokens;
		cgv::utils::split_to_tokens(line, tokens, "x", true, "", "", " x,");

		if(tokens.size() == 0)
			continue;

		std::string identifier = to_string(tokens[0]);
		if(identifier.length() == 0)
			continue;

		if(identifier.back() == ':')
			identifier = identifier.substr(0, identifier.length() - 1);

		if(identifier == "Size" || identifier == "Dimension") {
			int idx = 0;
			for(size_t i = 1; i < tokens.size(); ++i) {
				std::string str = to_string(tokens[i]);

				char* p_end;
				const long num = std::strtol(str.c_str(), &p_end, 10);
				if(str.c_str() != p_end) {
					resolution[idx] = static_cast<int>(num);
					++idx;
					if(idx > 2)
						break;
				}
			}
		} else if(identifier == "Spacing") {
			int idx = 0;
			for(size_t i = 1; i < tokens.size(); ++i) {
				std::string str = to_string(tokens[i]);

				char* p_end;
				const float num = std::strtof(str.c_str(), &p_end);
				if(str.c_str() != p_end) {
					spacing[idx] = num;
					++idx;
					if(idx > 2)
						break;
				}
			}
		} else {
			std::cout << "Warning: unknown identifier <" + identifier + ">" << std::endl;
		}
	}

	std::cout << "[resolution] = " << resolution << std::endl;
	std::cout << "[spacing]    = " << spacing << std::endl;

	if(cgv::math::min_value(resolution) < 0) {
		std::cout << "Error: could not read valid resolution." << std::endl;
		return;
	}

	if(cgv::math::min_value(spacing) < 0.0f) {
		std::cout << "Error: could not read valid spacing." << std::endl;
		return;
	}

	auto ctx_ptr = get_context();
	if(ctx_ptr) {
		auto& ctx = *ctx_ptr;

		vres = resolution;
		vspacing = spacing;

		size_t num_voxels = resolution.x() * resolution.y() * resolution.z();

		vol_data.clear();
		vol_data.resize(num_voxels, 0.0f);

		std::vector<unsigned char> raw_vol_data(num_voxels, 0u);

		FILE* fp = fopen(vox_file_name.c_str(), "rb");
		if(fp) {
			std::size_t nr = fread(raw_vol_data.data(), 1, num_voxels, fp);
			if(nr != num_voxels) {
				std::cout << "Error: could not read the expected number " << num_voxels << " of voxels but only " << nr << "." << std::endl;
				fclose(fp);
			}
		} else {
			std::cout << "Error: failed to read voxel file." << std::endl;
		}
		fclose(fp);

		for(size_t i = 0; i < num_voxels; ++i)
			vol_data[i] = static_cast<float>(raw_vol_data[i] / 255.0f);

		if(volume_tex.is_created())
			volume_tex.destruct(ctx);

		cgv::data::data_format vol_df(resolution[0], resolution[1], resolution[2], cgv::type::info::TypeId::TI_FLT32, cgv::data::ComponentFormat::CF_R);
		cgv::data::const_data_view vol_dv(&vol_df, vol_data.data());
		volume_tex.create(ctx, vol_dv, 0);

		fit_to_resolution();
	}

	create_histogram();
}

void volume_viewer::fit_to_resolution() {

	unsigned max_resolution = max_value(vres);
	cgv::vec3 scaling = static_cast<cgv::vec3>(vres) / static_cast<float>(max_resolution);

	volume_bounding_box.ref_min_pnt() = cgv::vec3(-0.5f*scaling);
	volume_bounding_box.ref_max_pnt() = cgv::vec3(+0.5f*scaling);

	update_bounding_box();
}

void volume_viewer::fit_to_spacing() {

	volume_bounding_box.ref_min_pnt() = cgv::vec3(-0.5f*vspacing);
	volume_bounding_box.ref_max_pnt() = cgv::vec3(+0.5f*vspacing);
	
	update_bounding_box();
}

void volume_viewer::fit_to_resolution_and_spacing() {

	unsigned max_resolution = max_value(vres);
	cgv::vec3 scaling = static_cast<cgv::vec3>(vres) / static_cast<float>(max_resolution);
	scaling *= vspacing;

	volume_bounding_box.ref_min_pnt() = cgv::vec3(-0.5f*scaling);
	volume_bounding_box.ref_max_pnt() = cgv::vec3(+0.5f*scaling);

	update_bounding_box();
}

void volume_viewer::create_histogram() {
	std::vector<unsigned> histogram(128, 0u);

	for(size_t i = 0; i < vol_data.size(); ++i) {
		size_t bucket = static_cast<size_t>(vol_data[i] * 128.0f);
		size_t min = 0;
		size_t max = 127;
		bucket = cgv::math::clamp(bucket, min, max);
		++histogram[bucket];
	}

	if(transfer_function_editor_ptr)
		transfer_function_editor_ptr->set_histogram_data(histogram);
}

#include <cgv/base/register.h>

cgv::base::factory_registration<volume_viewer> volume_viewer_fac("Volume Rendering", "shortcut='Ctrl-Alt-V';menu_text='New/Render/Volume Rendering'", true);
