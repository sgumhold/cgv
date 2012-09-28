#pragma once
#include <cgv/math/mat.h>
#include <cgv/math/vec.h>
#include <cgv/math/lin_solve.h>

namespace cgv {
	namespace math {

///A thin plate spline which represents 2d deformations
///See Fred L. Bookstein: "Principal Warps: Thin-Plate Splines 
///and the Decomposition of Deformation", 1989, IEEE Transactions on
///Pattern Analysis and Machine Intelligence, Vol. II, No. 6
template <typename T>
struct thin_plate_spline
{
	mat<T> controlpoints;
	mat<T> weights;
	mat<T> affine_transformation;

	///deform a 2d point 
	vec<T> map_position(const vec<T>& p)
	{
		assert(p.size() == 2);
		vec<T> r(2);
		r(0) = affine_transformation(0,0)
			+affine_transformation(1,0)*p(0)
			+affine_transformation(2,0)*p(1);
		r(1) = affine_transformation(0,1)
			+affine_transformation(1,1)*p(0)
			+affine_transformation(2,1)*p(1);
		for(unsigned i = 0;i < weights.nrows();i++)
		{
			T u_sqr_dist=U(sqr_length(p-controlpoints.col(i)));
			r(0)+=weights(i,0)*u_sqr_dist;
			r(1)+=weights(i,1)*u_sqr_dist;
		}
		return r;		
	}

/////////////// for affine purposes ///////////////////////////////
	vec<T> map_affine_position(const vec<T>& p)
	{
		assert(p.size() == 2);
		vec<T> r(2);
		
		vec<T> temp(4);
			temp(0) =		(affine_transformation(1,0) + affine_transformation(2,1))/2;
			temp(1) =	 	(affine_transformation(2,0) - affine_transformation(1,1))/2;
			temp(2) =	 -  temp(1);
			temp(3) =		temp(0);

		r(0) = affine_transformation(0,0)
			+temp(0) * p(0)
			+temp(1) * p(1);
		r(1) = affine_transformation(0,1)
			+temp(2) * p(0)
			+temp(3) * p(1);

		return r;		
	}
/////////////// for affine purposes ///////////////////////////////

	///deform 2d points stored as columns of the matrix points
	mat<T> map_positions(const mat<T>& points)
	{
		assert(points.nrows() == 2);
		mat<T> rpoints(points.nrows(),points.ncols());

		for(unsigned i = 0; i < points.ncols(); i++)
			rpoints.set_col(i,map_position(points.col(i)));	
		
		return rpoints;		
	}

	///basis function
	static T U(const T& sqr_dist)
	{
		static const T factor = (T)(1.0/(2.0*log((double)10)));
		if(sqr_dist == 0)
			return 0;
		return sqr_dist*log(sqr_dist)*factor;
	}

};





///fit thin plate spline to interpolate point correspondences
///suc that for columns i spline.map_position(points1.col(i)) == points2.col(i)
///points1 and points2 must contain at least 3 2d point correspondences
template <typename T>
void find_nonrigid_transformation(const mat<T>& points1,
								  const mat<T>& points2,
								  thin_plate_spline<T>& spline)
{
	assert(points1.nrows() == 2 && points2.nrows() == 2);
	assert(points1.ncols() == points2.ncols());
	assert(points1.ncols() > 2);//at least three points

	int n = points1.ncols();
	
	mat<T> L(n+3,n+3);
	

	for(int i = 0; i < n;i++)
		for(int j = 0; j < n;j++)
		{
			T sqr_dist = sqr_length(points1.col(i)-points1.col(j));
			L(i,j)=thin_plate_spline<T>::U(sqr_dist);		
		}

	for(int i = 0; i < n; i++)
	{
		L(i,n) = 1;
		L(i,n+1) = points1.col(i)(0);
		L(i,n+2) = points1.col(i)(1);
		L(n,i) = 1;
		L(n+1,i) = points1.col(i)(0);
		L(n+2,i) = points1.col(i)(1);
	}

	for(int i = n; i < n+3;i++)
	{
		for(int j = n; j < n+3;j++)
		{
			L(i,j)=0;
		}

	}

	mat<T> V(n+3,2);
	for(int i =0;i < n; i++)
	{
		V(i,0)=points2(0,i);
		V(i,1)=points2(1,i);
	}
	V(n,0)=V(n,1)=V(n+1,0)=V(n+1,1)=V(n+2,0)=V(n+2,1)=0;
	mat<T> W(n+3,2);

	svd_solve(L,V,W);
	
	spline.controlpoints=points1;
	spline.weights=W.sub_mat(0,0,n,2);
	
	spline.affine_transformation =W.sub_mat(n,0,3,2);
		
}

///A thin hyperplate spline which represents 3d deformations
///3d extension of the thin plate spline
template <typename T>
struct thin_hyper_plate_spline
{
	mat<T> controlpoints;
	mat<T> weights;
	mat<T> affine_transformation;

