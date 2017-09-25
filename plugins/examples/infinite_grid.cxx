#include <cgv/base/base.h>
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
using namespace cgv::signal;
using namespace cgv::render;
using namespace cgv::utils;

class infinite_grid : 
	public cgv::base::base,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::provider,
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
public:
	bool recreate_texture;
	int resolution, width;
	double scale;
	texture t;

	infinite_grid(int _r = 8) : 
		resolution(_r), width(1), scale(1), recreate_texture(true),
		t("[R,G,B,A]", TF_NEAREST, TF_LINEAR_MIPMAP_LINEAR, TW_REPEAT, TW_REPEAT) 
	{}

	void init_frame(context& ctx)
	{
		if (recreate_texture) {
			int n = (int)pow(2.0,resolution);
			data_format df(n,n,TI_UINT8,CF_RGBA);
			data_view dv(&df);
			int i,j;
			unsigned char* ptr = dv.get_ptr<unsigned char>();
			for (i=0; i<n; ++i)
				for (j=0; j<n; ++j) {
					ptr[0] = ptr[1] = ptr[2] = 255;
					ptr[3] = ((i<width)||(j<width))?150:0;
					ptr += 4;
				}
			t.destruct(ctx);
			t.create(ctx, dv);
		}
	}
	std::string get_type_name() const 
	{ 
		return "infinite grid"; 
	}
	void stream_stats(std::ostream& os)
	{
		oprintf(os, "infinite grid: resolution=%d, scale=%f\n", resolution, scale);
	}
	void stream_help(std::ostream& os)
	{
	}
	/// called after the drawing 
	void finish_frame(context& ctx)
	{
		glColor3d(1,1,1);
		glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_TRANSFORM_BIT | 
			         GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);

		glDisable(GL_CULL_FACE);
		//glDisable(GL_LIGHTING);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER,0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadIdentity();
		glScaled(scale,scale,scale);
		t.enable(ctx);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslated(-0.5*width*pow(0.5,resolution), -width*0.5*pow(0.5,resolution), 0);

		glBegin(GL_TRIANGLE_FAN);
			glTexCoord4f( 0, 0, 0, 1); glVertex4f( 0, 0, 0, 1);
			glTexCoord4f( 1, 0, 0, 0); glVertex4f( 1, 0, 0, 0);
			glTexCoord4f( 0, 1, 0, 0); glVertex4f( 0, 1, 0, 0);
			glTexCoord4f(-1, 0, 0, 0); glVertex4f(-1, 0, 0, 0);
			glTexCoord4f( 0,-1, 0, 0); glVertex4f( 0,-1, 0, 0);
			glTexCoord4f( 1, 0, 0, 0); glVertex4f( 1, 0, 0, 0);
		glEnd();

		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		t.disable(ctx);
		glPopAttrib();
	}
	/// called when texture needs to be recreated
	void on_recreate_texture()
	{
		if (find_control(width))
			find_control(width)->set("max", (int)pow(2.0,resolution));
		recreate_texture = true;
		post_redraw();
	}
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const { return "example/infinite grid"; }
	/// you must overload this for gui creation
	void create_gui() 
	{	
		add_decorator("infinite grid", "heading");
		connect_copy(add_control("scale", scale, "value_slider","min=0.1;max=100;ticks=true;log=true")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("resolution", resolution, "value_slider","min=1;max=10;ticks=true;log=true")
			->value_change,rebind(this,&infinite_grid::on_recreate_texture));
		connect_copy(add_control("width", width, "value_slider","min=1;ticks=true;log=true")
			->value_change,rebind(this,&infinite_grid::on_recreate_texture));
		find_control(width)->set("max", (int)pow(2.0,resolution));
	}
};

#include <cgv/base/register.h>

extern factory_registration_1<infinite_grid,int> ig_fac("new/infinite grid", 'I', 8, true);

