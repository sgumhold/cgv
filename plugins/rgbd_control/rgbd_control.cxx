#include <cgv/base/base.h>
#include <omp.h>
#include "rgbd_control.h"
#include <cgv_gl/gl/gl.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/utils/convert.h>
#include <cgv/utils/file.h>
#include <cgv/type/standard_types.h>
#include <cgv/math/ftransform.h>

using namespace std;
using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::type;
using namespace cgv::gui;
using namespace cgv::data;
using namespace cgv::utils;
using namespace cgv::render;
using namespace rgbd;


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
//	color("[R,G,B,A]", TF_NEAREST, TF_NEAREST),
	depth_fmt("uint16[L]"), 
	depth("uint16[L]", TF_NEAREST, TF_NEAREST)
{
	set_name("rgbd_control");
	do_protocol = false;
	flip[0] = flip[2] = true;
	flip[1] = false;
	nr_depth_frames = 0;
	nr_color_frames = 0;
	vis_mode = VM_POINTS;
	color_scale = 1;
	depth_scale = 1;
	near_mode = true;

	point_size = 3;

	remap_color = true;

	device_mode = DM_DEVICE;
	device_idx = 0;
	pitch = 0;
	x=y=z=0;
	aspect = 1;
	stopped = false;
	step_only = false;

	color_frame_changed = false;
	depth_frame_changed = false;
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
		rh.reflect_member("point_size", point_size) &&
		rh.reflect_member("device_mode", (int&)device_mode) &&
		rh.reflect_member("device_idx", device_idx);
}

///
void rgbd_control::on_set(void* member_ptr)
{
	if (member_ptr == &do_protocol) {
		if (do_protocol)
			kin.enable_protocol(protocol_path);
		else
			kin.disable_protocol();
	}
	if (member_ptr == &near_mode)
		kin.set_near_mode(near_mode);

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
	kin.stop();
	kin.detach();
}
/// adjust view
bool rgbd_control::init(cgv::render::context& ctx)
{
	cgv::render::view* view_ptr = find_view_as_node();
	if (view_ptr) {
		view_ptr->set_view_up_dir(vec3(0, -1, 0));
		view_ptr->set_focus(vec3(0, 0, 0));
	}
	return true;
}

///
void rgbd_control::init_frame(context& ctx)
{
	// ensure that shader programs are created
	const char* shader_prog_names[] = { "color_mode.glpr", "depth_mode.glpr", "point_mode.glpr" };
	unsigned i;
	for (i=0; i<3; ++i)
		if (!progs[i].is_created())
			progs[i].build_program(ctx, shader_prog_names[i]);

	if (device_mode != DM_DETACHED) {
		unsigned w=kin.get_width(), h=kin.get_height();
		if (!color.is_created() || attachment_changed)
			color.create(ctx, TT_2D, w, h);
		if (!depth.is_created() || attachment_changed)
			depth.create(ctx, TT_2D, w, h);
		if (color_frame_changed) {
			color.replace(ctx, 0, 0, color_data);
			color_frame_changed = false;
		}
		if (depth_frame_changed) {
			depth.replace(ctx, 0, 0, depth_data);
			depth_frame_changed = false;

			Pnt p = km.track(depth_data);
			mouse_pos(0) = p(0);
			mouse_pos(1) = p(1);
		}
	}
	else {
		if (color.is_created())
			color.destruct(ctx);
		if (depth.is_created())
			depth.destruct(ctx);
	}
	attachment_changed = false;
}

