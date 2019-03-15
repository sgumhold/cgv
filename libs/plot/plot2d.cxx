#include "plot2d.h"
#include <libs/cgv_gl/gl/gl.h>
#include <libs/cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace plot {

plot2d_config::plot2d_config()
{
	samples_per_row = 0;

	show_faces = true;
	face_color = rgb(0.7f,0.4f,0);

	face_illumination = PFI_PER_FACE;
}

void plot2d::set_uniforms(cgv::render::context& ctx, cgv::render::shader_program& prog, unsigned i)
{
	plot_base::set_uniforms(ctx, prog, i);
	prog.set_uniform(ctx, "face_color", ref_sub_plot2d_config(i).face_color);
	prog.set_uniform(ctx, "N", ref_sub_plot2d_config(i).samples_per_row);
	prog.set_uniform(ctx, "face_illumination", (int&) ref_sub_plot2d_config(i).face_illumination);
}

/// construct empty plot with default domain [0..1,0..1,0..1]
plot2d::plot2d()
{
	domain.ref_min_pnt() = vec3(0,0,0);
	domain.ref_max_pnt() = vec3(1,1,1);

	axis_directions[0] = vec3(1,0,0);
	axis_directions[1] = vec3(0, 1, 0);
	axis_directions[2] = vec3(0, 0, 2);
	center_location = vec3(0,0,0);

	axes[0].ticks[1].type = TT_LINE;
	axes[1].ticks[1].type = TT_LINE;
	axes[2].ticks[1].type = TT_LINE;
}

/// return number of axis
unsigned plot2d::get_nr_axes() const
{
	return 3;
}

/// adjust domain to data
void plot2d::adjust_domain_to_data(bool include_xy_plane)
{
	// compute bounding box
	domain = box3(samples.front().front(),samples.front().front());
	for (unsigned i=0; i<samples.size(); ++i) {
		for (unsigned j=0; j<samples[i].size(); ++j) {
			domain.add_point(samples[i][j]);
		}
	}
	if (include_xy_plane)  {
		if (domain.get_min_pnt()(2) > 0)
			domain.ref_min_pnt()(2) = 0;
		if (domain.get_max_pnt()(2) < 0)
			domain.ref_max_pnt()(2) = 0;
	}
}

unsigned plot2d::add_sub_plot(const std::string& name)
{
	// determine index of new sub plot
	unsigned i = get_nr_sub_plots();

	// create new config
	if (i == 0)
		configs.push_back(new plot2d_config());
	else
		configs.push_back(new plot2d_config(ref_sub_plot2d_config(i-1)));
	ref_sub_plot_config(i).name = name;

	// create new point container
	samples.push_back(std::vector<vec3>());

	// return sub plot index
	return i;
}

void plot2d::delete_sub_plot(unsigned i)
{
	delete configs[i];
	configs[i] = 0;
	configs.erase(configs.begin() + i);
	samples.erase(samples.begin() + i);
}

/// set the number of samples of the i-th sub plot to N
void plot2d::set_samples_per_row(unsigned N, unsigned i)
{
	ref_sub_plot2d_config(i).samples_per_row = N;
}

/// return the number of samples per row
unsigned plot2d::get_samples_per_row(unsigned i) const
{
	return const_cast<plot2d*>(this)->ref_sub_plot2d_config(i).samples_per_row;
}

/// return a reference to the plot base configuration of the i-th plot
plot2d_config& plot2d::ref_sub_plot2d_config(unsigned i)
{
	return static_cast<plot2d_config&>(ref_sub_plot_config(i));
}


/// return the samples of the i-th sub plot
std::vector<plot2d::vec3>& plot2d::ref_sub_plot_samples(unsigned i)
{
	return samples[i];
}


bool plot2d::init(cgv::render::context& ctx)
{
	return true;
}


