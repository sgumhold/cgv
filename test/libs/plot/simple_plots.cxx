#include <plot/plot1d.h>
#include <cgv/base/named.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
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

class simple_plots : public cgv::base::named, public cgv::render::drawable, public cgv::gui::provider
{
	cgv::plot::plot1d p1;
public:
	simple_plots()
	{
		set_name("simple plot example");

		p1.add_sub_plot("cos");
		sample_plot1d_interval<float>([&](float x) { return cos(x); },-3,3,100,p1.ref_sub_plot_samples(0));
		p1.set_sub_plot_colors(0, cgv::plot::plot1d::Clr(1,0,0));

		p1.add_sub_plot("sin");
		sample_plot1d_interval<float>([&](float x) { return sin(x); },-3,3,100,p1.ref_sub_plot_samples(1));
		p1.set_sub_plot_colors(1, cgv::plot::plot1d::Clr(1,1,0));
		
		p1.add_sub_plot("sinc");
		sample_plot1d_interval<float>([&](float x) { return (float)(sin(10*x)/(10*x+0.000001)); },-3,3,1000,p1.ref_sub_plot_samples(2));
		p1.set_sub_plot_colors(2, cgv::plot::plot1d::Clr(0,1,1));
		
		p1.adjust_domain_to_data();
		p1.set_extent(p1.get_domain().get_extent());
		p1.adjust_tick_marks_to_domain();
	}
	std::string get_type_name() const 
	{
		return "simple_plots";
	}
	void on_set(void* member_ptr)
	{
		post_redraw();
	}
	bool init(cgv::render::context& ctx)
	{
		return p1.init(ctx);
	}
	void draw(cgv::render::context& ctx)
	{
		glDisable(GL_LIGHTING);
		p1.draw(ctx);
		glEnable(GL_LIGHTING);
	}
	void create_gui()
	{
		p1.create_gui(this, *this);
	}
};

#include <cgv/base/register.h>

cgv::base::object_registration<simple_plots> sp_or("");