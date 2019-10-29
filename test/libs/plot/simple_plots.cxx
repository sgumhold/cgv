#include <plot/plot2d.h>
#include <cgv/base/node.h>
#include <cgv/media/color_scale.h>
#include <cgv/gui/provider.h>
#include <cgv/utils/file.h>
#include <cgv/render/drawable.h>
#include <cgv/signal/rebind.h>
#include <libs/cgv_gl/gl/gl.h>
#include <cgv/utils/file.h>
#include <cgv/utils/dir.h>
#include <cgv/utils/scan.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/utils/advanced_scan.h>
#include <cgv/base/register.h>

using namespace cgv::base;
using namespace cgv::reflect;
using namespace cgv::render;
using namespace cgv::gui;
using namespace cgv::media;
using namespace cgv::utils;
using namespace cgv::utils::file;
using namespace std;

void scan_dir(const std::string& path, const std::string& dir, vector<string>& dirs, vector<string>& files)
{
	string p = path + "/";
	if (!dir.empty())
		p += dir + "/";
	p += "*";
	void* h = find_first(p);
	while (h) {
		string p = dir;
		if (!p.empty())
			p += "/";
		p = find_name(h);
		if (find_directory(h)) {
			if (find_name(h) != ".." && find_name(h) != ".")
				dirs.push_back(p);
		}
		else
			files.push_back(p);
		h = find_next(h);
	}
}
string split_path(const string& path)
{
	string p(path);
	replace(p, '\\', '/');
	return p;
}

enum BrowserSelectionMode
{
	BSM_FILE,
	BSM_DIRECTORY,
	BSM_TREE
};

struct dataset
{
	std::string title;
	std::vector<std::string> headers;
	std::vector<std::vector<double> > data;
};

class browser_test : public node, public cgv::render::drawable, public cgv::gui::event_handler, public  cgv::gui::provider, public argument_handler
{
protected:
	unsigned nr_sub_plots;
	BrowserSelectionMode selection_mode;
	int selected_dataset;
	bool auto_x, auto_y;
	bool adjust_min;
	std::vector<std::string> headers;
	std::string file_path;
	bool incremental_build;
	cgv::gui::gui_group_ptr gp;
	std::map<void*, std::string> file_path_map;
	std::vector<rgb> sub_plot_colors;
	cgv::plot::plot2d p1;

	std::string edx, edy;
	std::vector<int> sub_plot_x_columns;
	std::vector<int> sub_plot_y_columns;
	std::vector<dataset*> datasets;

	bool is_directory(const std::string& file_name)
	{
		void* handle = cgv::utils::file::find_first(file_name);
		if (!handle)
			return false;
		return find_directory(handle);
	}
	bool consider_path_or_file(const std::string& name)
	{
		if (cgv::utils::file::exists(name)) {
			file_path = cgv::utils::file::get_path(name);
			post_recreate_gui();
			return true;
		}
		if (cgv::utils::dir::exists(name)) {
			file_path = name;
			post_recreate_gui();
			return true;
		}
		return false;
	}
	void set_sub_plot_colors()
	{
		sub_plot_colors.resize(nr_sub_plots);
		for (unsigned i = 0; i < nr_sub_plots; ++i) {
			double v = double(i) / (nr_sub_plots + 1);
			sub_plot_colors[i] = cgv::media::color_scale(v, cgv::media::CS_HUE);
			on_set(&sub_plot_colors[i]);
		}
	}
public:
	browser_test()
	{
		edx = "index=-1";
		edy = "empty=-1";
		nr_sub_plots = 6;
		for (unsigned i = 0; i < nr_sub_plots; ++i)
			p1.add_sub_plot(std::string("plot ")+cgv::utils::to_string(i+1));
		sub_plot_x_columns.resize(nr_sub_plots, -1);
		sub_plot_y_columns.resize(nr_sub_plots, -1);
		set_sub_plot_colors();
		adjust_min = true;
		incremental_build = true;
		selected_dataset = -1;
		file_path = "D:/research/papers/GP/work in progress/registration/GraphData";
		headers.resize(nr_sub_plots);
		auto_x = auto_y = true;
		selection_mode = BSM_DIRECTORY;
		p1.set_extent(vecn(3.0f, 2.0f));
		set_name("plot_explorer");
	}
	/// overload and implement this method to handle events
	bool handle(event& e)
	{
		if (e.get_kind() != cgv::gui::EID_MOUSE)
			return false;
		if (e.get_flags() != cgv::gui::EF_DND)
			return false;
		
		cgv::gui::mouse_event& me = reinterpret_cast<cgv::gui::mouse_event&>(e);
		switch (me.get_action()) {
		case cgv::gui::MA_DRAG:
		case cgv::gui::MA_MOVE:
		case cgv::gui::MA_LEAVE:
		case cgv::gui::MA_ENTER:
			return true;
		case cgv::gui::MA_RELEASE:
			consider_path_or_file(me.get_dnd_text());
			return true;
		}
		return false;
	}
	/// overload to stream help information to the given output stream
	void stream_help(std::ostream& os)
	{
		os << "drag and drop file or directory";
	}

