#include <plot/plot1d.h>
#include <cgv/base/node.h>
#include <cgv/gui/provider.h>
#include <cgv/utils/file.h>
#include <cgv/render/drawable.h>
#include <cgv/signal/rebind.h>
#include <libs/cgv_gl/gl/gl.h>

template <typename T>
void sample_plot1d_interval(std::function<T(T)> f, T xmin, T xmax, unsigned nr_x, std::vector<cgv::math::fvec<T,2> >& pnts) 
{
	pnts.clear();
	pnts.reserve(nr_x);

	for (unsigned i=0; i<nr_x; ++i) {
		T cx = xmin + (xmax-xmin)*i/(nr_x-1);
		pnts.push_back(cgv::math::fvec<T,2>(cx,f(cx)));
	}
}

template <typename T>
void sample_plot2d_rectangle(std::function<T(T,T)> f, T xmin, T xmax, T ymin, T ymax, unsigned nr_x, unsigned nr_y, std::vector<cgv::math::fvec<T,3> >& pnts) 
{
	pnts.clear();
	pnts.reserve(nr_x*nr_y);

	for (unsigned j=0; j<nr_y; ++j) {
		T cy = ymin + (ymax-ymin)*j/(nr_y-1);
		for (unsigned i=0; i<nr_x; ++i) {
			T cx = xmin + (xmax-xmin)*i/(nr_x-1);
			pnts.push_back(cgv::math::fvec<T,3>(cx,cy,f(cx,cy)));
		}
	}
}

void configure_simple_plots(cgv::plot::plot1d& p1)
{
	p1.add_sub_plot("cos");
	sample_plot1d_interval<float>([&](float x) { return cos(x); },-3,3,100,p1.ref_sub_plot_samples(0));
	p1.set_sub_plot_colors(0, cgv::plot::plot1d::Clr(1,0,0));

	p1.add_sub_plot("sin");
	sample_plot1d_interval<float>([&](float x) { return sin(x); },-3,3,100,p1.ref_sub_plot_samples(1));
	p1.set_sub_plot_colors(1, cgv::plot::plot1d::Clr(1,1,0));
		
	p1.add_sub_plot("sinc");
	sample_plot1d_interval<float>([&](float x) { return (float)(sin(10*x)/(10*x+0.000001)); },-3,3,1000,p1.ref_sub_plot_samples(2));
	p1.set_sub_plot_colors(2, cgv::plot::plot1d::Clr(0,1,1));
}


#include <random>

template <typename T, typename R>
void monte_carlo_triangle(std::vector<cgv::math::fvec<R,2> >& mean_pnts, std::vector<cgv::math::fvec<R,2> >& error_pnts, std::vector<cgv::math::fvec<R,2> >& reference_pnts, unsigned K, unsigned D, unsigned N, bool use_nml_dist)
{
//	std::mt19937_64 generator;
	std::default_random_engine generator;
	std::uniform_real_distribution<T> uni_dist(T(-1),T(1));
	std::normal_distribution<T> nml_dist;
	std::vector<T> values;
	values.resize(K);
	for (unsigned n=1; n<=N; ++n) {
		unsigned k;
		T mean = 0;
		for (k=0; k<K; ++k) {
			T F = 0;
			for (unsigned i=0; i<n; ++i) {
				T f = 1;
				for (unsigned d = 0; d < D; ++d) {
					T x, p;
					if (use_nml_dist) {
						do {
							x = nml_dist(generator);
						} while (x < T(-1) || x > T(1));
						p = 0.3989422802*exp(-0.5*x*x)/0.6826894920;
					}
					else {
						x = uni_dist(generator);
						p = T(0.5);
					}
					f *= 0.6695016643*exp(-x*x)/p;
				}
				F += f;
			}
			F /= n;
			values[k] = F;
			mean += F;
		}
		mean /= K;
		T V = 0;
		for (k=0; k<K; ++k) {
			T diff = values[k]-mean;
			V += diff*diff;
		}
		V /= (K-1);
		mean_pnts.push_back( cgv::math::fvec<R,2>(R(n),R(mean)) );
		error_pnts.push_back( cgv::math::fvec<R,2>(R(n),R(sqrt(V))) );
//		error_pnts.push_back( cgv::math::fvec<R,2>(R(n),R(sqrt(V*n))) );
		reference_pnts.push_back( cgv::math::fvec<R,2>(R(n),R(1/sqrt(n))) );
	}
}

