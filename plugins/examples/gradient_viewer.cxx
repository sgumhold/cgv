#include "gradient_viewer.h"
#include <cgv/defines/quote.h>
#include <cgv/math/ftransform.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/reflect/reflect_enum.h>
#include <cgv/signal/rebind.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv/media/image/image_reader.h>

namespace cgv {
	namespace reflect {
	}
}

gradient_viewer::gradient_viewer ()
// configure texture format, filtering and wrapping (no context necessary)
	: tf_tex("[R,G,B,A]"), volume_tex("flt32[R]"), gradient_tex("flt32[R,G,B,A]"), depth_tex("[D]")
{
	volume_bounding_box = cgv::box3(cgv::vec3(0.0f), cgv::vec3(1.0f));

	tf_tex.set_min_filter(cgv::render::TextureFilter::TF_LINEAR);
	tf_tex.set_mag_filter(cgv::render::TextureFilter::TF_LINEAR);
	tf_tex.set_wrap_s(cgv::render::TextureWrap::TW_CLAMP_TO_EDGE);
	tf_tex.set_wrap_t(cgv::render::TextureWrap::TW_CLAMP_TO_EDGE);

	volume_tex.set_min_filter(cgv::render::TF_LINEAR_MIPMAP_LINEAR);
	volume_tex.set_mag_filter(cgv::render::TF_LINEAR);
	volume_tex.set_wrap_s(cgv::render::TW_CLAMP_TO_BORDER);
	volume_tex.set_wrap_t(cgv::render::TW_CLAMP_TO_BORDER);
	volume_tex.set_wrap_r(cgv::render::TW_CLAMP_TO_BORDER);
	volume_tex.set_border_color(0.0f, 0.0f, 0.0f, 0.0f);

	gradient_tex.set_min_filter(cgv::render::TF_LINEAR);
	gradient_tex.set_mag_filter(cgv::render::TF_LINEAR);
	gradient_tex.set_wrap_s(cgv::render::TW_CLAMP_TO_EDGE);
	gradient_tex.set_wrap_t(cgv::render::TW_CLAMP_TO_EDGE);
	gradient_tex.set_wrap_r(cgv::render::TW_CLAMP_TO_EDGE);

	// an extra depth texture is used to enable mixing of opaque geometry and the volume
	depth_tex.set_min_filter(cgv::render::TF_NEAREST);
	depth_tex.set_mag_filter(cgv::render::TF_NEAREST);

	set_name("Gradient Viewer");

	show_volume = true;
	show_gradients = true;
	shape = S_ELLIPSOID;
	gradient_mode = GM_SOBEL_OPTIMIZED;

	vres = cgv::uvec3(128, 64, 29); // odd shaped resolution to show the shader is not limited to nice power-of-two resolutions

	do_calculate_gradients = false;

	astyle.length_scale = 0.05f;
	astyle.radius_relative_to_length = 0.05f;

	view_ptr = nullptr;
}

void gradient_viewer::stream_stats(std::ostream& os)
{
	os << "gradient_viewer: resolution=" << vres[0] << "x" << vres[1] << "x" << vres[2] << std::endl;
}

bool gradient_viewer::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return 
		rh.reflect_member("show_gradients", show_gradients)&&
		rh.reflect_member("show_volume", show_volume);
}

void gradient_viewer::stream_help(std::ostream& os) 
{
	os << "gradient_viewer: toggle <G>radient, toggle <V>olume\n";
}

bool gradient_viewer::handle(cgv::gui::event& e) 
{
	if (e.get_kind() != cgv::gui::EID_KEY)
		return false;
	auto& ke = static_cast<cgv::gui::key_event&>(e);
	if (ke.get_action() == cgv::gui::KA_RELEASE)
		return false;
	switch (ke.get_key()) {
	case 'V': show_volume = !show_volume; on_set(&show_volume); return true;
	case 'G': show_gradients = !show_gradients; on_set(&show_gradients); return true;
	default: break;
	}
	return false;
}

void gradient_viewer::on_set(void* member_ptr) 
{
	if (member_ptr == &gradient_mode)
		do_calculate_gradients = true;
	update_member(member_ptr);
	post_redraw();
}

