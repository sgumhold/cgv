#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/application.h>
#include <cgv/render/drawable.h>
#include <cgv/math/vec.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv_gl/gl/gl.h>
#include <vector>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::signal;
using namespace cgv::math;

class random_sampler : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::event_handler,  /// derive from handler to receive events and to be asked for a help string
	public cgv::gui::provider,
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
public:
	/// point type
	typedef vec<double> pnt_type;
	/// number of samples
	int n;
	/// list of samples
	std::vector<pnt_type> samples;
	/// different sampling types
	enum SamplingType { ST_UNIFORM, ST_REJECTION, ST_TRANSFORM, ST_SQUARE, ST_LAST };
	///
	enum TransformationType { TT_COORDINATES, TT_PROJECTION };
	///
	TransformationType transformation_type;
	/// selected sampling type
	SamplingType sampling_type;
	/// morphing parameter
	double lambda;
	/// transform from cylinder to carthesian coordinates
	static pnt_type car_from_cyl(const pnt_type& p)
	{
		return pnt_type(p(0)*cos(p(1)),p(0)*sin(p(1)));
	}
	/// transform from carthesian to cylinder coordinates
	static pnt_type cyl_from_car(const pnt_type& p)
	{
		return pnt_type(p.length(),atan2(p(1),p(0)));
	}
	///
	static double get_random(double min, double max)
	{
		return (max-min)*rand()/RAND_MAX+min;
	}	
	/// generate a single sample
	pnt_type generate_sample()
	{
		switch (sampling_type) {
		case ST_UNIFORM :
			return car_from_cyl(pnt_type(     get_random(0,1) , get_random(0,2*M_PI)));
		case ST_TRANSFORM :
			return car_from_cyl(pnt_type(sqrt(get_random(0,1)), get_random(0,2*M_PI)));
		case ST_REJECTION :
			{
				do {
					pnt_type p(pnt_type(get_random(-1,1),get_random(-1,1)));
					if (p.length() <= 1)
						return p;
				} while (true);
			}
		case ST_SQUARE :
			{
				pnt_type p(pnt_type(get_random(-1,1),get_random(-1,1)));
				return p;
			}
		}
		return pnt_type(0.0,0.0);
	}
	/// generate the sampling
	void generate_sampling()
	{
		samples.clear();
		for (int i=0; i<n; ++i)
			samples.push_back(generate_sample());
		post_redraw();
	}
	/// standard constructor
	random_sampler() : node("random sampler"), n(1000), sampling_type(ST_UNIFORM), lambda(0) 
	{
		transformation_type = TT_COORDINATES;
		generate_sampling();
	}
	/// return name of type
	std::string get_type_name() const 
	{ 
		return "random_sampler"; 
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &sampling_type)
			generate_sampling();
		post_redraw();
	}
	void stream_help(std::ostream& os)
	{
		os << "toggle sampling type (S)" << std::endl;
	}
	void draw(context& ctx)
	{
		glDisable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(2);
		glColor3f(0.5f,0.5f,0);
		if (sampling_type == ST_SQUARE) {
			ctx.tesselate_unit_square(); 
		}
		else
			ctx.tesselate_unit_disk(200);
		glColor3f(1,0.2f,0.0f);
		glPointSize(4);
		glEnable(GL_POINT_SMOOTH);
		glBegin(GL_POINTS);
		if (!(sampling_type == ST_UNIFORM && lambda > 0)) {
			for (unsigned int i=0; i<samples.size(); ++i) {
				glVertex2dv(&samples[i](0));
			}
		}
		else {
			if (transformation_type == TT_COORDINATES) {
				for (unsigned int i=0; i<samples.size(); ++i) {
					pnt_type c = cyl_from_car(samples[i]);
					double r = (1-lambda)*c(0)+lambda*sqrt(c(0));
					pnt_type p = car_from_cyl(pnt_type(r,c(1)));
					glVertex2dv(&p(0));
				}
			}
			else {
				for (unsigned int i=0; i<samples.size(); ++i) {
					pnt_type c = samples[i];
					c.normalize();
					pnt_type p = (1-lambda)*samples[i] + lambda*c;
					glVertex2dv(&p(0));
				}
			}
		}
		glEnd();
		glDisable(GL_POINT_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
			if (++(int&)sampling_type == ST_LAST)
				sampling_type = ST_UNIFORM;
			post_redraw();
			return true;
		}
		return false;
	}
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const 
	{
		return "example/random_sampler"; 
	}
	///
	void select_sampling(SamplingType st)
	{
		sampling_type = st;
		if (st != ST_UNIFORM) {
//			lambda = 0;
//			find_control(lambda)->redraw();
		}
		generate_sampling();
//		find_control(lambda)->set("active", st == ST_UNIFORM);
	}
	void select_state(const std::string& s)
	{
		application::get_window(0)->set("state",s);
	}
	void toggle_menu()
	{
		application::get_window(0)->set("menu",!application::get_window(0)->get<bool>("menu"));
	}
	void toggle_gui()
	{
		application::get_window(0)->set("gui",!application::get_window(0)->get<bool>("gui"));
	}
	/// you must overload this for gui creation
	void create_gui() 
	{	
		add_control("n", n, "value_slider", "min=1;max=100000;log=true");
		add_member_control(this, "sampling", sampling_type, "dropdown", "enums='uniform,rejection,transform,square'");
		/*connect_copy(add_button("uniform", "w=50", "")->click,rebind(this,&random_sampler::select_sampling, ST_UNIFORM));
		connect_copy(add_button("reject", "w=50", "")->click,rebind(this,&random_sampler::select_sampling, ST_REJECTION));
		connect_copy(add_button("transform", "w=50")->click,rebind(this,&random_sampler::select_sampling, ST_TRANSFORM));*/
		connect_copy(add_button("generate sampling","tooltip=\"my tooltip\"")->click,rebind(this,&random_sampler::generate_sampling));
		add_member_control(this, "transformation", transformation_type, "dropdown", "enums='coordinates,projection'");
		connect_copy(add_control("morph", lambda, "value_slider", "min=0;max=1")->value_change,rebind(static_cast<drawable*>(this),&drawable::post_redraw));
		connect_copy(add_button("regular", "w=50", "")->click,rebind(this,&random_sampler::select_state, std::string("regular")));
		connect_copy(add_button("minimize", "w=50", "")->click,rebind(this,&random_sampler::select_state, std::string("minimized")));
		connect_copy(add_button("maximize", "w=50", "")->click,rebind(this,&random_sampler::select_state, std::string("maximized")));
		connect_copy(add_button("fullscreen", "w=50")->click,rebind(this,&random_sampler::select_state, std::string("fullscreen")));
		connect_copy(add_button("menu", "w=50", "")->click,rebind(this,&random_sampler::toggle_menu));
		connect_copy(add_button("gui", "w=50")->click,rebind(this,&random_sampler::toggle_gui));
	}

};

#include <cgv/base/register.h>

factory_registration<random_sampler> rs_fac("New/Algorithms/Random Sampler", 'R', true);

