#pragma once
#include <ctime>
#include <algorithm>
#include "functions.h"
#include <cgv/math/vec.h>
#include <cgv/math/mat.h>
#include <cgv/math/eig.h>
#include <cgv/math/diag_mat.h>
#include <cgv/math/perm_mat.h>
#include <cgv/math/up_tri_mat.h>
#include <cgv/math/low_tri_mat.h>
#include <cgv/math/quat.h>
#include <set>
//#include <cgv/math/constants.h>

namespace cgv {
	namespace math {


/**
* High quality random number generator,
* which  is a little bit slower than typical random number generators
* but with a period  about 3.138 x 10^57.
*/
struct random
{
private:
	unsigned long long u,v,w;
	double storedval;
public:
	/// set a new seed
	void set_seed(unsigned long long seed)
	{
		w = 1;
		v = 4101842887655102017LL;
		u = seed^v;
		unsigned long long ull;
		uniform(ull);
		v = u; uniform(ull);
		w = v; uniform(ull);
	}

	///standard constructor uses system time as random seed
	random() : storedval(0.0)
	{
		unsigned long long seed = clock();
		set_seed(seed);
	}

	///constructor initializes random generator with given seed
	random(unsigned long long seed) : storedval(0.0)
	{
		set_seed(seed);
	}
	
	///generates 64bit pseudo random integer
	void uniform(unsigned long long& rv)
	{
		u = u*2862933555777941757LL + 7046029254386353087LL;
		v ^= v >> 17; v ^= v <<31; v ^= v >> 8;
		w = 4294957665U*(w & 0xffffffff) + (w >> 32);
		unsigned long long x = u ^(u << 21); x ^= x >>35; x^=x << 4;
		rv= (x+v)^w;
	}

	///generates a 32bit pseudo random unsigned integer
	void uniform(unsigned int& rv)
	{
		unsigned long long v;
		uniform(v);
		rv= (unsigned int) v;
	}

	///generates a 32bit pseudo random signed integer
	void uniform(int& rv)
	{
		unsigned long long v;
		uniform(v);
		rv = (int) v;
	}

	///generates a 32bit pseudo random signed integer
	void uniform(long& rv)
	{
		unsigned long long v;
		uniform(v);
		rv = (long) v;
	}

	///generates a 64bit pseudo random signed integer
	void uniform(long long& rv)
	{
		unsigned long long v;
		uniform(v);
		rv = (long long) v;
	}

	///generates a  pseudo random boolean
	void uniform(bool& rv)
	{
		unsigned long long v;
		uniform(v);
		rv= v%2 == 0;
	}

	///generates a pseudo random integer between min and max
	void uniform(const unsigned min,const unsigned max, unsigned& rv)
	{
		uniform(rv);
		rv =rv%(max-min+1)+min;
	}

	///generates a pseudo random integer between min and max
	void uniform(const int min,const int max, int& rv)
	{
		uniform(rv);
		rv =(int)(rv%(unsigned)(max-min+1))+min;
	}


	///generates a pseudo random double-precision floating point number uniformly distributed between 0 and 1
	void uniform(double& rv)
	{
		unsigned long long v;
		uniform(v);
		rv= 5.42101086242752217E-20 * v;
	}

	///generates a pseudo random double-precision floating point number uniformly distributed between min and max
	void uniform(const double min,const double max, double &rv)
	{
		double v;
		uniform(v);
		rv= (max-min)*v + min;
	}
	
	///generates a pseudo random single-precision floating point number uniformly distributed between 0 and 1
	void uniform(float & rv)
	{
		double v;
		uniform(v);
		rv =(float) v;
	}

	///generates a pseudo random single-precision floating point number uniformly distributed between 0 and 1
	void uniform(const float min,const float max, float& rv)
	{
		float f;
		uniform(f);
		rv= (max-min)*f + min;
	}

	///generates a pseudo random single precision vector with uniformly distribute components between 0 and 1
	void uniform( vec<float>& rv)
	{
		for(unsigned i = 0; i < rv.size();i++)
			uniform(rv(i));	
	}
	