void gradient_viewer::create_test_volume(cgv::render::context& ctx)
{
	// destruct previous textures
	volume_tex.destruct(ctx);
	gradient_tex.destruct(ctx);

	// configure extents
	int min_ext_axis = cgv::math::min_index(vres);
	float min_ext = (float)vres[min_ext_axis];
	cgv::vec3 vbox_ext = cgv::vec3(vres) / cgv::vec3(min_ext);
	cgv::vec3 vbox_min = -0.5f * vbox_ext;
	cgv::vec3 half_vres = 0.5f * cgv::vec3(vres);

	// compute volume data
	std::vector<float> vol_data(vres[0] * vres[1] * vres[2]);
	cgv::uvec3 voxel;
	unsigned i;
	for (i = 0, voxel[2] = 0; voxel[2] < vres[2]; ++voxel[2])
		for (voxel[1] = 0; voxel[1] < vres[1]; ++voxel[1])
			for (voxel[0] = 0; voxel[0] < vres[0]; ++voxel[0], ++i) {
				cgv::vec3 p = (cgv::vec3(voxel) - half_vres) / half_vres;
				if (shape == S_ELLIPSOID) {
					float dist = length(p);
					if (dist >= 0.5f && dist <= 1.0f && p[1] < 0.4f)
						vol_data[i] = 1.0;
				}
				else {
					if (p[0] > -0.5f && p[0] < 0.5f && p[1] > -0.5f && p[1] < 0.5f && p[2] > -0.5f && p[2] < 0.5f)
						vol_data[i] = 1.0f;
				}
			}
	// transfer volume data into volume texture and compute mipmaps
	cgv::data::data_format vol_df(vres[0], vres[1], vres[2], cgv::type::info::TypeId::TI_FLT32, cgv::data::ComponentFormat::CF_R);
	cgv::data::const_data_view vol_dv(&vol_df, &vol_data.front());
	volume_tex.create(ctx, vol_dv, 0);
	volume_tex.generate_mipmaps(ctx);

	// set the volume bounding box to later scale the rendering accordingly
	volume_bounding_box.ref_min_pnt() = vbox_min;
	volume_bounding_box.ref_max_pnt() = vbox_min + vbox_ext;

	// make sure gradient texture is recomputed
	do_calculate_gradients = true;
}