void configure_plots_montecarlo(cgv::plot::plot1d& p1)
{
	p1.add_sub_plot("mean_uni");
	p1.set_sub_plot_colors(0, cgv::plot::plot1d::Clr(1,0,0));

	p1.add_sub_plot("error_uni");
	p1.set_sub_plot_colors(1, cgv::plot::plot1d::Clr(0,1,0));

	p1.add_sub_plot("mean_nml");
	p1.set_sub_plot_colors(2, cgv::plot::plot1d::Clr(1,1,0));

	p1.add_sub_plot("error_nml");
	p1.set_sub_plot_colors(3, cgv::plot::plot1d::Clr(0,1,1));

	p1.add_sub_plot("reference");
	p1.set_sub_plot_colors(4, cgv::plot::plot1d::Clr(0,0,1));

	monte_carlo_triangle<double>(p1.ref_sub_plot_samples(0), p1.ref_sub_plot_samples(1), p1.ref_sub_plot_samples(4), 1000, 30, 100, false);
	p1.ref_sub_plot_samples(4).clear();
	monte_carlo_triangle<double>(p1.ref_sub_plot_samples(2), p1.ref_sub_plot_samples(3), p1.ref_sub_plot_samples(4), 1000, 30, 100, true);
}

class integrator_analizer : public cgv::plot::plot_types
{
	P2D y0;
	Crd h, t, zeta;
	bool plot_over_time;
public:
	integrator_analizer()
	{
		y0   = P2D(1,0);
		h    = 0.1f;
		t    = 6.283185308f;
		zeta = 0.0f;
		plot_over_time = true;
	}

	void create_plots(cgv::plot::plot1d& p1)
	{
		p1.add_sub_plot("explicit");
		p1.set_sub_plot_colors(0, cgv::plot::plot1d::Clr(1,0,0));

		p1.add_sub_plot("implicit");
		p1.set_sub_plot_colors(1, cgv::plot::plot1d::Clr(0,0,1));

		p1.add_sub_plot("semi-implicit");
		p1.set_sub_plot_colors(2, cgv::plot::plot1d::Clr(1,0,1));

		for (int s=0; s<3; ++s) {
			p1.ref_sub_plot1d_config(s).show_points = true;
			p1.ref_sub_plot1d_config(s).point_size = 4;
			p1.ref_sub_plot1d_config(s).line_width = 2;
		}

	}

	void compute_plots(cgv::plot::plot1d& p1)
	{
		unsigned i, n=(unsigned)floor(t/h);
		for (int k=0; k<3; ++k) {
			p1.ref_sub_plot_samples(k).resize(n);
			p1.ref_sub_plot_samples(k)[0] = y0;
		}
		for (i=1; i<n; ++i) {
			// explicit Euler
			{
				const P2D& pi = p1.ref_sub_plot_samples(0)[i-1];
				      P2D& pj = p1.ref_sub_plot_samples(0)[i];

				pj(0) = pi(0) + h*pi(1);
				pj(1) = pi(1) - h*(2*zeta*pi(1)+pi(0));
			}
			// implicit Euler
			{
				const P2D& pi = p1.ref_sub_plot_samples(1)[i-1];
				      P2D& pj = p1.ref_sub_plot_samples(1)[i];

				Crd c = 1/(1+2*h*(zeta+h));
				pj(0) = c*(pi(0)*(1 + 2*h*zeta)+h*pi(1));
				pj(1) = c*(pi(1)-h*pi(0));
			}
			// semi-implicit Euler
			{
				const P2D& pi = p1.ref_sub_plot_samples(2)[i-1];
				      P2D& pj = p1.ref_sub_plot_samples(2)[i];

				pj(1) = pi(1) - h*(2*zeta*pi(1)+pi(0));
				pj(0) = pi(0) + h*pj(1);
			}
		}
		if (plot_over_time) {
			for (int k=0; k<3; ++k) {
				for (i=0; i<n; ++i) {
					P2D& pi = p1.ref_sub_plot_samples(k)[i];
					pi(1) = pi(0);
					pi(0) = i*h;
				}
			}
		}
	}

