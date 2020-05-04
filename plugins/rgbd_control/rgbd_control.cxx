#include <cgv/base/base.h>
#include <omp.h>
#include "rgbd_control.h"
#include <cgv_gl/gl/gl.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/utils/convert.h>
#include <cgv/utils/file.h>
#include <cgv/utils/statistics.h>
#include <cgv/type/standard_types.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/svd.h>

using namespace std;
using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::type;
using namespace cgv::gui;
using namespace cgv::data;
using namespace cgv::utils;
using namespace cgv::render;
using namespace rgbd;

std::string get_stream_format_enum(const std::vector<rgbd::stream_format>& sfs)
{
	std::string enum_def = "enums='default=-1";
	for (const auto& sf : sfs) {
		enum_def += ",";
		enum_def += to_string(sf);
	}
	return enum_def + "'";
}


rgbd_control::dvec3 transform(const rgbd_control::dmat4& T, const rgbd_control::dvec3& p)
{
	rgbd_control::dvec4 q = T * rgbd_control::dvec4(p, 1);
	double iw = 1.0 / q(3);
	return rgbd_control::dvec3(iw*q(0), iw*q(1), q(2));
}

rgbd_control::dvec3 rgbd_control::transform_to_world(const dvec3& p_win) const
{
	double Z = 0.001*p_win(2);
	dvec2 xy_wrl = Z*(dvec2(p_win(0), p_win(1)) - ctr)/f_p;
	return dvec3(xy_wrl, Z);
}

float RawDepthToMeters(unsigned short depthValue)
{
    if (depthValue < 2047)
    {
        return float(1.0 / (double(depthValue) * -0.0030711016 + 3.3309495161));
    }
    return 0.0f;
}

cgv::math::fvec<float,3> DepthToWorld(int x, int y, unsigned short depthValue)
{
    static const double fx_d = 1.0 / 5.9421434211923247e+02;
    static const double fy_d = 1.0 / 5.9104053696870778e+02;
    static const double cx_d = 3.3930780975300314e+02;
    static const double cy_d = 2.4273913761751615e+02;

    cgv::math::fvec<float,3> result;
    const double depth = RawDepthToMeters(depthValue);
    result(0) = float((x - cx_d) * depth * fx_d);
    result(1) = float((y - cy_d) * depth * fy_d);
    result(2) = float(depth);
    return result;
}

cgv::math::fvec<float,2> WorldToPixel(const cgv::math::fvec<float,3>& pt)
{
	static const float T_data[] = { 1.9985242312092553e-02f, -7.4423738761617583e-04f, -1.0916736334336222e-02f };
	static const float R_data[] = { 9.9984628826577793e-01f, 1.2635359098409581e-03f, -1.7487233004436643e-02f,
                                    -1.4779096108364480e-03f, 9.9992385683542895e-01f, -1.2251380107679535e-02f,
									 1.7470421412464927e-02f, 1.2275341476520762e-02f, 9.9977202419716948e-01f };
   
    static const double fx_rgb = 5.2921508098293293e+02;
    static const double fy_rgb = 5.2556393630057437e+02;
    static const double cx_rgb = 3.2894272028759258e+02;
    static const double cy_rgb = 2.6748068171871557e+02;

	cgv::math::fmat<float,3,3> R(3,3, R_data, true);
	R.transpose();
	cgv::math::fvec<float,3> transformedPos = R*(pt + cgv::math::fvec<float,3>(3, T_data));
	const float invZ = 1.0f / transformedPos(2);

	cgv::math::fvec<float,2> result;
    result(0) = float(transformedPos(0) * fx_rgb * invZ + cx_rgb);
    result(1) = float(transformedPos(1) * fy_rgb * invZ + cy_rgb);
    return result;
}
 

///
rgbd_control::rgbd_control() : 
	color_fmt("uint8[B,G,R,A]"),
	infrared_fmt("uint16[L]"),
	depth_fmt("uint16[L]"), 
	depth("uint16[L]", TF_NEAREST, TF_NEAREST),
	infrared("uint16[L]"),
	depth_range(0.0f, 1.0f)
{
	ctr = dvec2(320, 224);
	f_p = dvec2(571.25,571.25);
	plane_depth = 500;
	clr_tra = dvec3(0.023, 0, 0);
	clr_rot = dquat(1, 0, 0, 0);
	clr_ctr = dvec2(320, 240);
	clr_f_p = dvec2(525.0, 525.0);

	validate_color_camera = true;
	T.identity();
	set_name("rgbd_control");
	do_protocol = false;
	flip[0] = flip[2] = true;
	flip[1] = false;
	nr_depth_frames = 0;
	nr_color_frames = 0;
	nr_infrared_frames = 0;
	vis_mode = VM_COLOR;
	color_scale = 1;
	depth_scale = 1;
	infrared_scale = 1;
	near_mode = true;
	acquire_next = false;
	always_acquire_next = false;

	prs.measure_point_size_in_pixel = false;
	prs.point_size = 0.2f;
	prs.blend_width_in_pixel = 0.0f;
	prs.blend_points = false;
	remap_color = true;

	device_mode = DM_DEVICE;
	device_idx = 0;
	pitch = 0;
	x=y=z=0;
	aspect = 1;
	stopped = false;
	step_only = false;

	stream_color = true;
	stream_depth = true;
	stream_infrared = false;
	color_stream_format_idx = -1;
	depth_stream_format_idx = -1;
	ir_stream_format_idx = -1;
	
	color_frame_changed = false;
	depth_frame_changed = false;
	infrared_frame_changed = false;
	attachment_changed = false;

	connect(get_animation_trigger().shoot, this, &rgbd_control::timer_event);
}

