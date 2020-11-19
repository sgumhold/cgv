#include <cgv/base/base.h> // this should be first header to avoid warning
#include <omp.h>
#include "rgbd_control.h"
#include <cgv_gl/gl/gl.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/utils/convert.h>
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>
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

///
rgbd_control::rgbd_control() : 
	color_fmt("uint8[B,G,R,A]"),
	infrared_fmt("uint16[L]"),
	depth_fmt("uint16[L]"), 
	depth("uint16[L]", TF_NEAREST, TF_NEAREST),
	infrared("uint16[L]"),
	depth_range(0.0f, 1.0f)
{
	set_name("rgbd_control");
	do_protocol = false;
	flip[0] = true;
	flip[1] = flip[2] = false;
	nr_depth_frames = 0;
	nr_color_frames = 0;
	nr_infrared_frames = 0;
	nr_mesh_frames = 0;
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

	device_mode = DM_DETACHED;
	device_idx = -2;
	pitch = 0;
	x=y=z=0;
	aspect = 1;
	stopped = false;
	step_only = false;

	stream_color = true;
	stream_depth = true;
	stream_infrared = false;
	stream_mesh = false;
	color_stream_format_idx = -1;
	depth_stream_format_idx = -1;
	ir_stream_format_idx = -1;
	
	color_frame_changed = false;
	depth_frame_changed = false;
	infrared_frame_changed = false;
	color_attachment_changed = depth_attachment_changed = infrared_attachment_changed = false;
	prs.measure_point_size_in_pixel = true;
	prs.point_size = 3.0f;
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
	if (member_ptr == &do_protocol) {
		if (do_protocol) {
			//prevent reading and writing the log on the same path 
			if (device_mode == DM_PROTOCOL) {
				do_protocol = false;
				rgbd_inp.disable_protocol();
			}
			bool path_exists = cgv::utils::dir::exists(protocol_path);
			if (!path_exists) {
				if (cgv::gui::question(protocol_path + " does not exist. Create it?", "No,Yes", 1)) {
					path_exists = cgv::utils::dir::mkdir(protocol_path);
					if (!path_exists)
						cgv::gui::message(protocol_path + " creation failed!");
				}
			}
			if (path_exists)
				rgbd_inp.enable_protocol(protocol_path);
			else {
				do_protocol = false;
				update_member(&do_protocol);
			}
		}
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
	if (device_mode != DM_DETACHED)
		on_start_cb();
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
		update_texture_from_frame(ctx, color, color_frame, color_attachment_changed, color_frame_changed);
		update_texture_from_frame(ctx, depth, depth_frame, depth_attachment_changed, depth_frame_changed);
		update_texture_from_frame(ctx, infrared, ir_frame, infrared_attachment_changed, infrared_frame_changed);
		update_texture_from_frame(ctx, warped_color, warped_color_frame, color_attachment_changed, (color_frame_changed|| depth_frame_changed) &&remap_color);
		if (color_frame_changed)
			color_attachment_changed = false;
		if (depth_frame_changed)
			depth_attachment_changed = false;
		if (infrared_frame_changed)
			infrared_attachment_changed = false;
		color_frame_changed = false;
		infrared_frame_changed = false;
		depth_frame_changed = false;
	}
	else {
		color.destruct(ctx);
		warped_color.destruct(ctx);
		infrared.destruct(ctx);
		depth.destruct(ctx);
	}
}