	///generates a pseudo random single precision vector with uniformly distribute components between min and max
	void uniform(const float min,const float max, vec<float>& rv)
	{
		for(unsigned i = 0; i < rv.size();i++)
			uniform(min,max,rv(i));
		
	}

	
	///generates a pseudo random double precision vector with uniformly distribute components between 0 and 1
	void uniform(vec<double>& rv)
	{
		for(unsigned i = 0; i < rv.size();i++)
			uniform(rv(i));	
	}
	
	///generates a pseudo random double precision vector with uniformly distribute components between min and max
	void uniform(const double min,const double max, vec<double>& rv)
	{
		for(unsigned i = 0; i < rv.size();i++)
			uniform(min,max,rv(i));	
	}

	///generates a pseudo random single precision full matrix with uniformly distribute components between 0 and 1
	void uniform(mat<float>& rv)
	{
		for(unsigned i = 0; i < rv.nrows(); i++)
			for(unsigned j = 0; j < rv.ncols(); j++)
			uniform(rv(i,j));	
	}
	
	///generates a pseudo random single precision full matrix with uniformly distribute components between min and max
	void uniform(const float min,const float max, mat<float>& rv)
	{
		for(unsigned i = 0; i < rv.nrows(); i++)
			for(unsigned j = 0; j < rv.ncols(); j++)
				uniform(min,max,rv(i,j));		
	}

	///generates a pseudo random permutation matrix 
	void uniform(perm_mat& rv)
	{
		unsigned j;
		for(unsigned i = 0; i < rv.size();i++)
		{
			uniform(j);
			j=(i+j)%rv.size();
			rv.swap(i,j);
		}
		std::random_shuffle(&(rv(0)),&(rv(rv.nrows())));	
	}

	///generates a pseudo random single precision upper triangular matrix with uniformly distribute components between 0 and 1
	void uniform(up_tri_mat<float>& rv)
	{
		for(unsigned i = 0; i < rv.nrows(); i++)
			for(unsigned j = i; j < rv.ncols(); j++)
			uniform(rv(i,j));	
	}

	

	///generates a pseudo random single precision upper triangular matrix with uniformly distribute components between min and max
	void uniform(const float min, const float max,up_tri_mat<float>& rv)
	{
		for(unsigned i = 0; i < rv.nrows(); i++)
			for(unsigned j = i; j < rv.ncols(); j++)
			uniform(min,max,rv(i,j));	
	}

	///generates a pseudo random double precision upper triangular matrix with uniformly distribute components between 0 and 1
	void uniform(up_tri_mat<double>& rv)
	{
		for(unsigned i = 0; i < rv.nrows(); i++)
			for(unsigned j = i; j < rv.ncols(); j++)
			uniform(rv(i,j));	
	}

	///generates a pseudo random double precision upper triangular matrix with uniformly distribute components between min and max
	void uniform(const double min, const double max,up_tri_mat<double>& rv)
	{
		for(unsigned i = 0; i < rv.nrows(); i++)
			for(unsigned j = i; j < rv.ncols(); j++)
			uniform(min,max,rv(i,j));	
	}

	///generates a pseudo random single precision lower triangular matrix with uniformly distribute components between 0 and 1
	void uniform( low_tri_mat<float>& rv)
	{
		for(unsigned i = 0; i < rv.nrows(); i++)
			for(unsigned j = 0; j <= i; j++)
				uniform(rv(i,j));		
	}
	
	///generates a pseudo random single precision lower triangular matrix with uniformly distribute components between min and max
	void uniform(const float min,const float max, low_tri_mat<float>& rv)
	{
		for(unsigned i = 0; i < rv.nrows(); i++)
			for(unsigned j = 0; j <= i; j++)
				uniform(min,max,rv(i,j));		
	}

	///generates a pseudo random double precision lower triangular matrix with uniformly distribute components between 0 and 1
	void uniform( low_tri_mat<double>& rv)
	{
		for(unsigned j = 0; j < rv.ncols(); j++)
			for(unsigned i = 0; i < j; i++)
				uniform(rv(i,j));		
	}
	
