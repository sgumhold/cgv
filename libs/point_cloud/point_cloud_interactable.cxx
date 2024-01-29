#include <algorithm>
#include "point_cloud_interactable.h"
#include <cgv/gui/trigger.h>
#include <cgv/gui/key_event.h>
#include <libs/point_cloud/ann_tree.h>
#include <cgv/base/find_action.h>
#include <cgv/signal/rebind.h>
#include <cgv/base/import.h>
#include <cgv/utils/file.h>
#include <cgv/render/view.h>
#include <cgv/gui/dialog.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/animate.h>
#include <cgv/gui/file_dialog.h>
#include <cgv/reflect/reflect_extern.h>
#include <cgv_gl/gl/gl.h>
#include <cgv/utils/tokenizer.h>

#define FILE_OPEN_TITLE "Open Point Cloud"
#define FILE_APPEND_TITLE "Append Point Cloud"
#define FILE_OPEN_FILTER "Point Clouds (apc,bpc):*.apc;*.bpc|Mesh Files (obj,ply,pct):*.obj;*.ply;*.pct|All Files:*.*"

#define FILE_SAVE_TITLE "Save Point Cloud"
#define FILE_SAVE_FILTER "Point Clouds (apc,bpc):*.apc;*.bpc|Mesh Files (obj,ply):*.obj;*.ply|All Files:*.*"

void point_cloud_interactable::update_file_name(const std::string& ffn, bool append)
{
	std::string new_path = cgv::utils::file::get_path(ffn);
	if (!new_path.empty()) {
		data_path = new_path;
		update_member(&data_path);
	}
	std::string new_name = cgv::utils::file::get_file_name(cgv::utils::file::drop_extension(ffn));
	if (append && !file_name.empty()) {
		file_name += std::string("#")+ new_name;
	}
	else
		file_name = new_name;
	new_file_name = file_name;
	update_member(&new_file_name);
}

// file io
bool point_cloud_interactable::save(const std::string& fn)
{
	if (!write(fn)) {
		cgv::gui::message(std::string("could not write ") + fn);
		return false;
	}
	update_file_name(fn);
	return true;
}
bool point_cloud_interactable::open(const std::string& _file_name)
{
	std::string fn = _file_name;
	if (!read(fn, &data_path)) {
		cgv::gui::message(last_error);
		return false;
	}
	on_point_cloud_change_callback(PCC_NEW_POINT_CLOUD);
	update_file_name(fn);
	return true;
}
bool point_cloud_interactable::open_directory(const std::string& dn)
{
	std::vector<std::string> file_names;
	void* handle = cgv::utils::file::find_first(directory_name + "/*.*");
	while (handle) {
		if (!cgv::utils::file::find_directory(handle))
			file_names.push_back(cgv::utils::file::find_name(handle));
		handle = cgv::utils::file::find_next(handle);
	}
	if (file_names.empty()) {
		std::cerr << "did not find files in directory <" << dn << ">" << std::endl;
		return false;
	}
	unsigned i;
	for (i = 0; i < file_names.size(); ++i) {
		std::cout << i << "(" << file_names.size() << "): " << file_names[i];
		std::cout.flush();
		if (!do_append && i == 0)
			open(directory_name + "/" + file_names[i]);
		else
			open_and_append(directory_name + "/" + file_names[i]);
		std::cout << std::endl;
	}
	for (i = 0; i < pc.get_nr_components(); ++i) {
		pc.component_color(i) = cgv::media::color<float, cgv::media::HLS, cgv::media::OPACITY>(float(i) / float(pc.get_nr_components() - 1), 0.5f, 1.0f, 1.0f);
	}
	use_component_colors = true;
	update_member(&use_component_colors);
	if (!do_append)
		on_point_cloud_change_callback(PCC_NEW_POINT_CLOUD);
	update_file_name(dn);
	return true;
}
bool point_cloud_interactable::open_and_append(const std::string& _file_name)
{
	std::string fn = _file_name;
	if (!append(fn, pc.get_nr_points() > 0, &data_path)) {
		cgv::gui::message(last_error);
		return false;
	}
	on_point_cloud_change_callback(PointCloudChangeEvent(PCC_POINTS_RESIZE + PCC_COMPONENTS_RESIZE));
	update_file_name(fn, true);
	return true;
}
bool point_cloud_interactable::open_or_append(cgv::gui::event& e, const std::string& file_name)
{
	cgv::utils::tokenizer T(file_name);
	T.set_ws("\n").set_sep("");
	std::vector<cgv::utils::token> toks;
	T.bite_all(toks);
	bool res = false;
	for (unsigned i = 0; i<toks.size(); ++i) {
		std::string file_path = cgv::base::find_data_file(to_string(toks[i]), "CM", "", data_path);
		if (e.get_modifiers() == cgv::gui::EM_ALT || i > 0)
			res |= open_and_append(file_path);
		else
			res |= open(file_path);
	}
	return res;
}


