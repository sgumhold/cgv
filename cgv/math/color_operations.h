#pragma once
#include <cgv/math/vec.h>
#include <cgv/math/mat.h>
#include <iostream>
#include <string>

namespace cgv{
	namespace math{


///creates different type of color map matrices
///colors are stored as columns in the resulting matrix
///the number of columns is defined by the parameter steps
///colormaps can be chosen by name :
/// possible colormaps are 
/// "gray" black .. white
/// "bone" grayscale colormap with a higher blue component (looks like xray images)
/// "jet" blue .. cyan .. yellow .. orange .. red
/// "hot" black .. red .. yellow .. white
/// "thermo" black .. magenta .. blue.. cyan..green .. yellow ..red.. white
/// "hsv" red .. yellow .. green .. cyan .. blue .. magenta .. red

template <typename T>
inline mat<T> create_color_map(unsigned steps=256,std::string name="jet")
{
	
	mat<T> map;
	map.resize(3,steps);
	
	if(name == "gray")
	{
		vec<T> ls = lin_space<T>(0,1,steps);
		
		//a linear grayscale colormap.
		for(unsigned i = 0; i < steps;i++)
		{
			map(0,i)=ls(i);
			map(1,i)=ls(i);
			map(2,i)=ls(i);		
		}
		return map;
	}
	if(name == "bone")
	{
		vec<T> x;
		x=lin_space<T>(0,1,steps);
		//bone
		//is a grayscale colormap with a higher value for the blue component. 
		//This colormap is useful for adding an "electronic" look to grayscale images.
		for(unsigned i = 0; i < steps;i++)
		{
			if(x(i) < 3.0f/4.0f)
				map(0,i)=(7.0f/8.0f * x(i));
			else
				map(0,i)=(11.0f/8.0f * x(i) - 3.0f/8.0f);
			
			if(x(i) < 3.0f/8.0f)
				map(1,i)=(7.0f/8.0f * x(i));
			else if(x(i) < 3.0f/4.0f)
				map(1,i)=(29.0f/24.0f * x(i) - 1.0f/8.0f);
			else
				map(1,i)=(7.0f/8.0f * x(i) + 1.0f/8.0f);

			if(x(i) < 3.0f/8.0f)
				map(2,i)= (29.0f/24.0f * x(i));
			else
				map(2,i)=(7.0f/8.0f * x(i) + 1.0f/8.0f);
	

		}
		return map;
	}
	if(name == "jet")
	{
		vec<T> x;
		x=lin_space<T>(0,1,steps);
		
		for(unsigned i = 0; i < steps;i++)
		{
			map(0,i)=map(1,i)=map(2,i)=0.0f;
			
			if((x(i) >= 3.0f/8.0f) && (x(i) < 5.0f/8.0f)) 
				map(0,i) = (4.0f * x(i) - 3.0f/2.0f);
			
			if((x(i) >= 5.0f/8.0f) && (x(i) < 7.0f/8.0f))
			{
				map(0,i)=1.0f;
			}
			if(x(i) >= 7.0f/8.0f)
				map(0,i)+= -4.0f * x(i) + 9.0f/2.0f;
			
			
			if(x(i) >= 1.0f/8.0f && x(i) < 3.0f/8.0f)
				map(1,i)= (4.0f * x(i) - 1.0f/2.0f);

			if(x(i) >= 3.0f/8.0f && x(i) < 5.0f/8.0f)
			{ 
				map(1,i)=1;
			}
			if(x(i) >= 5.0f/8.0f && x(i) < 7.0f/8.0f)
					map(1,i)+= (-4.0f * x(i) + 7.0f/2.0f);
			

			if(x(i) < 1.0f/8.0f)
				map(2,i)= (4.0f * x(i) + 1.0f/2.0f);

			if(x(i) >= 1.0f/8.0f && x(i) < 3.0f/8.0f)
			{
				map(2,i)=1.0;
			}
			if(x(i) >= 3.0f/8.0f && x(i) < 5.0f/8.0f)
				map(2,i) += -4.0f * x(i) + 5.0f/2.0f;
			

	

		}
		return map;
	}
	if(name == "hot")
	{
		vec<T> x;
		x=lin_space<T>(0,1,steps);
		
		for(unsigned i = 0; i < steps;i++)
		{
			map(0,i)=map(1,i)=map(2,i)=0.0f;
			
			if(x(i) <= 1.0f/3.0f) 
				map(0,i) =  3.0f*x(i);
			else
				map(0,i) =  1.0f;
			
		
			if(x(i) > 1.0f/3.0f && x(i) < 2.0f/3.0f)
				map(1,i)= 3.0f*x(i) -1.0f;
			if(x(i) >=2.0/3.0f)
				map(1,i)=1.0f;
			
			
			if(x(i) >= 2.0f/3.0f)
				map(2,i)=3.0f*x(i)-2.0f;
			

		}
		return map;
	}
	if(name == "hsv")
	{
		vec<T> x;
		x=lin_space<T>(0,1,steps);
		
		for(unsigned i = 0; i < steps;i++)
		{
			map(0,i)=map(1,i)=map(2,i)=0.0f;
			
			if(x(i) <= 1.0f/5.0f) 
				map(0,i) =  1.0f;
			if(x(i) > 1.0f/5.0f && x(i) <=2.0f/5.0f)
				map(0,i) =  -5.0f*x(i) + 2.0f;
			if(x(i) > 4.0f/5.0f)
				map(0,i) = 5.0f*x(i)-4.0f;

			if(x(i) <= 1.0f/5.0f) 
				map(1,i) =  5.0f*x(i);
			if(x(i) > 1.0f/5.0f && x(i) <=3.0f/5.0f)
				map(1,i) =  1.0f;
			if(x(i) > 3.0f/5.0f && x(i) <= 4.0f/5.0f)
				map(1,i) = -5.0f*x(i)+4.0f;


			
			if(x(i) > 2.0f/5.0f && x(i) <=3.0f/5.0f)
				map(2,i) =  5.0f*x(i)-2.0f;
			if(x(i) > 3.0f/5.0f && x(i) <= 4.0f/5.0f)
				map(2,i) = 1.0f;
			if(x(i) >4.0f/5.0f) 
				map(2,i) =  -5.0f*x(i)+5.0f;
				
			
			
		}
		return map;
	}
	if(name == "thermo")
	{
		vec<T> x;
		x=lin_space<T>(0,1,steps);
		
		for(unsigned i = 0; i < steps;i++)
		{
			map(0,i)=map(1,i)=map(2,i)=0.0f;
		

			if(x(i) <= 1.0f/7.0f) 
				map(0,i) =  7.0f*x(i);
			if(x(i) > 1.0f/7.0f && x(i) <=2.0f/7.0f)
				map(0,i) =  -7.0f*x(i) + 2.0f;
			if(x(i) > 4.0f/7.0f && x(i) <=5.0f/7.0f)
				map(0,i) = 7.0f*x(i)-4.0f;
			if(x(i) >5.0f/7.0f)
				map(0,i) = 1.0f;

			if(x(i) >=2.0f/7.0f && x(i) <= 3.0f/7.0f) 
				map(1,i) =  7.0f*x(i)-2.0f;
			else if(x(i) > 3.0f/7.0f && x(i) <=5.0f/7.0f)
				map(1,i) =  1.0f;
			else if(x(i) > 5.0f/7.0f && x(i) <=6.0f/7.0f)
				map(1,i) = -7.0f*x(i)+6.0f;
			else if(x(i) > 6.0f/7.0f)
				map(1,i) = 7.0f*x(i)-6.0f;

			if(x(i) <= 1.0f/7.0f) 
				map(2,i) = 7.0f*x(i);
			if(x(i) > 1.0f/7.0f && x(i) <=3.0f/7.0f)
				map(2,i) =  1.0f;
			if(x(i) > 3.0f/7.0f && x(i) <=4.0f/7.0f)
				map(2,i) = -7.0f*x(i)+4.0f;
			if(x(i) > 6.0f/7.0f)
				map(2,i) = 7.0f*x(i)-6.0f;


		}
		return map;

	}
	assert(false); //colormap not found
	return map;
}


template <typename T>
	math::vec<T> rgb_2_cmy(const math::vec<T>& rgb)
	{
		math::vec<T> cmy(3);
		cmy.fill(1);
		cmy-=rgb;
		return cmy;
	}

