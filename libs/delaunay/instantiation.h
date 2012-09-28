#pragma once

//template mesh_geometry<float>;
template mesh_geometry<double>;

//template triangle_mesh<mesh_geometry<float> >;
template triangle_mesh<mesh_geometry<double> >;
//template triangle_mesh<mesh_geometry_reference<float> >;
template triangle_mesh<mesh_geometry_reference<double> >;

//template delaunay_mesh<triangle_mesh<mesh_geometry<float> > >;
template delaunay_mesh<>;
//template delaunay_mesh<triangle_mesh<mesh_geometry_reference<float> > >;
template delaunay_mesh<triangle_mesh<mesh_geometry_reference<double> > >;

//template delaunay_mesh_with_hierarchy<delaunay_mesh<triangle_mesh<mesh_geometry<float> > > >;
template delaunay_mesh_with_hierarchy<>;