void plot2d::draw(cgv::render::context& ctx)
{
	if (!point_prog.is_created()) {
		if (!point_prog.build_program(ctx, "plot2d_point.glpr")) {
			std::cerr << "could not build GLSL program from plot2d_point.glpr" << std::endl;
		}
	}
	if (!line_prog.is_created()) {
		if (!line_prog.build_program(ctx, "plot2d_line.glpr")) {
			std::cerr << "could not build GLSL program from plot2d_line.glpr" << std::endl;
		}
	}
/*	if (!stick_prog.build_program(ctx, "plot2d_stick.glpr")) {
		std::cerr << "could not build GLSL program from plot2d_stick.glpr" << std::endl;
		return false;
	}
	if (!stick_prog.build_program(ctx, "plot2d_bar.glpr")) {
		std::cerr << "could not build GLSL program from plot2d_bar.glpr" << std::endl;
		return false;
	}
	*/
	if (!face_prog.is_created()) {
		if (!face_prog.build_program(ctx, "plot2d_face.glpr")) {
			std::cerr << "could not build GLSL program from plot2d_face.glpr" << std::endl;
		}
	}
	for (unsigned i=0; i<samples.size(); ++i) {
		glVertexPointer(3, GL_FLOAT, 0, &samples[i].front());
		glEnableClientState(GL_VERTEX_ARRAY);

		if (ref_sub_plot_config(i).show_points) {
			set_uniforms(ctx, point_prog, i);
			glPointSize(ref_sub_plot_config(i).point_size);
			if (ref_sub_plot_config(i).point_size > 1)
				glEnable(GL_POINT_SMOOTH);
			point_prog.enable(ctx);
			glDrawArrays(GL_POINTS, 0, samples[i].size());
			point_prog.disable(ctx);
			if (ref_sub_plot_config(i).point_size > 1)
				glDisable(GL_POINT_SMOOTH);
		}
		
		if (ref_sub_plot_config(i).show_sticks) {
			set_uniforms(ctx, line_prog, i);
			glLineWidth(ref_sub_plot_config(i).stick_width);
			line_prog.enable(ctx);
			unsigned M = samples[i].size() / get_samples_per_row(i);
			for (unsigned v=0; v<M; ++v)
				glDrawArrays(GL_LINE_STRIP, v*get_samples_per_row(i), get_samples_per_row(i));
			line_prog.disable(ctx);
		}
		
/*		if (ref_plot_config(i).show_sticks) {
			set_uniforms(ctx, stick_prog, i);
			stick_prog.enable(ctx);
			glDrawArrays(GL_POINTS, 0, samples[i].size());
			stick_prog.disable(ctx);
		}
		
		if (ref_plot_config(i).show_bars) {
			set_uniforms(ctx, bar_prog, i);
			bar_prog.enable(ctx);
			glDrawArrays(GL_POINTS, 0, samples[i].size());
			bar_prog.disable(ctx);
		}
		*/
		if (ref_sub_plot2d_config(i).show_faces) {
			glDisable(GL_CULL_FACE);
//			glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
			set_uniforms(ctx, face_prog, i);
			cgv::render::gl::set_lighting_parameters(ctx, face_prog);
			std::vector<unsigned> indices;
			indices.resize(2*get_samples_per_row(i));
			unsigned M = samples[i].size() / get_samples_per_row(i);
			face_prog.enable(ctx);
			for (unsigned v=1; v<M; ++v) {
				for (unsigned u=0; u<get_samples_per_row(i); ++u) {
					indices[2*u] = u+(v-1)*get_samples_per_row(i);
					indices[2*u+1] = u+v*get_samples_per_row(i);
				}
				glDrawElements(GL_TRIANGLE_STRIP, 2*get_samples_per_row(i), GL_UNSIGNED_INT, &indices.front());
			}
			face_prog.disable(ctx);
			glEnable(GL_CULL_FACE);
		}
		
		glDisableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, 0);
	}
}

void plot2d::clear(cgv::render::context& ctx)
{
	point_prog.destruct(ctx);
	line_prog.destruct(ctx);
	face_prog.destruct(ctx);
}

void plot2d::create_config_gui(cgv::base::base* bp, cgv::gui::provider& p, unsigned i)
{
	plot2d_config& pbc = ref_sub_plot2d_config(i);

	p.add_decorator("faces", "heading", "level=3;w=100", " ");
	p.add_member_control(bp, "show",  pbc.show_faces, "toggle", "w=50");
	p.add_member_control(bp, "illum", pbc.face_illumination, "dropdown", "enums='none,per face,per vertex'");
	p.add_member_control(bp, "color", pbc.face_color);

	plot_base::create_config_gui(bp, p, i);
}


	}
}