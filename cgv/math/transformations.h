#pragma once
#include "vec.h"
#include "mat.h"

namespace cgv {
namespace math {


///creates a 3x3 scale matrix
template <typename T>
const mat<T> scale_33(const T&sx, const T &sy, const T&sz)
{
	mat<T> m(3,3);
	m(0,0)=sx; m(0,1)= 0; m(0,2)= 0; 
	m(1,0)=0; m(1,1)=sy; m(1,2)= 0; 
	m(2,0)=0; m(2,1)= 0; m(2,2)= sz; 
	return m;
}

///creates a 3x3 uniform scale matrix
template <typename T>
const mat<T> scale_33(const T&s)
{
	mat<T> m(3,3);
	m(0,0)=s; m(0,1)= 0; m(0,2)= 0; 
	m(1,0)=0; m(1,1)=s; m(1,2)= 0; 
	m(2,0)=0; m(2,1)= 0; m(2,2)= s; 

	return m;
}



///creates a 3x3 rotation matrix around the x axis, the angle is in degree   
template<typename T>
const mat<T> rotatex_33(const T& angle)
{
	T angler = angle*(T)3.14159/(T)180.0;
	mat<T> m(3,3);
    m(0,0)=1; m(0,1)= 0; m(0,2)= 0; 
	m(1,0)=0; m(1,1)= (T)cos((double)angler); 
	m(1,2)= -(T)sin((double)angler); 
	m(2,0)=0; m(2,1)= (T)sin((double)angler); 
	m(2,2)= (T)cos((double)angler); 
	
    return m;
}

///creates a 3x3 rotation matrix around the y axis, the angle is in degree  
template<typename T>
const mat<T> rotatey_33(const T& angle)
{
	T angler=angle*(T)3.14159/(T)180.0;
    mat<T> m(3,3);
    m(0,0)=(T)cos((double)angler); m(0,1)= 0; 
	m(0,2)= (T)sin((double)angler); 
	m(1,0)=0; m(1,1)=1; m(1,2)= 0;
	m(2,0)=-(T)sin((double)angler); m(2,1)= 0; 
	m(2,2)= (T)cos((double)angler); 

	
    return m;
}

///creates a 3x3 rotation matrix around the z axis, the angle is in degree  
template<typename T>
const mat<T> rotatez_33(const T& angle)
{
	T angler=angle*(T)3.14159/(T)180.0;
	mat<T> m(3,3);
    m(0,0)=(T)cos((double)angler); 
	m(0,1)= -(T)sin((double)angler); m(0,2)= 0; 
	m(1,0)=(T)sin((double)angler); 
	m(1,1)= (T)cos((double)angler); m(1,2)= 0; 
	m(2,0)=0; m(2,1)= 0; m(2,2)= 1; 
    return m;
}

///creates a 2x2 rotation matrix around the z axis, the angle is in degree  
template<typename T>
const mat<T> rotate_22(const T& angle)
{
	T angler=angle*(T)3.14159/(T)180.0;
	mat<T> m(2,2);
    m(0,0)=(T)cos((double)angler); 
	m(0,1)= -(T)sin((double)angler);  
	m(1,0)=(T)sin((double)angler); 
	m(1,1)= (T)cos((double)angler); 
    return m;
}

template<typename T>
const mat<T> rotate_33(const T &dirx, const T &diry, const T&dirz,
					const T& angle)
{
	
	T angler = angle*(T)3.14159/(T)180.0;
	mat<T> m(3,3);
	T rcos = cos(angler);
	T rsin = sin(angler);
	m(0,0)=       rcos + dirx*dirx*((T)1-rcos);
	m(0,1)= -dirz*rsin + diry*dirx*((T)1-rcos);
	m(0,2)=  diry*rsin + dirz*dirx*((T)1-rcos);
	

	m(1,0)= dirz*rsin + dirx*diry*((T)1-rcos);
	m(1,1)=       rcos + diry*diry*((T)1-rcos);
	m(1,2)=  -dirx*rsin + dirz*diry*((T)1-rcos);
	

	m(2,0)=  -diry*rsin + dirx*dirz*((T)1-rcos);
	m(2,1)= dirx*rsin + diry*dirz*((T)1-rcos);
	m(2,2)=		  rcos + dirz*dirz*((T)1-rcos);
	
	
	return m;
}


///creates a 3x3 euler rotation matrix from yaw, pitch and roll given in degree 
template<typename T> 
const mat<T> rotate_euler_33(const T& yaw, const T& pitch,const T& roll)
{
	T yawd= (T)(yaw*3.14159/180.0);
	T pitchd=(T)(pitch*3.14159/180.0);
	T rolld=(T)(roll*3.14159/180.0);
    T cosy =(T) cos(yawd);
    T siny =(T) sin(yawd);
    T cosp =(T) cos(pitchd);
    T sinp =(T) sin(pitchd);
    T cosr =(T) cos(rolld);
    T sinr =(T) sin(rolld);
    mat<T> m(3,3);
    
    m(0,0) = cosr*cosy - sinr*sinp*siny;
    m(0,1) = -sinr*cosp;
    m(0,2) = cosr*siny + sinr*sinp*cosy;
    

    m(1,0) = sinr*cosy + cosr*sinp*siny;
    m(1,1) = cosr*cosp;
    m(1,2) = sinr*siny - cosr*sinp*cosy;
    

    m(2,0) = -cosp*siny;
    m(2,1) = sinp;
    m(2,2) = cosp*cosy;
   

   
    return m;
}

template <typename T>
const mat<T> star(const vec<T>& v)
{
	mat<T> m(3,3);
	
	m(0,0) = 0;
    m(0,1) = -v(2);
    m(0,2) = v(1);
    

    m(1,0) = v(2);
    m(1,1) = 0;
    m(1,2) = -v(0);
    

    m(2,0) = -v(1);
    m(2,1) = v(0);
    m(2,2) = 0;
   

	return m;
}

///creates a 3x3 rotation matrix from a rodrigues vector
template<typename T> 
const mat<T> rotate_rodrigues_33(const vec<T>& r)
{
	T theta = length(r);
	if(theta == 0)
		return identity<T>(3);

	vec<T> rn   = r/theta;
	T cos_theta = cos(theta);
	T sin_theta = sin(theta);
   
    return cos_theta*identity<T>(3) + ((T)1-cos_theta)*dyad(rn,rn) + sin_theta*star(rn);
}

template<typename T> 
const mat<T> rotate_rodrigues_33(const T&rx, const T& ry, const T&rz)
{
	vec<T> r(3);
	r(0) = rx;
	r(1) = ry;
	r(2) = rz;
	
	return rotate_rodrigues_33(r);
}


///creates a homogen 3x3 shear matrix with given shears shx in x direction, and shy in y direction
template<typename T>
const mat<T> shearxy_33(const T &shx, const T &shy)
{
	mat<T> m(3,3);
	m(0,0)=1; m(0,1)= 0; m(0,2)= shx;
	m(1,0)=0; m(1,1)= 1; m(1,2)= shy;
	m(2,0)=0; m(2,1)= 0; m(2,2)= 1; 
	
	return m;
}


///creates a homogen 3x3 shear matrix with given shears shx in x direction, and shy in y direction
template<typename T>
const mat<T> shearxz_33(const T&shx, const T&shz)
{
	mat<T> m(3,3);
	m(0,0)=1; m(0,1)= shx; m(0,2)= 0; 
	m(1,0)=0; m(1,1)= 1; m(1,2)= 0;
	m(2,0)=0; m(2,1)= shz; m(2,2)= 1; 
	return m;
}

///creates a homogen 3x3 shear matrix with given shears shy in y direction, and shz in z direction
template<typename T>
const mat<T> shearyz_33(const T&shy, const T&shz)
{
	mat<T> m(3,3);
	m(0,0)=1; m(0,1)= 0; m(0,2)= 0; 
	m(1,0)=shy; m(1,1)= 1; m(1,2)= 0; 
	m(2,0)=shz; m(2,1)= 0; m(2,2)= 1;
	
	return m;
}


///creates a homogen 3x3 shear matrix 
template<typename T>
const mat<T> shear_33(const T &syx, const T &szx,
				   const T &sxy, const T &szy,
				   const T &sxz, const T &syz)
{
	mat<T> m(3,3);
	m(0,0)=1; m(0,1)= syx; m(0,2)= szx; 
	m(1,0)=sxy; m(1,1)= 1; m(1,2)= szy; 
	m(2,0)=sxz; m(2,1)= syz; m(2,2)= 1; 
	
	return m;
}

///creates a 4x4 translation matrix
template <typename T>
const mat<T> translate_44(const T&x, const T &y, const T&z)
{
	mat<T> m(4,4);
	m(0,0)=1; m(0,1)= 0; m(0,2)= 0; m(0,3)= x;
	m(1,0)=0; m(1,1)= 1; m(1,2)= 0; m(1,3)= y;
	m(2,0)=0; m(2,1)= 0; m(2,2)= 1; m(2,3)= z;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
	return m;
}

///creates a 4x4 translation matrix
template <typename T>
const mat<T> translate_44(const vec<T> &v)
{
	mat<T> m(4,4);
	m(0,0)=1; m(0,1)= 0; m(0,2)= 0; m(0,3)= v(0);
	m(1,0)=0; m(1,1)= 1; m(1,2)= 0; m(1,3)= v(1);
	m(2,0)=0; m(2,1)= 0; m(2,2)= 1; m(2,3)= v(2);
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
	return m;
}

///creates a 4x4 scale matrix
template <typename T>
const mat<T> scale_44(const T&sx, const T &sy, const T&sz)
{
	mat<T> m(4,4);
	m(0,0)=sx; m(0,1)= 0; m(0,2)= 0; m(0,3)= 0;
	m(1,0)=0; m(1,1)=sy; m(1,2)= 0; m(1,3)= 0;
	m(2,0)=0; m(2,1)= 0; m(2,2)= sz; m(2,3)= 0;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
	return m;
}

template<typename T>
const mat<T> scale_44(const cgv::math::vec<T> s)
{
	assert(s.size() == 3);
	return scale_44(s(0),s(1),s(2));
}

///creates a 4x4 uniform scale matrix
template <typename T>
const mat<T> scale_44(const T&s)
{
	mat<T> m(4,4);
	m(0,0)=s; m(0,1)= 0; m(0,2)= 0; m(0,3)= 0;
	m(1,0)=0; m(1,1)=s; m(1,2)= 0; m(1,3)= 0;
	m(2,0)=0; m(2,1)= 0; m(2,2)= s; m(2,3)= 0;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
	return m;
}



///creates a 4x4 rotation matrix around the x axis, the angle is in degree   
template<typename T>
const mat<T> rotatex_44(const T& angle)
{
	T angler = angle*(T)3.14159/(T)180.0;
	mat<T> m(4,4);
    m(0,0)=1; m(0,1)= 0; m(0,2)= 0; m(0,3)= 0;
	m(1,0)=0; m(1,1)= (T)cos((double)angler); 
	m(1,2)= -(T)sin((double)angler); m(1,3)= 0;
	m(2,0)=0; m(2,1)= (T)sin((double)angler); 
	m(2,2)= (T)cos((double)angler); m(2,3)= 0;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
    return m;
}

///creates a 4x4 rotation matrix around the y axis, the angle is in degree  
template<typename T>
const mat<T> rotatey_44(const T& angle)
{
	T angler=angle*(T)3.14159/(T)180.0;
    mat<T> m(4,4);
    m(0,0)=(T)cos((double)angler); m(0,1)= 0; 
	m(0,2)= (T)sin((double)angler); m(0,3)= 0;
	m(1,0)=0; m(1,1)=1; m(1,2)= 0; m(1,3)= 0;
	m(2,0)=-(T)sin((double)angler); m(2,1)= 0; 
	m(2,2)= (T)cos((double)angler); m(2,3)= 0;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
    return m;
}

///creates a 4x4 rotation matrix around the z axis, the angle is in degree  
template<typename T>
const mat<T> rotatez_44(const T& angle)
{
	T angler=angle*(T)3.14159/(T)180.0;
	mat<T> m(4,4);
    m(0,0)=(T)cos((double)angler); 
	m(0,1)= -(T)sin((double)angler); m(0,2)= 0; m(0,3)= 0;
	m(1,0)=(T)sin((double)angler); 
	m(1,1)= (T)cos((double)angler); m(1,2)= 0; m(1,3)= 0;
	m(2,0)=0; m(2,1)= 0; m(2,2)= 1; m(2,3)= 0;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
    return m;
}


template<typename T>
const mat<T> rotate_44(const T &dirx, const T &diry, const T&dirz,
					const T& angle)
{
	
	T angler = angle*(T)3.14159/(T)180.0;
	mat<T> m(4,4);
	T rcos = cos(angler);
	T rsin = sin(angler);
	m(0,0)=       rcos + dirx*dirx*((T)1-rcos);
	m(0,1)= -dirz*rsin + diry*dirx*((T)1-rcos);
	m(0,2)=  diry*rsin + dirz*dirx*((T)1-rcos);
	

	m(1,0)= dirz*rsin + dirx*diry*((T)1-rcos);
	m(1,1)=       rcos + diry*diry*((T)1-rcos);
	m(1,2)=  -dirx*rsin + dirz*diry*((T)1-rcos);
	

	m(2,0)=  -diry*rsin + dirx*dirz*((T)1-rcos);
	m(2,1)= dirx*rsin + diry*dirz*((T)1-rcos);
	m(2,2)=		  rcos + dirz*dirz*((T)1-rcos);
		
	m(3,0)= (T)0;
	m(3,1)= (T)0;	
	m(3,2)= (T)0;
	m(0,3)= (T)0;
	m(1,3)= (T)0;
	m(2,3)= (T)0;
	m(3,3)= (T)1;
	return m;
}


template<typename T>
const mat<T> rotate_33(const cgv::math::vec<T>& dir, const T& angle)
{
	
	assert(dir.size() == 3);
	cgv::math::vec<T> vdir =dir;
	vdir.normalize();
	return rotate_33<T>(vdir(0),vdir(1),vdir(2), angle);
}

template<typename T>
const mat<T> rotate_44(const cgv::math::vec<T>& dir, const T& angle)
{
	
	assert(dir.size() == 3);
	cgv::math::vec<T> vdir =dir;
	vdir.normalize();
	return rotate_44<T>(vdir(0),vdir(1),vdir(2), angle);
}


///creates a 4x4 euler rotation matrix from yaw, pitch and roll given in degree 
template<typename T> 
const mat<T> rotate_euler_44(const T& yaw, const T& pitch,const T& roll)
{
	T yawd= yaw*3.14159/180.0;
	T pitchd=pitch*3.14159/180.0;
	T rolld=roll*3.14159/180.0;
    T cosy =(T) cos(yawd);
    T siny =(T) sin(yawd);
    T cosp =(T) cos(pitchd);
    T sinp =(T) sin(pitchd);
    T cosr =(T) cos(rolld);
    T sinr =(T) sin(rolld);
    mat<T> m(4,4);
    
    m(0,0) = cosr*cosy - sinr*sinp*siny;
    m(0,1) = -sinr*cosp;
    m(0,2) = cosr*siny + sinr*sinp*cosy;
    m(0,3) = 0;

    m(1,0) = sinr*cosy + cosr*sinp*siny;
    m(1,1) = cosr*cosp;
    m(1,2) = sinr*siny - cosr*sinp*cosy;
    m(1,3) = 0;

    m(2,0) = -cosp*siny;
    m(2,1) = sinp;
    m(2,2) = cosp*cosy;
    m(2,3) = 0;

    m(3,0) = m(3,1)= m(3,2) =0;
    m(3,3) = 1;
    return m;
}


///creates a homogen 4x4 shear matrix with given shears shx in x direction, and shy in y direction
template<typename T>
const mat<T> shearxy_44(const T &shx, const T &shy)
{
	mat<T> m(4,4);
	m(0,0)=1; m(0,1)= 0; m(0,2)= shx; m(0,3)= 0;
	m(1,0)=0; m(1,1)= 1; m(1,2)= shy; m(1,3)= 0;
	m(2,0)=0; m(2,1)= 0; m(2,2)= 1; m(2,3)= 0;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
	return m;
}


///creates a homogen 4x4 shear matrix with given shears shx in x direction, and shy in y direction
template<typename T>
const mat<T> shearxz_44(const T&shx, const T&shz)
{
	mat<T> m(4,4);
	m(0,0)=1; m(0,1)= shx; m(0,2)= 0; m(0,3)= 0;
	m(1,0)=0; m(1,1)= 1; m(1,2)= 0; m(1,3)= 0;
	m(2,0)=0; m(2,1)= shz; m(2,2)= 1; m(2,3)= 0;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
	return m;
}

///creates a homogen 4x4 shear matrix with given shears shy in y direction, and shz in z direction
template<typename T>
const mat<T> shearyz_44(const T&shy, const T&shz)
{
	mat<T> m(4,4);
	m(0,0)=1; m(0,1)= 0; m(0,2)= 0; m(0,3)= 0;
	m(1,0)=shy; m(1,1)= 1; m(1,2)= 0; m(1,3)= 0;
	m(2,0)=shz; m(2,1)= 0; m(2,2)= 1; m(2,3)= 0;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
	return m;
}


///creates a homogen 4x4 shear matrix 
template<typename T>
const mat<T> shear_44(const T &syx, const T &szx,
				   const T &sxy, const T &szy,
				   const T &sxz, const T &syz)
{
	mat<T> m(4,4);
	m(0,0)=1; m(0,1)= syx; m(0,2)= szx; m(0,3)= 0;
	m(1,0)=sxy; m(1,1)= 1; m(1,2)= szy; m(1,3)= 0;
	m(2,0)=sxz; m(2,1)= syz; m(2,2)= 1; m(2,3)= 0;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
	return m;
}





///creates a perspective transformation matrix in the same way as gluPerspective does
template<typename T>
const mat<T> perspective_44(const T& fovy, const T&aspect, const T& znear, 
						 const T& zfar)
{
	T fovyr = (T)(fovy*3.14159/180.0);
	T f = (T)(cos(fovyr/2.0f)/sin(fovyr/2.0f));
	mat<T> m(4,4);
	m(0,0)=f/aspect; m(0,1)= 0; m(0,2)= 0; m(0,3)= 0;
	m(1,0)=0; m(1,1)= f; m(1,2)= 0; m(1,3)= 0;
	m(2,0)=0; m(2,1)= 0; m(2,2)= (zfar+znear)/(znear-zfar); 
	m(2,3)= (2*zfar*znear)/(znear-zfar);
	m(3,0)=0; m(3,1)= 0; m(3,2)= -1; m(3,3)= 0;
	return m;
}

///creates a viewport transformation matrix 
template <typename T>
mat<T> viewport_44(const T& xoff, const T yoff, const T& width,
		const T& height)
{
	mat<T> m(4,4);
	T a = width/(T)2.0;
	T b = height/(T)2.0;
	m(0,0)=a; m(0,1)= 0; m(0,2)= 0; m(0,3)= xoff+(T)0.5;
	m(1,0)=0; m(1,1)= b; m(1,2)= 0; m(1,3)= yoff+(T)0.5;
	m(2,0)=0; m(2,1)= 0; m(2,2)= (T)0.5; m(2,3)=(T)0.5 ;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
	return m;
}


///creates a look at transformation matrix in the same way as gluLookAt does
template<typename T>
const mat<T> look_at_44(const T &eyex, const T &eyey, const T &eyez,
		const T& centerx, const T& centery, const T& centerz, 
		const T& upx, const T& upy, const T& upz)
{
	vec<T> center(centerx,centery,centerz);
	vec<T> eye(eyex,eyey,eyez);
	vec<T> up(upx,upy,upz);

	vec<T> f = normalize(center-eye);
	up=normalize(up);
	vec<T> s = normalize(cross(f,up));
	vec<T> u = normalize(cross(s,f));

	mat<T> m(4,4);
	m(0,0)=s(0); m(0,1)=s(1) ; m(0,2)= s(2); m(0,3)= 0;
	m(1,0)=u(0); m(1,1)=u(1) ; m(1,2)= u(2); m(1,3)= 0;
	m(2,0)=-f(0); m(2,1)= -f(1); m(2,2)= -f(2); m(2,3)= 0;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
    m = m*translate_44(-eyex,-eyey,-eyez);
	return m;
}

template<typename T>
const mat<T> look_at_44(vec<T> eye, vec<T> center, vec<T> up)
{
	

	vec<T> f = normalize(center-eye);
	up=normalize(up);
	vec<T> s = normalize(cross(f,up));
	vec<T> u = normalize(cross(s,f));

	mat<T> m(4,4);
	m(0,0)=s(0); m(0,1)=s(1) ; m(0,2)= s(2); m(0,3)= 0;
	m(1,0)=u(0); m(1,1)=u(1) ; m(1,2)= u(2); m(1,3)= 0;
	m(2,0)=-f(0); m(2,1)= -f(1); m(2,2)= -f(2); m(2,3)= 0;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
    m = m*translate_44(-eye);
	return m;
}

///creates a frustrum projection matrix in the same way as glFrustum does
template<typename T>
const mat<T> frustrum_44(const T& left, const T&right,
        const T& bottom, const T& top,
        const T& znear, const T& zfar)
{
    T e = 2*znear/(right - left);
    T f = 2*znear/(top-bottom);
	T A = (right +left)/(right -left);
    T B = (top+bottom)/(top-bottom);
    T C = -(zfar + znear)/(zfar-znear);
    T D = -(2*zfar*znear)/(zfar-znear);
	mat<T> m(4,4);
	m(0,0)=e; m(0,1)= 0; m(0,2)= A; m(0,3)= 0;
	m(1,0)=0; m(1,1)= f; m(1,2)= B; m(1,3)= 0;
	m(2,0)=0; m(2,1)= 0; m(2,2)= C; m(2,3)= D;
	m(3,0)=0; m(3,1)= 0; m(3,2)= -1; m(3,3)= 0;
	return m;
}
    
///creates an orthographic projection matrix in the same way as glOrtho does
template<typename T>
const mat<T> ortho_44(const T& left, const T&right,
        const T& bottom, const T& top,
        const T& znear, const T& zfar)
{
    T a = 2/(right - left);
    T b = 2/(top-bottom);
	T c = -2/(zfar-znear);
    T tx = (right+left)/(right-left);
    T ty = (top+bottom)/(top-bottom);
    T tz = (zfar+znear)/(zfar-znear);
	mat<T> m(4,4);
	m(0,0)=a; m(0,1)= 0; m(0,2)= 0; m(0,3)= -tx;
	m(1,0)=0; m(1,1)= b; m(1,2)= 0; m(1,3)= -ty;
	m(2,0)=0; m(2,1)= 0; m(2,2)= c; m(2,3)= -tz;
	m(3,0)=0; m(3,1)= 0; m(3,2)= 0; m(3,3)= 1;
	return m;
}


///creates an orthographic projection matrix in the same way as glOrtho2d does
template<typename T>
const mat<T> ortho2d_44(const T& left, const T&right,
        const T& bottom, const T& top)
{
	return ortho_44<T>(left,right,bottom,top,(T)-1.0,(T)1.0);
}


///creates a picking matrix like gluPickMatrix with pixel (0,0) in the lower left corner if flipy=false 
template<typename T>
const mat<T> pick_44(const T& x,const T& y,const T& width, const T& height,int viewport[4],const mat<double> &modelviewproj, bool flipy=true)
{
	mat<T> m(4,4);
	T sx, sy;
	T tx, ty;
	sx = viewport[2] / width;
	sy = viewport[3] / height;
	tx = (T)(viewport[2] + 2.0 * (viewport[0] - x)) / width;
	if(flipy)
		ty = (T)(viewport[3] + 2.0 * (viewport[1] - (viewport[3]-y))) / height;
	else
		ty = (T)(viewport[3] + 2.0 * (viewport[1] - y)) / height;

	m(0,0) = sx; m(0,1) = 0; m(0,2) = 0; m(0,3) = tx;
	m(1,0) = 0; m(1,1) = sy; m(1,2) = 0; m(1,3) = ty;
	m(2,0) = 0; m(2,1) = 0; m(2,2) = 1; m(2,3) = 0;
	m(3,0) = 0; m(3,1) = 0; m(3,2) = 0; m(3,3) = 1;
	return m*modelviewproj;
} 

//extract rotation angles from combined rotation matrix
template <typename T>
void decompose_R_2_RxRyRz(const mat<T>& R33,T &angle_x, T& angle_y, T& angle_z)
{

	angle_y =  asin( R33(0,2));        /* Calculate Y-axis angle */
    T C     =  cos( angle_y );
	T trx,atry;
    angle_y    *=  (T)(180/3.14159);
	if ( std::abs( C ) > (T)0.005 )             /* Gimbal lock? */
      {
      trx      =  R33(2,2) / C;           /* No, so get X-axis angle */
      atry      = -R33(1,2)  / C;
      angle_x  = atan2( atry, trx ) * (T)(180/3.14159);
      trx      =  R33(0,0) / C;            /* Get Z-axis angle */
      atry      = -R33(0,1) / C;
      angle_z  = atan2( atry, trx ) * (T)(180/3.14159);
      }
    else                                 /* Gimbal lock has occurred */
      {
      angle_x  = 0;                      /* Set X-axis angle to zero */
      trx      =  R33(1,1);                 /* And calculate Z-axis angle */
      atry      =  R33(1,0);
      angle_z  = atan2( atry, trx ) * (T)(180/3.14159);
      }

    /* return only positive angles in [0,360] */
    if (angle_x < 0) angle_x += (T)360;
    if (angle_y < 0) angle_y += (T)360;
    if (angle_z < 0) angle_z += (T)360;
	//std::cout << angle_x << " "<< angle_y << " " <<angle_z <<std::endl;
}



template <typename T>
const mat<T> extract_frustrum_planes(const mat<T>& modelviewprojection) 
{
	mat<T> frustrum_planes(4,6);
   
	frustrum_planes.set_col(0,modelviewprojection.row(3)-modelviewprojection.row(0));//right
	frustrum_planes.set_col(1,modelviewprojection.row(3)+modelviewprojection.row(0));//left
	frustrum_planes.set_col(2,modelviewprojection.row(3)-modelviewprojection.row(1));//top
	frustrum_planes.set_col(3,modelviewprojection.row(3)+modelviewprojection.row(1));//bottom
	frustrum_planes.set_col(4,modelviewprojection.row(3)-modelviewprojection.row(2));//far
	frustrum_planes.set_col(5,modelviewprojection.row(3)+modelviewprojection.row(2));//near

     // Normalize all planes
     for(unsigned i=0;i<6;i++)
       frustrum_planes.set_col(i,frustrum_planes.col(i)/length(frustrum_planes.col(i).sub_vec(0,3)));
	 return frustrum_planes;

}

}
}