void gradient_viewer::calculate_gradient_texture(cgv::render::context& ctx)
{
	//********** compute gradient texture with compute shader *************
	unsigned group_size = 4;

	cgv::uvec3 num_groups = ceil(cgv::vec3(vres) / (float)group_size);

	if (!gradient_tex.is_created())
		gradient_tex.create(ctx, cgv::render::TT_3D, vres[0], vres[1], vres[2]);

	// bind textures as 3D images (compare uniform declarations in compute shader gradient_3d.glcs)
	const int volume_tex_handle = (const int&)volume_tex.handle - 1;
	const int gradient_tex_handle = (const int&)gradient_tex.handle - 1;
	glBindImageTexture(0, volume_tex_handle, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(1, gradient_tex_handle, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// enable, configure, run and disable program
	gradient_3d_prog.enable(ctx);
	gradient_3d_prog.set_uniform(ctx, "resolution", vres);
	gradient_3d_prog.set_uniform(ctx, "gradient_mode", (int)gradient_mode);
	glDispatchCompute(num_groups[0], num_groups[1], num_groups[2]);

	// do something else

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	gradient_3d_prog.disable(ctx);

	// clear 3D image bindings
	glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
	glBindImageTexture(1, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	// read texture into memory
	std::vector<cgv::vec4> gradient_data(vres[0] * vres[1] * vres[2], cgv::vec4(0.0f));
	gradient_tex.enable(ctx, 0);
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, (void*)gradient_data.data());
	gradient_tex.disable(ctx);

	//************** construct geometry data for arrow rendering

	// clear previous data
	positions.clear();
	directions.clear();
	colors.clear();

	// configure extents
	int min_ext_axis = cgv::math::min_index(vres);
	float min_ext = (float)vres[min_ext_axis];
	cgv::vec3 vbox_ext = cgv::vec3(vres) / cgv::vec3(min_ext);
	cgv::vec3 vbox_min = -0.5f * vbox_ext;
	cgv::vec3 half_vres = 0.5f * cgv::vec3(vres);
	cgv::vec3 inv_vres = cgv::vec3(1.0f) / cgv::vec3(vres);

	// Loop over all voxels and place arrows to visualize the gradients
	cgv::uvec3 voxel;
	unsigned i;
	for (i = 0, voxel[2] = 0; voxel[2] < vres[2]; ++voxel[2])
		for (voxel[1] = 0; voxel[1] < vres[1]; ++voxel[1])
			for (voxel[0] = 0; voxel[0] < vres[0]; ++voxel[0], ++i) {

				cgv::vec3 p = 0.5f * vbox_ext * (cgv::vec3(voxel) - half_vres) / half_vres;

				cgv::vec3 grad3 = reinterpret_cast<const cgv::vec3&>(gradient_data[i]);

				if (grad3.sqr_length() > 0.0f) {
					positions.push_back(p);
					cgv::vec3 grad = normalize(grad3);
					directions.push_back(grad);
					grad = 0.5f * grad + 0.5f;
					colors.push_back(cgv::rgb(grad[0], grad[1], grad[2]));
				}
			}

	do_calculate_gradients = false;
}

void gradient_viewer::clear(cgv::render::context& ctx)
{
	cgv::render::ref_volume_renderer(ctx, -1);
	cgv::render::ref_arrow_renderer(ctx, -1);
}

bool gradient_viewer::init(cgv::render::context& ctx)
{
	cgv::render::ref_volume_renderer(ctx, 1);
	cgv::render::ref_arrow_renderer(ctx, 1);

	view_ptr = find_view_as_node();
	if (!view_ptr)
		return false;

	// construct compute shader program
	if (!gradient_3d_prog.build_program(ctx, "gradient_3d.glpr", true)) {
		std::cerr << "ERROR in gradient_viewer ::init() ... could not build program " << name << ".glpr" << std::endl;
		return false;
	}

	// load image data of transfer function to texture
	cgv::data::data_format format;
	cgv::media::image::image_reader image(format);
	cgv::data::data_view image_data;
	//if (!image.read_image(QUOTE_SYMBOL_VALUE(INPUT_DIR) "/res/inferno.bmp", tf_data) &&
	if (!image.read_image("res://inferno.bmp", image_data))
		abort();

	// add an opacity channel to the transfer function
	unsigned w = (unsigned)image_data.get_format()->get_width();
	unsigned char* src_ptr = image_data.get_ptr<unsigned char>();

	std::vector<unsigned char> tf_data(4*w, 0u);

	for(unsigned i = 0; i < w; ++i) {
		float alpha = static_cast<float>(i / 4) / static_cast<float>(w);
		tf_data[4 * i + 0] = src_ptr[3 * i + 0];
		tf_data[4 * i + 1] = src_ptr[3 * i + 1];
		tf_data[4 * i + 2] = src_ptr[3 * i + 2];
		tf_data[4 * i + 3] = static_cast<unsigned char>(255.0f * alpha);
	}

	cgv::data::data_view tf_data_view = cgv::data::data_view(new cgv::data::data_format(w, 1, TI_UINT8, cgv::data::CF_RGBA), tf_data.data());
	tf_tex.create(ctx, tf_data_view, 0);

	create_test_volume(ctx);
	return true;
}

void gradient_viewer::init_frame(cgv::render::context& ctx) {

	if(do_calculate_gradients) {
		calculate_gradient_texture(ctx);
		update_member(&do_calculate_gradients);
	}

	if(depth_tex.is_created() && (ctx.get_width() != depth_tex.get_width() || ctx.get_height() != depth_tex.get_height()))
		depth_tex.destruct(ctx);

	if(!depth_tex.is_created())
		depth_tex.create(ctx, cgv::render::TT_2D, ctx.get_width(), ctx.get_height());
}

void gradient_viewer::draw(cgv::render::context& ctx) 
{
	if (show_gradients && !positions.empty()) {
		cgv::render::arrow_renderer& ar = cgv::render::ref_arrow_renderer(ctx);
		ar.set_render_style(astyle);
		ar.set_position_array(ctx, positions);
		ar.set_direction_array(ctx, directions);
		ar.set_color_array(ctx, colors);
		ar.render(ctx, 0, positions.size());
	}
}

void gradient_viewer::after_finish(cgv::render::context& ctx)
{
	if (!view_ptr)
		return;

	// copy the contents of the depth buffer from the opaque geometry into the extra depth texture
	depth_tex.replace_from_buffer(ctx, 0, 0, 0, 0, ctx.get_width(), ctx.get_height());

	cgv::vec3 eye_pos = view_ptr->get_eye();
	cgv::vec3 view_dir = view_ptr->get_view_dir();
	if (show_volume) {
		auto& vr = cgv::render::ref_volume_renderer(ctx);
		vr.set_render_style(vstyle);
		vr.set_volume_texture(&volume_tex);
		vr.set_transfer_function_texture(&tf_tex);
		vr.set_depth_texture(&depth_tex);
		vr.set_bounding_box(volume_bounding_box);
		vr.transform_to_bounding_box(true);

		vr.render(ctx, 0, 0);
	}
}

void gradient_viewer::create_gui() 
{
	add_decorator("Gradient Viewer", "heading", "level=2");

	add_member_control(this, "show volume", show_volume, "check");
	add_member_control(this, "show gradients", show_gradients, "check");
	add_member_control(this, "gradient mode", gradient_mode, "dropdown", "enums='central differences,Sobel,optimized Sobel'");
	add_member_control(this, "calculate gradients", do_calculate_gradients, "toggle", "");

	if(begin_tree_node("Volume rendering", vstyle, false)) {
		align("\a");
		add_gui("vstyle", vstyle);
		align("\b");
		end_tree_node(vstyle);
	}

	if(begin_tree_node("Arrow rendering", astyle, false)) {
		align("\a");
		add_gui("astyle", astyle);
		align("\b");
		end_tree_node(astyle);
	}
}

#include <cgv/base/register.h>

cgv::base::factory_registration<gradient_viewer> gradient_viewer_fac("New/GPGPU/Gradient Viewer", 'G');