///
bool rgbd_control::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return
		rh.reflect_member("protocol_path", protocol_path) &&
		rh.reflect_member("do_protocol", do_protocol) &&
		rh.reflect_member("vis_mode", (int&)vis_mode) &&
		rh.reflect_member("color_scale", color_scale) &&
		rh.reflect_member("depth_scale", depth_scale) &&
		rh.reflect_member("infrared_scale", infrared_scale) &&
		rh.reflect_member("device_mode", (int&)device_mode) &&
		rh.reflect_member("device_idx", device_idx);
}

///
void rgbd_control::on_set(void* member_ptr)
{
	if (member_ptr >= &clr_rot && member_ptr < &clr_rot + 1) {
		clr_rot(3) = sqrt(1 - reinterpret_cast<dvec3&>(clr_rot).sqr_length());
	}
	/*
	if (member_ptr >= &T && member_ptr < &T + 1) {
		P.clear();
		C.clear();
		std::vector<vec3> P_win;
		std::vector<vec3> P_wrl;
		for (unsigned d = 500; d < 4000; d += 100)
			for (unsigned y = 0; y < depth_fmt.get_height(); y += 16)
				for (unsigned x = 0; x < depth_fmt.get_width(); x += 16) {
					float point[3];
					if (rgbd_inp.map_pixel_to_point(x, y, 8*d, FF_DEPTH_RAW, point)) {
						P.push_back(vec3(3, point));
						C.push_back(rgba8(255, 0, 0, 255));
						dvec3 q = transform(T, dvec3(x, y, d));
						P.push_back(vec3((float)q(0), (float)q(1), (float)q(2)));
						C.push_back(rgba8(0, 255, 0, 255));
					}
				}				
	}
	if ((member_ptr >= &ctr && member_ptr < &ctr + 1) || 
		(member_ptr >= &f_p && member_ptr < &f_p + 1)) {
		P.clear();
		C.clear();
		std::vector<vec3> P_win;
		std::vector<vec3> P_wrl;
		for (unsigned d = 500; d < 4000; d += 100)
			for (unsigned y = 0; y < depth_fmt.get_height(); y += 16)
				for (unsigned x = 0; x < depth_fmt.get_width(); x += 16) {
					float point[3];
					if (rgbd_inp.map_pixel_to_point(x, y, 8 * d, FF_DEPTH_RAW, point)) {
						P.push_back(vec3(3, point));
						C.push_back(rgba8(255, 0, 0, 255));
						dvec3 q = transform_to_world(dvec3(x, y, d));
						P.push_back(vec3((float)q(0), (float)q(1), (float)q(2)));
						C.push_back(rgba8(0, 255, 0, 255));
					}
				}
	}
	if ((member_ptr == &plane_depth) || (member_ptr == &validate_color_camera) ||
		(member_ptr >= &clr_rot && member_ptr < &clr_rot + 1) ||
		(member_ptr >= &clr_tra && member_ptr < &clr_tra + 1) ||
		(member_ptr >= &clr_ctr && member_ptr < &clr_ctr + 1) ||
		(member_ptr >= &clr_f_p && member_ptr < &clr_f_p + 1)) {
		P.clear();
		C.clear();
		if (validate_color_camera) {
			std::vector<short> pixel_coords;
			pixel_coords.resize(depth_fmt.get_width() * depth_fmt.get_height() * 2, 0);
			rgbd_inp.map_depth_to_color_pixel(FF_DEPTH_RAW, depth2_data.get_ptr<unsigned char>(), &pixel_coords.front());
			for (unsigned y = 0; y < depth_fmt.get_height(); ++y)
				for (unsigned x = 0; x < depth_fmt.get_width(); ++x) {
					float point[3];
					if (rgbd_inp.map_pixel_to_point(x, y, 8 * plane_depth, FF_DEPTH_RAW, point)) {
						vec3 p(3, point);
						dvec3 Cp = clr_rot.apply(dvec3(p))+clr_tra;
						dvec2 Cxy = clr_f_p*reinterpret_cast<dvec2&>(Cp) / Cp(2) + clr_ctr;
						int Ccx = int(Cxy(0) + 0.5);
						int Ccy = int(Cxy(1) + 0.5);
						P.push_back(p);

						unsigned i = 2 * (y*depth_fmt.get_width() + x);
						int cx = pixel_coords[i];
						int cy = pixel_coords[i + 1];
						rgba8 col(50, 50, 50, 255);
						if (cx == 320 || cy == 240)
							col[0] = 255;
						else if (((cx & 31) == 0) || ((cy & 31) == 0))
							col[0] = 128;
						if (Ccx == 320 || Ccy == 240)
							col[1] = 255;
						else if (((Ccx & 31) == 0) || ((Ccy & 31) == 0))
							col[1] = 128;
						C.push_back(col);
					}
				}
		}
		else {
			for (unsigned y = 0; y < depth_fmt.get_height(); ++y)
				for (unsigned x = 0; x < depth_fmt.get_width(); ++x) {
					float point[3];
					if (rgbd_inp.map_pixel_to_point(x, y, 8 * plane_depth, FF_DEPTH_RAW, point)) {
						P.push_back(vec3(3, point));
						if (((x & 15) == 0) || ((y & 15) == 0))
							C.push_back(rgba8(255, 0, 0, 255));
						else
							C.push_back(rgba8(255, 255, 0, 255));
					}
				}
		}
	}
	*/
	if (member_ptr == &do_protocol) {
		if (do_protocol)
			rgbd_inp.enable_protocol(protocol_path);
		else
			rgbd_inp.disable_protocol();
	}
	if (member_ptr == &near_mode)
		rgbd_inp.set_near_mode(near_mode);

	update_member(member_ptr);
	post_redraw();
}