	void create_gui(cgv::base::base* b, cgv::gui::provider& p)
	{
		p.add_member_control(b, "plot_over_time",    plot_over_time,    "check");
		p.add_member_control(b, "h",    h,    "value_slider", "min=0.00001;max=1;step=0.000001;log=true;ticks=true");
		p.add_member_control(b, "zeta", zeta, "value_slider", "min=0;max=100;step=0.00001;log=true;ticks=true");
		p.add_member_control(b, "t",    t,    "value_slider", "min=1;max=100;ticks=true");
	}
};

class simple_plots : public cgv::base::node, public cgv::render::drawable, public cgv::gui::provider
{
	cgv::plot::plot1d p1;

	integrator_analizer ia;
public:
	simple_plots()
	{
		set_name("simple plot example");

		// configure_simple_plots(p1);
		// configure_plots_montecarlo(p1);
		ia.create_plots(p1);
		ia.compute_plots(p1);

		p1.adjust_domain_to_data();
		p1.set_extent(p1.get_domain().get_extent());
		//p1.set_width(1, false);
		//p1.set_height(1, false);
		p1.adjust_tick_marks_to_domain();
		p1.set_label_font(16, cgv::media::font::FFA_REGULAR, "Tahoma");
	}
	std::string get_type_name() const 
	{
		return "simple_plots";
	}
	void on_adjust_domain()
	{
		p1.adjust_domain_to_data();
		p1.set_extent(p1.get_domain().get_extent());
		post_redraw();
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr >= &ia && member_ptr < &ia+1)
			ia.compute_plots(p1);
		post_redraw();
	}
	void on_register()
	{
		if (cgv::render::get_shader_config()->shader_path.empty()) {
			std::string prog_name = cgv::base::ref_prog_name();
			std::string prog_path = cgv::utils::file::get_path(prog_name);
			cgv::render::get_shader_config()->shader_path = prog_path+"/glsl";
		}
	}
	bool init(cgv::render::context& ctx)
	{
		ctx.set_bg_clr_idx(4);
		get_root()->set("show_stats", false);
		return p1.init(ctx);
	}
	void init_frame(cgv::render::context& ctx)
	{
		static bool my_tab_selected = false;
		if (!my_tab_selected) {
			my_tab_selected = true;
			cgv::gui::gui_group_ptr gg = ((provider*)this)->get_parent_group();
			if (gg) {
				cgv::gui::gui_group_ptr tab_group = gg->get_parent()->cast<cgv::gui::gui_group>();
				if (tab_group) {
					cgv::base::base_ptr c = gg;
					tab_group->select_child(c, true);
				}
			}
		}
	}
	void draw(cgv::render::context& ctx)
	{
		glDisable(GL_LIGHTING);
		p1.draw(ctx);
		glEnable(GL_LIGHTING);
	}
	void create_gui()
	{
		connect_copy(add_button("adjust domain")->click, cgv::signal::rebind(this, &simple_plots::on_adjust_domain));
		ia.create_gui(this, *this);
		p1.create_gui(this, *this);

	//	cgv::gui::gui_group_ptr gg = get_parent_group();
		//gg->get_parent()->cast<cgv::gui::gui_group>()->select_child(cgv::base::base_ptr(gg),true);
	}
};

#include <cgv/base/register.h>

cgv::base::object_registration<simple_plots> sp_or("");