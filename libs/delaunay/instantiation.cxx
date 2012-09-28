#include "mesh_geometry.cxx"
#include "triangle_mesh.cxx"
#include "delaunay_mesh.cxx"
#include "delaunay_mesh_with_hierarchy.cxx"

//template mesh_geometry<float>;
//template triangle_mesh<mesh_geometry<float> >;
//template triangle_mesh<mesh_geometry_reference<float> >;
//template delaunay_mesh<triangle_mesh<mesh_geometry<float> > >;
//template delaunay_mesh<triangle_mesh<mesh_geometry_reference<float> > >;
//template delaunay_mesh_with_hierarchy<delaunay_mesh<triangle_mesh<mesh_geometry<float> > > >;

template class mesh_geometry<double>;
template class triangle_mesh<mesh_geometry<double> >;
template class triangle_mesh<mesh_geometry_reference<double> >;

template class delaunay_mesh<>;
template class delaunay_mesh<triangle_mesh<mesh_geometry_reference<double> > >;
template class delaunay_mesh_with_hierarchy<>;

