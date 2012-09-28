cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Public headers
# TODO: To be verified
set(PUBLIC_HEADERS
	adjacency_list.h
	bi_polynomial.h
	chol.h
	color_operations.h
	constants.h
	det.h
	diag_mat.h
	distance_transform.h
	eig.h
	fibo_heap.h
	fmat.h
	functions.h
	fvec.h
	gaussj.h
	intersections.h
	interval.h
	inv.h
	lib_begin.h
	lin_solve.h
	low_tri_mat.h
	lu.h
	mat.h
	mfunc.h
	model_comp.h
	mst.h
	perm_mat.h
	plane.h
	point_operations.h
	polar.h
	polynomial.h
	qem.h
	qr.h
	quaternion.h
	quat.h
	random.h
	ransac.h
	rigid_transform.h
	solve_polynom.h
	sparse_les.h
	sparse_mat.h
	sphere.h
	statistics.h
	svd.h
	thin_plate_spline.h
	transformations.h
	tri_diag_mat.h
	union_find.h
	up_tri_mat.h
	vec.h)

# Source files
set(SOURCES
	functions.cxx
	solve_polynom.cxx
	sparse_les.cxx)

# Definition of the component
cgv_add_core_component(math
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	SOURCES ${SOURCES})