	///deform 2d point p
	vec<T> map_position(const vec<T>& p)
	{
	
		assert(p.size() == 3);
		vec<T> r(3);
		r(0) = affine_transformation(0,0)
			  +affine_transformation(1,0)*p(0)
			  +affine_transformation(2,0)*p(1)
		  	  +affine_transformation(3,0)*p(2);
		
		r(1) = affine_transformation(0,1)
			  +affine_transformation(1,1)*p(0)
			  +affine_transformation(2,1)*p(1)
			  +affine_transformation(3,1)*p(2);
		
		r(2) = affine_transformation(0,2)
			  +affine_transformation(1,2)*p(0)
		  	  +affine_transformation(2,2)*p(1)
			  +affine_transformation(3,2)*p(2);

		for(unsigned i = 0;i < weights.nrows();i++)
		{
			T u=length(p-controlpoints.col(i));
			r(0)+=weights(i,0)*u;
			r(1)+=weights(i,1)*u;
			r(2)+=weights(i,2)*u;
		}
		return r;
			
	}

	///deform 3d points stored as columns of the matrix points
	mat<T> map_positions(const mat<T>& points)
	{
		mat<T> rpoints(points.nrows(),points.ncols());
		assert(points.nrows() == 3);
		for(unsigned i = 0; i < points.ncols(); i++)
		{
			rpoints.set_col(i,map_position(points.col(i)));	
		}
		return rpoints;
	}

};


///fit thin hyperplate spline to interpolate point correspondences
///such that for columns i spline.map_position(points1.col(i)) == points2.col(i)
///points1 and points2 must contain at least 4 3d point correspondences
template <typename T>
void find_nonrigid_transformation(const mat<T>& points1,
								  const mat<T>& points2,
								  thin_hyper_plate_spline<T>& spline)
{
	assert(points1.nrows() == 3 && points2.nrows()==3);
	assert(points1.ncols() == points2.ncols());	
	assert(points1.nrows()==4);

	int n = points1.ncols();
	
	mat<T> L(n+4,n+4);
	

	for(int i = 0; i < n;i++)
		for(int j = 0; j < n;j++)
		{
			L(i,j)=length(points1.col(i)-points1.col(j));	
		}

	for(int i = 0; i < n; i++)
	{
		L(i,n) = 1;
		L(i,n+1) = points1.col(i)(0);
		L(i,n+2) = points1.col(i)(1);
		L(i,n+3) = points1.col(i)(2);
		L(n,i) = 1;
		L(n+1,i) = points1.col(i)(0);
		L(n+2,i) = points1.col(i)(1);
		L(n+3,i) = points1.col(i)(2);

	}

	for(int i = n; i < n+4;i++)
	{
		for(int j = n; j < n+4;j++)
		{
			L(i,j)=0;
		}
	}
	

	mat<T> V(n+4,3);
	for(int i =0;i < n; i++)
	{
		V(i,0)=points2(0,i);
		V(i,1)=points2(1,i);
		V(i,2)=points2(2,i);
	}
	V(n,0)=V(n,1)=V(n,2)=V(n+1,0)=V(n+1,1)=V(n+1,2)=V(n+2,0)=V(n+2,1)=V(n+2,2)
		=V(n+3,0)=V(n+3,1)=V(n+3,2)=0;
	
	mat<T> W(n+4,3);
	svd_solve(L,V,W);

	spline.controlpoints=points1;
	spline.weights=W.sub_mat(0,0,n,3);
	spline.affine_transformation =W.sub_mat(n,0,4,3);
	
	
		
}


///apply thin-plate-spline deformation in-place (without producing a copy of the points).
///This method should be used if a large number of points have to be deformed
template <typename T>
void apply_nonrigid_transformation(const thin_plate_spline<T>& s, mat<T>& points)
{
	assert(points.nrows() == 2);
	for(unsigned i = 0; i < points.ncols(); i++)
	{
		points.set_col(i,s.map_position(points.col(i)));	
	}
}

///apply thin-hyper-plate-spline deformation in-place (without producing a copy of the points).
///This method should be used if a large number of points have to be deformed
template <typename T>
void apply_nonrigid_transformation(const thin_hyper_plate_spline<T>& s, mat<T>& points)
{
	assert(points.nrows() == 3);
	for(unsigned i = 0; i < points.ncols(); i++)
	{
		points.set_col(i,s.map_position(points.col(i)));	
	}
}


}




}