	///generates a pseudo random double precision lower triangular matrix with uniformly distribute components between min and max
	void uniform(const double min,const double max, low_tri_mat<double>& rv)
	{
		for(unsigned j = 0; j < rv.ncols(); j++)
			for(unsigned i = 0; i < j; i++)
				uniform(min,max,rv(i,j));		
	}

	///generates a pseudo random single precision diagonal matrix with uniformly distribute components between 0 and 1
	void uniform(diag_mat<float>& rv)
	{
		for(unsigned i = 0; i < rv.nrows(); i++)
			uniform(rv(i));	
	}
	
	///generates a pseudo random single precision diagonal matrix with uniformly distribute components between min and max
	void uniform(const float min,const float max, diag_mat<float>& rv)
	{
			for(unsigned i = 0; i < rv.nrows(); i++)
				uniform(min,max,rv(i));		
	}

	///generates a pseudo random double precision diagonal matrix with uniformly distribute components between 0 and 1
	void uniform(diag_mat<double>& rv)
	{
		for(unsigned i = 0; i < rv.nrows(); i++)
			uniform(rv(i));	
	}
	
	///generates a pseudo random double precision diagonal matrix with uniformly distribute components between min and max
	void uniform(const double min,const double max, diag_mat<double>& rv)
	{
			for(unsigned i = 0; i < rv.nrows(); i++)
				uniform(min,max,rv(i));		
	}


	///generates a pseudo random double full matrix with uniformly distribute components between 0 and 1
	void uniform(mat<double>& rv)
	{
		for(unsigned i = 0; i < rv.nrows(); i++)
			for(unsigned j = 0; j < rv.ncols(); j++)
				uniform(rv(i,j));	
	}
	
	///generates a pseudo random double full matrix with uniformly distribute components between min and max
	void uniform(const double min,const double max, mat<double>& rv)
	{
		for(unsigned  i = 0; i < rv.nrows(); i++)
			for(unsigned j = 0; j < rv.ncols(); j++)
				uniform(min,max,rv(i,j));		
	}



	///generates a normal deviate double-precision floating point value with mu = 0 and sigma = 1
	void normal(double& rv)
	{
		double v1,v2,rsq,fac;
		if(storedval == 0.)
		{
			do{
				uniform(v1);
				uniform(v2);

				v1 = 2.0 * v1-1.0;
				v2 = 2.0 * v2-1.0;
				rsq = v1*v1+v2*v2;
			}while(rsq >= 1.0 || rsq == 0.0);
			fac = sqrt(-2.0*std::log(rsq)/rsq);
			storedval = v1*fac;
			rv= v2*fac;

		}else
		{
			fac = storedval;
			storedval = 0.;
			rv= fac;
		}
	}

	///generates a normal deviate single-precision floating point value with mu = 0 and sigma = 1
	void normal(float& rv)
	{
		double d;
		normal(d);
		rv = (float)d;
	}

	///generates a normal deviate double-precision floating point value with mu and sigma 
	void normal(const double mu, const double sigma, double &rv)
	{	
		double d;
		normal(d);
		rv =mu + sigma * d;
	}

	///generates a normal deviate single-precision floating point value with mu and sigma 
	void normal(const float mu, const float sigma, float& rv)
	{	float f;
		normal(f);
		rv= mu + sigma * f;
	}

	///generates a single precision random direction (uniformly distributed position on the unit sphere)
	void uniform_direction(vec<float>& v)
	{
		
		for(unsigned i = 0;i < v.size(); i++)
			 normal(v(i));
		v.normalize();
	}

	
	///generates a double precision random direction (uniformly distributed position on the unit sphere)
	void uniform_direction(vec<double>& v)
	{
		for(unsigned i = 0;i < v.size();i++)
			 normal(v(i));
		v.normalize();
	}

	///generates a single precision random orientation represented as a unit quaternion
	void uniform_quat_orientation(vec<float>& q)
	{
		float x[3];
		uniform(x[0]);
		uniform(x[1]);
		uniform(x[2]);
		float  a, b, c, d, s;
		float  z, r, theta, omega;       
		

		z = x[0];
		r = sqrt( 1 - z * z );
		theta = 2.0f * 3.14159f * x[1];
		omega = 3.14159f * x[2];


		s = sin( omega );
		a = cos( omega );
		b = s * cos( theta ) * r;
		c = s * sin( theta ) * r;
		d = s * z;


		q.set(b,c,d,a);
	}