void point_cloud_interactable::interact_callback(double t, double dt)
{

	if (interact_state == IS_FULL_FRAME || interact_state == IS_DRAW_FULL_FRAME)
		return;

	if (interact_state == IS_INTERMEDIATE_FRAME)
		interact_state = IS_WAIT_INTERACTION_TO_STOP;
	else {
		interact_state = IS_DRAW_FULL_FRAME;
		post_redraw();
	}

}


void point_cloud_interactable::ensure_tree_ds()
{
	if (tree_ds_out_of_date) {
		if (tree_ds)
			delete tree_ds;
		tree_ds = new ann_tree;
		tree_ds->build(pc);
		tree_ds_out_of_date = false;
	}
}
void point_cloud_interactable::build_neighbor_graph()
{
	// use component wise implementation if we do have more than one component 
	if (pc.has_components() && pc.get_nr_components() > 1) {
		build_neighbor_graph_componentwise();
		return;
	}

	ng.clear();
	ensure_tree_ds();
	cgv::utils::statistics he_stats;
	ng.build(pc.get_nr_points(), k, *tree_ds, &he_stats);
	if (do_symmetrize)
		ng.symmetrize();
	on_point_cloud_change_callback(PCC_NEIGHBORGRAPH_CREATE);

	std::cout << "half edge statistics " << he_stats << std::endl;
	std::cout << "v " << pc.get_nr_points()
		<< ", he = " << ng.nr_half_edges
		<< " ==> " << (float)ng.nr_half_edges / ((unsigned)(pc.get_nr_points())) << " half edges per vertex" << std::endl;
}

void point_cloud_interactable::build_neighbor_graph_componentwise()
{
	// prepare neighbor graph data structure
	ng.clear();
	ng.resize(pc.get_nr_points());

	// iterate components
	std::cout << "build_neighbor_graph_componentwise(" << pc.get_nr_components() << "):"; std::cout.flush();
	for (Idx ci = 0; ci < (Idx)pc.get_nr_components(); ++ci) {
		std::cout << " " << ci << ":"; std::cout.flush();
		ann_tree* T = new ann_tree;
		std::vector<Idx> C(1, Idx(ci));
		T->build(pc, C);
		Idx n = Idx(pc.component_point_range(ci).nr_points);
		Idx offset = Idx(pc.component_point_range(ci).index_of_first_point);
		for (Idx l = 0; l < n; ++l) {
			Idx i = l + offset;
			std::vector<Idx>& Ni = ng[i];
			T->extract_neighbors(i, k, Ni);
			for (auto& ni : Ni) 
				ni += offset;
			ng.nr_half_edges += k;
		}
		delete T;
		std::cout << "*"; std::cout.flush();

		if (do_symmetrize) {
			for (Idx l = 0; l < n; ++l) {
				Idx i = l + offset;
				std::vector<Idx>& Ni = ng[i];
				for (size_t o = 0; o < Ni.size(); ++o) {
					std::vector<Idx>& Nj = ng[Ni[o]];
					if (std::find(Nj.begin(), Nj.end(), i) == Nj.end()) {
						Nj.push_back(i);
						++ng.nr_half_edges;
					}
				}
			}
			std::cout << "s"; std::cout.flush();
		}
	}
	std::cout << std::endl;

	on_point_cloud_change_callback(PCC_NEIGHBORGRAPH_CREATE);
}

