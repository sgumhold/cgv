#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/data/data_view.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/render/texture.h>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::data;
using namespace cgv::render;
using namespace cgv::signal;
using namespace cgv::utils;

class textured_shape : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::provider,
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
public:
	int n;
	texture t;
	bool boost_animation;
	// support for different objects
	enum Object { 
		CUBE, SPHERE, SQUARE, LAST_OBJECT 
	} object;

	textured_shape(int _n = 1024) : node("textured primitiv"), n(_n), t("uint8[L]")
	{
		object = SQUARE;
		boost_animation = false;
	}

	std::string get_type_name() const 
	{ 
		return "textured_shape"; 
	}
	void stream_stats(std::ostream& os)
	{
//		oprintf(os, "min_filter: %s [edit:<F>], anisotropy: %f [edit:<A>]\n", filter_names[min_filter], anisotropy);
	}
	void stream_help(std::ostream& os)
	{
//		os << "select object: <1> ... cube, <2> ... sphere, <3> ... square" << std::endl;
	}
	bool init(context& ctx)
	{
		data_format df(n,n,TI_FLT32,CF_L);
		data_view dv(&df);
		int i,j;
		float* ptr = (float*)dv.get_ptr<unsigned char>();
		for (i=0; i<n; ++i)
			for (j=0; j<n; ++j)
				ptr[i*n+j] = (float)(((i/8)&1) ^((j/8)&1));
		t.create(ctx, dv);
		return true;
	}
	void draw(context& ctx)
	{
		glColor3d(1,1,0);
		t.enable(ctx);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		switch (object) {
		case CUBE :	ctx.tesselate_unit_cube(); break;
		case SPHERE : ctx.tesselate_unit_sphere(100); break;
		case SQUARE :
			glDisable(GL_CULL_FACE);
				glBegin(GL_QUADS);
					glNormal3d(0,0,1);
					glTexCoord2d(0,0);
					glVertex3d(-1,-1,0);
					glTexCoord2d(1,0);
					glVertex3d(1,-1,0);
					glTexCoord2d(1,1);
					glVertex3d(1,1,0);
					glTexCoord2d(0,1);
					glVertex3d(-1,1,0);
				glEnd();
			glEnable(GL_CULL_FACE);
			break;
		default: break;
		}
		t.disable(ctx);
		std::cout << ".";
		std::cout.flush();
		if (boost_animation) {
			post_redraw();
		}
	}
	///
	void update_texture_state()
	{
		t.set_wrap_s(t.get_wrap_s());
		post_redraw();
	}
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const { return "example/texture"; }
	/// you must overload this for gui creation
	void create_gui() 
	{	
		connect_copy(add_control("boost_animation", boost_animation, "check")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("shape", object, "cube,sphere,square")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("mag filter", t.mag_filter, "nearest,linear")
			->value_change,rebind(this,&textured_shape::update_texture_state));
		connect_copy(add_control("min filter", t.min_filter, 
			"nearest,linear,nearest mp nearest,linear mp nearest,nearest mp linear,linear mp linear,anisotropy")
			->value_change,rebind(this,&textured_shape::update_texture_state));
		connect_copy(add_control("anisotropy", t.anisotropy, "value_slider",
			"min=1;max=16;ticks=true;log=true")
			->value_change,rebind(this,&textured_shape::update_texture_state));
		connect_copy(add_control("wrap s", t.wrap_s,
			"repeat,clamp,clamp to edge,clamp to border,mirror clamp,mirror clamp to edge,mirror clamp to border")
			->value_change,rebind(this,&textured_shape::update_texture_state));
		connect_copy(add_control("wrap t", t.wrap_t,
			"repeat,clamp,clamp to edge,clamp to border,mirror clamp,mirror clamp to edge,mirror clamp to border")
			->value_change,rebind(this,&textured_shape::update_texture_state));
		connect_copy(add_control("priority", t.priority, "value_slider",
			"min=0;max=1;ticks=true")
			->value_change,rebind(this,&textured_shape::update_texture_state));
	}

};

#include <cgv/base/register.h>

extern factory_registration_1<textured_shape,int> tp_fac("new/textured primitive", 'T', 1024, true);

