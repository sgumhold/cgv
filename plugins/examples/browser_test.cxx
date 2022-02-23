#include <cgv/base/named.h>
#include <cgv/gui/provider.h>
#include <cgv/media/color.h>
#include <cgv/utils/scan.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/utils/file.h>

using namespace cgv::base;
using namespace cgv::reflect;
using namespace cgv::gui;
using namespace cgv::media;
using namespace cgv::utils;
using namespace cgv::utils::file;
using namespace std;

class browser_test : public named, public provider
{
protected:
	std::string file_path;
	bool incremental_build;
	void scan_dir(const std::string& path, const std::string& dir, vector<string>& dirs, vector<string>& files)
	{
		string p = path+"/";
		if (!dir.empty())
			p += dir+"/";
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
public:
	browser_test()
	{
		incremental_build = true;
		file_path = "C:";
	}
	std::string get_type_name() const
	{
		return "browser_test";
	}
	std::string get_parent_type() const
	{
		return "tree_group";
	}
	bool self_reflect(reflection_handler& rh)
	{
		return rh.reflect_member("file_path", file_path);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &file_path)
			post_recreate_gui();
	}
	void create_gui_rec(const std::string& path, gui_group_ptr ggp)
	{
		vector<string> dirs, files;
		std::cout << "scanning " << path << std::endl;
		scan_dir(path, "", dirs, files);
		unsigned i;
		for (i=0; i<dirs.size(); ++i) {
			gui_group_ptr new_ggp = ggp->add_group(split_path(dirs[i]),"", "", "");
			create_gui_rec(path+"/"+dirs[i], new_ggp);
		}
		for (i=0; i<files.size(); ++i) {
			ggp->add_button(split_path(files[i])+"\t"+to_string(cgv::utils::file::size(path+"/"+files[i])),"","");
		}
	}
	void button_callback(button& B)
	{
		std::cout << "clicked button " << B.get_name() << std::endl;
		parent_group->unselect_child(base_ptr(&B));
	}
	void create_gui_base(const std::string& path, gui_group_ptr ggp, bool first_level = true)
	{
		vector<string> dirs, files;
		std::cout << "scanning " << path << std::endl;
		scan_dir(path, "", dirs, files);
		unsigned i;
		for (i=0; i<dirs.size(); ++i) {
			gui_group_ptr new_ggp = ggp->add_group(split_path(dirs[i]),"", "", "");
			new_ggp->set("color",0xffff88);
			if (first_level)
				create_gui_base(path+"/"+dirs[i], new_ggp, false);
		}
		for (i=0; i<files.size(); ++i) {
			button_ptr B = ggp->add_button(split_path(files[i])+"\tfile","","");
			B->set("color",0xffcccc);
			connect(B->click, this, &browser_test::button_callback);
		}
		
	}
	void select_cb(base_ptr b, bool select)
	{
		if (select)
			std::cout << "select ";
		else
			std::cout << "unselect ";
		if (b->cast<named>())
			std::cout << b->cast<named>()->get_name();
		else
			std::cout << b->get_type_name();
		std::cout << std::endl;
	}
	void remove_closed_children(gui_group_ptr g)
	{
		for (unsigned i=0; i<g->get_nr_children(); ++i) {
			gui_group_ptr cg = g->get_child(i)->cast<gui_group>();
			if (cg) {
				if (parent_group->is_open_child_group(cg)) {
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
		for (unsigned i=0; i<g->get_nr_children(); ++i) {
			gui_group_ptr cg = g->get_child(i)->cast<gui_group>();
			if (cg) {
				if (parent_group->is_open_child_group(cg)) {
					std::cout << "recurse on " << cg->get_name() << std::endl;
					scan_closed_children(cg, path+cg->get_name()+"/");
				}
				else {
					create_gui_base(path+cg->get_name(), cg, false);
				}
			}
		}
	}
	void open_cb(gui_group_ptr g, bool open)
	{
		if (open) {
			std::cout << "opened " << g->get_name();
			std::string path;
			gui_group_ptr pg = g;
			while (pg && pg != parent_group) {
				path = pg->get_name() + "/" + path;
				pg = pg->get_parent()->cast<gui_group>();
			}
			path = file_path + "/" + path;
			std::cout << " start_path = " << path << std::endl;
			scan_closed_children(g, path);
			std::cout << std::endl;
		}
		else {
			std::cout << "closing " << g->get_name() << ":" << std::endl;;
			remove_closed_children(g);
			std::cout << std::endl;
		}
	}
	void create_gui()
	{
		parent_group->multi_set("column_heading_0='path';column_width_0=200;column_heading_1='size';column_width_1=100");
		parent_group->set("color",0xbbbbff);
		connect(parent_group->on_selection_change,this,&browser_test::select_cb);
		connect(parent_group->on_open_state_change,this,&browser_test::open_cb);
		parent_group->set("label", file_path);
//		create_gui_rec(file_path, parent_group);
		create_gui_base(file_path, parent_group);
	}
};

#include <cgv/base/register.h>

cgv::base::factory_registration<browser_test> browser_test_1("browser_test", "menu_text=\"New/GUI/Browser Test\"");