	void handle_args(std::vector<std::string>& args)
	{
		consider_path_or_file(args[0]);
	}

	void adjust_axes()
	{
		if (auto_x) {
			p1.adjust_domain_axis_to_data(0, adjust_min, true);
			if (!adjust_min) {
				box2 D = p1.get_domain();
				D.ref_min_pnt()(0) = 0;
				p1.set_domain(D);
			}
		}
		if (auto_y) {
			p1.adjust_domain_axis_to_data(1, adjust_min, true);
			if (!adjust_min) {
				box2 D = p1.get_domain();
				D.ref_min_pnt()(1) = 0;
				p1.set_domain(D);
			}
		}
		p1.adjust_tick_marks_to_domain();
	}

	void construct_plot(dataset* dp)
	{
		for (unsigned i = 0; i < nr_sub_plots; ++i) {
			p1.ref_sub_plot_samples(i).clear();
			p1.ref_sub_plot_strips(i).clear();
		
			int cx = sub_plot_x_columns[i];
			int cy = sub_plot_y_columns[i];
			if (cy == -1)
				continue;
			
			if (cy >= int(dp->data.size()))
				continue;

			bool has_x = false;
			if (cx > -1 && cx < int(dp->data.size()) && dp->data[cy].size() <= dp->data[cx].size())
				has_x = true;

			p1.ref_sub_plot_samples(i).resize(dp->data[cy].size());
			for (size_t j = 0; j < dp->data[cy].size(); ++j)
				p1.ref_sub_plot_samples(i)[j] = vec2(has_x? float(dp->data[cx][j]):float(j + 1), float(dp->data[cy][j]));
		}
		adjust_axes();
		post_redraw();
	}
	void construct_multi_plot(const std::vector<dataset*> dps)
	{
		for (unsigned i = 0; i < nr_sub_plots; ++i) {
			p1.ref_sub_plot_samples(i).clear();
			p1.ref_sub_plot_strips(i).clear();

			int cx = sub_plot_x_columns[i];
			int cy = sub_plot_y_columns[i];
			if (cy == -1)
				continue;

			size_t fst = 0;
			p1.ref_sub_plot_strips(i).resize(dps.size());
			for (size_t k = 0; k < dps.size(); ++k) {

				if (cy >= int(dps[k]->data.size()))
					continue;

				bool has_x = false;
				if (cx > -1 && cx < int(dps[k]->data.size()) && dps[k]->data[cy].size() <= dps[k]->data[cx].size())
					has_x = true;

				p1.ref_sub_plot_strips(i)[k] = dps[k]->data[cy].size();

				p1.ref_sub_plot_samples(i).resize(fst + dps[k]->data[cy].size());
				for (size_t j = 0; j < dps[k]->data[cy].size(); ++j)
					p1.ref_sub_plot_samples(i)[fst + j] = vec2(has_x ? float(dps[k]->data[cx][j]) : float(j + 1), float(dps[k]->data[cy][j]));

				fst += dps[k]->data[cy].size();

			}
		}
		adjust_axes();
		post_redraw();
	}
	std::string compute_column_enum_def(dataset* dp)
	{
		std::string res;
		for (const auto& h : dp->headers) {
			res += res.empty() ? "" : ",";
			res += h;
		}
		return res;
	}
	void update_gui(dataset* dp)
	{
		//
		std::string ed = compute_column_enum_def(dp);
		edx = std::string("index=-1,") + ed;
		edy = std::string("empty=-1,") + ed;
		for (unsigned i = 0; i < nr_sub_plots; ++i) {
			if (find_control(sub_plot_x_columns[i]))
				find_control(sub_plot_x_columns[i])->set("enums", edx);
			if (find_control(sub_plot_y_columns[i]))
				find_control(sub_plot_y_columns[i])->set("enums", edy);
		}
		if (find_control(selected_dataset))
			find_control(selected_dataset)->set("max", datasets.size() == 0 ? 0 : datasets.size() - 1);

	}