	///generates a double precision random orientation 
	///represented as a unit quaternion
	void uniform_quat_orientation(vec<double>& q)
	{
		q.resize(4);
		double x[3];
		uniform(x[0]);
		uniform(x[1]);
		uniform(x[2]);
		double  a, b, c, d, s;
		double  z, r, theta, omega;       
		

		z = x[0];
		r = sqrt( 1 - z * z );
		theta = 2.0 * 3.14159 * x[1];
		omega = 3.14159 * x[2];


		s = sin( omega );
		a = cos( omega );
		b = s * cos( theta ) * r;
		c = s * sin( theta ) * r;
		d = s * z;


		q.set(b,c,d,a);
	}
	

	///generates a single precision random orientation represented as a rotation matrix
	void uniform_orientation(mat<float>& m)
	{
		if(m.nrows() == 3 && m.ncols() == 3)
		{
			vec<float> q(4);
			uniform_quat_orientation(q);
			m = quat_2_mat_33(q);
			return;
		}
		if(m.nrows() == 4 && m.ncols() == 4)
		{
			vec<float> q(4);
			uniform_quat_orientation(q);
			m = quat_2_mat_44(q);
			return;
		}
		assert(false);

	}

	///generates a double precision random orientation represented as a rotation matrix
	void uniform_orientation( mat<double>& m)
	{
		if(m.nrows() == 3 && m.ncols() == 3)
		{
			vec<double> q(4);
			uniform_quat_orientation(q);
			m = quat_2_mat_33(q);
			return;
		}
		if(m.nrows() == 4 && m.ncols() == 4)
		{
			vec<double> q(4);
			uniform_quat_orientation(q);
			m = quat_2_mat_44(q);
			return;
		}
		assert(false);
	}


	///creates an uniform distributed random point in unit box (0,0,...,0)..(1,1,...,1); if dim(p)=0, it is set to 3
	void uniform_point_in_unit_box(vec<double>& p)
	{
		if (p.size() == 0)
			p.resize(3);
		for (unsigned i=0; i<p.size(); ++i)
			uniform(p(i));
	}

	///creates an uniform distributed random point in  box minp..maxp
	void uniform_point_in_box(const vec<double>& minp,const vec<double>& maxp, vec<double>& p)
	{
		assert(minp.size() == maxp.size());
		if (p.size() != minp.size())
			p.resize(minp.size());
		for (unsigned i=0; i<p.size(); ++i)
			uniform(minp(i),maxp(i),p(i));
	}

	///creates an uniform distributed random point in unit sphere
	void uniform_point_in_unit_ball(vec<double>& p)
	{
		uniform_direction(p);
		double r;
		uniform(r);
		r = pow(r, 1.0/p.size());
		p *= r;
	}
	///creates an uniform distributed random point in triangle p1,p2,p3
	void uniform_point_in_triangle(const vec<double>& p1, const vec<double>& p2, const vec<double>& p3,vec<double> p)
	{
		double u,v;
		uniform(u);
		uniform(v);
		if(u+v > 1.0)
		{
			u=1.0-u;
			v=1.0-v;
		}
		p= u*p1 + v*p2 +(1.0-u-v)*p3;
	}

	///creates an uniform distributed random point on the surface of a sphere with given center and radius
	void uniform_point_on_sphere(const vec<double>& center,const double& radius,
		vec<double>& p)
	{
		for(unsigned i = 0;i < p.size();i++)
			 normal(p(i));
		p.normalize();
		p=center+radius*p;
	}


	///creates a vector of k unique indices drawn from 0 to n-1 
	void uniform_nchoosek(unsigned n, unsigned k, vec<unsigned>& indices)
	{
		std::set<unsigned> s;	
		unsigned v;
		while(s.size() < k)
		{
			uniform(0,n-1,v);
			s.insert(v);
		}

		indices.resize(k);
		unsigned i=0;
		for(std::set<unsigned>::iterator it =s.begin(); it != s.end();it++,i++)
			indices(i)=*it;
			
	}

	


};


}
}