bool point_cloud_interactable::get_picked_point(int x, int y, unsigned& index)
{
	cgv::math::fvec<double, 3> world_location;
	if (!get_world_location(x, y, *view_ptr, world_location))
		return false;
	//  unproject to world coordinates with smaller (closer to eye) z-value one	
	Pnt p_pick_world = world_location;

	// find closest point
	int i_closest = -1;
	if (accelerate_picking) {
		ensure_tree_ds();
		i_closest = tree_ds->find_closest(p_pick_world);
	}
	else {
		int n = (int)pc.get_nr_points();
		float smallest_sqr_dist = 0;
		for (int i = 0; i < n; ++i) {
			if (i_closest == -1) {
				i_closest = i;
				smallest_sqr_dist = (pc.pnt(i) - p_pick_world).sqr_length();
			}
			else {
				float new_sqr_dist = (pc.pnt(i) - p_pick_world).sqr_length();
				if (new_sqr_dist < smallest_sqr_dist) {
					i_closest = i;
					smallest_sqr_dist = new_sqr_dist;
				}
			}
		}
	}
	if (i_closest == -1)
		return false;
	index = i_closest;
	return true;
}



void point_cloud_interactable::compute_normals()
{
	if (ng.empty())
		build_neighbor_graph();
	ne.compute_weighted_normals(reorient_normals && pc.has_normals());
	on_point_cloud_change_callback(PCC_NORMALS);
	post_redraw();
}
void point_cloud_interactable::recompute_normals()
{
	if (ng.empty())
		build_neighbor_graph();
	if (!pc.has_normals())
		compute_normals();
	//	ne.compute_bilateral_weighted_normals(reorient_normals);
	ne.compute_plane_bilateral_weighted_normals(reorient_normals);
	on_point_cloud_change_callback(PCC_NORMALS);
	post_redraw();
}
void point_cloud_interactable::toggle_normal_orientations()
{
	if (!pc.has_normals())
		return;
	for (Idx i = 0; i < Idx(pc.get_nr_points()); ++i)
		pc.nml(i) = -pc.nml(i);
	on_point_cloud_change_callback(PCC_NORMALS);
	post_redraw();
}
void point_cloud_interactable::orient_normals()
{
	if (ng.empty())
		build_neighbor_graph();
	if (!pc.has_normals())
		compute_normals();
	ne.orient_normals();
	on_point_cloud_change_callback(PCC_NORMALS);
	post_redraw();
}
void point_cloud_interactable::orient_normals_to_view_point()
{
	if (ensure_view_pointer()) {
		if (ng.empty())
			build_neighbor_graph();
		Pnt view_point = view_ptr->get_eye();
		ne.orient_normals(view_point);
		on_point_cloud_change_callback(PCC_NORMALS);
		post_redraw();
	}
}

/// call this before using the view ptr for the first time
bool point_cloud_interactable::ensure_view_pointer()
{
	return cgv::base::ensure_by_find(this, view_ptr);
}


point_cloud_interactable::point_cloud_interactable() : ne(pc, ng)
{
	set_name("Point Cloud Viewer");

	view_ptr = 0;

	accelerate_picking = true;
	tree_ds_out_of_date = true;
	tree_ds = 0;

	do_append = false;
	do_auto_view = true;

	show_nmls = false;
	interact_point_step = 1;
	show_point_count = 0;
	show_point_start = 0;
	interact_delay = 0.15;

	interact_state = IS_INTERMEDIATE_FRAME;

	cgv::signal::connect(interact_trigger.shoot, this, &point_cloud_interactable::interact_callback);
	interact_trigger.schedule_recuring(interact_delay);

	show_neighbor_graph = false;
	k = 30;
	do_symmetrize = false;
	reorient_normals = true;
}
void point_cloud_interactable::auto_set_view()
{
	if (pc.get_nr_points() == 0)
		return;

	std::vector<cgv::render::view*> view_ptrs;
	cgv::base::find_interface<cgv::render::view>(get_node(), view_ptrs);
	if (view_ptrs.empty()) {
		cgv::gui::message("could not find a view to adjust!!");
		return;
	}
	cgv::gui::animate_with_rotation(view_ptrs[0]->ref_view_up_dir(), cgv::dvec3(0, 1, 0), 0.5)->set_base_ptr(this);
	cgv::gui::animate_with_geometric_blend(view_ptrs[0]->ref_y_extent_at_focus(), 1.5*pc.box().get_extent()(1), 0.5)->set_base_ptr(this);
	cgv::gui::animate_with_linear_blend(view_ptrs[0]->ref_focus(), cgv::dvec3(pc.box().get_center()), 0.5)->set_base_ptr(this);
	post_redraw();
}