	dataset* read_file(const std::string& path, bool debug = false)
	{
		if (debug)
			std::cout << "reading " << path << std::endl;
		std::string content;
		if (!cgv::utils::file::read(path, content, true))
			return 0;
		std::vector<line> lines;
		split_to_lines(content, lines);

		// find first data line
		size_t i0;
		size_t nr_cols = 0;
		for (i0 = 0; i0 < lines.size(); ++i0) {
			std::vector<token> tokens;
			split_to_tokens(lines[i0].begin, lines[i0].end, tokens, "");
			if (tokens.size() > 0) {
				bool failed = false;
				for (size_t j = 0; j < tokens.size(); ++j) {
					double v;
					if (!is_double(tokens[j].begin, tokens[j].end, v)) {
						failed = true;
						break;
					}
				}
				if (!failed) {
					nr_cols = tokens.size();
					break;
				}
			}
		}
		if (nr_cols == 0)
			return false;

		dataset* dp = new dataset;
		std::vector<token> tokens;
		if (i0 > 1)
			dp->title = to_string(lines[0]);
		else
			dp->title = path;

		if (i0 > 0)
			split_to_tokens(lines[i0-1].begin, lines[i0 - 1].end, tokens, "");
		
		dp->headers.resize(nr_cols);
		size_t c;
		for (c = 0; c < nr_cols; ++c) {
			if (c >= tokens.size()) {
				dp->headers[c] = "column ";
				dp->headers[c] += to_string(c + 1);
			}
			else {
				dp->headers[c] = to_string(tokens[c]);
			}
			if (debug)
				std::cout << "name of column " << c + 1 << ": " << dp->headers[c] << std::endl;
		}
		
		size_t nr = 0;
		std::vector<double> v;
		v.resize(nr_cols);
		dp->data.clear();
		dp->data.resize(nr_cols);
		for (size_t i = i0; i < lines.size(); ++i) {
			tokens.clear();
			split_to_tokens(lines[i].begin, lines[i].end, tokens, "");
			if (tokens.size() != nr_cols)
				continue;
			size_t c, nr_success = 0;
			for (c = 0; c < nr_cols; ++c) {
				if (is_double(tokens[c].begin, tokens[c].end, v[c])) {
					++nr_success;
				}
			}
			if (nr_success < nr_cols)
				continue;
			++nr;
			for (c = 0; c < nr_cols; ++c) {
				dp->data[c].resize(nr);
				dp->data[c][nr - 1] = v[c];
			}
		}
		if (debug)
			std::cout << "extracted " << nr_cols << " x " << nr << " data values" << std::endl;
		return dp;
	}

	unsigned read_directory(const std::string& path)
	{
		vector<string> dirs, files;
		std::cout << "scanning " << path << std::endl;
		scan_dir(path, "", dirs, files);
		unsigned cnt = 0;
		for (unsigned i = 0; i < files.size(); ++i) {
			dataset* dp = read_file(path + "/" + files[i]);
			if (dp) {
				if (cnt == 0)
					clear_datasets();
				++cnt;
				datasets.push_back(dp);
			}
		}
		return cnt;
	}
	std::string get_type_name() const
	{
		return "browser_test";
	}
	bool self_reflect(reflection_handler& rh)
	{
		return rh.reflect_member("file_path", file_path);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &file_path)
			recreate_browser_gui();
		if (member_ptr == &adjust_min || 
			member_ptr == &p1.get_domain_config_ptr()->axis_configs[0].log_scale || 
			member_ptr == &p1.get_domain_config_ptr()->axis_configs[1].log_scale) {
			adjust_axes();
		}

