#include <plot/plot1d.h>
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
	std::vector<cgv::plot::plot_base::Clr> sub_plot_colors;
	cgv::plot::plot1d p1;

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
		edx = "enums='index=-1'";
		edy = "enums='empty=-1'";
		nr_sub_plots = 6;
		for (unsigned i = 0; i < nr_sub_plots; ++i)
			p1.add_sub_plot("");
		sub_plot_x_columns.resize(nr_sub_plots, -1);
		sub_plot_y_columns.resize(nr_sub_plots, -1);
		set_sub_plot_colors();
		adjust_min = true;
		incremental_build = true;
		selected_dataset = -1;
		file_path = "S:/data/EvaluationData";
		headers.resize(nr_sub_plots);
		auto_x = auto_y = true;
		selection_mode = BSM_DIRECTORY;
		p1.set_extent(cgv::plot::plot_types::P2D(3.0f, 2.0f));
		p1.tick_length[0] = 5;
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
			if (!adjust_min)
				p1.ref_domain().ref_min_pnt()(0) = 0;
		}
		if (auto_y) {
			p1.adjust_domain_axis_to_data(1, adjust_min, true);
			if (!adjust_min)
				p1.ref_domain().ref_min_pnt()(1) = 0;
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
				p1.ref_sub_plot_samples(i)[j] = cgv::plot::plot1d::P2D(has_x? float(dp->data[cx][j]):float(j + 1), float(dp->data[cy][j]));
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
					p1.ref_sub_plot_samples(i)[fst + j] = cgv::plot::plot1d::P2D(has_x ? float(dps[k]->data[cx][j]) : float(j + 1), float(dps[k]->data[cy][j]));

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
		if (member_ptr == &adjust_min || member_ptr == &p1.ref_axis_config(0).log_scale || member_ptr == &p1.ref_axis_config(1).log_scale) {
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
				p1.ref_sub_plot1d_config(i).line_color = sub_plot_colors[i];
				p1.ref_sub_plot_config(i).point_color = sub_plot_colors[i];
				p1.ref_sub_plot_config(i).bar_color    = sub_plot_colors[i];
				p1.ref_sub_plot_config(i).stick_color = sub_plot_colors[i];
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
		add_member_control(this, "log", p1.ref_axis_config(0).log_scale, "toggle", "w=30", " ");

		add_member_control(this, "y axis", auto_y, "check", "w=10;align='L'", " ");
		connect(add_button("adj", "w=30", " ")->click, this, &browser_test::adjust_y);
		add_member_control(this, "log", p1.ref_axis_config(1).log_scale, "toggle", "w=30", " ");
		add_member_control(this, "", p1.tick_length[0], "value", "w=40;step=1;max=20;min=1");

		add_member_control(this, "dataset", selected_dataset, "value_slider", "min=-1");
		find_control(selected_dataset)->set("max", datasets.size() == 0 ? 0 : datasets.size() - 1);
		/*
		align("%Y-=8");
		for (size_t c = 0; c < nr_sub_plots; ++c) {
			add_member_control(this, c < headers.size() ? headers[c] : "", (bool&)column_selection[c], "check");
		}
		align("%Y+=8");
		*/
		for (unsigned i = 0; i < nr_sub_plots; ++i) {
			add_member_control(this, std::string("plot ") + to_string(i + 1), sub_plot_colors[i], "", "w=20", " ");
			add_member_control(this, "", p1.ref_sub_plot1d_config(i).show_plot, "check", "w=20", "");
			add_member_control(this, "*", p1.ref_sub_plot1d_config(i).show_points, "toggle", "w=20", "");
			add_member_control(this, "-", p1.ref_sub_plot1d_config(i).show_lines, "toggle", "w=20", "");
			add_member_control(this, "|", p1.ref_sub_plot1d_config(i).show_sticks, "toggle", "w=20", "  ");
			add_member_control(this, "x", (cgv::type::DummyEnum&)sub_plot_x_columns[i], "dropdown", edx + ";w=50", "  ");
			add_member_control(this, "y", (cgv::type::DummyEnum&)sub_plot_y_columns[i], "dropdown", edy + ";w=50", "");
			align("\n");
		}

		p1.create_gui(this, *this);
	}
/*
	std::string get_parent_type() const
	{
		return "layout_group";
	}

	void recreate_gui()
	{
		if (!ap) {
			provider::recreate_gui();
			return;
		}
		if (!parent_group)
			return;
		int yscroll = parent_group->get<int>("yscroll");
		parent_group->remove_all_children();
		parent_group->release_all_managed_objects();
		create_dynamic_gui();
		parent_group->set("dolayout", true);
		parent_group->set("yscroll", yscroll);
	}
	*/
	void create_gui()
	{
		// pp = parent_group;
		// parent_group->multi_set("layout=table;border_style=framed");
		// ap = add_group("", "align_group", "w=500;h=400", "cX\n");
		// parent_group = ap;
		
		create_browser_gui();

		// bp = add_group("", "align_group", "w=500;h=400", "cX\n");
		// parent_group = bp;

		create_dynamic_gui();
	}
};


#include <cgv/base/register.h>

cgv::base::object_registration<browser_test> sp_or("");

#ifdef CGV_FORCE_STATIC
cgv::base::registration_order_definition ro_def("stereo_view_interactor;browser_test");
#endif
