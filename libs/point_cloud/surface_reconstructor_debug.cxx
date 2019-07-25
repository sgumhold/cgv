
#include "surface_reconstructor.h"
#include <libs/cgv_gl/gl/gl.h>
#include <libs/cgv_gl/gl/gl_tools.h>


void surface_reconstructor::draw_colored_point(const Pnt& p, float size, float r, float g, float b) const
{
	glColor3f(r,g,b);
	glPointSize(size*point_size);
	glBegin(GL_POINTS);
	glVertex3fv(&p[0]);
	glEnd();
}

void surface_reconstructor::draw_colored_point(unsigned int vi, float size, float r, float g, float b) const
{
	draw_colored_point(pc->pnt(vi),size,r,g,b);
}
void surface_reconstructor::draw_colored_corner(unsigned int vi, unsigned int j, float size, float r, float g, float b, float weight) const
{
	draw_colored_point(compute_corner_point(vi,j,weight),size,r,g,b);
}


void surface_reconstructor::draw_edge(unsigned int vi, unsigned int vj, float width) const
{
	glLineWidth(width*line_width);
	glBegin(GL_LINES);
	glVertex3fv(&pc->pnt(vi)[0]);
	glVertex3fv(&pc->pnt(vj)[0]);
	glEnd();
}

void surface_reconstructor::draw_colored_edge(const Pnt& p, const Pnt& q, float width, float r, float g, float b) const
{
	glColor3f(r,g,b);
	glLineWidth(width*line_width);
	glBegin(GL_LINES);
	glVertex3fv(p);
	glVertex3fv(q);
	glEnd();
}


void surface_reconstructor::draw_colored_edge(unsigned int vi, unsigned int vj, float width, float r, float g, float b) const
{
	draw_colored_edge(pc->pnt(vi),pc->pnt(vj),width,r,g,b);
}

void surface_reconstructor::draw_colored_edge(unsigned int vi, unsigned int vj, float width, 
															 float ri, float gi, float bi, float rj, float gj, float bj) const
{
	glLineWidth(width*line_width);
	glBegin(GL_LINES);
	glColor3f(ri,gi,bi);
	glVertex3fv(&pc->pnt(vi)[0]);
	glColor3f(rj,gj,bj);
	glVertex3fv(&pc->pnt(vj)[0]);
	glEnd();
}

void surface_reconstructor::draw_colored_triangle(unsigned int vi, unsigned int vj, unsigned int vk, float r, float g, float b) const
{
	glEnable(GL_LIGHTING);
	glColor3f(r,g,b);
	const Pnt& pi = pc->pnt(vi)[0];
	const Pnt& pj = pc->pnt(vj)[0];
	const Pnt& pk = pc->pnt(vk)[0];
	Nml n = normalize(cross(pj-pi,pk-pi));
	glNormal3fv(&n[0]);
	glBegin(GL_TRIANGLES);
	glVertex3fv(&pi[0]);
	glVertex3fv(&pj[0]);
	glVertex3fv(&pk[0]);
	glEnd();
	glDisable(GL_LIGHTING);
}