std::string point_cloud_interactable::get_type_name() const
{
	return "point_cloud_interactable";
}
bool point_cloud_interactable::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return
		srh.reflect_member("do_append", do_append) &&
		srh.reflect_member("use_component_colors", use_component_colors) &&
		srh.reflect_member("use_component_transformations", use_component_transformations) &&
		srh.reflect_member("do_auto_view", do_auto_view) &&
		srh.reflect_member("data_path", data_path) &&
		srh.reflect_member("file_name", new_file_name) &&
		srh.reflect_member("directory_name", directory_name) &&
		srh.reflect_member("interact_point_step", interact_point_step) &&
		srh.reflect_member("surfel_style", surfel_style) &&
		srh.reflect_member("normal_style", normal_style) &&
		srh.reflect_member("box_style", box_style) &&
		srh.reflect_member("box_wire_style", box_wire_style) &&
		srh.reflect_member("show_points", show_points) &&
		srh.reflect_member("show_nmls", show_nmls) &&
		srh.reflect_member("show_boxes", show_boxes) &&
		srh.reflect_member("show_box", show_box) &&
		srh.reflect_member("sort_points", sort_points) &&
		srh.reflect_member("show_neighbor_graph", show_neighbor_graph) &&
		srh.reflect_member("k", k) &&
		srh.reflect_member("do_symmetrize", do_symmetrize) &&
		srh.reflect_member("reorient_normals", reorient_normals);
}
void point_cloud_interactable::stream_help(std::ostream& os)
{
	os << "PC: open (Ctrl-O), append (Ctrl-A), toggle <p>oints, <n>ormals, <b>ox, <g>graph, <i>llum" << std::endl;
}
void point_cloud_interactable::stream_stats(std::ostream& os)
{
	os << "PC: #P=" << pc.get_nr_points()
		<< ", #N=" << (pc.has_normals() ? pc.get_nr_points() : 0)
		<< ", #C=" << (pc.has_colors() ? pc.get_nr_points() : 0) 
		<< ", B=" << pc.box().get_center() << "<" << pc.box().get_extent() << ">" << std::endl;
}


void point_cloud_interactable::draw_edge_color(unsigned int vi, unsigned int j, bool is_symm, bool is_start) const
{
	if (is_symm)
		glColor3f(0.5f, 0.5f, 0.5f);
	else {
		if (is_start)
			glColor3f(1, 0.5f, 0.5f);
		else
			glColor3f(1, 1, 0.5f);
	}
}
void point_cloud_interactable::draw_graph(cgv::render::context& ctx)
{
	if (!show_neighbor_graph)
		return;

	glDisable(GL_LIGHTING);
	glColor3f(0.5f, 0.5f, 0.5f);
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	for (unsigned int vi = 0; vi<ng.size(); ++vi) {
		const std::vector<Idx> &Ni = ng[vi];
		for (unsigned int j = 0; j<Ni.size(); ++j) {
			unsigned int vj = Ni[j];
			// check for symmetric case and only draw once
			if (ng.is_directed_edge(vj, vi)) {
				if (vi < vj) {
					draw_edge_color(vi, j, true, true);
					glArrayElement(vi);
					draw_edge_color(vi, j, true, false);
					glArrayElement(vj);
				}
			}
			else {
				draw_edge_color(vi, j, false, true);
				glArrayElement(vi);
				draw_edge_color(vi, j, false, false);
				glArrayElement(vj);
			}
		}
	}
	glEnd();
	glEnable(GL_LIGHTING);
}
bool point_cloud_interactable::init(cgv::render::context& ctx)
{
	if (!gl_point_cloud_drawable::init(ctx))
		return false;

	get_root()->set("bg_index", 4);

	return true;
}
void point_cloud_interactable::init_frame(cgv::render::context& ctx)
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
	gl_point_cloud_drawable::init_frame(ctx);
}
void point_cloud_interactable::draw(cgv::render::context& ctx)
{
	if (pc.get_nr_points() == 0)
		return;

	glVertexPointer(3, GL_FLOAT, 0, &(pc.pnt(0).x()));
	glEnableClientState(GL_VERTEX_ARRAY);
	draw_graph(ctx);
	glDisableClientState(GL_VERTEX_ARRAY);


	if (interact_state != IS_DRAW_FULL_FRAME)
		std::swap(show_point_step, interact_point_step);

	gl_point_cloud_drawable::draw(ctx);

	if (interact_state != IS_DRAW_FULL_FRAME) {
		std::swap(show_point_step, interact_point_step);
		interact_state = IS_INTERMEDIATE_FRAME;
	}
	else
		interact_state = IS_FULL_FRAME;
}




