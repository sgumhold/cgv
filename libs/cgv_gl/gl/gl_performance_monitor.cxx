#include "gl_performance_monitor.h"
#include <cgv_gl/gl/gl.h>
#include <cgv/render/attribute_array_binding.h>
#include <cgv/render/shader_program.h>

namespace cgv {
	namespace render {
		namespace gl {

gl_performance_monitor::gl_performance_monitor()
{
}
void gl_performance_monitor::draw_computed_bars(cgv::render::context& ctx, cgv::render::shader_program& prog)
{
	if (positions.empty())
		return;
	std::vector<vec2> P;
	P.clear();
	for (const auto& p : positions)
		P.push_back(vec2(p));
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_position_index(), P);
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_color_index(), colors);
	glDrawArrays(GL_LINES, 0, (GLsizei)positions.size());
}

void gl_performance_monitor::draw_bars(cgv::render::context& ctx, cgv::render::shader_program& prog)
{
	cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_color_index());

	float xmin = (float)placement.get_min_pnt()(0);
	float ymin = (float)placement.get_min_pnt()(1);
	float xmax = (float)placement.get_max_pnt()(0);
	float ymax = (float)placement.get_max_pnt()(1);
	float w = xmax - xmin + 1;
	float h = ymax - ymin + 1;
	int dy = (int)(h / nr_display_cycles);

	std::swap(ymin, ymax);
	ymin = ctx.get_height() - 1 - ymin;
	ymax = ctx.get_height() - 1 - ymax;

	if (data.empty())
		return;
	// draw history of frames and determine local frame index of min and max
	int min_i = -1, max_i = -1;
	if (data.size() > 1) {
		// go through all but current frame 
		int x = placement.get_min_pnt()(0);
		for (unsigned i = 0; i < data.size() - 1; ++i, ++x) {
			const frame_data&  fdata = data[i];
			compute_colors(fdata);
			compute_positions(x, int(ymin), 0, dy, fdata);
			draw_computed_bars(ctx, prog);
			// check for min
			if (min_i == -1 || data[i][data[i].size() - 2].time < data[min_i][data[min_i].size() - 2].time)
				min_i = i;
			// check for max
			if (max_i == -1 || data[i][data[i].size() - 2].time > data[max_i][data[max_i].size() - 2].time)
				max_i = i;
		}
		if (!bar_config.empty()) {
			int y = int(ymin - (bar_line_width + 4) / 2 - 4);
			glLineWidth((float)bar_line_width);
			for (int c = 0; c < (int)bar_config.size(); ++c) {
				switch (bar_config[c]) {
				case PMB_MIN:
					if (min_i != -1)
						draw_bar(ctx, prog, y, data[min_i]);
					break;
				case PMB_MAX:
					if (max_i != -1)
						draw_bar(ctx, prog, y, data[max_i]);
					break;
				case PMB_CUR:
					draw_bar(ctx, prog, y, data.back());
					break;
				case PMB_AVG:
					std::cerr << "avg not yet supported" << std::endl;
					break;
				}
				y -= bar_line_width + 1;
			}
		}
	}
	cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_color_index());
	glLineWidth(1.0f);
}
void gl_performance_monitor::draw_bar(cgv::render::context& ctx, cgv::render::shader_program& prog, int y, const frame_data& fdata)
{
	compute_colors(fdata);
	compute_positions(placement.get_min_pnt()(0),y,placement.get_extent()(0)/nr_display_cycles, 0, fdata);
	draw_computed_bars(ctx, prog);
}

void checkClientState()
{
	GLint at;
	glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, &at);
	std::cout << "AT="
		<< at << " ";
	std::cout << (glIsEnabled(GL_VERTEX_ARRAY)?"V":"v") 
			  << (glIsEnabled(GL_NORMAL_ARRAY)?"N":"n")
			  << (glIsEnabled(GL_COLOR_ARRAY)?"C":"c");

	for (int i=0; i<8; ++i) {
		glClientActiveTexture(GL_TEXTURE0+i);
		std::cout << (glIsEnabled(GL_TEXTURE_COORD_ARRAY)?("0"+std::to_string(i)):".");
	}
	std::cout << std::endl;
}
void gl_performance_monitor::draw_lines(cgv::render::context& ctx, cgv::render::shader_program& prog)
{
	// collect lines first
	std::vector<vec2> lines;
	float xmin = (float)placement.get_min_pnt()(0);
	float ymin = (float)placement.get_min_pnt()(1);
	float xmax = (float)placement.get_max_pnt()(0);
	float ymax = (float)placement.get_max_pnt()(1);
	float w = xmax - xmin + 1;
	float h = ymax - ymin + 1;
	float dy = h / nr_display_cycles;

	std::swap(ymin, ymax);
	ymin = ctx.get_height() - 1 - ymin;
	ymax = ctx.get_height() - 1 - ymax;

	lines.push_back(vec2(xmin - 1, ymax + 1));
	lines.push_back(vec2(xmax + 1, ymax + 1));
	lines.push_back(vec2(xmax + 1, ymax + 1));
	lines.push_back(vec2(xmax + 1, ymin - 1));
	lines.push_back(vec2(xmax + 1, ymin - 1));
	lines.push_back(vec2(xmin - 1, ymin - 1));
	lines.push_back(vec2(xmin - 1, ymin - 1));
	lines.push_back(vec2(xmin - 1, ymax + 1));

	if (nr_display_cycles > 1) {
		float y = ymax - dy;
		for (int i = 1; i < nr_display_cycles; ++i) {
			lines.push_back(vec2(xmin, y));
			lines.push_back(vec2(xmax, y));
			y -= dy;
		}
	}

	ctx.set_color(plot_color);
	cgv::render::attribute_array_binding::set_global_attribute_array(ctx, prog.get_position_index(), lines);
	glDrawArrays(GL_LINES, 0, (GLsizei) lines.size());
}

void gl_performance_monitor::draw(cgv::render::context& ctx)
{
	//int xmin     = placement.get_min_pnt()(0);
	//int ymin     = placement.get_min_pnt()(1);
	//int xmax     = placement.get_max_pnt()(0);
	//int ymax     = placement.get_max_pnt()(1);
	//int w        = xmax - xmin + 1;
	//int h        = ymax - ymin + 1;
	//int dy       = h/nr_display_cycles;

	GLboolean is_depth = glIsEnabled(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	ctx.push_pixel_coords();
		GLfloat lw;
		glGetFloatv(GL_LINE_WIDTH, &lw);
		glLineWidth(1.0f);
		cgv::render::shader_program& prog = ctx.ref_default_shader_program();
		cgv::render::attribute_array_binding::enable_global_array(ctx, prog.get_position_index());
		prog.enable(ctx);
			draw_bars(ctx, prog);
			draw_lines(ctx, prog);
		prog.disable(ctx);
		cgv::render::attribute_array_binding::disable_global_array(ctx, prog.get_position_index());
		glLineWidth(lw);
	ctx.pop_pixel_coords();
	if (is_depth)
		glEnable(GL_DEPTH_TEST);
}

		}
	}
}