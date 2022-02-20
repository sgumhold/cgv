#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/media/illum/phong_material.hh>
#include <cgv/render/drawable.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv/render/shader_code.h>
#include <cgv/render/shader_program.h>

#ifdef WIN32
#pragma warning(disable:4996)
#endif

using namespace cgv::base;
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::media::illum;

class shader_test : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::provider,
	public cgv::gui::event_handler,
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
public:
	enum ShadingType { ST_GOURAUD, ST_BLINN_PHONG } shading_type;
	shader_code vs, fs, ls;
	shader_program prog;
	phong_material mat;
	int resolution;
	shader_test() : node("shader test"), shading_type(ST_GOURAUD)
	{
		resolution  = 10;
	}

	std::string get_type_name() const 
	{ 
		return "shader_test"; 
	}
	bool init(context& ctx)
	{
		if (!prog.build_program(ctx, "shader_test.glpr"))
			return false;
		return true;
		if (!vs.read_and_compile(ctx, "shader_test.glvs")) {
			std::cout << "error reading vertex shader\n" << vs.last_error.c_str() << std::endl;
			return false;
		}
		if (!fs.read_and_compile(ctx, "shader_test.glfs")) {
			std::cout << "error reading fragment shader\n" << vs.last_error.c_str() << std::endl;
			return false;
		}
		if (!ls.read_and_compile(ctx, "lighting.glsl", ST_FRAGMENT)) {
			std::cout << "error reading lighting shader\n" << ls.last_error.c_str() << std::endl;
			return false;
		}
		if (!prog.create(ctx)) {
			std::cout << "error creating program\n" << prog.last_error.c_str() << std::endl;
			return false;
		}
		prog.attach_code(ctx, vs);
		prog.attach_code(ctx, fs);
		prog.attach_code(ctx, ls);
		if (!prog.link(ctx)) {
			std::cout << "link error\n" << prog.last_error.c_str() << std::endl;
			return false;
		}
		return true;
	}
	bool handle(event& e)
	{
		if (e.get_kind() != EID_KEY)
			return false;
		key_event& ke = static_cast<key_event&>(e);
		if (ke.get_action() != KA_PRESS)
			return false;
		switch (ke.get_key()) {
		case 'S' :
			if (shading_type == ST_GOURAUD)
				shading_type = ST_BLINN_PHONG;
			else
				shading_type = ST_GOURAUD;
			update_member(&shading_type);
			post_redraw();
			return true;
		case 'H' :
			resolution = 200;
			post_redraw();
			update_member(&resolution);
			return true;
		case 'L' :
			resolution = 10;
			post_redraw();
			update_member(&resolution);
			return true;
		}
		return false;
	}
	void stream_help(std::ostream& os)
	{
		os << "shader_test: toggle <s>hader type, <l>ow / <h>igh resolution" << std::endl;
	}
	void draw(context& ctx)
	{
		ctx.disable_phong_shading();
		ctx.enable_material(mat);
		if (shading_type == ST_BLINN_PHONG && prog.is_linked()) {
			prog.enable(ctx);
			gl::set_lighting_parameters(ctx, prog);
		}
		ctx.tesselate_unit_sphere(resolution);
		if (shading_type == ST_BLINN_PHONG && prog.is_linked())
			prog.disable(ctx);
		ctx.disable_material(mat);
	}
	void clear(context& ctx)
	{
		vs.destruct(ctx);
		fs.destruct(ctx);
		prog.destruct(ctx);
	}
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const { return "example/shader"; }
	/// you must overload this for gui creation
	void create_gui() 
	{	
		connect_copy(add_control("resolution", resolution, "value_slider", "min=3;max=200;ticks=true;log=true")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("ambient", mat.ref_ambient())
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("diffuse", mat.ref_diffuse())
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("specular", mat.ref_specular())
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("emission", mat.ref_emission())
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("shininess", mat.ref_shininess(), "value_slider", "min=0;max=128;ticks=true;log=true")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("shading", shading_type, "Gouraud,Blinn-Phong")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	}

};

#include <cgv/base/register.h>

factory_registration<shader_test> shader_test_fac("New/Render/Shader Test", 'X', true);