void point_cloud_interactable::configure_subsample_controls()
{
	if (find_control(show_point_begin)) {
		find_control(show_point_begin)->set("max", show_point_end);
		find_control(show_point_end)->set("min", show_point_begin);
		find_control(show_point_end)->set("max", pc.get_nr_points());
		find_control(show_point_count)->set("max", pc.get_nr_points());
		find_control(show_point_start)->set("max", pc.get_nr_points() - show_point_count);
	}
}
void point_cloud_interactable::handle_args(std::vector<std::string>& args)
{
	for (unsigned ai = 0; ai < args.size(); ++ai) {
		if (cgv::utils::file::exists(args[ai])) {
			if (open_and_append(args[ai])) {
				args.erase(args.begin() + ai);
				--ai;
			}
		}
	}
}
bool point_cloud_interactable::handle(cgv::gui::event& e)
{
	if (e.get_kind() == cgv::gui::EID_KEY) {
		cgv::gui::key_event& ke = static_cast<cgv::gui::key_event&>(e);
		if (ke.get_action() == cgv::gui::KA_PRESS || ke.get_action() == cgv::gui::KA_REPEAT) {
			switch (ke.get_key()) {
			case '=' :
			case '+' :
				surfel_style.point_size += 1;
				on_set(&surfel_style.point_size); 
				return true;
			case '-' :
				if (surfel_style.point_size > 0) {
					surfel_style.point_size -= 1;
					if (surfel_style.point_size < 1)
						surfel_style.point_size = 1;
					on_set(&surfel_style.point_size); 
				}
				return true;
			case 'P' :
				show_points = !show_points;
				on_set(&show_points); 
				return true;
			case 'C' :
				if (ke.get_modifiers() == 0) {
					if (surfel_style.map_color_to_material == cgv::render::MS_FRONT_AND_BACK)
						surfel_style.map_color_to_material = cgv::render::CM_NONE;
					else
						++(int&)surfel_style.map_color_to_material;
					on_set(&surfel_style.map_color_to_material);
				}
				else if (ke.get_modifiers() == cgv::gui::EM_SHIFT) {
					if (surfel_style.culling_mode == cgv::render::CM_FRONTFACE)
						surfel_style.culling_mode = cgv::render::CM_OFF;
					else
						++(int&)surfel_style.culling_mode;
					on_set(&surfel_style.culling_mode);
				}
				return true;
			case 'B' :
				if (ke.get_modifiers() == 0) {
					surfel_style.blend_points = !surfel_style.blend_points;
					on_set(&surfel_style.blend_points);
				}
				else if (ke.get_modifiers() == cgv::gui::EM_SHIFT) {
					show_boxes = !show_boxes;
					on_set(&show_boxes);
				}
				return true;
			case 'N' :
				if (ke.get_modifiers() == 0) {
					show_nmls = !show_nmls;
					on_set(&show_nmls);
				}
				else if(ke.get_modifiers() == cgv::gui::EM_CTRL+ cgv::gui::EM_ALT) {
					if (!pc.has_normals()) {
						pc.create_normals();
						compute_normals();
					}
					else 
						recompute_normals();

					on_point_cloud_change_callback(PCC_NORMALS);
					post_redraw();
				}
				return true;
			case 'I' :
				if (surfel_style.illumination_mode == cgv::render::IM_TWO_SIDED)
					surfel_style.illumination_mode = cgv::render::IM_OFF;
				else
					++(int&)surfel_style.illumination_mode;
				on_set(&surfel_style.illumination_mode);
				return true;
			case 'G' :
				show_neighbor_graph = !show_neighbor_graph;
				on_set(&show_neighbor_graph); 
				return true;
			case 'O' :
				if (ke.get_modifiers() == 0) {
					surfel_style.orient_splats = !surfel_style.orient_splats;
					on_set(&surfel_style.orient_splats);
				}
				else if (ke.get_modifiers() == cgv::gui::EM_SHIFT) {
					orient_normals();
					on_point_cloud_change_callback(PCC_NORMALS);
					post_redraw();
					return true;
				}
				else if (ke.get_modifiers() == cgv::gui::EM_ALT) {
					orient_normals_to_view_point();
					on_point_cloud_change_callback(PCC_NORMALS);
					post_redraw();
					return true;
				}
				else if (ke.get_modifiers() == cgv::gui::EM_CTRL) {
					std::string fn = cgv::gui::file_open_dialog(FILE_OPEN_TITLE, FILE_OPEN_FILTER);
					if (!fn.empty())
						open(fn);
					return true;
				}
				else if (ke.get_modifiers() == cgv::gui::EM_CTRL + cgv::gui::EM_SHIFT) {
					std::string fn = cgv::gui::file_open_dialog("Open Component Transformations", "Text Files (txt):*.txt|All Files:*.*");
					if (!fn.empty()) {
						if (!pc.read_component_transformations(fn)) {
							std::cerr << "error reading component transformation file " << fn << std::endl;
						}
						post_redraw();
					}
					return true;
				}
				return false;
			case 'A' :
				if (ke.get_modifiers() == cgv::gui::EM_CTRL) {
					std::string fn = cgv::gui::file_open_dialog(FILE_APPEND_TITLE, FILE_OPEN_FILTER);
					if (!fn.empty())
						open_and_append(fn);
					return true;
				}
				return false;
			case 'S' :
				if (ke.get_modifiers() == cgv::gui::EM_CTRL + cgv::gui::EM_SHIFT) {
					std::string fn = cgv::gui::file_save_dialog(FILE_SAVE_TITLE, FILE_OPEN_FILTER);
					if (!fn.empty())
						save(fn);
					return true;
				}
				else if (ke.get_modifiers() == cgv::gui::EM_ALT) {
					sort_points = !sort_points;
					on_set(&sort_points);
					return true;
				}
				return false;
			case cgv::gui::KEY_Space :
				if (pc.get_nr_points() == 0)
					return false;
				auto_set_view();
				return true;
			}
		}
	}
	else {
		if (e.get_kind() == cgv::gui::EID_MOUSE) {
			if (e.get_flags() == cgv::gui::EF_DND) {
				cgv::gui::mouse_event& me = static_cast<cgv::gui::mouse_event&>(e);
				if (me.get_action() == cgv::gui::MA_RELEASE) {
					open_or_append(e, me.get_dnd_text());
				}
				return true;
			}
		}
	}
	return false;
}
void point_cloud_interactable::on_point_cloud_change_callback(PointCloudChangeEvent pcc_event)
{
	if (use_component_transformations != pc.has_component_transformations()) {
		use_component_transformations = pc.has_component_transformations();
		on_set(&use_component_transformations);
	}
	if (use_component_colors != pc.has_component_colors()) {
		use_component_colors = pc.has_component_colors();
		on_set(&use_component_colors);
	}
	if (!pc.has_normals() != surfel_style.illumination_mode == cgv::render::IM_OFF) {
		if (pc.has_normals())
			surfel_style.illumination_mode = cgv::render::IM_ONE_SIDED;
		else
			surfel_style.illumination_mode = cgv::render::IM_OFF;
		update_member(&surfel_style.illumination_mode);
	}
	if (((pcc_event & PCC_POINTS_MASK) == PCC_POINTS_RESIZE) || ((pcc_event & PCC_POINTS_MASK) == PCC_NEW_POINT_CLOUD)) {
		tree_ds_out_of_date = true;
		if (tree_ds) {
			delete tree_ds;
			tree_ds = 0;
		}
		ng.clear();
		show_point_end = pc.get_nr_points();
		show_point_begin = 0;

		update_member(&show_point_begin);
		update_member(&show_point_end);

		show_point_count = pc.get_nr_points();
		update_member(&show_point_count);

		interact_point_step = std::max((unsigned)(show_point_count / 1000000), 1u);
		nr_draw_calls = std::max((unsigned)(show_point_count / 1000000), 1u);
		update_member(&interact_point_step);
		update_member(&nr_draw_calls);

		configure_subsample_controls();
	}
	if ((pcc_event & PCC_POINTS_MASK) == PCC_NEW_POINT_CLOUD && do_auto_view) {
		auto_set_view();
	}
	post_redraw();
}
void point_cloud_interactable::on_set(void* member_ptr)
{
	if (member_ptr == &ne.localization_scale || member_ptr == &ne.normal_sigma || member_ptr == &ne.bw_type || member_ptr == &ne.plane_distance_scale) {
		on_point_cloud_change_callback(PCC_WEIGHTS);
	}
	if (member_ptr == &new_file_name) {
		if (ref_tree_node_visible_flag(new_file_name)) {
			save(new_file_name);
		}
		else {
			if (!do_append)
				open(new_file_name);
			else
				open_and_append(new_file_name);
		}
	}
	if (member_ptr == &directory_name) {
		open_directory(directory_name);
	}
	if (member_ptr == &show_point_start) {
		show_point_begin = show_point_start;
		show_point_end = show_point_start + show_point_count;
		update_member(&show_point_begin);
		update_member(&show_point_end);
		configure_subsample_controls();
	}
	if (member_ptr == &show_point_count) {
		if (show_point_start + show_point_count > pc.get_nr_points()) {
			show_point_start = pc.get_nr_points() - show_point_count;
			show_point_begin = show_point_start;
			update_member(&show_point_begin);
			update_member(&show_point_start);
		}
		show_point_end = show_point_start + show_point_count;
		update_member(&show_point_end);
		configure_subsample_controls();
	}
	if (member_ptr == &show_point_begin) {
		show_point_start = show_point_begin;
		show_point_count = show_point_end - show_point_begin;
		update_member(&show_point_start);
		update_member(&show_point_count);
		configure_subsample_controls();
	}
	if (member_ptr == &show_point_end) {
		show_point_count = show_point_end - show_point_begin;
		update_member(&show_point_count);
		configure_subsample_controls();
	}
	if (member_ptr == &interact_delay) {
		interact_trigger.stop();
		interact_trigger.schedule_recuring(interact_delay);
	}
	if (member_ptr == &use_component_colors) {
		surfel_style.use_group_color = use_component_colors;
		update_member(&surfel_style.use_group_color);
	}
	if (member_ptr == &use_component_transformations) {
		surfel_style.use_group_transformation = use_component_transformations;
		update_member(&surfel_style.use_group_transformation);
	}
	update_member(member_ptr);
	post_redraw();
}
void point_cloud_interactable::create_gui()
{
	add_decorator(get_name(), "heading", "level=2");
	bool show = begin_tree_node("IO", data_path, true, "level=3;options='w=40';align=' '");
	add_member_control(this, "append", do_append, "toggle", "w=60", " ");
	add_member_control(this, "auto_view", do_auto_view, "toggle", "w=80");

	if (show) {
		align("\a");
		add_gui("data_path", data_path, "directory", "w=170;title='Select Data Directory'");
		add_gui("file_name", new_file_name, "file_name",
			"w=130;"
			"open=true;open_title='" FILE_OPEN_TITLE "';open_filter='" FILE_OPEN_FILTER "';"
			"save=true;save_title='" FILE_SAVE_TITLE "';save_filter='" FILE_SAVE_FILTER "'"
		);
		add_gui("directory_name", directory_name, "directory", "w=170;title='Select Data Directory';tooltip='read all point clouds from a directory'");
		align("\b");
		end_tree_node(data_path);
	}
	show = begin_tree_node("points", show_points, false, "level=3;options='w=70';align=' '");
	add_member_control(this, "show", show_points, "toggle", "w=40", " ");
	add_member_control(this, "sort", sort_points, "toggle", "w=40", " ");
	add_member_control(this, "blnd", surfel_style.blend_points, "toggle", "w=40");
	if (show) {
		align("\a");
		add_gui("surfel_style", surfel_style);
		if (begin_tree_node("subsample", show_point_step, false, "level=3")) {
			align("\a");
			add_member_control(this, "nr_draw_calls", nr_draw_calls, "value_slider", "min=1;max=100;log=true;ticks=true");
			add_member_control(this, "interact step", interact_point_step, "value_slider", "min=1;max=100;log=true;ticks=true");
			add_member_control(this, "interact delay", interact_delay, "value_slider", "min=0.01;max=1;log=true;ticks=true");
			add_member_control(this, "show step", show_point_step, "value_slider", "min=1;max=20;log=true;ticks=true");
			add_decorator("range control", "heading", "level=3");
			add_member_control(this, "begin", show_point_begin, "value_slider", "min=0;max=10;ticks=true");
			add_member_control(this, "end", show_point_end, "value_slider", "min=0;max=10;ticks=true");
			add_decorator("window control", "heading", "level=3");
			add_member_control(this, "start", show_point_start, "value_slider", "min=0;max=10;ticks=true");
			add_member_control(this, "width", show_point_count, "value_slider", "min=0;max=10;ticks=true");
			configure_subsample_controls();
			align("\b");
			end_tree_node(show_point_step);
		}
		add_member_control(this, "accelerate_picking", accelerate_picking, "check");
		align("\b");
		end_tree_node(show_points);
	}
	show = begin_tree_node("components", pc.components, false, "level=3;options='w=140';align=' '");
	add_member_control(this, "clr", use_component_colors, "toggle", "w=30", " ");
	add_member_control(this, "tra", use_component_transformations, "toggle", "w=30");
	if (show) {
		align("\a");
		if (begin_tree_node("component colors", pc.component_colors, false, "level=3")) {
			align("\a");
			for (unsigned i = 0; i < pc.component_colors.size(); ++i) {
				add_member_control(this, std::string("C") + cgv::utils::to_string(i), pc.component_colors[i]);
			}
			align("\b");
			end_tree_node(pc.component_colors);
		}
		if (begin_tree_node("group transformations", pc.component_translations, false, "level=3")) {
			align("\a");
			for (unsigned i = 0; i < pc.component_translations.size(); ++i) {
				add_gui(std::string("T") + cgv::utils::to_string(i), pc.component_translations[i]);
				add_gui(std::string("Q") + cgv::utils::to_string(i), (HVec&)pc.component_rotations[i], "direction");
			}
			align("\b");
			end_tree_node(pc.component_translations);
		}
		align("\b");
		end_tree_node(pc.components);
	}
	show = begin_tree_node("neighbor graph", show_neighbor_graph, false, "level=3;options='w=160';align=' '");
	add_member_control(this, "show", show_neighbor_graph, "toggle", "w=50");
	if (show) {
		add_member_control(this, "k", k, "value_slider", "min=3;max=50;log=true;ticks=true");
		add_member_control(this, "symmetrize", do_symmetrize, "toggle");
		cgv::signal::connect_copy(add_button("build")->click, cgv::signal::rebind(this, &point_cloud_interactable::build_neighbor_graph));
		end_tree_node(show_neighbor_graph);
	}

	show = begin_tree_node("normals", show_nmls, false, "level=3;options='w=160';align=' '");
	add_member_control(this, "show", show_nmls, "toggle", "w=50");
	if (show) {
		cgv::signal::connect_copy(add_button("toggle orientation")->click, cgv::signal::rebind(this, &point_cloud_interactable::toggle_normal_orientations));
		add_member_control(this, "bilateral weight approach", ne.bw_type, "dropdown", "enums='normal,plane'");
		add_member_control(this, "localization_scale", ne.localization_scale, "value_slider", "min=0.1;max=2;log=true;ticks=true");
		add_member_control(this, "normal_sigma", ne.normal_sigma, "value_slider", "min=0.01;max=2;log=true;ticks=true");
		add_member_control(this, "plane_distance_scale", ne.plane_distance_scale, "value_slider", "min=0.01;max=2;log=true;ticks=true");
		add_member_control(this, "reorient", reorient_normals, "toggle");
		cgv::signal::connect_copy(add_button("compute")->click, cgv::signal::rebind       (this, &point_cloud_interactable::compute_normals));
		cgv::signal::connect_copy(add_button("recompute")->click, cgv::signal::rebind     (this, &point_cloud_interactable::recompute_normals));
		cgv::signal::connect_copy(add_button("orient")->click, cgv::signal::rebind        (this, &point_cloud_interactable::orient_normals));
		cgv::signal::connect_copy(add_button("orient to view")->click, cgv::signal::rebind(this, &point_cloud_interactable::orient_normals_to_view_point));
		add_gui("normal_style", normal_style);
		end_tree_node(show_nmls);
	}

/*	show = begin_tree_node("surfrec", show_surfrec, false, "level=3;w=100;align=' '");
	add_member_control(this, "show", show_surfrec, "toggle", "w=50");
	if (show) {
		add_member_control(this, "debug_mode", SR.debug_mode, "enums='none,symmetrize,make_consistent,cycle_filter'");
		end_tree_node(show_surfrec);
	}
	*/
	show = begin_tree_node("box", show_box, false, "level=3;options='w=140';align=' '");
	add_member_control(this, "main", show_box, "toggle", "w=30", " ");
	add_member_control(this, "com", show_boxes, "toggle", "w=30");
	if (show) {
		add_gui("color", box_color);
		add_gui("box_style", box_style);
		add_gui("box_wire_style", box_wire_style);
		end_tree_node(show_box);
	}
}

