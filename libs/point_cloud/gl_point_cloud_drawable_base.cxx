#include "gl_point_cloud_drawable_base.h"
#include <cgv/utils/file.h>
#include <cgv/utils/scan.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>
#include <cgv/base/import.h>

using namespace std;
using namespace cgv::render;
using namespace cgv::utils::file;

gl_point_cloud_drawable_base::gl_point_cloud_drawable_base() 
{
	view_ptr = 0;

	show_point_step = 1;
	show_point_begin = 0;
	show_point_end = 0;

	sort_points = true;
	show_points = true;
	show_nmls = true;
	show_boxes = true;

	box_color = color_type(0.5f, 0.5f, 0.5f, 1.0f);
	box_style.illumination_mode = cgv::render::IM_TWO_SIDED;
	box_style.culling_mode = cgv::render::CM_FRONTFACE;
}

bool gl_point_cloud_drawable_base::read(const std::string& _file_name)
{
	std::string fn = cgv::base::find_data_file(_file_name, "cpD");
	if (fn.empty()) {
		cerr << "point cloud " << _file_name << " not found" << endl;
		return false;
	}
	if (!pc.read(fn)) {
		cerr << "could not read point cloud " << fn << endl;
		return false;
	}
	file_name = drop_extension(_file_name);
	show_point_begin = 0;
	show_point_end = pc.get_nr_points();

	// clear component info
	use_component_colors = pc.has_component_colors();
	use_component_transformations = pc.has_component_transformations();

	post_redraw();
	return true;
}

bool gl_point_cloud_drawable_base::append(const std::string& _file_name, bool add_component)
{
	std::string fn = cgv::base::find_data_file(_file_name, "cpD");
	if (fn.empty()) {
		cerr << "point cloud " << _file_name << " not found" << endl;
		return false;
	}
	point_cloud pc1;
	if (!pc1.read(fn)) {
		cerr << "could not read point cloud " << fn << endl;
		return false;
	}
	if (!file_name.empty())
		file_name += " and "; 
	file_name += drop_extension(fn);

	// construct component if necessary
	if (add_component) {
		if (!pc.has_components()) {
			pc.create_components();
			pc.create_component_colors();
			pc.create_component_tranformations();
		}
		if (!pc1.has_components())
			pc.add_component();
	}
	pc.append(pc1);
	component_show_flags.resize(pc.get_nr_components());
	std::fill(component_show_flags.begin(), component_show_flags.end(), true);

	show_point_begin = 0;
	show_point_end = pc.get_nr_points();
	return true;
}

bool gl_point_cloud_drawable_base::write(const std::string& fn)
{
	if (!pc.write(fn)) {
		cerr << "could not write point cloud " << fn << endl;
		return false;
	}
	file_name = drop_extension(fn);
	return true;
}

void gl_point_cloud_drawable_base::render_boxes(context& ctx, group_renderer& R, cgv::render::group_render_style& RS)
{
	R.set_position_array(ctx, &pc.box(0).get_min_pnt(), pc.get_nr_components(), sizeof(Box));
	R.set_color_array(ctx, &pc.component_color(0), pc.get_nr_components());
	R.set_group_rotations(ctx, &pc.component_rotation(0), pc.get_nr_components());
	R.set_group_translations(ctx, &pc.component_translation(0), pc.get_nr_components());
	bool tmp1 = RS.use_group_color;
	RS.use_group_color = false;
	R.validate_and_enable(ctx);
	glDrawArrays(GL_POINTS, 0, pc.get_nr_components());
	R.disable(ctx);
	RS.use_group_color = tmp1;
}

void gl_point_cloud_drawable_base::draw_box(cgv::render::context& ctx, const Box& box, const color_type& clr)
{
	b_renderer.set_position_array(ctx, &box.get_min_pnt(), 1);
	b_renderer.set_extent_array(ctx, &box.get_max_pnt(), 1);
	b_renderer.set_color_array(ctx, &clr, 1);
	bool tmp1 = box_style.use_group_color;
	bool tmp2 = box_style.use_group_transformation;
	box_style.use_group_color = false;
	box_style.use_group_transformation = false;
	b_renderer.validate_and_enable(ctx);
	glDrawArrays(GL_POINTS, 0, 1);
	b_renderer.disable(ctx);
	box_style.use_group_color = tmp1;
	box_style.use_group_transformation = tmp2;

	bw_renderer.set_position_array(ctx, &box.get_min_pnt(), 1);
	bw_renderer.set_extent_array(ctx, &box.get_max_pnt(), 1);
	bw_renderer.set_color_array(ctx, &clr, 1);
	tmp1 = box_wire_style.use_group_color;
	tmp2 = box_wire_style.use_group_transformation;
	box_wire_style.use_group_color = false;
	box_wire_style.use_group_transformation = false;
	bw_renderer.validate_and_enable(ctx);
	glDrawArrays(GL_POINTS, 0, 1);
	bw_renderer.disable(ctx);
	box_style.use_group_color = tmp1;
	box_style.use_group_transformation = tmp2;
}

void gl_point_cloud_drawable_base::draw_boxes(context& ctx)
{
	if (!show_boxes)
		return;

	draw_box(ctx, pc.box(), box_color);

	if (pc.has_components()) {
		Box b;
		for (unsigned ci = 0; ci < pc.get_nr_components(); ++ci)
			b.add_axis_aligned_box(pc.box(ci));

		b_renderer.set_extent_array(ctx, &pc.box(0).get_max_pnt(), pc.get_nr_components(), sizeof(Box));
		render_boxes(ctx, b_renderer, box_style);

		bw_renderer.set_extent_array(ctx, &pc.box(0).get_max_pnt(), pc.get_nr_components(), sizeof(Box));
		render_boxes(ctx, bw_renderer, box_wire_style);
	}
}

