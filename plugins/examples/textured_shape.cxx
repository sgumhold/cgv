#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/data/data_view.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/render/texture.h>
#include <cgv/media/color.h> 

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
	typedef cgv::media::color<float,cgv::media::RGB,cgv::media::OPACITY> color_type;
	int n;
	texture t;
	bool boost_animation;
	// support for different objects
	enum Object { 
		CUBE, SPHERE, SQUARE, LAST_OBJECT 
	} object;
	enum TextureSelection { CHECKER, WAVES, ALHAMBRA, CARTUJA } texture_selection;
	float texture_frequency, texture_frequency_aspect;
	float texture_u_offset, texture_v_offset;
	float texture_rotation;
	float texture_scale,  texture_aspect;
	color_type border_color;
	color_type current_color;
	color_type frame_color;
	float frame_width;

	textured_shape(int _n = 1024) : node("textured primitiv"), n(_n), t("uint8[L]"), border_color(1,0,0,0), frame_color(1,1,1,1), current_color(1,1,0,0)
	{
		frame_width = 2;
		object = SQUARE;
		texture_selection = CHECKER;
		texture_frequency = 50;
		texture_frequency_aspect = 1;
		texture_scale = 1;
		texture_aspect = 1;
		texture_rotation = 0;
		texture_u_offset = 0;
		texture_v_offset = 0;
		texture_selection = ALHAMBRA;
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
	void init_frame(context& ctx)
	{
		if (t.is_created())
			return;

		if (texture_selection == ALHAMBRA) {
			std::cout << "before read bump texture" << std::endl;		
			if (!t.create_from_image(ctx,"res://alhambra.png", (int*)&n)) {
				std::cout << "could not read" << std::endl ;
				exit(0);
			}
			update_member(&n);
			return;
		}
	
		if (texture_selection == CARTUJA) {
			t.create_from_image(ctx,"res://cartuja.png", (int*)&n);
			update_member(&n);
			return;
		}
	
		data_format df(n,n,TI_FLT32,CF_L);
		data_view dv(&df);
		int i,j;
		float* ptr = (float*)dv.get_ptr<unsigned char>();
		for (i=0; i<n; ++i)
			for (j=0; j<n; ++j)
				if (texture_selection == CHECKER)
					ptr[i*n+j] = (float)(((i/8)&1) ^((j/8)&1));
				else
					ptr[i*n+j] = 
					   (float)(0.5*(pow(cos(M_PI*texture_frequency/texture_frequency_aspect*i/(n-1)),3)*
								   sin(M_PI*texture_frequency*j/(n-1))+1));
		t.create(ctx, dv);
	}
	void draw_scene(context& ctx)
	{
		switch (object) {
		case CUBE :	ctx.tesselate_unit_cube(); break;
		case SPHERE : 
			ctx.tesselate_unit_sphere(100); break;
		case SQUARE :
			glDisable(GL_CULL_FACE);
				glBegin(GL_QUADS);
					glNormal3d(0,0,1);
					glTexCoord2d(0,0);
					glVertex3d(0,0,0);
					glTexCoord2d(1,0);
					glVertex3d(1,0,0);
					glTexCoord2d(1,1);
					glVertex3d(1,1,0);
					glTexCoord2d(0,1);
					glVertex3d(0,1,0);
				glEnd();
			glEnable(GL_CULL_FACE);
			break;
		default: break;
		}
	}
	void draw(context& ctx)
	{
		if (frame_width > 0) {
			glPushMatrix();
			glScaled(1.0/texture_scale,texture_aspect/texture_scale,1.0/texture_scale);
			glRotated(-texture_rotation, 0, 0, 1);
			glTranslated(-texture_u_offset, -texture_v_offset,0);
			glColor4fv(&frame_color[0]);
			glLineWidth(frame_width);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_LIGHTING);
			draw_scene(ctx);
			glEnable(GL_LIGHTING);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);				
			glPopMatrix();
		}

		glColor4fv(&current_color[0]);
		t.set_border_color(border_color[0],border_color[1],border_color[2],border_color[3]);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glTranslated(texture_u_offset, texture_v_offset,0);
		glRotated(texture_rotation, 0, 0, 1);
		glScaled(texture_scale,texture_scale/texture_aspect,texture_scale);
		t.enable(ctx);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		draw_scene(ctx);
		t.disable(ctx);
		glPopMatrix();
		
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
		t.set_wrap_t(t.get_wrap_t());
		post_redraw();
	}
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const { return "example/texture"; }
	// destruct texture for reconstruction in next draw call
	void on_texture_change()
	{
		if (!get_context())
			return;
		get_context()->make_current();
		t.destruct(*get_context());
		post_redraw();
	}

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
			"repeat,clamp,clamp to edge,clamp to border,mirror clamp,mirror clamp to edge,mirror clamp to border,mirrored repeat")
			->value_change,rebind(this,&textured_shape::update_texture_state));
		connect_copy(add_control("wrap t", t.wrap_t,
			"repeat,clamp,clamp to edge,clamp to border,mirror clamp,mirror clamp to edge,mirror clamp to border,mirrored repeat")
			->value_change,rebind(this,&textured_shape::update_texture_state));
		connect_copy(add_control("border_color", border_color)
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("priority", t.priority, "value_slider",
			"min=0;max=1;ticks=true")
			->value_change,rebind(this,&textured_shape::update_texture_state));
		connect_copy(add_control("current_color", current_color)
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));

		add_decorator("Texture Properties", "heading");
		connect_copy(add_control("texture", texture_selection, "checker,waves,alhambra,cartuja")
			->value_change,rebind(this,&textured_shape::on_texture_change));
		connect_copy(add_control("frequency", texture_frequency, "value_slider", "min=0;max=200;log=true;ticks=true")
			->value_change,rebind(this,&textured_shape::on_texture_change));
		connect_copy(add_control("frequency aspect", texture_frequency_aspect, "value_slider", "min=0.1;max=10;log=true;ticks=true")
			->value_change,rebind(this,&textured_shape::on_texture_change));
		connect_copy(add_control("texture resolution", n, "value_slider", "min=4;max=1024;log=true;ticks=true")
			->value_change,rebind(this,&textured_shape::on_texture_change));

		add_decorator("Transformation", "heading", "level=2");
		connect_copy(add_control("texture u offset", texture_u_offset, "value_slider", "min=-1;max=1;ticks=true")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("texture v offset", texture_v_offset, "value_slider", "min=-1;max=1;ticks=true")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("texture rotation", texture_rotation, "value_slider", "min=-180;max=180;ticks=true")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("texture scale", texture_scale, "value_slider", "min=0.01;max=100;log=true;ticks=true")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("texture aspect", texture_aspect, "value_slider", "min=0.1;max=10;log=true;ticks=true")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));

		add_decorator("Frame", "heading", "level=2");
		connect_copy(add_control("width", frame_width, "value_slider", "min=0;max=20;ticks=true")
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_control("color", frame_color)
			->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
	}

};

#include <cgv/base/register.h>

extern factory_registration_1<textured_shape,int> tp_fac("new/textured primitive", 'T', 1024, true);

