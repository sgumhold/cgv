#include "debug_draw.h"

#include <cgv_gl/gl/gl.h>

namespace trajectory {
namespace util {
	namespace debug {
		void draw_point(const point_t &point, const float size)
		{
			glPointSize(size);

			glColor3f(point.col.x(), point.col.y(), point.col.z());
			glBegin(GL_POINTS);
			glVertex3f(point.p.x(), point.p.y(), point.p.z());
			glEnd();
		}

		void draw_line(const line_t &line)
		{
			glLineWidth(3.0f);

			glBegin(GL_LINES);
			glColor3f(1.0f, 1.0f, 0.0f);
			glVertex3f(line.a.x(), line.a.y(), line.a.z());
			glColor3f(line.col.x(), line.col.y(), line.col.z());
			glVertex3f(line.b.x(), line.b.y(), line.b.z());
			glEnd();
		}

		void draw_quad(const rect_t &rect)
		{
			glColor4f(rect.col.x(), rect.col.y(), rect.col.z(), rect.col.w());
			glBegin(GL_QUADS);
			glVertex3f(rect.rect[2].x(), rect.rect[2].y(), rect.rect[2].z());
			glVertex3f(rect.rect[3].x(), rect.rect[3].y(), rect.rect[3].z());
			glVertex3f(rect.rect[1].x(), rect.rect[1].y(), rect.rect[1].z());
			glVertex3f(rect.rect[0].x(), rect.rect[0].y(), rect.rect[0].z());

			glEnd();
		}
	} // namespace debug
} // namespace util
} // namespace trajectory