	template <typename T>
	void rgb_2_cmy(math::mat<T>& m)
	{
		for(unsigned i = 0; i < m.ncols();i++)
			m.set_col(i,rgb_2_cmy(m.col(i)));
		
	}

	template <typename T>
	math::vec<T> cmy_2_rgb(const math::vec<T>& cmy)
	{
		math::vec<T> rgb(3);
		rgb.fill(1);
		rgb-=cmy;
		return rgb;
	}

	template <typename T>
	void cmy_2_rgb(math::mat<T>& m)
	{
		for(unsigned i = 0; i < m.ncols();i++)
			m.set_col(i,cmy_2_rgb(m.col(i)));
		
	}

	

	template <typename T>
	math::vec<T> rgb_2_hsv(const math::vec<T>& rgb)
	{
		T _min, _max, _delta;
		_min = minimum( rgb(0), rgb(1), rgb(2) );
		_max = maximum( rgb(0), rgb(1), rgb(2) );
		vec<T> hsv(3);

		hsv(2) = _max;				// v
		_delta = _max - _min;
		if( _max != (T)0 )
			hsv(1) = _delta / _max;		// s
		else 
		{	
			hsv(1) = (T)0;
			hsv(0) = (T)-1;
			return hsv;
		}
		if( rgb(0) == _max )
			hsv(0) = ( rgb(1) - rgb(2) ) / _delta;		// between yellow & magenta
		else if( rgb(1) == _max )
			hsv(0) = 2 + ( rgb(2) - rgb(0) ) / _delta;	// between cyan & yellow
		else
			hsv(0) = 4 + ( rgb(0) - rgb(1) ) / _delta;	// between magenta & cyan
		hsv(0) *= (T)60;				// degrees
		if(	hsv(0) < (T)0 )
			hsv(0) += (T)360;
		return hsv;

	}