/// overload to handle register events that is sent after the instance has been registered
void rgbd_control::on_register()
{
	on_device_select_cb();
//	if (device_mode != DM_DETACHED)
//		on_start_cb();
}

/// overload to handle unregistration of instances
void rgbd_control::unregister()
{
	rgbd_inp.stop();
	rgbd_inp.detach();
}
/// adjust view
bool rgbd_control::init(cgv::render::context& ctx)
{
	ctx.set_bg_clr_idx(3);
	cgv::render::view* view_ptr = find_view_as_node();
	if (view_ptr) {
		view_ptr->set_view_up_dir(vec3(0, -1, 0));
		view_ptr->set_focus(vec3(0, 0, 0));
	}
	cgv::render::ref_point_renderer(ctx, 1);
	return true;
}

///
void rgbd_control::clear(cgv::render::context& ctx)
{
	color.destruct(ctx);
	warped_color.destruct(ctx);
	depth.destruct(ctx);
	rgbd_prog.destruct(ctx);
	cgv::render::ref_point_renderer(ctx, -1);
}

void rgbd_control::update_texture_from_frame(context& ctx, texture& tex, const frame_type& frame, bool recreate, bool replace)
{
	if (frame.is_allocated()) {
		if (recreate)
			tex.destruct(ctx);
		string fmt_descr("uint");
		switch (frame.pixel_format) {
		case PF_I: // infrared
		case PF_DEPTH:
		case PF_DEPTH_AND_PLAYER:
		case PF_CONFIDENCE:
			fmt_descr += to_string(frame.nr_bits_per_pixel) + "[L]";
			break;
		case PF_RGB:   // 24 or 32 bit rgb format with byte alignment
			fmt_descr += frame.nr_bits_per_pixel == 24 ? "8[R,G,B]" : "8[R,G,B,A]";
			break;
		case PF_BGR:   // 24 or 24 bit bgr format with byte alignment
			fmt_descr += frame.nr_bits_per_pixel == 24 ? "8[B,G,R]" : "8[B,G,R,A]";
			break;
		case PF_RGBA:  // 32 bit rgba format
			fmt_descr += "8[R,G,B,A]";
			break;
		case PF_BGRA:  // 32 bit brga format
			fmt_descr += "8[B,G,R,A]";
			break;
		case PF_BAYER: // 8 bit raw bayer pattern values
			fmt_descr += "8[L]";
			break;
		}
		if (tex.get_width() != frame.width || tex.get_height() != frame.height) {
			tex.destruct(ctx);
		}
		if (!tex.is_created()) {
			tex.set_component_format(fmt_descr);
			tex.create(ctx, TT_2D, frame.width, frame.height);
			replace = true;
		}
		if (replace) {
			data_format df(fmt_descr);
			df.set_width(frame.width);
			df.set_height(frame.height);
			cgv::data::const_data_view dv(&df, &frame.frame_data.front());
			tex.replace(ctx, 0, 0, dv);
		}
	}
	else {
		tex.destruct(ctx);
	}
}

///
void rgbd_control::init_frame(context& ctx)
{
	if (!rgbd_prog.is_created())
		rgbd_prog.build_program(ctx, "rgbd_shader.glpr");

	if (device_mode != DM_DETACHED) {
		update_texture_from_frame(ctx, color, color_frame, attachment_changed, color_frame_changed);
		update_texture_from_frame(ctx, depth, depth_frame, attachment_changed, depth_frame_changed);
		update_texture_from_frame(ctx, infrared, ir_frame, attachment_changed, infrared_frame_changed);
		update_texture_from_frame(ctx, warped_color, warped_color_frame, attachment_changed, (color_frame_changed|| depth_frame_changed) &&remap_color);
		color_frame_changed = false;
		infrared_frame_changed = false;
		/*
		if (depth_frame_changed) {
			vec3 p = km.track(depth_frame);
			mouse_pos(0) = p(0);
			mouse_pos(1) = p(1);
		}
		*/
		depth_frame_changed = false;
	}
	else {
		color.destruct(ctx);
		warped_color.destruct(ctx);
		infrared.destruct(ctx);
		depth.destruct(ctx);
	}
	attachment_changed = false;
}

