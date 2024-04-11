#include <cgv/base/node.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/application.h>
#include <cgv/render/drawable.h>
#include <cgv/render/shader_program.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/math/ftransform.h>
#include <cgv/utils/ostream_printf.h>
#include <cgv_gl/gl/gl.h>
#include <vector>
#include <random>

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
	std::vector<cgv::vec2> samples;
	int sqrt_n, m;
	enum FunctionType { FT_SIN, FT_SIN_REC, FT_GAUSS, FT_LAST };
	enum SamplingType { ST_UNIFORM, ST_STRATIFIED, ST_N_ROOK, ST_LAST };
	double I[3];
	double f;
	double sigma[3];

	//std::string ft_str, st_str;
	FunctionType function_type;
	SamplingType sampling_type;

	double eval_func(const cgv::vec2& p) const
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
		std::default_random_engine g;
		std::uniform_real_distribution<float> d(0.0f, 1.0f);
		samples.clear();
		int n = sqrt_n*sqrt_n;
		int i,j;
		float inv_sqrt_n = 1.0f/sqrt_n;
		float inv_n = 1.0f/n;
		switch (sampling_type) {
		case ST_UNIFORM :
			for (i=0; i<n; ++i)
				samples.push_back(cgv::vec2(d(g),d(g)));
			break;
		case ST_STRATIFIED :
			for (i=0; i<sqrt_n; ++i)
				for (j=0; j<sqrt_n; ++j)
					samples.push_back(cgv::vec2(inv_sqrt_n*(i+d(g)),inv_sqrt_n*(j+d(g))));
			break;
		case ST_N_ROOK :
			{
				std::vector<int> indices;
				indices.resize(n);
				for (i=0; i<n; ++i)
					indices[i] = i;
				for (i=0; i<n; ++i) {
					j = (int)((n-i)*d(g));
					if (j == n-i)
						j = n-i-1;
					samples.push_back(cgv::vec2(inv_n*(i+d(g)),inv_n*(indices[j]+d(g))));
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
		//ft_str = "sin(fr�)";
		//st_str = "uniform";
	}
	/// return name of type
	std::string get_type_name() const 
	{ 
		return "n_rook_estimator"; 
	}
	void stream_help(std::ostream& os)
	{
		os << "n_rook_estimator: toggle function (F) / sampling (S)" << std::endl;
	}
	void draw(context& ctx)
	{
		// store affected parts of opengl state
		GLboolean tmp_smooth = glIsEnabled(GL_POINT_SMOOTH);
		GLfloat tmp_width, tmp_size;
		glGetFloatv(GL_LINE_WIDTH, &tmp_width);
		glGetFloatv(GL_POINT_SIZE, &tmp_size);

		std::vector<cgv::vec2> P;

		int n = sqrt_n * sqrt_n;
		shader_program& prog = ctx.ref_default_shader_program();

		ctx.push_modelview_matrix();
			ctx.mul_modelview_matrix(translate4<double>(-1, -1, 0)*scale4<double>(2, 2, 2));

			ctx.set_cursor(cgv::dvec2(0.0, 0.0).to_vec(), "(0,0)", ta, x_offset, y_offset);
			ctx.output_stream() << "(0,0)" << std::endl;

			P.resize(4);
			P[0] = cgv::vec2(0.0f, 0.0f);
			P[1] = cgv::vec2(1.0f, 0.0f);
			P[2] = cgv::vec2(1.0f, 1.0f);
			P[3] = cgv::vec2(0.0f, 1.0f);
		
			glLineWidth(2);

			prog.enable(ctx);
			int pos_idx = prog.get_position_index();
			attribute_array_binding::enable_global_array(ctx, pos_idx);
			attribute_array_binding::set_global_attribute_array(ctx, pos_idx, P);
			ctx.set_color(cgv::rgb(0, 0.4f, 0));
			glDrawArrays(GL_LINE_LOOP, 0, 4);
		
			if (sampling_type == ST_STRATIFIED) {
				P.clear();
				float inv_sqrt_n = 1.0f / sqrt_n;
				for (int i = 1; i < sqrt_n; ++i) {
					P.push_back(cgv::vec2(0.0f, i*inv_sqrt_n));
					P.push_back(cgv::vec2(1.0f, i*inv_sqrt_n));
					P.push_back(cgv::vec2(i*inv_sqrt_n, 0));
					P.push_back(cgv::vec2(i*inv_sqrt_n, 1));
				}
				glLineWidth(1);

				attribute_array_binding::set_global_attribute_array(ctx, pos_idx, P);
				ctx.set_color(cgv::rgb(0, 0.6f, 0));
				glDrawArrays(GL_LINES, 0, (GLsizei)P.size());
			}

			attribute_array_binding::set_global_attribute_array(ctx, pos_idx, samples);
			ctx.set_color(cgv::rgb(0.4f, 0.1f, 0));
			glPointSize(2);
			glEnable(GL_POINT_SMOOTH);
			glDrawArrays(GL_POINTS, 0, (GLsizei)samples.size());

			if (sampling_type == ST_N_ROOK) {
				float inv_n = 1.0f/n;
				P.clear();
				for (int i=0; i<n; ++i) {
					cgv::vec2 c(inv_n*(int)(samples[i](0)*n), inv_n*(int)(samples[i](1)*n));
					P.push_back(c);
					P.push_back(cgv::vec2(c(0)+inv_n,c(1)));
					P.push_back(cgv::vec2(c(0), c(1) + inv_n));

					P.push_back(cgv::vec2(c(0) + inv_n, c(1)));
					P.push_back(cgv::vec2(c(0)+inv_n,c(1)+inv_n));
					P.push_back(cgv::vec2(c(0), c(1) + inv_n));
				}
				attribute_array_binding::set_global_attribute_array(ctx, pos_idx, P);
				ctx.set_color(cgv::rgb(0.3f, 0.7f, 1));
				glDrawArrays(GL_TRIANGLES, 0, (GLsizei)P.size());
			}
			prog.disable(ctx);
			attribute_array_binding::disable_global_array(ctx, pos_idx);
		ctx.pop_modelview_matrix();

		// recover opengl state
		if (!tmp_smooth)
			glDisable(GL_POINT_SMOOTH);
		glPointSize(tmp_size);
		glLineWidth(tmp_width);
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
			on_set(&sampling_type);
			return true;
		case 'F' : 
			if (++(int&)function_type == FT_LAST)
				function_type = FT_SIN;
			on_set(&function_type);
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
	void on_set(void* member_ptr)
	{
		if ( (member_ptr == &sampling_type) ||
			 (member_ptr == &sqrt_n) )
			generate_sampling();
		update_member(member_ptr);
		post_redraw();
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
		add_member_control(this, "sqrt(n)", sqrt_n, "value_slider", "min=1;max=1000;log=true;ticks=true");
		add_member_control(this, "sampling type", sampling_type, "dropdown", "enums='uniform, stratified, n_rook'");
		if (begin_tree_node("analyze", function_type)) {
			align("\a");
			add_member_control(this, "f", function_type, "dropdown", "enums='sin(fr�),sin(f/r�),exp(-4fr�)'");
			add_control("m", m, "value_slider", "min=1;max=1000;log=true;ticks=true");
			connect_copy(add_button("analyze")->click, rebind(this, &n_rook_estimator::analyze));
			add_view("I_uniform", I[0], "", "w=62;align='B'", " ");
			add_view("I_strat",   I[1], "", "w=61;align='B'", " ");
			add_view("I_n-rook",  I[2], "", "w=61;align='B'");
			add_view("s_uniform", sigma[0], "", "w=62;align='B'", " ");
			add_view("s_strat",   sigma[1], "", "w=61;align='B'", " ");
			add_view("s_n-rook",  sigma[2], "", "w=61;align='B'");
			align("\b");
		}
		if (begin_tree_node("text label", ta)) {
			align("\a");
			add_member_control(this, "alignment", ta, "dropdown", "enums='none=0,left=1,right=2,top=4,bottom=8,top_left=5,top_right=6,bottom_left=9,bottom_right=10'");
			add_member_control(this, "x offset", x_offset, "value_slider", "min=-20;max=20;step=1");
			add_member_control(this, "y offset", y_offset, "value_slider", "min=-20;max=20;step=1");
			align("\b");
		}
	}

};

#include <cgv/base/register.h>

factory_registration<n_rook_estimator> nr_fac("New/Algorithms/N Rook Estimator", 'Q', true);