	template <typename T>
	void rgb_2_hsv(math::mat<T>& m)
	{
		for(unsigned i = 0; i < m.ncols();i++)
			m.set_col(i,rgb_2_hsv(m.col(i)));	
	}

	template <typename T>
	math::vec<T> hsv_2_rgb(const math::vec<T>& hsv)
	{
		math::vec<T> rgb(3);
		int i;
		T f, p, q, t;
		if( hsv(1) == 0 ) {
			// achromatic (grey)
			rgb.fill(hsv(2));
			return rgb;
		}
		T h =hsv(0)/ (T)60;			// sector 0 to 5
		i = (int)floor( hsv(0) );
		f = h - (T)i;			// factorial part of h
		p = hsv(2) * ( (T)1 - hsv(1) );
		q = hsv(2) * ( (T)1 - hsv(1) * f );
		t = hsv(2) * ( (T)1 - hsv(1) * ( (T)1 - f ) );
		switch( i ) {
			case 0:
				rgb(0) = hsv(2);
				rgb(1) = t;
				rgb(2) = p;
				break;
			case 1:
				rgb(0) = q;
				rgb(1) = hsv(2);
				rgb(2) = p;
				break;
			case 2:
				rgb(0) = p;
				rgb(1) = hsv(2);
				rgb(2) = t;
				break;
			case 3:
				rgb(0) = p;
				rgb(1) = q;
				rgb(2) = hsv(2);
				break;
			case 4:
				rgb(0) = t;
				rgb(1) = p;
				rgb(2) = hsv(2);
				break;
			default:		// case 5:
				rgb(0) = hsv(2);
				rgb(1) = p;
				rgb(2) = q;
				break;
		}
		return rgb;
	}

	template <typename T>
	void hsvb_2_rgb(math::mat<T>& m)
	{
		for(unsigned i = 0; i < m.ncols();i++)
			m.set_col(i,hsv_2_rgb(m.col(i)));	
	}