/// overload to draw the content of this drawable
void rgbd_control::draw(context& ctx)
{
	ctx.push_modelview_matrix();
	vec3 flip_vec(flip[0] ? -1.0f : 1.0f, flip[1] ? -1.0f : 1.0f, flip[2] ? -1.0f : 1.0f);
	ctx.mul_modelview_matrix(cgv::math::scale4<double>(flip_vec[0], -flip_vec[1], flip_vec[2]));
	if (P.size() > 0) {
		ctx.mul_modelview_matrix(cgv::math::translate4<double>(-1.5f,0,0));
		cgv::render::point_renderer& pr = ref_point_renderer(ctx);
		pr.set_render_style(prs);
		pr.set_position_array(ctx, P);
		if (C.size() == P.size())
			pr.set_color_array(ctx, C);
		pr.render(ctx, 0, P.size());
		ctx.mul_modelview_matrix(cgv::math::translate4<double>(3, 0, 0));
	}
	// transform to image coordinates
	ctx.mul_modelview_matrix(cgv::math::scale4<double>(aspect, -1, 1));
	// enable shader program
	if (rgbd_prog.is_created()) {
		if (stream_color && color.is_created()) {
			color.enable(ctx, 0);
			rgbd_prog.set_uniform(ctx, "color_texture", 0);
			if (remap_color && warped_color.is_created()) {
				warped_color.enable(ctx, 3);
				rgbd_prog.set_uniform(ctx, "warped_color_texture", 3);
			}
		}
		if (stream_depth && depth.is_created()) {
			depth.enable(ctx, 1);
			rgbd_prog.set_uniform(ctx, "depth_texture", 1);
		}
		if (stream_infrared && infrared.is_created()) {
			infrared.enable(ctx, 2);
			rgbd_prog.set_uniform(ctx, "infrared_texture", 2);
		}
		rgbd_prog.set_uniform(ctx, "rgbd_mode", (int) vis_mode);
		rgbd_prog.set_uniform(ctx, "color_scale", color_scale);
		rgbd_prog.set_uniform(ctx, "infrared_scale", infrared_scale);
		rgbd_prog.set_uniform(ctx, "depth_scale", depth_scale);
		rgbd_prog.set_uniform(ctx, "min_depth", depth_range[0]);
		rgbd_prog.set_uniform(ctx, "max_depth", depth_range[1]);
		rgbd_prog.enable(ctx);
		glDisable(GL_CULL_FACE);
		ctx.tesselate_unit_square();
		glEnable(GL_CULL_FACE);
		rgbd_prog.disable(ctx);
		if (stream_infrared&& infrared.is_created())
			infrared.disable(ctx);
		if (stream_depth && depth.is_created())
			depth.disable(ctx);
		if (stream_color && color.is_created()) {
			color.disable(ctx);
			if (remap_color&& warped_color.is_created())
				warped_color.disable(ctx);
		}
	}
	// restore gl state
	ctx.pop_modelview_matrix();
}

/// 
bool rgbd_control::handle(cgv::gui::event& e)
{
	if (e.get_kind() != cgv::gui::EID_KEY)
		return false;
	cgv::gui::key_event& ke = static_cast<cgv::gui::key_event&>(e);
	if (ke.get_action() == cgv::gui::KA_RELEASE)
		return false;
	switch (ke.get_key()) {
	case cgv::gui::KEY_Space :
		if (stopped)
			on_start_cb();
		else
			on_stop_cb();
		break;
	case 'X':
		if (ke.get_modifiers() == 0) {
			flip[0] = !flip[0];
			on_set(&flip[0]);
			return true;
		}
		return false;
	case 'Y':
		if (ke.get_modifiers() == 0) {
			flip[1] = !flip[1];
			on_set(&flip[1]);
			return true;
		}
		return false;
	case 'Z':
		if (ke.get_modifiers() == 0) {
			flip[2] = !flip[2];
			on_set(&flip[2]);
			return true;
		}
		return false;
	case 'N':
		if (ke.get_modifiers() == 0) {
			near_mode = !near_mode;
			on_set(&near_mode);
			return true;
		}
		return false;
	case 'R':
		if (ke.get_modifiers() == 0) {
			remap_color = !remap_color;
			on_set(&remap_color);
			return true;
		}
		return false;
	case 'C':
		if (ke.get_modifiers() == 0) {
			vis_mode = VM_COLOR;
			on_set(&vis_mode);
			return true;
		}
		else if (ke.get_modifiers() == cgv::gui::EM_CTRL) {
			calibrate_device();
			return true;
		}
		else if (ke.get_modifiers() == cgv::gui::EM_ALT) {
			on_set(&ctr(0));
			return true;
		}
		return false;
	case 'A':
		if (ke.get_modifiers() == 0) {
			always_acquire_next = !always_acquire_next;
			on_set(&always_acquire_next);
			return true;
		}
		return false;
	case 'D':
		if (ke.get_modifiers() == 0) {
			vis_mode = VM_DEPTH;
			on_set(&vis_mode);
			return true;
		}
		return false;
	case 'I':
		if (ke.get_modifiers() == 0) {
			vis_mode = VM_INFRARED;
			on_set(&vis_mode);
			return true;
		}
		return false;
	case 'W':
		if (ke.get_modifiers() == 0) {
			vis_mode = VM_WARPED;
			on_set(&vis_mode);
			return true;
		}
		return false;
	case 'P':
		acquire_next = true;
		return true;
	}
	return false;
}
/// 
void rgbd_control::stream_help(std::ostream& os)
{
	os << "rgbd_control: select vismode <C|D>; toggle <R>emapColor, <N>earMode, flip <X|Y|Z>" << std::endl;
}