		if (member_ptr == &selected_dataset) {
			if (selected_dataset >= 0 && selected_dataset < int(datasets.size())) {
				update_gui(datasets[selected_dataset]);
				construct_plot(datasets[selected_dataset]);
			}
			else
				construct_multi_plot(datasets);
		}
		bool recompute_plots = false;
		for (unsigned i = 0; i < nr_sub_plots; ++i) {
			if (member_ptr == &sub_plot_colors[i]) {
				p1.set_sub_plot_colors(i, sub_plot_colors[i]);
			}
			if (member_ptr == &sub_plot_x_columns[i] || member_ptr == &sub_plot_y_columns[i]) {
				recompute_plots = true;
			}
		}
		if (recompute_plots) {
			if (datasets.size() == 1 || selected_dataset != -1)
				construct_plot(datasets[selected_dataset == -1 ? 0 : selected_dataset]);
			else
				construct_multi_plot(datasets);
		}
		update_member(member_ptr);
		post_redraw();
	}

	void adjust_x(button& B)
	{
		p1.adjust_domain_axis_to_data(0, adjust_min, true);
		p1.adjust_tick_marks_to_domain();
		post_redraw();
	}
	void adjust_y(button& B)
	{
		p1.adjust_domain_axis_to_data(1, adjust_min, true);
		p1.adjust_tick_marks_to_domain();
		post_redraw();
	}
	void button_callback(button& B)
	{
		std::cout << "clicked button " << B.get_name() << " path = " << file_path_map[&B] << std::endl;
		gp->unselect_child(base_ptr(&B));
	}

	void create_gui_base(const std::string& path, gui_group_ptr ggp, bool first_level = true)
	{
		vector<string> dirs, files;
		std::cout << "scanning " << path << std::endl;
		scan_dir(path, "", dirs, files);
		unsigned i;
		for (i = 0; i < dirs.size(); ++i) {
			gui_group_ptr new_ggp = ggp->add_group(split_path(dirs[i]), "", "", "");
			new_ggp->set("color", 0xffff88);
			gui_group& g = *new_ggp;
			file_path_map[&g] = path + "/" + dirs[i];
			if (first_level)
				create_gui_base(path + "/" + dirs[i], new_ggp, false);
		}
		for (i = 0; i < files.size(); ++i) {
			button_ptr bp = ggp->add_button(split_path(files[i]) + "\tfile", "", "");
			bp->set("color", 0xffcccc);
			button& b = *bp;
			file_path_map[&b] = path + "/" + files[i];
			connect(bp->click, this, &browser_test::button_callback);
		}

	}
	void clear_datasets()
	{
		for (dataset* dp : datasets)
			delete dp;
		datasets.clear();
	}
	void select_cb(base_ptr b, bool select)
	{
		if (select) {
			if (b->cast<button>()) {
				dataset* dp = read_file(file_path_map[&*(b->cast<button>())]);
				if (dp) {
					clear_datasets();
					datasets.push_back(dp);
					selected_dataset = -1;
					on_set(&selected_dataset);
				}
			}
			else if (b->cast<gui_group>()) {
				if (read_directory(file_path_map[&*(b->cast<gui_group>())]) > 0) {
					if (selected_dataset > int(datasets.size()))
						selected_dataset = datasets.size()-1;
					update_gui(datasets[selected_dataset == -1 ? 0 : selected_dataset]);
					if (selected_dataset == -1)
						construct_multi_plot(datasets);
					else
						construct_plot(datasets[selected_dataset]);
				}
			}
		}
	}
	void remove_closed_children(gui_group_ptr g)
	{
		for (unsigned i = 0; i < g->get_nr_children(); ++i) {
			gui_group_ptr cg = g->get_child(i)->cast<gui_group>();
			if (cg) {
				if (gp->is_open_child_group(cg)) {
					std::cout << "recurse on " << cg->get_name() << std::endl;
					remove_closed_children(cg);
				}
				else {
					cg->remove_all_children();
					std::cout << "removing children of " << cg->get_name() << std::endl;
				}
			}
		}
	}
	void scan_closed_children(gui_group_ptr g, const std::string& path)
	{
		for (unsigned i = 0; i < g->get_nr_children(); ++i) {
			gui_group_ptr cg = g->get_child(i)->cast<gui_group>();
			if (cg) {
				if (gp->is_open_child_group(cg)) {
					std::cout << "recurse on " << cg->get_name() << std::endl;
					scan_closed_children(cg, path + cg->get_name() + "/");
				}
				else {
					create_gui_base(path + cg->get_name(), cg, false);
				}
			}
		}
	}
	void open_cb(gui_group_ptr g, bool open)
	{
		if (open) {
			std::string path;
			gui_group_ptr pg = g;
			while (pg && pg != gp) {
				path = pg->get_name() + "/" + path;
				pg = pg->get_parent()->cast<gui_group>();
			}
			path = file_path + "/" + path;
			scan_closed_children(g, path);
		}
		else {
			remove_closed_children(g);
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
		p1.init_frame(ctx);
	}
	void draw(cgv::render::context& ctx)
	{
		glDisable(GL_LIGHTING);
		p1.draw(ctx);
		glEnable(GL_LIGHTING);
	}

	void recreate_browser_gui()
	{
		gp->remove_all_children();
		gp->release_all_managed_objects();
		create_gui_base(file_path, gp);
	}
	void create_browser_gui()
	{
		add_decorator("browser", "heading", "level=3;w=100", " ");
		add_member_control(this, "mode", selection_mode, "dropdown", "enums='file,dir,tree';w=50");
		add_gui("path", file_path, "directory", "align=' '");
		file_path_map.clear();
		gp = add_group("", "tree_group", "w=300;h=400;column_heading_0='path';column_width_0=200;column_heading_1='size';column_width_1=100");
		gp->set("color", 0xbbbbff);
		connect(gp->on_selection_change, this, &browser_test::select_cb);
		connect(gp->on_open_state_change, this, &browser_test::open_cb);
		create_gui_base(file_path, gp);
	}

	void create_dynamic_gui()
	{
		add_decorator("plot", "heading", "level=3;w=100", " ");
		add_member_control(this, "adjust_min", adjust_min, "toggle", "w=70");

		add_member_control(this, "x axis", auto_x, "check", "w=10;align='L'", " ");
		connect(add_button("adj", "w=30", " ")->click, this, &browser_test::adjust_x);
		add_member_control(this, "log", p1.get_domain_config_ptr()->axis_configs[0].log_scale, "toggle", "w=30", " ");
		add_member_control(this, "", p1.get_domain_config_ptr()->axis_configs[0].primary_ticks.length, "value", "w=40;step=1;max=20;min=1");

		add_member_control(this, "y axis", auto_y, "check", "w=10;align='L'", " ");
		connect(add_button("adj", "w=30", " ")->click, this, &browser_test::adjust_y);
		add_member_control(this, "log", p1.get_domain_config_ptr()->axis_configs[1].log_scale, "toggle", "w=30", " ");
		add_member_control(this, "", p1.get_domain_config_ptr()->axis_configs[1].primary_ticks.length, "value", "w=40;step=1;max=20;min=1");

		add_member_control(this, "dataset", selected_dataset, "value_slider", "min=-1");
		find_control(selected_dataset)->set("max", datasets.size() == 0 ? 0 : datasets.size() - 1);

		for (unsigned i = 0; i < nr_sub_plots; ++i) {
			add_member_control(this, std::string("plot ") + to_string(i + 1), sub_plot_colors[i], "", "w=20", " ");
			add_member_control(this, "", p1.ref_sub_plot2d_config(i).show_plot, "check", "w=20", "");
			add_member_control(this, "*", p1.ref_sub_plot2d_config(i).show_points, "toggle", "w=20", "");
			add_member_control(this, "-", p1.ref_sub_plot2d_config(i).show_lines, "toggle", "w=20", "");
			add_member_control(this, "|", p1.ref_sub_plot2d_config(i).show_sticks, "toggle", "w=20", "  ");
			add_member_control(this, "x", (cgv::type::DummyEnum&)sub_plot_x_columns[i], "dropdown", std::string("enums='")+edx+"';w=50", "  ");
			add_member_control(this, "y", (cgv::type::DummyEnum&)sub_plot_y_columns[i], "dropdown", std::string("enums='")+edy+"';w=50", "");
			align("\n");
		}

		p1.create_gui(this, *this);
	}

	void create_gui()
	{
		create_browser_gui();
		create_dynamic_gui();
	}
};