	template <typename T>
	math::vec<T> rgb_2_yiq(const math::vec<T>& rgb)
	{
		math::vec<T> yiq(3);
		yiq(0) = (T)0.299*rgb(0) + (T)0.587*rgb(1) + (T)0.114*rgb(2);
		yiq(1) = (T)0.596*rgb(0) - (T)0.275*rgb(1) - (T)0.321*rgb(2);
		yiq(2) = (T)0.212*rgb(0) - (T)0.523*rgb(1) + (T)0.311*rgb(2);
		return yiq;
	}

	template <typename T>
	T rgb_2_gray(const math::vec<T>& rgb)
	{	
		return (T)0.299*rgb(0) + (T)0.587*rgb(1) + (T)0.114*rgb(2);
	}

	template <typename T>
	void rgb_2_gray(const math::mat<T>& rgb, math::vec<T>& g)
	{
		g.resize(rgb.ncols());

		for(unsigned i = 0; i < m.ncols();i++)
			g(i)= (T)0.299*rgb(0,i) + (T)0.587*rgb(1,i) + (T)0.114*rgb(2,i);

	}

	template <typename T>
	void rgb_2_gray(math::mat<T>& rgb)
	{
		

		for(unsigned i = 0; i < rgb.ncols();i++)
		{
			rgb(0,i)= (T)0.299*rgb(0,i) + (T)0.587*rgb(1,i) + (T)0.114*rgb(2,i);
			rgb(1,i)=rgb(2,i)=rgb(0,i);
		}

	}

	template <typename T>
	void rgb_2_yiq(math::mat<T>& m)
	{
		for(unsigned i = 0; i < m.ncols();i++)
			m.set_col(i,rgb_2_yiq(m.col(i)));	
	}

	template <typename T>
	math::vec<T> yiq_2_rgb(const math::vec<T>& yiq)
	{
		math::vec<T> rgb(3);
		rgb(0) = yiq(0) + (T)0.956*yiq(1) + (T)0.621*yiq(2);
		rgb(1) = yiq(0) - (T)0.272*yiq(1) - (T)0.647*yiq(2);
		rgb(2) = yiq(0) - (T)1.105*yiq(1) + (T)1.702*yiq(2);
		return rgb;
	}

	template <typename T>
	void yiq_2_rgb(math::mat<T>& m)
	{
		for(unsigned i = 0; i < m.ncols();i++)
			m.set_col(i,yiq_2_rgb(m.col(i)));	
	}

	template <typename T>
	math::vec<T> rgb_2_xyz(const math::vec<T>& rgb)
	{
		math::vec<T> xyz(3);
		xyz(0) =  (T)0.412453*rgb(0) + (T)0.357580*rgb(1) + (T)0.180423*rgb(2);
		xyz(1) =  (T)0.212671*rgb(0) + (T)0.715160*rgb(1) + (T)0.072169*rgb(2);
		xyz(2) =  (T)0.019334*rgb(0) + (T)0.119193*rgb(1) + (T)0.950227*rgb(2);	
		return xyz;
	}

	///convert matrix columns from rgb to xyz
	template <typename T>
	void rgb_2_xyz(math::mat<T>& m)
	{
		for(unsigned i = 0; i < m.ncols();i++)
			m.set_col(i,rgb_2_xyz(m.col(i)));	
	}

	///convert colo vector from xyz to rgb
	template <typename T>
	math::vec<T> xyz_2_rgb(const math::vec<T>& xyz)
	{
		math::vec<T> rgb(3);
		rgb(0) =  (T)3.240479*xyz(0) - (T)1.537150*xyz(1) - (T)0.498535*xyz(2);
		rgb(1) = -(T)0.969256*xyz(0) + (T)1.875992*xyz(1) + (T)0.041556*xyz(2);
		rgb(2) =  (T)0.055648*xyz(0) - (T)0.204043*xyz(1) + (T)1.057311*xyz(2);
		return rgb;
	}

	///convert matrix columns from xyz to rgb
	template <typename T>
	void xyz_2_rgb(math::mat<T>& m)
	{
		for(unsigned i = 0; i < m.ncols();i++)
			m.set_col(i,xyz_2_rgb(m.col(i)));	
	}




}
}
