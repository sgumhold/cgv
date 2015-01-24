#include "gl_performance_monitor.h"
#include <cgv_gl/gl/gl.h>

namespace cgv {
	namespace render {
		namespace gl {

gl_performance_monitor::gl_performance_monitor()
{
}

void gl_performance_monitor::prepare_draw_lines()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
}

void gl_performance_monitor::draw_lines()
{

	glVertexPointer(2, GL_INT, 0, &positions[0]);
	glColorPointer(3, GL_FLOAT, 0, &colors[0]);
	glDrawArrays(GL_LINES, 0, (GLsizei) positions.size());
}

void gl_performance_monitor::finish_draw_lines()
{
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}


void gl_performance_monitor::draw_bar(int y, const frame_data& fdata)
{
	compute_colors(fdata);
	compute_positions(placement.get_min_pnt()(0),y,placement.get_extent()(0)/nr_display_cycles, 0, fdata);
	draw_lines();
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
		std::cout << (glIsEnabled(GL_TEXTURE_COORD_ARRAY)?("0"+i):".");
	}
	std::cout << std::endl;
}

void gl_performance_monitor::draw(cgv::render::context& ctx)
{
	int xmin     = placement.get_min_pnt()(0);
	int ymin     = placement.get_min_pnt()(1);
	int xmax     = placement.get_max_pnt()(0);
	int ymax     = placement.get_max_pnt()(1);
	int w        = xmax - xmin + 1;
	int h        = ymax - ymin + 1;
	int dy       = h/nr_display_cycles;
	
	ctx.push_pixel_coords();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	// draw plot lines 
	glLineWidth(1.0f);
	glColor3fv(&plot_color[0]);
	glBegin(GL_LINE_LOOP);
	glVertex2i(xmin-1, ymax+1);
	glVertex2i(xmax+1, ymax+1);
	glVertex2i(xmax+1, ymin-1);
	glVertex2i(xmin-1, ymin-1);
	glEnd();

	if (nr_display_cycles > 1) {
		int y  = ymax-dy;
		glBegin(GL_LINES);
		for (int i=1; i<nr_display_cycles; ++i) {
			glVertex2i(xmin, y);
			glVertex2i(xmax, y);
			y -= dy;
		}
		glEnd();
	}

	if (!data.empty()) {
		// draw history of frames and determine local frame index of min and max
		int min_i = -1, max_i = -1;
		if (data.size() > 1) {

			prepare_draw_lines();
			// go through all but current frame 
			int x = xmin;
			for (unsigned i=0; i < data.size()-1; ++i, ++x) {
				const frame_data&  fdata = data[i];
				compute_colors(fdata);
				compute_positions(x,ymax,0,-dy, fdata);
				// draw
				draw_lines();
				// check for min
				if (min_i == -1 || data[i][data[i].size()-2].time < data[min_i][data[min_i].size()-2].time)
					min_i = i;
				// check for max
				if (max_i == -1 || data[i][data[i].size()-2].time > data[max_i][data[max_i].size()-2].time)
					max_i = i;
			}
			if (!bar_config.empty()) {
				int y = ymax + (bar_line_width+4)/2;
				glLineWidth((float)bar_line_width);
				for (int c=0; c < (int)bar_config.size(); ++c) {
					switch (bar_config[c]) {
					case PMB_MIN :
						if (min_i != -1)
							draw_bar(y, data[min_i]);
						break;
					case PMB_MAX :
						if (max_i != -1)
							draw_bar(y, data[max_i]);
						break;
					case PMB_CUR :
						draw_bar(y, data.back());
						break;
					case PMB_AVG :
						std::cerr << "avg not yet supported" << std::endl;
						break;
					}
					y += bar_line_width+1;
				}
			}
			finish_draw_lines();
		}
	}
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glLineWidth(1.0f);
	ctx.pop_pixel_coords();
}

		}
	}
}