#include <cgv/base/register.h>

cgv::base::object_registration<browser_test> sp_or("");

#ifdef CGV_FORCE_STATIC
cgv::base::registration_order_definition ro_def("stereo_view_interactor;browser_test");
#endif






/*
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
	sample_plot1d_interval<float>([&](float x) { return cos(x); }, -3, 3, 100, p1.ref_sub_plot_samples(0));
	p1.set_sub_plot_colors(0, cgv::plot::plot1d::Clr(1, 0, 0));

	p1.add_sub_plot("sin");
	sample_plot1d_interval<float>([&](float x) { return sin(x); }, -3, 3, 100, p1.ref_sub_plot_samples(1));
	p1.set_sub_plot_colors(1, cgv::plot::plot1d::Clr(1, 1, 0));

	p1.add_sub_plot("sinc");
	sample_plot1d_interval<float>([&](float x) { return (float)(sin(10 * x) / (10 * x + 0.000001)); }, -3, 3, 1000, p1.ref_sub_plot_samples(2));
	p1.set_sub_plot_colors(2, cgv::plot::plot1d::Clr(0, 1, 1));
}


float sqr(float x) { return x*x; }

void configure_ribbon_plots(cgv::plot::plot1d& p1, float beta)
{
	float cb2 = cos(beta)*cos(beta);
	float sb2 = sin(beta)*sin(beta);

	p1.add_sub_plot("cos");
	sample_plot1d_interval<float>([&](float x) { return sqrt(sb2 + cb2*sqr(cos(x))); }, -M_PI, M_PI, 1000, p1.ref_sub_plot_samples(0));
	p1.set_sub_plot_colors(0, cgv::plot::plot1d::Clr(1, 0, 0));

	p1.add_sub_plot("slerp");
	sample_plot1d_interval<float>([&](float x) {
		return
			sqrt(sb2+cb2*
			sqr(
				cos(x)*sin(fabs(cos(x))*M_PI_2) +
				sin(x)*sin((1.0 - fabs(cos(x)))*M_PI_2)
			));
	}, -M_PI, M_PI, 1000, p1.ref_sub_plot_samples(1));
	p1.set_sub_plot_colors(1, cgv::plot::plot1d::Clr(1, 1, 0));

	p1.add_sub_plot("lerp");
	sample_plot1d_interval<float>([&](float x) {
		return
			sqrt(sb2+cb2*
			sqr(
				cos(x)*fabs(cos(x)) +
				sin(x)*(1.0 - fabs(cos(x)))
			));
	}, -M_PI, M_PI, 1000, p1.ref_sub_plot_samples(2));
	p1.set_sub_plot_colors(2, cgv::plot::plot1d::Clr(0, 1, 1));

	for (int s = 0; s<3; ++s) {
		p1.ref_sub_plot1d_config(s).line_width = 3;
		if (s != 2) {
			p1.ref_sub_plot1d_config(s).show_sticks = true;
			p1.ref_sub_plot1d_config(s).stick_width = 1;
		}
	}


//	p1.add_sub_plot("sin");
//	sample_plot1d_interval<float>([&](float x) { return fabs(sin(x)); }, -M_PI, M_PI, 1000, p1.ref_sub_plot_samples(3));
//	p1.set_sub_plot_colors(3, cgv::plot::plot1d::Clr(1, 0, 1));

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
	float beta;
	//integrator_analizer ia;
public:
	simple_plots()
	{
		set_name("simple plot example");
		beta = 0;
		configure_ribbon_plots(p1, beta);
		//configure_simple_plots(p1);
		// configure_plots_montecarlo(p1);
		//ia.create_plots(p1);
		//ia.compute_plots(p1);

		p1.adjust_domain_to_data();
		//p1.set_extent(p1.get_domain().get_extent());
		p1.set_extent(cgv::plot::plot1d::V2D(2 * M_PI, 3.0));
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
//		if (member_ptr >= &ia && member_ptr < &ia+1)
	//		ia.compute_plots(p1);
		if (member_ptr == &beta) {
			p1.delete_sub_plot(2);
			p1.delete_sub_plot(1);
			p1.delete_sub_plot(0);
			configure_ribbon_plots(p1, beta);
		}

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
		add_member_control(this, "beta", beta, "value_slider", "min=0;max=1.5;ticks=true;step=0.01");
//		ia.create_gui(this, *this);
		p1.create_gui(this, *this);

	//	cgv::gui::gui_group_ptr gg = get_parent_group();
		//gg->get_parent()->cast<cgv::gui::gui_group>()->select_child(cgv::base::base_ptr(gg),true);
	}
};
*/