/// overload to draw the content of this drawable
void rgbd_control::draw(context& ctx)
{
	/*
	glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);

	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	// draw points
	if (point_size > 0 && P.size() > 0) {
		glPushMatrix();
		glScaled(1,-1,-1);
		glTranslated(aspect,0,0);
		glVertexPointer(3, GL_FLOAT, 0, &P[0][0]);
		glColorPointer(3, GL_FLOAT, 0, &C[0][0]);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glPointSize(point_size);
		glEnable(GL_POINT_SMOOTH);
		glDrawArrays(GL_POINTS, 0, P.size());

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
	*/
	// transform to image coordinates
	ctx.push_modelview_matrix();
	vec3 flip_vec(flip[0] ? -1.0f : 1.0f, flip[1] ? -1.0f : 1.0f, flip[2] ? -1.0f : 1.0f);
	ctx.mul_modelview_matrix(cgv::math::scale4<double>(flip_vec[0]*aspect, flip_vec[1], flip_vec[2]));
	// enable shader program
	if (progs[vis_mode].is_created()) {
		color.enable(ctx, 0);
		depth.enable(ctx, 1);
		progs[vis_mode].set_uniform(ctx, "color_texture", 0);
		progs[vis_mode].set_uniform(ctx, "depth_texture", 1);
		progs[vis_mode].set_uniform(ctx, "color_scale",color_scale);
		progs[vis_mode].set_uniform(ctx, "depth_scale",32*depth_scale);
		progs[vis_mode].enable(ctx);
	}
	// or standard texture mapping
	else {
		ctx.ref_default_shader_program(true).enable(ctx);
		if (vis_mode == VM_COLOR)
			color.enable(ctx);
		else
			depth.enable(ctx);
	}
	// draw points or square
	if (vis_mode == VM_POINTS) {
		glPointSize(point_size);
		progs[vis_mode].set_uniform(ctx, "w", (int)kin.get_width());
		progs[vis_mode].set_uniform(ctx, "h", (int)kin.get_height());
		glDrawArraysInstanced(GL_POINTS, 0, 1, kin.get_width()*kin.get_height());
	}
	else {
		glDisable(GL_CULL_FACE);
		ctx.tesselate_unit_square();
		glEnable(GL_CULL_FACE);
	}

	// disable shader program or texture
	// enable shader program
	if (progs[vis_mode].is_created()) {
		progs[vis_mode].disable(ctx);
		depth.disable(ctx);
		color.disable(ctx);
	}
	// or standard texture mapping
	else {
		if (vis_mode == VM_COLOR)
			color.disable(ctx);
		else
			depth.disable(ctx);
		ctx.ref_default_shader_program(true).disable(ctx);
	}

	// restore gl state
	ctx.pop_modelview_matrix();

	//glPopAttrib();
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
	case 'D':
		if (ke.get_modifiers() == 0) {
			vis_mode = VM_DEPTH;
			on_set(&vis_mode);
			return true;
		}
		return false;
	case 'P':
		if (ke.get_modifiers() == 0) {
			vis_mode = VM_POINTS;
			on_set(&vis_mode);
			return true;
		}
		return false;
	}
	return false;
}
/// 
void rgbd_control::stream_help(std::ostream& os)
{
	os << "rgbd_control: select vismode <C|D|P>; toggle <R>emapColor, <N>earMode, flip <X|Y|Z>" << std::endl;
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
	add_decorator("Point Cloud", "heading", "level=2");
	connect_copy(add_button("construct")->click, rebind(this, &rgbd_control::construct_point_cloud));	
	connect_copy(add_control("point_size", point_size, "value_slider", "min=0;max=20;ticks=true")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	add_decorator("Base", "heading", "level=2");
	connect_copy(add_control("pitch", pitch, "value_slider", "min=-1;max=1;ticks=true")->value_change, rebind(this, &rgbd_control::on_pitch_cb));	
	add_view("x",x);
	add_view("y",y);
	add_view("z",z);
	add_decorator("Camera", "heading", "level=2");
	connect_copy(add_control("vis_mode", vis_mode, "dropdown", "enums='color,depth,points'")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("color_scale", color_scale, "value_slider", "min=0.1;max=100;log=true;ticks=true")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	connect_copy(add_control("depth_scale", depth_scale, "value_slider", "min=0.1;max=100;log=true;ticks=true")->value_change, rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	add_view("nr_color_frames", nr_color_frames);
	add_view("nr_depth_frames", nr_depth_frames);
	connect_copy(add_button("st&art", "shortcut='a'")->click, rebind(this, &rgbd_control::on_start_cb));
	connect_copy(add_button("&step", "shortcut='s'")->click, rebind(this, &rgbd_control::on_step_cb));
	connect_copy(add_button("st&op", "shortcut='o'")->click, rebind(this, &rgbd_control::on_stop_cb));
	connect_copy(add_button("save")->click, rebind(this, &rgbd_control::on_save_cb));
	connect_copy(add_button("save point cloud")->click, rebind(this, &rgbd_control::on_save_point_cloud_cb));
	connect_copy(add_button("load")->click, rebind(this, &rgbd_control::on_load_cb));
}

void rgbd_control::construct_point_cloud()
{
	unsigned i,j;
	unsigned w = kin.get_width();
	unsigned h = kin.get_height();
	unsigned short* depth_ptr = depth_data.get_ptr<unsigned short>();
	unsigned char*  color_ptr = color_data.get_ptr<unsigned char>();
	P.reserve(w*h);
	C.reserve(w*h);
	for (j=0; j<h; ++j) {
		unsigned short* depth_line_ptr = depth_ptr+j*w;
		for (i=0; i<w; ++i) {
			if (*depth_line_ptr < 2047) {
				Pnt pnt = DepthToWorld(i,j,*depth_line_ptr);
				Tex tex = WorldToPixel(pnt);
				Clr clr(0,0,0);
				int x = (int) tex(0);
				if (x >= 0 && x < (int)w) {
					int y = (int) tex(1);
					if (y >= 0 && y < (int)h) {
						unsigned char* color_pixel_ptr = color_ptr + (y*w+x)*4;
						clr[0] = color_pixel_ptr[2]*1.0f/255;
						clr[1] = color_pixel_ptr[1]*1.0f/255;
						clr[2] = color_pixel_ptr[0]*1.0f/255;
					}
				}
				P.push_back(pnt);
				C.push_back(clr);
			}
			++depth_line_ptr;
		}
	}
	post_redraw();
}


void rgbd_control::timer_event(double t, double dt)
{
	unsigned w = kin.get_width();
	unsigned h = kin.get_height();
	if (kin.is_attached()) {
		IMU_measurement m;
		if (kin.put_IMU_measurement(m, 10)) {
			x = m.linear_acceleration[0];
			y = m.linear_acceleration[1];
			z = m.linear_acceleration[2];
			update_member(&x);
			update_member(&y);
			update_member(&z);
		}
		if (kin.is_started() && !attachment_changed) {
			depth_frame_changed = kin.get_frame(rgbd::FF_DEPTH_RAW, depth_data.get_ptr<unsigned char>(), 0);
			P.clear();
			if (depth_frame_changed) {
				++nr_depth_frames;
				update_member(&nr_depth_frames);
			}

			color_frame_changed = kin.get_frame(rgbd::FF_COLOR_RGB32, color_data.get_ptr<unsigned char>(), 0);
			C.clear();
			if (color_frame_changed) {
				++nr_color_frames;
				update_member(&nr_color_frames);
			}

			if (color_frame_changed || depth_frame_changed) {
				if (remap_color)
					kin.map_color_to_depth(rgbd::FF_DEPTH_RAW, depth_data.get_ptr<unsigned char>(), rgbd::FF_COLOR_RGB32, color_data.get_ptr<unsigned char>());
				post_redraw();
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
	kin.set_near_mode(near_mode);
	kin.start(IS_COLOR_AND_DEPTH);
	stopped = false;
}

void rgbd_control::on_step_cb()
{
	kin.start(IS_COLOR_AND_DEPTH);
	stopped = false;
	step_only = true;
}

void rgbd_control::on_stop_cb()
{
	kin.stop();
	stopped = true;
}

void rgbd_control::on_save_cb()
{
	std::string fn = cgv::gui::file_save_dialog("base file name", "Frame Files (rgb,dep):*.rgb;*.dep");
	if (fn.empty())
		return;
	std::string fnc = fn+".rgb";
	std::string fnd = fn+".dep";
	if (!cgv::utils::file::write(fnc, color_data.get_ptr<char>(), color_data.get_format()->get_nr_bytes()*color_data.get_format()->get_entry_size(), false))
		std::cerr << "could not write " << fnc << std::endl;
	if (!cgv::utils::file::write(fnd, depth_data.get_ptr<char>(), depth_data.get_format()->get_nr_bytes()*depth_data.get_format()->get_entry_size(), false))
		std::cerr << "could not write " << fnd << std::endl;
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
		fwrite(&P[0][0],sizeof(Pnt),n,fp) == n;
	if (C.size() == n)
		success = success && (fwrite(&C[0][0],sizeof(Clr),n,fp) == n);
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
	std::string fnd = fn+".dep";
	std::string c, d;
	if (cgv::utils::file::read(fnc, c, false)) {
		memcpy(color_data.get_ptr<unsigned char>(), &c[0], color_data.get_format()->get_nr_bytes()*color_data.get_format()->get_entry_size());
		color_frame_changed = true;
	}
	else
		std::cerr << "could not read " << fnc << std::endl;
	if (cgv::utils::file::read(fnd, d, false)) {
		memcpy(depth_data.get_ptr<unsigned char>(), &d[0], depth_data.get_format()->get_nr_bytes()*depth_data.get_format()->get_entry_size());
		depth_frame_changed = true;
	}
	else
		std::cerr << "could not read " << fnd << std::endl;
	post_redraw();
}

void rgbd_control::on_device_select_cb()
{
	kin.detach();
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
			kin.attach(rgbd_input::get_serial(device_idx));
			kin.set_pitch(pitch);
		}
	}
	else if (device_mode == DM_PROTOCOL) {
		kin.attach_path(protocol_path);
		update_member(&device_idx);
		// on_device_select_cb();
	}
	if (device_mode != DM_DETACHED) {
		color_fmt.set_width(kin.get_width());
		color_fmt.set_height(kin.get_height());
		depth_fmt.set_width(kin.get_width());
		depth_fmt.set_height(kin.get_height());
		aspect = (float)kin.get_width()/kin.get_height();
		color_data = data_view(&color_fmt);
		depth_data = data_view(&depth_fmt);
	}
	attachment_changed = true;
	post_redraw();
}

void rgbd_control::on_pitch_cb()
{
	if (kin.is_attached())
		kin.set_pitch(pitch);
}

#include "lib_begin.h"

extern CGV_API object_registration<rgbd_control> kc_or("");