///
void rgbd_control::create_gui()
{
	add_decorator("rgbd", "heading", "level=1");
	unsigned n = rgbd_input::get_nr_devices();
	string device_def = "enums='detached=-2;dummy=-1";
	for (unsigned i=0; i<n; ++i) {
		device_def += ",";
		device_def += rgbd_input::get_serial(i);
	}
	device_def += "'";

	connect_copy(add_control("device", (DummyEnum&) device_idx, "dropdown", device_def)->value_change, rebind(this, &rgbd_control::on_device_select_cb));
	add_member_control(this, "near_mode", near_mode, "toggle");
	add_member_control(this, "remap_color", remap_color, "toggle");
	add_member_control(this, "flip x", flip[0], "toggle", "w=66", " ");
	add_member_control(this, "flip y", flip[1], "toggle", "w=66", " ");
	add_member_control(this, "flip z", flip[2], "toggle", "w=66");
	add_view("nr_color_frames", nr_color_frames);
	add_view("nr_infrared_frames", nr_infrared_frames);
	add_view("nr_depth_frames", nr_depth_frames);

	if (begin_tree_node("Device", nr_color_frames, true, "level=2")) {
		align("\a");
		add_member_control(this, "stream_color", stream_color, "check");
		add_member_control(this, "stream_depth", stream_depth, "check");
		add_member_control(this, "stream_infrared", stream_infrared, "check");
		add_member_control(this, "color_stream_format", (DummyEnum&)color_stream_format_idx, "dropdown", get_stream_format_enum(color_stream_formats));
		add_member_control(this, "depth_stream_format", (DummyEnum&)depth_stream_format_idx, "dropdown", get_stream_format_enum(depth_stream_formats));
		add_member_control(this, "ir_stream_format", (DummyEnum&)ir_stream_format_idx, "dropdown", get_stream_format_enum(ir_stream_formats));
		connect_copy(add_button("st&art", "shortcut='a'")->click, rebind(this, &rgbd_control::on_start_cb));
		connect_copy(add_button("&step", "shortcut='s'")->click, rebind(this, &rgbd_control::on_step_cb));
		connect_copy(add_button("st&op", "shortcut='o'")->click, rebind(this, &rgbd_control::on_stop_cb));
		connect_copy(add_button("save")->click, rebind(this, &rgbd_control::on_save_cb));
		connect_copy(add_button("save point cloud")->click, rebind(this, &rgbd_control::on_save_point_cloud_cb));
		connect_copy(add_button("load")->click, rebind(this, &rgbd_control::on_load_cb));
		align("\b");
		end_tree_node(nr_color_frames);
	}
	if (begin_tree_node("Base", pitch, false, "level=2")) {
		align("\a");
		connect_copy(add_control("pitch", pitch, "value_slider", "min=-1;max=1;ticks=true")->value_change, rebind(this, &rgbd_control::on_pitch_cb));
		add_view("x", x);
		add_view("y", y);
		add_view("z", z);
		align("\b");
		end_tree_node(pitch);
	}
	if (begin_tree_node("Visualization", vis_mode, true, "level=2")) {
		align("\a");
		connect_copy(add_control("vis_mode", vis_mode, "dropdown", "enums='color,depth,infrared,warped_color'")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("color_scale", color_scale, "value_slider", "min=0.1;max=100;log=true;ticks=true")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("infrared_scale", infrared_scale, "value_slider", "min=0.1;max=100;log=true;ticks=true")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(add_control("depth_scale", depth_scale, "value_slider", "min=0.1;max=100;log=true;ticks=true")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		add_gui("depth_range", depth_range, "ascending", "options='min=0;max=1;ticks=true'");
		align("\b");
		end_tree_node(vis_mode);
	}
	if (begin_tree_node("Point Cloud", always_acquire_next, false, "level=2")) {
		align("\a");
		add_member_control(this, "always_acquire_next", always_acquire_next, "toggle");

		for (unsigned i = 0; i < 4; ++i)
			for (unsigned j = 0; j < 4; ++j)
				add_member_control(this, std::string("T") + to_string(i) + to_string(j), T(i, j),
					"value_slider", "min=-1;max=1;log=true;step=0.00001;ticks=true");

		if (begin_tree_node("point style", prs)) {
			align("\a");
			add_gui("point style", prs);
			align("\b");
			end_tree_node(prs);
		}
		align("\b");
		end_tree_node(always_acquire_next);
	}
	if (begin_tree_node("Calibration", ctr, false, "level=2")) {
		align("\a");
		add_member_control(this, "plane_depth", plane_depth, "value_slider", "min=500;max=4000;ticks=true");
		add_member_control(this, "validate_color_camera", validate_color_camera, "toggle");
		add_member_control(this, "Dcx", ctr(0), "value_slider", "min=300;max=340;step=0.00001;ticks=true");
		add_member_control(this, "Dcy", ctr(1), "value_slider", "min=210;max=250;step=0.00001;ticks=true");
		add_member_control(this, "Dfx", f_p(0), "value_slider", "min=550;max=600;log=true;step=0.00001;ticks=true");
		add_member_control(this, "Dfy", f_p(1), "value_slider", "min=550;max=600;log=true;step=0.00001;ticks=true");
		add_member_control(this, "tx", clr_tra(0), "value_slider", "min=-0.05;max=0.05;step=0.00001;ticks=true");
		add_member_control(this, "ty", clr_tra(1), "value_slider", "min=-0.05;max=0.05;step=0.00001;ticks=true");
		add_member_control(this, "tz", clr_tra(2), "value_slider", "min=-0.05;max=0.05;step=0.00001;ticks=true");
		add_member_control(this, "qx", clr_rot(0), "value_slider", "min=-0.05;max=0.05;step=0.00001;ticks=true");
		add_member_control(this, "qy", clr_rot(1), "value_slider", "min=-0.05;max=0.05;step=0.00001;ticks=true");
		add_member_control(this, "qz", clr_rot(2), "value_slider", "min=-0.05;max=0.05;step=0.00001;ticks=true");
		add_member_control(this, "Ccx", clr_ctr(0), "value_slider", "min=300;max=340;step=0.00001;ticks=true");
		add_member_control(this, "Ccy", clr_ctr(1), "value_slider", "min=210;max=250;step=0.00001;ticks=true");
		add_member_control(this, "Cfx", clr_f_p(0), "value_slider", "min=550;max=600;log=true;step=0.00001;ticks=true");
		add_member_control(this, "Cfy", clr_f_p(1), "value_slider", "min=550;max=600;log=true;step=0.00001;ticks=true");
		align("\b");
		end_tree_node(ctr);
	}
}

size_t rgbd_control::construct_point_cloud()
{
	/*
	if (remap_color)
		rgbd_inp.map_color_to_depth(rgbd::FF_DEPTH_RAW, depth2_data.get_ptr<unsigned char>(), rgbd::FF_COLOR_RGB32, color2_data.get_ptr<unsigned char>());
	P2.clear();
	C2.clear();
	std::vector<vec3> Q;
	unsigned short* d_ptr = depth2_data.get_ptr<unsigned short>();
	unsigned char* c_ptr = color2_data.get_ptr<unsigned char>();
	for (unsigned y = 0; y < depth_fmt.get_height(); ++y)
		for (unsigned x = 0; x < depth_fmt.get_width(); ++x) {
			float point[3];
			if (rgbd_inp.map_pixel_to_point(x, y, *d_ptr++, FF_DEPTH_RAW, point)) {
				P2.push_back(vec3(3, point));
				C2.push_back(reinterpret_cast<rgba8&>(*c_ptr));
				Q.push_back(vec3(float(x), float(y), (float)d_ptr[-1]));
				rgba8& col = C2.back();
				std::swap(col[0], col[2]);
				col[3] = 255;
			}
			c_ptr += color_fmt.get_entry_size();
		}
	compute_homography(P2, Q);
	*/
	return P2.size();
}
void rgbd_control::calibrate_device()
{
	/*
	std::vector<vec3> P_win;
	std::vector<vec3> P_wrl;
	for (unsigned d = 500; d < 4000; d+=100)
		for (unsigned y = 0; y < depth_fmt.get_height(); y += 16)
			for (unsigned x = 0; x < depth_fmt.get_width(); x += 16) {
				float point[3];
				if (rgbd_inp.map_pixel_to_point(x, y, 8*d, FF_DEPTH_RAW, point)) {
					P_wrl.push_back(vec3(3, point));
					P_win.push_back(vec3((float)x, (float)y, (float)d));
				}
			}
	std::cout << "nr points = " << P_win.size() << " (expected " << 34 * depth_fmt.get_width()*depth_fmt.get_height() / 16 / 16 << ")" << std::endl;
	compute_homography(P_win, P_wrl);
	*/
}

void rgbd_control::compute_homography(const std::vector<vec3>& P, const std::vector<vec3>& Q)
{
	/*
	dvecn m(16);
	cgv::math::mat<double> M(16, 16, 0.0);
	size_t k;
	for (k = 0; k < P.size(); ++k) {
		dvec4 hp = dvec4(P[k],1);
		dvec3 q  = Q[k];
		// construct first constraint 
		reinterpret_cast<dvec4&>(m(0)) = hp;
		reinterpret_cast<dvec4&>(m(4)).zeros();
		reinterpret_cast<dvec4&>(m(8)).zeros();
		reinterpret_cast<dvec4&>(m(12)) = -q(0)*hp;
		M += dyad(m, m);
		// construct second constraint 
		reinterpret_cast<dvec4&>(m(0)).zeros();
		reinterpret_cast<dvec4&>(m(4)) = hp;
		reinterpret_cast<dvec4&>(m(8)).zeros();
		reinterpret_cast<dvec4&>(m(12)) = -q(1)*hp;
		M += dyad(m, m);
		// construct third constraint 
		reinterpret_cast<dvec4&>(m(0)).zeros();
		reinterpret_cast<dvec4&>(m(4)).zeros();
		reinterpret_cast<dvec4&>(m(8)) = hp;
//		reinterpret_cast<dvec4&>(m(12)) = -q(2)*hp;
		reinterpret_cast<dvec4&>(m(12)) = -dvec4(0,0,0,1);
		M += dyad(m, m);
	}
	//M *= 1.0 / (3 * P.size());
	cgv::math::mat<double> U, V;
	cgv::math::diag_mat<double> S;
	if (!cgv::math::svd(M, U, S, V, true, 100)) {
		std::cerr << "svd failed" << std::endl;
		abort();
	}
	std::cout << "singular values = " << S << std::endl;
	m = U.col(15);

	dvecn m2 = M * m;
	double l2 = m2.length();
	if (l2 != 0)
		m2 /= l2;
	std::cout << "validate result: " << l2 << ", " << dot(m, m2) << std::endl;
	double scale = 1.0 / m(15);
	dmat4 H;
	H.set_row(0, scale*reinterpret_cast<dvec4&>(m(0)));
	H.set_row(1, scale*reinterpret_cast<dvec4&>(m(4)));
	H.set_row(2, reinterpret_cast<dvec4&>(m(8)));
	H.set_row(3, scale*reinterpret_cast<dvec4&>(m(12)));
	std::cout << "H =\n" << H << std::endl;

	T = H;
	for (unsigned i = 0; i < 16; ++i)
		update_member(&T(i / 4, i % 4));
	on_set(&T);
	dvec3 error;
	error.zeros();
	for (k = 0; k < P.size(); ++k) {
		dvec3 q = transform(H, P[k]);
		if (k < 10)
			std::cout << "comp: ";
		for (unsigned i = 0; i < 3; ++i) {
			error(i) += fabs(Q[k](i) - q(i));
			if (k < 10)
				std::cout << "  " << P[k](i) << "->" << q(i) << "=" << Q[k](i);
		}
		if (k < 10)
			std::cout << std::endl;
	}
	error *= 1.0 / P.size();
	std::cout << "avg errors: " << error << std::endl;
	*/
}

void rgbd_control::timer_event(double t, double dt)
{
	// in case a point cloud is being constructed
	if (future_handle.valid()) {
		// check for termination of thread
		if (future_handle.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			size_t N = future_handle.get();
			// copy computed point cloud
			P = P2;
			C = C2;
			post_redraw();
		}
	}
	if (rgbd_inp.is_started()) {
		IMU_measurement m;
		if (rgbd_inp.put_IMU_measurement(m, 10)) {
			x = m.linear_acceleration[0];
			y = m.linear_acceleration[1];
			z = m.linear_acceleration[2];
			update_member(&x);
			update_member(&y);
			update_member(&z);
		}
		if (rgbd_inp.is_started()) {
			bool new_frame;
			bool found_frame = false;
			do {
				new_frame = false;
				if (stream_color) {
					bool new_color_frame_changed = rgbd_inp.get_frame(IS_COLOR, color_frame, 0);
					if (new_color_frame_changed) {
						++nr_color_frames;
						color_frame_changed = new_color_frame_changed;
						new_frame = true;
						update_member(&nr_color_frames);
					}
				}
				if (stream_depth) {
					bool new_depth_frame_changed = rgbd_inp.get_frame(IS_DEPTH, depth_frame, 0);
					if (new_depth_frame_changed) {
						++nr_depth_frames;
						depth_frame_changed = new_depth_frame_changed;
						new_frame = true;
						update_member(&nr_depth_frames);
					}
				}
				if (stream_infrared) {
					bool new_infrared_frame_changed = rgbd_inp.get_frame(IS_INFRARED, ir_frame, 0);
					if (new_infrared_frame_changed) {
						++nr_infrared_frames;
						infrared_frame_changed = new_infrared_frame_changed;
						new_frame = true;
						update_member(&nr_infrared_frames);
					}
				}
				if (new_frame)
					found_frame = true;
			} while (new_frame);
			if (found_frame)
				post_redraw();
			if (stream_color && stream_depth && color_frame.is_allocated() && depth_frame.is_allocated() &&
				(color_frame_changed || depth_frame_changed) ) {
				if (!future_handle.valid() && (always_acquire_next || acquire_next)) {
					acquire_next = false;
					color_frame_2 = color_frame;
					depth_frame_2 = depth_frame;
					future_handle = std::async(&rgbd_control::construct_point_cloud, this);
				}
				else {
					if (remap_color)
						rgbd_inp.map_color_to_depth(depth_frame, color_frame, warped_color_frame);
				}
			}
		}
	}
	/*
	else if (!stopped && device_idx == -1) {
		if (stream_depth_file_name_base.empty()) {
			unsigned char* c = color_data.get_ptr<unsigned char>();
			for (unsigned j=0; j<h; ++j)
				for (unsigned i=0; i<w; ++i) {
					*c++ = (unsigned char) (i & 255);
					*c++ = (unsigned char) (j & 255);
					*c++ = (unsigned char) ((i*j) & 255);
					*c++ = 255;
				}
			unsigned short* s = depth_data.get_ptr<unsigned short>();
			for (unsigned j=0; j<h; ++j)
				for (unsigned i=0; i<w; ++i)
					*s++ = i;

			color_frame_changed = depth_frame_changed = true;
			post_redraw();
		}
		else {
			std::string fnd = stream_depth_file_name_base+to_string((int)depth_stream_idx)+".dep";
			if (!cgv::utils::file::exists(fnd)) {
				depth_stream_idx = 0;
				fnd = stream_depth_file_name_base+to_string((int)depth_stream_idx)+".dep";
			}
			++depth_stream_idx;
			if (cgv::utils::file::exists(fnd)) {
				std::string d;
				if (cgv::utils::file::read(fnd, d, false)) {
					memcpy(depth_data.get_ptr<unsigned char>(), &d[0], depth_data.get_format()->get_size()*depth_data.get_format()->get_entry_size());
					depth_frame_changed = true;
					post_redraw();
				}
				else
					std::cerr << "could not read " << fnd << std::endl;
			}
		}
	}
	*/
	if (!stopped && step_only) {
		on_stop_cb();
		step_only = false;
	}
}

void rgbd_control::on_start_cb()
{
	rgbd_inp.set_near_mode(near_mode);
	InputStreams is = IS_NONE;
	bool use_default = false;
	std::vector<stream_format> sfs;
	if (stream_color) {
		is = InputStreams(is + IS_COLOR);
		if (color_stream_format_idx == -1)
			use_default = true;
		else
			sfs.push_back(color_stream_formats[color_stream_format_idx]);
	}
	if (stream_depth) {
		is = InputStreams(is + IS_DEPTH);
		if (depth_stream_format_idx == -1)
			use_default = true;
		else
			sfs.push_back(depth_stream_formats[depth_stream_format_idx]);
	}
	if (stream_infrared) {
		is = InputStreams(is + IS_INFRARED);
		if (ir_stream_format_idx == -1)
			use_default = true;
		else
		sfs.push_back(ir_stream_formats[ir_stream_format_idx]);
	}
	if (use_default) {
		sfs.clear();
		if (!rgbd_inp.start(InputStreams(is), sfs)) {
			cgv::gui::message("could not start kinect device");
			return;
		}
		else {
			for (const auto& sf : sfs) {
				auto ci = std::find(color_stream_formats.begin(), color_stream_formats.end(), sf);
				if (ci != color_stream_formats.end()) {
					color_stream_format_idx = ci - color_stream_formats.begin();
					update_member(&color_stream_format_idx);
				}
				auto di = std::find(depth_stream_formats.begin(), depth_stream_formats.end(), sf);
				if (di != depth_stream_formats.end()) {
					depth_stream_format_idx = di - depth_stream_formats.begin();
					update_member(&depth_stream_format_idx);
				}
				auto ii = std::find(ir_stream_formats.begin(), ir_stream_formats.end(), sf);
				if (ii != ir_stream_formats.end()) {
					ir_stream_format_idx = ii - ir_stream_formats.begin();
					update_member(&ir_stream_format_idx);
				}
			}
		}
	}
	else {
		if (!rgbd_inp.start(sfs)) {
			cgv::gui::message("could not start kinect device");
			return;
		}
	}
	if (stream_infrared)
		aspect = float(ir_stream_formats[ir_stream_format_idx].width) / ir_stream_formats[ir_stream_format_idx].height;
	if (stream_color)
		aspect = float(color_stream_formats[color_stream_format_idx].width) / color_stream_formats[color_stream_format_idx].height;
	if (stream_depth)
		aspect = float(depth_stream_formats[depth_stream_format_idx].width) / depth_stream_formats[depth_stream_format_idx].height;

	stopped = false;

	post_redraw();
}

void rgbd_control::on_step_cb()
{
	on_start_cb();
	if (!stopped)
		step_only = true;
}

void rgbd_control::on_stop_cb()
{
	rgbd_inp.stop();
	stopped = true;
}

void rgbd_control::on_save_cb()
{
	std::string fn = cgv::gui::file_save_dialog("base file name", "Frame Files (rgb,dep):*.rgb;*.dep;*.ir");
	if (fn.empty())
		return;
	std::string fnc = fn+".rgb";
	std::string fnd = fn + ".dep";
	std::string fni = fn + ".ir";
	if (stream_color && color_frame.is_allocated()) {
		if (!color_frame.write(fnc))
			std::cerr << "could not write " << fnc << std::endl;
	}
	if (stream_depth&& depth_frame.is_allocated()) {
		if (!depth_frame.write(fnd))
			std::cerr << "could not write " << fnd << std::endl;
	}
	if (stream_infrared && ir_frame.is_allocated()) {
		if (!ir_frame.write(fni))
			std::cerr << "could not write " << fni << std::endl;
	}
}

void rgbd_control::on_save_point_cloud_cb()
{
	std::string fn = cgv::gui::file_save_dialog("point cloud", "Point Cloud Files (bpc,apc,obj):*.bpc;*.apc;*.obj");
	if (fn.empty())
		return;
	FILE* fp = fopen(fn.c_str(), "wb");
	if (!fp)
		return;

	unsigned int n = (unsigned int)P.size();
	unsigned int m = 0;
	unsigned int m1 = m;
	if (C.size() == n)
		m1 = 2*n+m;
	bool success = 
		fwrite(&n,sizeof(unsigned int),1,fp) == 1 &&
		fwrite(&m1,sizeof(unsigned int),1,fp) == 1 &&
		fwrite(&P[0][0],sizeof(vec3),n,fp) == n;
	if (C.size() == n)
		success = success && (fwrite(&C[0][0],sizeof(rgba8),n,fp) == n);
	fclose(fp);
}

void rgbd_control::on_load_cb()
{
	on_stop_cb();
	std::string fn = cgv::gui::file_open_dialog("base file name", "Frame Files (rgb,dep):*.rgb;*.dep");
	if (fn.empty())
		return;
	fn = cgv::utils::file::drop_extension(fn);
	std::string fnc = fn+".rgb";
	std::string fnd = fn + ".dep";
	std::string fni = fn + ".ir";
	std::string c, d;
	if (color_frame.read(fnc))
		color_frame_changed = true;
	else
		std::cerr << "could not read " << fnc << std::endl;

	if (depth_frame.read(fnd))
		depth_frame_changed = true;
	else
		std::cerr << "could not read " << fnd << std::endl;

	if (ir_frame.read(fni))
		infrared_frame_changed = true;
	else
		std::cerr << "could not read " << fni << std::endl;

	post_redraw();
}

void rgbd_control::update_stream_formats()
{
	color_stream_formats.clear();
	rgbd_inp.query_stream_formats(IS_COLOR, color_stream_formats);
	if (find_control(color_stream_format_idx))
		find_control(color_stream_format_idx)->multi_set(get_stream_format_enum(color_stream_formats));
	depth_stream_formats.clear();
	rgbd_inp.query_stream_formats(IS_DEPTH, depth_stream_formats);
	if (find_control(depth_stream_format_idx))
		find_control(depth_stream_format_idx)->multi_set(get_stream_format_enum(depth_stream_formats));
	ir_stream_formats.clear();
	rgbd_inp.query_stream_formats(IS_INFRARED, ir_stream_formats);
	if (find_control(ir_stream_format_idx))
		find_control(ir_stream_format_idx)->multi_set(get_stream_format_enum(ir_stream_formats));
}

void rgbd_control::on_device_select_cb()
{
	rgbd_inp.detach();
	if (device_idx == -1)
		device_mode = DM_PROTOCOL;
	else if (device_idx == -2)
		device_mode = DM_DETACHED;

	if (device_mode == DM_DEVICE) {
		unsigned nr = rgbd_input::get_nr_devices();
		if (nr == 0) {
			device_mode = DM_DETACHED;
			update_member(&device_mode);
		}
		else {
			if (device_idx >= (int)nr) {
				device_idx = 0;
				update_member(&device_idx);
			}
			if (rgbd_inp.attach(rgbd_input::get_serial(device_idx))) {
				update_stream_formats();
				rgbd_inp.set_pitch(pitch);
			}
			else {
				device_mode = DM_DETACHED;
				update_member(&device_mode);
			}
		}
	}
	else if (device_mode == DM_PROTOCOL) {
		rgbd_inp.attach_path(protocol_path);
		update_member(&device_idx);
		// on_device_select_cb();
	}
	attachment_changed = true;
	post_redraw();
}

void rgbd_control::on_pitch_cb()
{
	if (rgbd_inp.is_attached())
		rgbd_inp.set_pitch(pitch);
}

#include "lib_begin.h"

extern CGV_API object_registration<rgbd_control> kc_or("");