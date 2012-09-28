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
using namespace cgv::signal;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::utils;
using namespace cgv::math;

class n_rook_estimator : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::event_handler,  /// derive from handler to receive events and to be asked for a help string
	public cgv::gui::provider,
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
public:
	int x_offset, y_offset;
	TextAlignment ta;
	///
	static double get_random(double min, double max)
	{
		return (max-min)*rand()/RAND_MAX+min;
	}	
	/// point type
	typedef vec<double> pnt_type;
	std::vector<pnt_type> samples;
	int sqrt_n, m;
	enum FunctionType { FT_SIN, FT_SIN_REC, FT_GAUSS, FT_LAST };
	enum SamplingType { ST_UNIFORM, ST_STRATIFIED, ST_N_ROOK, ST_LAST };
	double I[3];
	double f;
	double sigma[3];
	std::string ft_str, st_str;
	FunctionType function_type;
	SamplingType sampling_type;

	double eval_func(const pnt_type& p) const
	{
		switch (function_type) {
		case FT_SIN :
			return sin(f*p.sqr_length());
		case FT_SIN_REC :
			return sin(f/p.sqr_length());
		case FT_GAUSS :
			return exp(-4*f*p.sqr_length());
		default:
			return 1;
		}
		return 1;
	}

	void generate_sampling()
	{
		samples.clear();
		int n = sqrt_n*sqrt_n;
		int i,j;
		double inv_sqrt_n = 1.0/sqrt_n;
		double inv_n = 1.0/n;
		switch (sampling_type) {
		case ST_UNIFORM :
			for (i=0; i<n; ++i)
				samples.push_back(pnt_type(get_random(0,1),get_random(0,1)));
			break;
		case ST_STRATIFIED :
			for (i=0; i<sqrt_n; ++i)
				for (j=0; j<sqrt_n; ++j)
					samples.push_back(pnt_type(inv_sqrt_n*(i+get_random(0,1)),inv_sqrt_n*(j+get_random(0,1))));
			break;
		case ST_N_ROOK :
			{
				std::vector<int> indices;
				indices.resize(n);
				for (i=0; i<n; ++i)
					indices[i] = i;
				for (i=0; i<n; ++i) {
					j = (int)get_random(0,n-i);
					if (j == n-i)
						j = n-i-1;
					samples.push_back(pnt_type(inv_n*(i+get_random(0,1)),inv_n*(indices[j]+get_random(0,1))));
					std::swap(indices[j],indices[n-i-1]);
				}
			}
			break;
		default:
			break;
		}
		post_redraw();
	}
	/// standard constructor
	n_rook_estimator() : node("n-rook estimator"), sqrt_n(5), m(50), sampling_type(ST_UNIFORM)
	{
		x_offset = y_offset = 0;
		ta = TA_NONE;
		I[0] = I[1] = I[2] = 0;
		sigma[0] = sigma[1] = sigma[2] = 0;
		f = 1;
		function_type = FT_SIN;
		generate_sampling();
		ft_str = "sin(fr²)";
		st_str = "uniform";
	}
	/// return name of type
	std::string get_type_name() const 
	{ 
		return "n_rook_estimator"; 
	}
	void stream_help(std::ostream& os)
	{
		os << "toggle function (F) / sampling (S)" << std::endl;
	}
	void draw(context& ctx)
	{
		glPushMatrix();
		glTranslated(-1,-1,0);
		glScaled(2,2,2);
		int n = sqrt_n*sqrt_n;
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(2);
		glColor3f(1,1,0);
		glBegin(GL_LINE_LOOP);
		glVertex2d(0,0);
		glVertex2d(1,0);
		glVertex2d(1,1);
		glVertex2d(0,1);
		glEnd();
		ctx.set_cursor(vec<double>(0.0,0.0),"(0,0)",ta,x_offset,y_offset);
		ctx.output_stream() << "(0,0)" << std::endl;
		glLineWidth(1);
		glColor3f(1,1,1);
		if (sampling_type == ST_STRATIFIED) {
			double inv_sqrt_n = 1.0/sqrt_n;
			for (int i=1; i<sqrt_n; ++i) {
				glBegin(GL_LINES);
				glVertex2d(0,i*inv_sqrt_n);
				glVertex2d(1,i*inv_sqrt_n);
				glVertex2d(i*inv_sqrt_n,0);
				glVertex2d(i*inv_sqrt_n,1);
				glEnd();
			}
		}
		glColor3f(1,1,1);
		glPointSize(2);
		glEnable(GL_POINT_SMOOTH);
		glBegin(GL_POINTS);
		for (unsigned int i=0; i<samples.size(); ++i)
			glVertex2dv(&samples[i](0));
		glEnd();
		glDisable(GL_POINT_SMOOTH);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (sampling_type == ST_N_ROOK) {
			double inv_n = 1.0/n;
			glColor3f(1,0.2f,0.2f);
			glBegin(GL_QUADS);
			for (int i=0; i<n; ++i) {
				pnt_type c(inv_n*(int)(samples[i](0)*n), inv_n*(int)(samples[i](1)*n));
				glVertex2d(c(0),c(1));
				glVertex2d(c(0)+inv_n,c(1));
				glVertex2d(c(0)+inv_n,c(1)+inv_n);
				glVertex2d(c(0),c(1)+inv_n);
			}
			glEnd();
		}
		glPopMatrix();
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
		case 'F' : 
			if (++(int&)function_type == FT_LAST)
				function_type = FT_SIN;
			post_redraw();
			return true;
		}
		return false;
	}
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const 
	{
		return "example/n_rook_estimator"; 
	}
	///
	void select_sampling(SamplingType st)
	{
		sampling_type = st;
		switch (st) {
		case ST_UNIFORM : st_str = "uniform"; break;
		case ST_STRATIFIED : st_str = "stratified"; break;
		case ST_N_ROOK : st_str = "n-rook"; break;
		default: break;
		}
		find_view(st_str)->update();
		
		generate_sampling();
	}
	void select_function(FunctionType ft)
	{
		function_type = ft;
		switch (ft) {
		case FT_SIN : ft_str = "sin(fr²)"; break;
		case FT_SIN_REC : ft_str = "sin(f/r²)"; break;
		case FT_GAUSS : ft_str = "exp(-4fr²)"; break;
		default: break;
		}
		find_view(ft_str)->update();
	}
	void select_sqrt_n()
	{
		generate_sampling();
	}
	void analyze_estimator()
	{
		double sum = 0;
		double sqr_sum = 0;
		int n = sqrt_n*sqrt_n;
		for (int i=0; i<m; ++i) {
			generate_sampling();
			double F = 0;
			for (int j=0; j<n; ++j)
				F += eval_func(samples[j]);
			F /= n;

			sum += F;
			sqr_sum += F*F;
		}
		sum /= m;
		sqr_sum /= m;
		I[sampling_type] = sum;
		sigma[sampling_type] = sqrt(sqr_sum - sum*sum);
	}
	void analyze()
	{
		SamplingType tmp = sampling_type;
		sampling_type = ST_UNIFORM;
		analyze_estimator();
		sampling_type = ST_STRATIFIED;
		analyze_estimator();
		sampling_type = ST_N_ROOK;
		analyze_estimator();
		sampling_type = tmp;
		for (int i=0; i<3; ++i) {
			find_view(I[i])->update();
			find_view(sigma[i])->update();
		}
	}
	/// you must overload this for gui creation
	void create_gui() 
	{	
		connect_copy(add_control("sqrt(n)", sqrt_n, "value_slider", "min=1;max=1000;log=true;ticks=true")->value_change,
			rebind(this, &n_rook_estimator::select_sqrt_n));

		add_view("", st_str, "", "w=50", "");
		connect_copy(add_button("uniform", "w=70", "")->click,rebind(this,&n_rook_estimator::select_sampling, ST_UNIFORM));
		connect_copy(add_button("stratified", "w=70", "")->click,rebind(this,&n_rook_estimator::select_sampling, ST_STRATIFIED));
		connect_copy(add_button("n-rook", "w=70")->click,rebind(this,&n_rook_estimator::select_sampling, ST_N_ROOK));

		add_control("f", f, "value_slider", "min=0;max=100;log=true;ticks=true");

		add_view("", ft_str, "", "w=50", "");
		connect_copy(add_button("sin(fr²)",   "w=70", "")->click,rebind(this,&n_rook_estimator::select_function, FT_SIN));
		connect_copy(add_button("sin(f/r²)", "w=70", "")->click,rebind(this,&n_rook_estimator::select_function, FT_SIN_REC));
		connect_copy(add_button("exp(-4fr²)", "w=70")->click,rebind(this,&n_rook_estimator::select_function, FT_GAUSS));

		add_view("I_uniform", I[0], "", "w=70", "");
		add_view("I_strat", I[1], "", "w=70", "");
		add_view("I_n-rook", I[2], "", "w=70");
		add_view("s_uniform", sigma[0], "", "w=70", "");
		add_view("s_strat", sigma[1], "", "w=70", "");
		add_view("s_n-rook", sigma[2], "", "w=70");
		add_control("m", m, "value_slider", "min=1;max=1000;log=true;ticks=true");
		connect_copy(add_button("analyze")->click, rebind(this, &n_rook_estimator::analyze));

		add_control("x_offset", x_offset, "value_slider", "min=-20;max=20;step=1");
		add_control("y_offset", y_offset, "value_slider", "min=-20;max=20;step=1");
		add_control("alignment", ta, "TA_NONE=0,TA_LEFT=1,TA_RIGHT=2,TA_TOP=4,TA_BOTTOM=8,TA_TOP_LEFT=5,TA_TOP_RIGHT=6,TA_BOTTOM_LEFT=9,TA_BOTTOM_RIGHT=10");
		
		connect_copy(find_control(x_offset)->value_change,rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(find_control(y_offset)->value_change,rebind(static_cast<drawable*>(this), &drawable::post_redraw));
		connect_copy(find_control(ta)->value_change,rebind(static_cast<drawable*>(this), &drawable::post_redraw));
	}

};

#include <cgv/base/register.h>

extern factory_registration<n_rook_estimator> nr_fac("new/n_rook_estimator", 'Q', true);

