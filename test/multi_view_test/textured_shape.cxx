#include <cgv/base/node.h>
#include <cgv/math/ftransform.h>
#include <cgv/signal/rebind.h>
#include <cgv/data/data_view.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/render/texture.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/textured_material.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/media/color.h> 

class textured_shape : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::provider,
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
public:
	cgv::render::texture* t_ptr;

	// animation
	bool boost_animation;

	// shape selection
	enum Object { 
		CUBE, SPHERE, SQUARE, LAST_OBJECT 
	} object;
	
	// texture generation
	enum TextureSelection { CHECKER, WAVES, ALHAMBRA, CARTUJA } texture_selection;
	float texture_frequency, texture_frequency_aspect;
	int n; // resolution

	// texture transformation
	float texture_u_offset, texture_v_offset;
	float texture_rotation;
	float texture_scale,  texture_aspect;
	
	// texture parameters
	rgba border_color;

	// appearance parameters
	rgba frame_color;
	float frame_width;
	cgv::render::textured_material mat;

	textured_shape(int _n = 1024) : 
		node("textured primitiv"), 
		n(_n), 
		border_color(1,0,0,0), 
		frame_color(1,1,1,1)
	{
		t_ptr = 0;
		frame_width = 2;
		object = CUBE;
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
	//	cgv::utils::oprintf(os, "min_filter: %s [edit:<F>], anisotropy: %f [edit:<A>]\n", filter_names[min_filter], anisotropy);
	}
	void stream_help(std::ostream& os)
	{
//		os << "select object: <1> ... cube, <2> ... sphere, <3> ... square" << std::endl;
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &texture_selection ||
			member_ptr == &texture_frequency ||
			member_ptr == &texture_frequency_aspect ||
			member_ptr == &n)
			on_texture_change();
		if (member_ptr == &t_ptr->mag_filter ||
			member_ptr == &t_ptr->min_filter ||
			member_ptr == &t_ptr->anisotropy ||
			member_ptr == &t_ptr->wrap_s ||
			member_ptr == &t_ptr->wrap_t ||
			member_ptr == &t_ptr->priority)
			update_texture_state();

		update_member(member_ptr);
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		//int di = mat.add_image_file("res://alhambra.png");
		//mat.set_diffuse_index(di);
		//if (mat.ensure_textures(ctx))
		//	t_ptr = mat.get_texture(di);
		//else
		//	return false;
		return true;
	}
	void init_frame(cgv::render::context& ctx)
	{
		if (mat.get_diffuse_index() == -1) {
			int di = mat.add_image_file("res://alhambra.png");
			mat.set_diffuse_index(di);
			if (mat.ensure_textures(ctx)) {
				t_ptr = mat.get_texture(di);
				post_recreate_gui();
			}
		}

		if (t_ptr->is_created())
			return;

		if (texture_selection == ALHAMBRA) {
			std::cout << "before read bump texture" << std::endl;		
			if (!t_ptr->create_from_image(ctx,"res://alhambra.png", (int*)&n)) {
				std::cout << "could not read" << std::endl ;
				exit(0);
			}
			update_member(&n);
			return;
		}
	
		if (texture_selection == CARTUJA) {
			t_ptr->create_from_image(ctx,"res://cartuja.png", (int*)&n);
			update_member(&n);
			return;
		}
	
		cgv::data::data_format df(n,n, cgv::type::info::TI_FLT32, cgv::data::CF_L);
		cgv::data::data_view dv(&df);
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
		t_ptr->create(ctx, dv);
	}
	void draw_scene(cgv::render::context& ctx, bool wireframe)
	{
		switch (object) {
		case CUBE :	ctx.tesselate_unit_cube(false, wireframe); break;
		case SPHERE : 
			ctx.tesselate_unit_sphere(25, false, wireframe); break;
		case SQUARE :
			glDisable(GL_CULL_FACE);
			ctx.tesselate_unit_square(false, wireframe); break;
			glEnable(GL_CULL_FACE);
			break;
		default: break;
		}
	}
	void draw(cgv::render::context& ctx)
	{
		ctx.push_modelview_matrix();
		ctx.mul_modelview_matrix(
			cgv::math::translate4<float>(0.0f, 1.2f, 0.0f) *
			cgv::math::scale4<float>(vec3(0.2f))
		);
		if (frame_width > 0) {
			ctx.ref_default_shader_program().enable(ctx);
			ctx.set_color(frame_color);
			glLineWidth(frame_width);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			draw_scene(ctx, true);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);				
			ctx.ref_default_shader_program().disable(ctx);
		}
		cgv::render::shader_program& prog = ctx.ref_surface_shader_program(true);
		prog.enable(ctx);
			t_ptr->set_border_color(border_color[0], border_color[1], border_color[2], border_color[3]);
			ctx.enable_material(mat);
				prog.set_uniform(ctx, "use_texture_matrix", true);
				prog.set_uniform(ctx, "texture_matrix", 
					cgv::math::translate4<float>(texture_u_offset, texture_v_offset, 0.0f)*
					cgv::math::rotate4<float>(texture_rotation, 0, 0, 1)*
					cgv::math::scale4<float>(texture_scale, texture_scale / texture_aspect, texture_scale)
				);
				draw_scene(ctx, false);
				prog.set_uniform(ctx, "use_texture_matrix", false);
			ctx.disable_material(mat);
		prog.disable(ctx);

		if (boost_animation) {
			post_redraw();
		}
		ctx.pop_modelview_matrix();
	}
	///
	void update_texture_state()
	{
		t_ptr->set_wrap_s(t_ptr->get_wrap_s());
		t_ptr->set_wrap_t(t_ptr->get_wrap_t());
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
		t_ptr->destruct(*get_context());
		post_redraw();
	}

	/// you must overload this for gui creation
	void create_gui() 
	{	
		add_member_control(this, "boost_animation", boost_animation, "check");
		add_member_control(this, "shape", object, "dropdown", "enums='cube,sphere,square'");
		if (t_ptr) {
			add_member_control(this, "mag filter", t_ptr->mag_filter, "dropdown", "enums='nearest,linear'");
			add_member_control(this, "min filter", t_ptr->min_filter, "dropdown", "enums='nearest,linear,nearest mp nearest,linear mp nearest,nearest mp linear,linear mp linear,anisotropy'");
			add_member_control(this, "anisotropy", t_ptr->anisotropy, "value_slider", "min=1;max=16;ticks=true;log=true");
			add_member_control(this, "wrap s", t_ptr->wrap_s, "dropdown", "enums='repeat,clamp,clamp to edge,clamp to border,mirror clamp,mirror clamp to edge,mirror clamp to border,mirrored repeat'");
			add_member_control(this, "wrap t", t_ptr->wrap_t, "dropdown", "enums='repeat,clamp,clamp to edge,clamp to border,mirror clamp,mirror clamp to edge,mirror clamp to border,mirrored repeat'");
		}
		add_member_control(this, "border_color", border_color);
		if (t_ptr)
			add_member_control(this, "priority", t_ptr->priority, "value_slider", "min=0;max=1;ticks=true");

		add_decorator("Texture Properties", "heading");
		add_member_control(this, "texture", texture_selection, "dropdown", "enums='checker,waves,alhambra,cartuja'");
		add_member_control(this, "frequency", texture_frequency, "value_slider", "min=0;max=200;log=true;ticks=true");
		add_member_control(this, "frequency aspect", texture_frequency_aspect, "value_slider", "min=0.1;max=10;log=true;ticks=true");
		add_member_control(this, "texture resolution", n, "value_slider", "min=4;max=1024;log=true;ticks=true");

		add_decorator("Transformation", "heading", "level=2");
		add_member_control(this, "texture u offset", texture_u_offset, "value_slider", "min=-1;max=1;ticks=true");
		add_member_control(this, "texture v offset", texture_v_offset, "value_slider", "min=-1;max=1;ticks=true");
		add_member_control(this, "texture rotation", texture_rotation, "value_slider", "min=-180;max=180;ticks=true");
		add_member_control(this, "texture scale", texture_scale, "value_slider", "min=0.01;max=100;log=true;ticks=true");
		add_member_control(this, "texture aspect", texture_aspect, "value_slider", "min=0.1;max=10;log=true;ticks=true");

		add_decorator("Colors", "heading", "level=2");
		add_member_control(this, "width", frame_width, "value_slider", "min=0;max=20;ticks=true");
		add_member_control(this, "color", frame_color);
		add_gui("material", static_cast<cgv::media::illum::surface_material&>(mat));
	}

};

#include <cgv/base/register.h>

cgv::base::object_registration_1<textured_shape,int> tp_obj_reg(1024, "textured_primitive");