/// overload to draw the content of this drawable
void rgbd_control::draw(context& ctx)
{
	ctx.push_modelview_matrix();
	vec3 flip_vec(flip[0] ? -1.0f : 1.0f, flip[1] ? -1.0f : 1.0f, flip[2] ? -1.0f : 1.0f);
	ctx.mul_modelview_matrix(cgv::math::scale4<double>(flip_vec[0], -flip_vec[1], -flip_vec[2]));
	if (P.size() > 0) {
		ctx.mul_modelview_matrix(cgv::math::translate4<double>(-1.5f,0, 0));
		ctx.mul_modelview_matrix(cgv::math::scale4<double>(0.5, 0.5, 0.5));
		cgv::render::point_renderer& pr = ref_point_renderer(ctx);
		pr.set_render_style(prs);
		pr.set_position_array(ctx, P);
		if (C.size() == P.size())
			pr.set_color_array(ctx, C);
		if (pr.validate_and_enable(ctx)) {
			glDrawArrays(GL_POINTS, 0, (GLsizei)P.size());
			pr.disable(ctx);	
		}
		ctx.mul_modelview_matrix(cgv::math::scale4<double>(2, 2, 2));
		ctx.mul_modelview_matrix(cgv::math::translate4<double>(3, 0, 0));
	}

	if (M_POINTS.size() > 0) {
		ctx.mul_modelview_matrix(cgv::math::translate4<double>(1.5, 0, 0));
		if (M_TRIANGLES.size() > 0) {
			ctx.mul_modelview_matrix(cgv::math::scale4<double>(1.0/128, 1.0/128, -1.0/128));
			shader_program& prog = rgbd_prog;
			if (prog.is_created()) {
				color.enable(ctx, 0);
				prog.set_uniform(ctx, "color_texture", 0);
				prog.enable(ctx);
				glDisable(GL_CULL_FACE);
				float* tex_coords = nullptr;
				if (M_UV.size() > 0) {
					tex_coords = reinterpret_cast<float*>(M_UV.data());
				}
				ctx.draw_faces(reinterpret_cast<float*>(M_POINTS.data()), nullptr,tex_coords,
					reinterpret_cast<int32_t*>(M_TRIANGLES.data()), nullptr, reinterpret_cast<int32_t*>(M_TRIANGLES.data()), M_TRIANGLES.size(), 3);
				glEnable(GL_CULL_FACE);
				prog.disable(ctx);
				color.disable(ctx);
			}
			ctx.mul_modelview_matrix(cgv::math::scale4<double>(128, 128, -128));
		}
		else {
			cgv::render::point_renderer& pr = ref_point_renderer(ctx);
			pr.set_render_style(prs);
			pr.set_position_array(ctx, M_POINTS);
			std::vector<rgba8> colors(M_POINTS.size(),rgba8(127,127,127,255));
			pr.set_color_array(ctx,colors);
			if (pr.validate_and_enable(ctx)) {
				glDrawArrays(GL_POINTS, 0, (GLsizei)M_POINTS.size());
				pr.disable(ctx);
			}
		}
		ctx.mul_modelview_matrix(cgv::math::translate4<double>(-1.5, 0, 0));
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
	string device_def = "enums='detached=-2;protocol=-1";
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
	add_view("nr_mesh_frames", nr_mesh_frames);

	if (begin_tree_node("Device", nr_color_frames, true, "level=2")) {
		align("\a");
		add_member_control(this, "stream_color", stream_color, "check");
		add_member_control(this, "stream_depth", stream_depth, "check");
		add_member_control(this, "stream_infrared", stream_infrared, "check");
		add_member_control(this, "stream_mesh", stream_mesh, "check");
		add_member_control(this, "color_stream_format", (DummyEnum&)color_stream_format_idx, "dropdown", get_stream_format_enum(color_stream_formats));
		add_member_control(this, "depth_stream_format", (DummyEnum&)depth_stream_format_idx, "dropdown", get_stream_format_enum(depth_stream_formats));
		add_member_control(this, "ir_stream_format", (DummyEnum&)ir_stream_format_idx, "dropdown", get_stream_format_enum(ir_stream_formats));
		connect_copy(add_button("st&art", "shortcut='a'")->click, rebind(this, &rgbd_control::on_start_cb));
		connect_copy(add_button("&step", "shortcut='s'")->click, rebind(this, &rgbd_control::on_step_cb));
		connect_copy(add_button("st&op", "shortcut='o'")->click, rebind(this, &rgbd_control::on_stop_cb));
		align("\b");
		end_tree_node(nr_color_frames);
	}
	if (begin_tree_node("IO", do_protocol, true, "level=2")) {
		align("\a");
		add_gui("protocol_path", protocol_path, "directory", "w=150");
		add_member_control(this, "write_async", rgbd_inp.protocol_write_async, "toggle");
		add_member_control(this, "do_protocol", do_protocol, "toggle");
		connect_copy(add_button("clear protocol")->click, rebind(this, &rgbd_control::on_clear_protocol_cb));
		connect_copy(add_button("save")->click, rebind(this, &rgbd_control::on_save_cb));
		connect_copy(add_button("save point cloud")->click, rebind(this, &rgbd_control::on_save_point_cloud_cb));
		connect_copy(add_button("load")->click, rebind(this, &rgbd_control::on_load_cb));
		align("\b");
		end_tree_node(do_protocol);
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
		if (begin_tree_node("point style", prs)) {
			align("\a");
			add_gui("point style", prs);
			align("\b");
			end_tree_node(prs);
		}
		align("\b");
		end_tree_node(always_acquire_next);
	}
}

size_t rgbd_control::construct_point_cloud()
{

	P2.clear();
	C2.clear();
	const unsigned short* depths = reinterpret_cast<const unsigned short*>(&depth_frame_2.frame_data.front());
	const unsigned char* colors = reinterpret_cast<const unsigned char*>(&color_frame_2.frame_data.front());
	if (remap_color) {
		rgbd_inp.map_color_to_depth(depth_frame_2, color_frame_2, warped_color_frame_2);
		colors = reinterpret_cast<const unsigned char*>(&warped_color_frame_2.frame_data.front());
	}
	unsigned bytes_per_pixel = color_frame_2.nr_bits_per_pixel / 8;
	int i = 0;
	float s = 1.0f / 255;
	for (int y = 0; y < depth_frame_2.height; ++y)
		for (int x = 0; x < depth_frame_2.width; ++x) {
			vec3 p;
			rgba8 point_color;
			if (rgbd_inp.map_depth_to_point(x, y, depths[i], &p[0])) {
				switch (color_frame_2.pixel_format) {
				case PF_BGR:
				case PF_BGRA:
					if (color_frame_2.nr_bits_per_pixel == 32) {
						point_color = rgba8(colors[bytes_per_pixel * i + 2], colors[bytes_per_pixel * i + 1], colors[bytes_per_pixel * i], colors[bytes_per_pixel * i + 3]);
					} else {
						point_color = rgba8(colors[bytes_per_pixel * i + 2], colors[bytes_per_pixel * i + 1], colors[bytes_per_pixel * i], 255);
					}
					break;
				case PF_RGB:
				case PF_RGBA:
					if (color_frame_2.nr_bits_per_pixel == 32) {
						point_color = rgba8(colors[bytes_per_pixel * i], colors[bytes_per_pixel * i + 1], colors[bytes_per_pixel * i + 2], colors[bytes_per_pixel * i + 3]);
					} else {
						point_color = rgba8(colors[bytes_per_pixel * i], colors[bytes_per_pixel * i + 1], colors[bytes_per_pixel * i + 2], 255);
					}
					break;
				case PF_BAYER:
					point_color = rgba8(colors[i], colors[i], colors[i], 255);
					break;
				}
				//filter points without color for 32 bit formats
				static const rgba8 filter_color = rgba8(0, 0, 0, 0);
				if (!(point_color ==  filter_color)) {
					C2.push_back(point_color);
					// flipping y to make it the same direction as in pixel y coordinate
					p[1] = -p[1];
					P2.push_back(p);
				}
			}
			++i;
		}
	/* debug code to print out bounding box of points */
	/*
	box3 box;
	for (const auto& p : P2)
		box.add_point(p);
	std::cout << "constructed " << P2.size() << " points with box = " << box << std::endl;
	*/
	return P2.size();
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
				if (stream_mesh) {
					bool new_mesh_frame_changed = rgbd_inp.get_frame(IS_MESH, mesh_frame, 0);
					if (new_mesh_frame_changed) {
						++nr_mesh_frames;
						mesh_frame_changed = new_mesh_frame_changed;
						new_frame = true;
						update_member(&nr_mesh_frames);
						M_POINTS.clear();
						M_TRIANGLES.clear();
						M_UV.clear();

						if (mesh_frame.frame_data.size() > sizeof(uint32_t)) {														
							if (mesh_frame.pixel_format == PF_POINTS_AND_TRIANGLES) {
								mesh_data_view mesh(mesh_frame.frame_data.data(), mesh_frame.frame_data.size());
								if (mesh.is_valid()) {
									M_POINTS.resize(mesh.points_size);
									std::copy(mesh.points, mesh.points + mesh.points_size, M_POINTS.data());
									M_TRIANGLES.resize(mesh.triangles_size);
									std::copy(mesh.triangles, mesh.triangles + mesh.triangles_size, M_TRIANGLES.data());
									M_UV.resize(mesh.uv_size);
									std::copy(mesh.uv, mesh.uv + mesh.uv_size, M_UV.data());
								}
							}
						}
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
					future_handle = std::async(std::launch::async, &rgbd_control::construct_point_cloud, this);
				}
				else {
					if (remap_color)
						rgbd_inp.map_color_to_depth(depth_frame, color_frame, warped_color_frame);
				}
			}
		}
	}
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
	rgbd_inp.stop();
	stopped = true;
	rgbd_inp.detach();
	if (device_idx == -1)
		device_mode = DM_PROTOCOL;
	else if (device_idx == -2)
		device_mode = DM_DETACHED;
	else 
		device_mode = DM_DEVICE;

	bool reset_format_indices = false;
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
				reset_format_indices = true;
				rgbd_inp.set_pitch(pitch);
			}
			else {
				device_mode = DM_DETACHED;
				update_member(&device_mode);
			}
		}
	}
	else if (device_mode == DM_PROTOCOL) {
		if (cgv::utils::dir::exists(protocol_path)) {
			rgbd_inp.attach_path(protocol_path);
			update_stream_formats();
			reset_format_indices = true;
			update_member(&device_idx);
		}
		else {
			cgv::gui::message(protocol_path + " does not exist!");
			device_mode = DM_DETACHED;
			device_idx = -2;
			update_member(&device_mode);
			update_member(&device_idx);
		}
	}
	if (reset_format_indices) {
		color_stream_format_idx = -1;
		depth_stream_format_idx = -1;
		ir_stream_format_idx = -1;
		update_member(&color_stream_format_idx);
		update_member(&depth_stream_format_idx);
		update_member(&ir_stream_format_idx);
	}
	color_attachment_changed = depth_attachment_changed = infrared_attachment_changed = true;
	post_redraw();
}

void rgbd_control::on_pitch_cb()
{
	if (rgbd_inp.is_attached())
		rgbd_inp.set_pitch(pitch);
}

void rgbd_control::on_clear_protocol_cb()
{
	//delete previosly recorded data
	if (!rgbd_inp.is_started() || (rgbd_inp.is_started() && !do_protocol)){
		rgbd_inp.clear_protocol(protocol_path);
	}
}

#include "lib_begin.h"

extern CGV_API object_registration<rgbd_control> kc_or("");