void gl_point_cloud_drawable_base::draw_points(context& ctx)
{
	if (!ensure_view_pointer())
		exit(0);

	if (!show_points)
		return;

	if (pc.has_components()) {
		p_renderer.set_group_colors(ctx, &pc.component_color(0), pc.get_nr_components());
		p_renderer.set_group_rotations(ctx, &pc.component_rotation(0), pc.get_nr_components());
		p_renderer.set_group_translations(ctx, &pc.component_translation(0), pc.get_nr_components());
		p_renderer.set_group_index_attribute(ctx, &pc.component_index(0), pc.get_nr_points());
	}
	p_renderer.set_position_array(ctx, &pc.pnt(0), pc.get_nr_points(), sizeof(Pnt)*show_point_step);
	if (pc.has_colors())
		p_renderer.set_color_array(ctx, &pc.clr(0), pc.get_nr_points(), sizeof(Clr)*show_point_step);
	if (pc.has_normals())
		p_renderer.set_normal_array(ctx, &pc.nml(0), pc.get_nr_points(), sizeof(Nml)*show_point_step);
	p_renderer.validate_and_enable(ctx);

	std::size_t n = (show_point_end - show_point_begin) / show_point_step;
	GLint offset = show_point_begin / show_point_step;

	if (sort_points && ensure_view_pointer()) {
		struct sort_pred {
			const point_cloud& pc;
			const Pnt& view_dir;
			unsigned show_point_step;
			bool operator () (GLint i, GLint j) const {
				return dot(pc.pnt(show_point_step*i), view_dir) > dot(pc.pnt(show_point_step*j), view_dir);
			}
			sort_pred(const point_cloud& _pc, const Pnt& _view_dir, unsigned _show_point_step) : pc(_pc), view_dir(_view_dir), show_point_step(_show_point_step) {}
		};
		Pnt view_dir = view_ptr->get_view_dir();
		std::vector<GLint> indices;
		indices.resize(n);
		size_t i;
		for (i = 0; i < indices.size(); ++i)
			indices[i] = (GLint)i + offset;
		std::sort(indices.begin(), indices.end(), sort_pred(pc, view_dir, show_point_step));

		glDepthFunc(GL_ALWAYS);
		glDrawElements(GL_POINTS, n, GL_UNSIGNED_INT, &indices[0]);
		glDepthFunc(GL_LESS);
	}
	else {
		glDrawArrays(GL_POINTS, offset, n);
	}

	p_renderer.disable(ctx);
}

void gl_point_cloud_drawable_base::draw_normals(context& ctx)
{
	if (!show_nmls || !pc.has_normals())
		return;
	n_renderer.set_normal_scale(pc.box().get_extent().length() / sqrt(float(pc.get_nr_points())));
	n_renderer.set_position_array(ctx, &pc.pnt(0), pc.get_nr_points(), sizeof(Pnt)*show_point_step);
	if (pc.has_colors())
		n_renderer.set_color_array(ctx, &pc.clr(0), pc.get_nr_points(), sizeof(Clr)*show_point_step);
	if (pc.has_normals())
		n_renderer.set_normal_array(ctx, &pc.nml(0), pc.get_nr_points(), sizeof(Nml)*show_point_step);
	n_renderer.validate_and_enable(ctx);
	std::size_t n = (show_point_end - show_point_begin) / show_point_step;
	GLint offset = show_point_begin / show_point_step;
	glDrawArrays(GL_POINTS, offset, n);
	n_renderer.disable(ctx);
}


bool gl_point_cloud_drawable_base::init(cgv::render::context& ctx)
{
	if (!p_renderer.init(ctx))
		return false;
	p_renderer.set_render_style(point_style);
	if (!n_renderer.init(ctx))
		return false;
	n_renderer.set_render_style(normal_style);
	if (!b_renderer.init(ctx))
		return false;
	b_renderer.set_render_style(box_style);
	b_renderer.set_position_is_center(false);
	if (!bw_renderer.init(ctx))
		return false;
	bw_renderer.set_render_style(box_wire_style);
	bw_renderer.set_position_is_center(false);
	return true;
}

void gl_point_cloud_drawable_base::clear(cgv::render::context& ctx)
{
	p_renderer.clear(ctx);
	n_renderer.clear(ctx);
	b_renderer.clear(ctx);
	bw_renderer.clear(ctx);
}

void gl_point_cloud_drawable_base::draw(context& ctx)
{
	if (pc.get_nr_points() == 0)
		return;

	draw_boxes(ctx);
	draw_normals(ctx);
	draw_points(ctx);
}



#include <cgv/base/find_action.h>
#include <cgv/render/view.h>

bool gl_point_cloud_drawable_base::ensure_view_pointer()
{
	cgv::base::base_ptr bp(dynamic_cast<cgv::base::base*>(this));
	if (bp) {
		vector<cgv::render::view*> views;
		cgv::base::find_interface<cgv::render::view>(bp, views);
		if (!views.empty()) {
			view_ptr = views[0];
			return true;
		}
	}
	return false;
}

#ifdef REGISTER_SHADER_FILES
#include <cgv/base/register.h>
#include <point_cloud_shader_inc.h>
#endif
