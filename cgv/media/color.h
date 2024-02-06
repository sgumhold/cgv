#pragma once

#include <iostream>
#include <limits>
#include <cgv/defines/assert.h>
#include <cgv/type/func/promote.h>
#include <cgv/type/func/promote_const.h>
#include <cgv/type/traits/max.h>
#include <cgv/type/info/type_name.h>
#include "color_model.hh"
#include <math.h>

namespace cgv {
namespace media {

template <typename T>
struct color_one
{
	static T value() { return type::traits::max<T>::value; }
};
template <>
struct color_one<float>
{
	static float value() { return 1.0f; }
};
template <>
struct color_one<double>
{
	static double value() { return 1.0; }
};
template <ColorModel cm>
struct color_model_traits {
	static const unsigned int nr_components = 3;
};
template <>
struct color_model_traits<LUM> {
	static const unsigned int nr_components = 1;
};
template <AlphaModel am>
struct alpha_model_traits {
	static const unsigned int nr_components = 1;
};
template <>
struct alpha_model_traits<NO_ALPHA> {
	static const unsigned int nr_components = 0;
};

/** minimal data interface for color used to implement conversion. */
template <typename T, ColorModel cm, AlphaModel am = NO_ALPHA>
class color_base
{
public:
	/// static and const access to number of color components
	static const unsigned int nr_color_components = color_model_traits<cm>::nr_components;
	/// static and const access to number of alpha components
	static const unsigned int nr_alpha_components = alpha_model_traits<am>::nr_components;
	/// static and const access to total number of components
	static const unsigned int nr_components = nr_color_components + nr_alpha_components;
	/// type of color components
	typedef T component_type;
	/// standard constructor does not initialize components
	color_base() { }
	/// copy constructor uses color conversion if necessary
	template <typename T2, ColorModel cm2, AlphaModel am2>
	color_base(const color_base<T2, cm2, am2>& c2) {
		convert_color(c2, *this);
	}
	/// access to components
	T& at(unsigned int i) { return components[i]; }
	/// const access to components
	const T& at(unsigned int i) const { return components[i]; }
	/// return alpha component
	T& alpha() { return components[nr_color_components]; }
	/// return alpha component
	const T& alpha() const { return components[nr_color_components]; }
protected:
	T components[nr_components];
};

// forward declaration of color class
template <typename T, ColorModel cm = RGB, AlphaModel = NO_ALPHA> 
class color;

/// captures reference to alpha component of color
template <typename T, AlphaModel am> 
struct alpha_reference
{
	T& alpha_ref;
	template <ColorModel cm>
	alpha_reference(color_base<T, cm, am>& c) : alpha_ref(c.alpha()) {}
};
/// specialize for color types without alpha where reference is to static dummy alpha
template <typename T>
struct alpha_reference<T,NO_ALPHA>
{
	T& ref_dummy_alpha() { static T dummy_alpha = color_one<T>::value(); return dummy_alpha; }
	T& alpha_ref;
	template <ColorModel cm>
	alpha_reference(color_base<T,cm,NO_ALPHA>& c) : alpha_ref(ref_dummy_alpha()) {}
};
/// captures reference to alpha component of color
template <typename T, AlphaModel am> 
struct const_alpha_reference
{
	const T& alpha_ref;
	template <ColorModel cm>
	const_alpha_reference(const color_base<T, cm, am>& c) : alpha_ref(c.alpha()) {}
};
/// specialize for color types without alpha where reference is to static dummy alpha
template <typename T>
struct const_alpha_reference<T,NO_ALPHA>
{
	const T& ref_dummy_alpha() { static T dummy_alpha = color_one<T>::value(); return dummy_alpha; }
	const T& alpha_ref;
	template <ColorModel cm>
	const_alpha_reference(const color_base<T,cm,NO_ALPHA>& c) : alpha_ref(ref_dummy_alpha()) {}
};

/*********************************************************************
**
** component type conversion
**
*********************************************************************/

/// template for conversion of color components
template <typename T1, typename T2>
inline void convert_color_component(const T1& from, T2& to) {
	to = (T2)((typename type::func::promote<T1,T2>::type)(from)*color_one<T2>::value()/color_one<T1>::value());
}
/// template for conversion of color components
template <>
inline void convert_color_component(const unsigned int& from, unsigned short& to) {
	to = from >> 16;
}
/// template for conversion of color components
template <>
inline void convert_color_component(const unsigned int& from, unsigned char& to) {
	to = from >> 24;
}
/// template for conversion of color components
template <>
inline void convert_color_component(const unsigned short& from, unsigned char& to) {
	to = from >> 8;
}
/// template for conversion of color components
template <>
inline void convert_color_component(const unsigned short& from, unsigned int& to) {
	to = (unsigned int) from << 16;
}
/// template for conversion of color components
template <>
inline void convert_color_component(const unsigned char& from, unsigned int& to) {
	to = (unsigned int)from << 24;
}
/// template for conversion of color components
template <>
inline void convert_color_component(const unsigned char& from, unsigned short& to) {
	to = (unsigned short)from << 8;
}

/*********************************************************************
** alpha model conversion
*********************************************************************/
/// template for conversion of one alpha model to another, by default conversion is implemented via conversion to OPACITY
template <typename T1, AlphaModel am1, typename T2, AlphaModel am2>
inline void convert_alpha_model(const_alpha_reference<T1,am1> from, alpha_reference<T2,am2> to)
{
	color<typename type::func::promote<T1,T2>::type,LUM,OPACITY> tmp;
	alpha_reference<typename type::func::promote<T1, T2>::type, OPACITY> ref(tmp);
	convert_alpha_model(from,tmp);
	convert_alpha_model(tmp,to);
}
/// init opacity to 1
template <typename T1, typename T2>
void convert_alpha_model(const_alpha_reference<T1, NO_ALPHA> from, alpha_reference<T2, OPACITY> to)
{
	to.alpha_ref = color_one<T2>::value();
}
/// init transparency to 0
template <typename T1, typename T2>
void convert_alpha_model(const_alpha_reference<T1, NO_ALPHA> from, alpha_reference<T2, TRANSPARENCY> to)
{
	to.alpha_ref = T2(0);
}
/// init extinction to 1
template <typename T1, typename T2>
void convert_alpha_model(const_alpha_reference<T1, NO_ALPHA> from, alpha_reference<T2, EXTINCTION> to)
{
	to.alpha_ref = color_one<T2>::value();
}
/// conversion from opacity to transparency
template <typename T1, typename T2>
void convert_alpha_model(const_alpha_reference<T1, OPACITY> from, alpha_reference<T2, TRANSPARENCY> to)
{
	convert_color_component(from.alpha_ref, to.alpha_ref);
	to.alpha_ref = color_one<T2>::value() - to.alpha_ref;
}
/// conversion from transparency to opacity
template <typename T1, typename T2>
void convert_alpha_model(const_alpha_reference<T1, TRANSPARENCY> from, alpha_reference<T2, OPACITY> to)
{
	convert_color_component(from.alpha_ref, to.alpha_ref);
	to.alpha_ref = color_one<T2>::value() - to.alpha_ref;
}
/// conversion from opacity to extinction
template <typename T1, typename T2>
void convert_alpha_model(const_alpha_reference<T1, OPACITY> from, alpha_reference<T2, EXTINCTION> to)
{
	double tmp;
	convert_color_component(from.alpha_ref, tmp);
	tmp = -log(1 - tmp);
	convert_color_component(tmp, to.alpha_ref);
}
/// conversion from extinction to opacity
template <typename T1, typename T2>
void convert_alpha_model(const_alpha_reference<T1, EXTINCTION> from, alpha_reference<T2, OPACITY> to)
{
	double tmp;
	convert_color_component(from.alpha_ref, tmp);
	tmp = 1 - exp(-tmp);
	convert_color_component(tmp, to.alpha_ref);
}
/// nothing to be done if target alpha model is no alpha
template <typename T1, AlphaModel am1, typename T2>
void convert_alpha_model(const_alpha_reference<T1, am1> from, alpha_reference<T2, NO_ALPHA> to)
{
}
/// nothing to be done if target alpha model is no alpha, specialize also for source alpha being the same in order to avoid ambiguous calls
template <typename T1, typename T2>
void convert_alpha_model(const_alpha_reference<T1, NO_ALPHA> from, alpha_reference<T2, NO_ALPHA> to)
{
}
/// only alpha component conversion necessary if the alpha models are the same
template <typename T1, AlphaModel am, typename T2>
void convert_alpha_model(const_alpha_reference<T1, am> from, alpha_reference<T2, am> to)
{
	convert_color_component(from.alpha_ref, to.alpha_ref);
}

/*********************************************************************
**
** color model conversions
**
*********************************************************************/
/// template for conversion of one color model to another one, by default conversion is implemented via a conversion to XYZ.
template <typename T1, ColorModel cm1, typename T2, ColorModel cm2>
inline void convert_color_model(const color_base<T1, cm1>& from, color_base<T2, cm2>& to)
{
	color_base<typename type::func::promote<T1, T2>::type, XYZ> tmp;
	convert_color_model(from, tmp);
	convert_color_model(tmp, to);
}
/// only color component conversion necessary if the color models are the same
template <typename T1, ColorModel cm, typename T2>
void convert_color_model(const color_base<T1, cm>& from, color_base<T2, cm>& to)
{
	for (unsigned int i = 0; i < color_base<T1, cm>::nr_components; ++i)
		convert_color_component(from.at(i), to.at(i));
}
/// avoid infinite recursion by default conversion to XYZ to not implemented
template <typename T1, ColorModel cm1, typename T2>
void convert_color_model(const color_base<T1, cm1>& from, color_base<T2, XYZ>& to)
{
	std::cerr << "conversion not implemented" << std::endl;
}
/// avoid infinite recursion by default conversion from XYZ to not implemented
template <typename T1, ColorModel cm2, typename T2>
void convert_color_model(const color_base<T1, XYZ>& from, color_base<T2, cm2>& to)
{
	std::cerr << "conversion not implemented" << std::endl;
}
/// specialization for conversion from LUM to XYZ
template <typename T1, typename T2>
void convert_color_model(const color_base<T1, LUM>& c1, color_base<T2, XYZ>& c2) {
	c2.at(0) = 0;
	c2.at(2) = 0;
	convert_color_component(c1.at(0), c2.at(1));
}
/// specialization for conversion from XYZ to LUM
template <typename T1, typename T2>
void convert_color_model(const color_base<T1, XYZ>& c1, color_base<T2, LUM>& c2) {
	convert_color_component(c1.at(1), c2.at(0));
}
/// specialization for conversion from RGB to XYZ
template <typename T1, typename T2>
void convert_color_model(const color_base<T1, RGB>& _c1, color_base<T2, XYZ>& c2) {
	color_base<double, RGB> c1;
	convert_color_model(_c1,c1);
	convert_color_component(0.412453 * c1.at(0) + 0.357580 * c1.at(1) + 0.180423 * c1.at(2), c2.at(0));
	convert_color_component(0.212671 * c1.at(0) + 0.715160 * c1.at(1) + 0.072169 * c1.at(2), c2.at(1));
	convert_color_component(0.019334 * c1.at(0) + 0.119193 * c1.at(1) + 0.950227 * c1.at(2), c2.at(2));
}
/// specialization for conversion from XYZ to RGB
template <typename T1, typename T2>
void convert_color_model(const color_base<T1, XYZ>& _c1, color_base<T2, RGB>& c2) {
	color_base<double, XYZ> c1(_c1);
	convert_color_model(_c1, c1);
	convert_color_component(3.2404813432 * c1.at(0) - 1.5371515163 * c1.at(1) - 0.4985363262 * c1.at(2), c2.at(0));
	convert_color_component(-0.9692549500 * c1.at(0) + 1.8759900015 * c1.at(1) + 0.0415559266 * c1.at(2), c2.at(1));
	convert_color_component(0.0556466391 * c1.at(0) - 0.2040413384 * c1.at(1) + 1.0573110696 * c1.at(2), c2.at(2));
}
/// specialization for conversion from RGB to HLS
template <typename T1, typename T2>
void convert_color_model(const color_base<T1, RGB>& rgb, color_base<T2, HLS>& hls) {
	double mx, mn;
	convert_color_component(std::max(rgb.at(0), std::max(rgb.at(1), rgb.at(2))), mx);
	convert_color_component(std::min(rgb.at(0), std::min(rgb.at(1), rgb.at(2))), mn);
	double LL = (mx + mn) / 2;
	if (mn == mx) {
		hls.at(0) = hls.at(2) = 0;
		convert_color_component(LL, hls.at(1));
	}
	else {
		double DM = mx - mn;
		double SM = mx + mn;
		double SS = (LL <= 0.5) ? DM / SM : DM / (2 - SM);
		color_base<double, RGB> c;
		convert_color_model(rgb, c);
		double HH;
		if (c.at(0) == mx) HH = (c.at(1) - c.at(2)) / DM;
		else if (c.at(1) == mx) HH = 2 + (c.at(2) - c.at(0)) / DM;
		else HH = 4 + (c.at(0) - c.at(1)) / DM;
		if (HH < 0) HH += 6;
		HH /= 6;
		convert_color_component(HH, hls.at(0));
		convert_color_component(LL, hls.at(1));
		convert_color_component(SS, hls.at(2));
	}
}
/// specialization for conversion from HLS to RGB
template <typename T1, typename T2>
void convert_color_model(const color_base<T1, HLS>& hls, color_base<T2, RGB>& rgb) {
	double HH, LL, SS;
	convert_color_component(hls.at(0), HH); HH *= 6;
	convert_color_component(hls.at(1), LL);
	convert_color_component(hls.at(2), SS);
	int    I = (int)HH;
	double F = HH - I;
	while (I >= 6) I -= 6;
	double mx = (LL <= 0.5) ? LL * (1 + SS) : LL + SS - LL * SS;
	double mn = 2 * LL - mx;
	double DM = mx - mn;
	if (SS == 0) {
		T2 tmp;
		convert_color_component(LL, tmp);
		rgb.at(0) = rgb.at(1) = rgb.at(2) = tmp;
	}
	else {
		switch (I) {
		case 0:
			convert_color_component(mx, rgb.at(0));
			convert_color_component(mn + F * DM, rgb.at(1));
			convert_color_component(mn, rgb.at(2));
			break;
		case 1:
			convert_color_component(mn + (1 - F) * DM, rgb.at(0));
			convert_color_component(mx, rgb.at(1));
			convert_color_component(mn, rgb.at(2));
			break;
		case 2:
			convert_color_component(mn, rgb.at(0));
			convert_color_component(mx, rgb.at(1));
			convert_color_component(mn + F * DM, rgb.at(2));
			break;
		case 3:
			convert_color_component(mn, rgb.at(0));
			convert_color_component(mn + (1 - F) * DM, rgb.at(1));
			convert_color_component(mx, rgb.at(2));
			break;
		case 4:
			convert_color_component(mn + F * DM, rgb.at(0));
			convert_color_component(mn, rgb.at(1));
			convert_color_component(mx, rgb.at(2));
			break;
		case 5:
			convert_color_component(mx, rgb.at(0));
			convert_color_component(mn, rgb.at(1));
			convert_color_component(mn + (1 - F) * DM, rgb.at(2));
			break;
		}
	}
}
/// specialization for conversion from HLS to XYZ via conversion to RGB
template <typename T1, typename T2>
void convert_color_model(const color_base<T1, HLS>& c1, color_base<T2, XYZ>& c2) {
	color_base<typename type::func::promote<T1, T2>::type, RGB> tmp;
	convert_color_model(c1, tmp);
	convert_color_model(tmp, c2);
}
/// specialization for conversion from XYZ to HLS via conversion to RGB
template <typename T1, typename T2>
void convert_color_model(const color_base<T1, XYZ>& c1, color_base<T2, HLS>& c2) {
	color_base<typename type::func::promote<T1, T2>::type, RGB> tmp;
	convert_color_model(c1, tmp);
	convert_color_model(tmp, c2);
}

/// template for conversion of one color to another, where the color model and the alpha model is converted
template <typename T1, ColorModel cm1, AlphaModel am1, typename T2, ColorModel cm2, AlphaModel am2>
inline void convert_color(const color_base<T1,cm1,am1>& from, color_base<T2,cm2,am2>& to)
{
	convert_color_model(reinterpret_cast<const color_base<T1,cm1>&>(from),reinterpret_cast<color_base<T2,cm2>&>(to));
	convert_alpha_model(const_alpha_reference<T1,am1>(from),alpha_reference<T2,am2>(to));
}

/// the rgb_color_interface adds access function to the R, G, and B-channels of the color, where write access is only granted for an RGB-color
template <typename ta_derived> struct rgb_color_interface {};
/// read only implementation of rgb color interface including automatic conversion
template <typename T, ColorModel cm, AlphaModel am> 
struct rgb_color_interface<color<T,cm,am> > : public color_base<T,cm,am>
{
	/// convert color to RGB and return R component
	T R() const { return color_base<T,RGB>(*this).at(0); }
	/// convert color to RGB and return G component
	T G() const { return color_base<T, RGB>(*this).at(1); }
	/// convert color to RGB and return B component
	T B() const { return color_base<T, RGB>(*this).at(2); }
};
/// read and write access implementation of rgb color interface
template <typename T, AlphaModel am>
struct rgb_color_interface<color<T,RGB,am> > : public color_base<T, RGB,am>
{
	/// write access to R component of RGB color
	T& R()             { return this->at(0); }
	/// read access to R component of RGB color
	const T& R() const { return this->at(0); }
	/// write access to G component of RGB color
	T& G()             { return this->at(1); }
	/// read access to G component of RGB color
	const T& G() const { return this->at(1); }
	/// write access to B component of RGB color
	T& B()             { return this->at(2); }
	/// read access to B component of RGB color
	const T& B() const { return this->at(2); }
};

/// the hls_color_interface adds access function to the H, L, and S-channels of the color, where write access is only granted for an HLS-color
template <typename ta_derived> struct hls_color_interface {};
/// read only implementation of hls color interface including automatic conversion
template <typename T, ColorModel cm, AlphaModel am> 
struct hls_color_interface<color<T,cm,am> > : public rgb_color_interface<color<T, cm, am> >
{
	/// convert color to HLS and return H component
	T H() const { return color_base<T, HLS>(*this).at(0); }
	/// convert color to HLS and return L component
	T L() const { return color_base<T, HLS>(*this).at(1); }
	/// convert color to HLS and return S component
	T S() const { return color_base<T, HLS>(*this).at(2); }
};
/// read and write access implementation of hls color interface
template <typename T, AlphaModel am>
struct hls_color_interface<color<T,HLS,am> > : public rgb_color_interface<color<T, HLS, am> >
{
	/// write access to H component of HLS color
	T& H()             { return this->at(0); }
	/// read access to H component of HLS color
	const T& H() const { return this->at(0); }
	/// write access to L component of HLS color
	T& L()             { return this->at(1); }
	/// read access to L component of HLS color
	const T& L() const { return this->at(1); }
	/// write access to S component of HLS color
	T& S()             { return this->at(2); }
	/// read access to S component of HLS color
	const T& S() const { return this->at(2); }
};

/// the xyz_color_interface adds access function to the X, Y, and Z-channels of the color, where write access is only granted for an XYZ-color
template <typename ta_derived> struct xyz_color_interface {};
/// read only implementation of xyz color interface including automatic conversion
template <typename T, ColorModel cm, AlphaModel am> 
struct xyz_color_interface<color<T,cm,am> > : public hls_color_interface<color<T, cm, am> >
{
	/// convert color to XYZ and return X component
	T X() const { return color_base<T,XYZ>(*this).at(0); }
	/// convert color to XYZ and return Y component
	T Y() const { return color_base<T, XYZ>(*this).at(1); }
	/// convert color to XYZ and return Z component
	T Z() const { return color_base<T, XYZ>(*this).at(2); }
};
/// read and write access implementation of xyz color interface
template <typename T, AlphaModel am>
struct xyz_color_interface<color<T,XYZ,am> > : public hls_color_interface<color<T, XYZ, am> >
{
	/// write access to X component of XYZ color
	T& X()             { return this->at(0); }
	/// read access to X component of XYZ color
	const T& X() const { return this->at(0); }
	/// write access to Y component of XYZ color
	T& Y()             { return this->at(1); }
	/// read access to Y component of XYZ color
	const T& Y() const { return this->at(1); }
	/// write access to Z component of XYZ color
	T& Z()             { return this->at(2); }
	/// read access to Z component of XYZ color
	const T& Z() const { return this->at(2); }
};

/// the opacity_alpha_interface adds access function to the opacity, where write access is only granted for an OPACITY-alpha
template <typename ta_derived> struct opacity_alpha_interface {};
/// read only implementation of opacity alpha interface including automatic conversion
template <typename T, ColorModel cm, AlphaModel am> 
struct opacity_alpha_interface<color<T,cm,am> > : public xyz_color_interface<color<T, cm, am> >
{
	/// convert alpha to opacity
	T opacity() const { 
		color<T,LUM,OPACITY> opa;
		alpha_reference<T, OPACITY> opa_ref(opa);
		convert_alpha_model(const_alpha_reference<T, am>(*this), opa_ref);
		return opa_ref.alpha_ref; 
	}
};
/// read and write access implementation of opacity alpha interface
template <typename T, ColorModel cm>
struct opacity_alpha_interface<color<T,cm,OPACITY> > :public xyz_color_interface<color<T, cm, OPACITY> >
{
	/// write access to opacity component 
	T& opacity()       { return this->alpha(); }
	/// read access to opacity component
	const T& opacity() const { return this->alpha(); }
};

/// the transparency_alpha_interface adds access function to the opacity, where write access is only granted for an OPACITY-alpha
template <typename ta_derived> struct transparency_alpha_interface{};
/// read only implementation of transparency alpha interface including automatic conversion
template <typename T, ColorModel cm, AlphaModel am> 
struct transparency_alpha_interface<color<T,cm,am> > : public opacity_alpha_interface<color<T, cm, am> >
{
	/// convert alpha to transparency
	T transparency() const { 
		color<T, LUM, TRANSPARENCY> tra;
		alpha_reference<T, TRANSPARENCY> tra_ref(tra);
		convert_alpha_model(const_alpha_reference<T, am>(*this), tra_ref);
		return tra_ref.alpha_ref;
	}
};
/// read and write access implementation of transparency alpha interface
template <typename T, ColorModel cm>
struct transparency_alpha_interface<color<T,cm,TRANSPARENCY> > : public opacity_alpha_interface<color<T, cm, TRANSPARENCY> >
{
	/// write access to transparency component 
	T& transparency()       { return this->alpha(); }
	/// read access to transparency component
	const T& transparency() const { return this->alpha(); }
};

/// the extinction_alpha_interface adds access function to the opacity, where write access is only granted for an OPACITY-alpha
template <typename ta_derived> struct extinction_alpha_interface {};
/// read only implementation of extinction alpha interface including automatic conversion
template <typename T, ColorModel cm, AlphaModel am> 
struct extinction_alpha_interface<color<T,cm,am> > : public transparency_alpha_interface<color<T, cm, am> >
{
	/// convert alpha to extinction
	T extinction() const { 
		color<T, LUM, EXTINCTION> ext;
		alpha_reference<T, EXTINCTION> ext_ref(ext);
		convert_alpha_model(const_alpha_reference<T, am>(*this), ext_ref);
		return ext_ref.alpha_ref;
	}
};
/// read and write access implementation of extinction alpha interface
template <typename T, ColorModel cm>
struct extinction_alpha_interface<color<T,cm,EXTINCTION> > : public transparency_alpha_interface<color<T, cm, EXTINCTION> >
{
	/// write access to extinction component 
	T& extinction()       { return this->alpha(); }
	/// read access to extinction component
	const T& extinction() const { return this->alpha(); }
};

/// represent a color with components of given type and color and alpha model as specified.
template <typename T, ColorModel cm, AlphaModel am>
class color : public extinction_alpha_interface<color<T,cm,am> >
{
public:
	/// static and const access to color model
	static const ColorModel clr_mod = cm;
	/// static and const access to alpha model
	static const AlphaModel alp_mod = am;
	/// static and const access to number of color components
	static const unsigned int nr_color_components = color_base<T,cm,am>::nr_color_components;
	/// static and const access to number of alpha components
	static const unsigned int nr_alpha_components = color_base<T, cm, am>::nr_alpha_components;
	/// static and const access to total number of components
	static const unsigned int nr_components = color_base<T, cm, am>::nr_components;
	/// type of color components
	typedef T component_type;
	/// standard constructor does not initialize components
	color() { }
	/// set all components to given value
	color(const T& c) { *this = c; }
	/// set first two components to given values
	color(const T& c0, const T& c1) { 
		this->components[0] = c0; 
		if (nr_components > 1)
			this->components[1] = c1;
	}
	/// set first three components to given values
	color(const T& c0, const T& c1, const T& c2) { 
		this->components[0] = c0;
		if (nr_components > 1)
			this->components[1] = c1;
		if (nr_components > 2)
			this->components[2] = c2;
	}
	/// set all four components to given values
	color(const T& c0, const T& c1, const T& c2, const T& c3) { 
		this->components[0] = c0;
		if (nr_components > 1)
			this->components[1] = c1;
		if (nr_components > 2)
			this->components[2] = c2;
		if (nr_components > 3)
			this->components[3] = c3;
	}
	/// construct from non-alpha color plus alpha
	color(const color<T, cm>& c, T a) {
		this->at(nr_components - 1) = a; // looks bit like a hack but avoids invalid access for NO_ALPHA
		for (unsigned i = 0; i < nr_color_components; ++i)
			this->at(i) = c[i];
	}
	/// copy constructor uses color conversion if necessary
	template <typename T2, ColorModel cm2, AlphaModel am2>
	color(const color<T2,cm2,am2>& c2) {
		convert_color(c2, *this);
	}
	/// assign to color with potential color conversion
	template <typename T2, ColorModel cm2, AlphaModel am2>
	color<T,cm,am>& operator = (const color<T2,cm2,am2>& c2) {
		convert_color(c2, *this);
		return *this;
	}
	/// assign all components including alpha to \c c
	color<T,cm,am>& operator = (const T& c) {
		std::fill(this->components, this->components+nr_components, c);
		return *this;
	}
	/// drop the alpha component if any by a cast operator
	      color<T,cm>& drop_alpha()       { return *((color<T,cm>*)this); }
	const color<T,cm>& drop_alpha() const { return *((const color<T,cm>*)this); }
	/// multiply with color
	template <typename T2, ColorModel cm2, AlphaModel am2>
	color<T,cm,am>& operator *= (const color<T2,cm2,am2>& c2) {
		color<T,cm,am> tmp(c2);
		for (int i=0; i<nr_components; ++i)
			this->at(i) *= tmp[i];
		return *this;
	}
	/// post multiply with color
	template <typename T2, ColorModel cm2, AlphaModel am2>
	color<T,cm,am> operator * (const color<T2,cm2,am2>& c2) {
		color<T,cm,am> res(*this);
		res *= c2;
		return res;
	}
	/// multiply with constant
	color<T,cm,am>& operator *= (const T& c) {
		for (unsigned i=0; i<nr_components; ++i)
			this->at(i) *= c;
		return *this;
	}
	/// post multiply with constant
	template <typename T2>
	color<typename type::func::promote<T,T2>::type,cm,am> operator * (const T2& c) const {
		color<typename type::func::promote<T,T2>::type,cm,am> res;
		for (unsigned i=0; i<nr_components; ++i)
			res[i] = this->at(i)*c;
		return res;
	}
	/// add color
	template <typename T2, ColorModel cm2, AlphaModel am2>
	color<T,cm,am>& operator += (const color<T2,cm2,am2>& c2) {
		color<T,cm,am> tmp(c2);
		for (unsigned i=0; i<nr_components; ++i)
			this->at(i) += tmp[i];
		return *this;
	}
	/// add color
	template <typename T2, ColorModel cm2, AlphaModel am2>
	color<T,cm,am> operator + (const color<T2,cm2,am2>& c2) {
		color<T,cm,am> res(*this);
		res += c2;
		return res;
	}
	/// add constant
	color<T,cm,am>& operator += (const T& c) {
		for (unsigned i=0; i<nr_components; ++i)
			this->at(i) += c;
		return *this;
	}
	/// add constant
	color<T,cm,am> operator + (const T& c) {
		color<T,cm,am> res(*this);
		res += c;
		return res;
	}
	/// clamp to the given range, which defaults to [0,1] of the component type
	void clamp(const T& mn = 0, const T& mx = color_one<T>::value(), bool skip_alpha = false) {
		unsigned nr = skip_alpha ? nr_color_components : nr_components;
		for (unsigned i = 0; i < nr; ++i)
			if (this->at(i) < mn)
				this->at(i) = mn;
			else if (this->at(i) > mx)
				this->at(i) = mx;
	}
	/// access to components
	T& operator [] (unsigned int i) { return this->at(i); }
	/// const access to components
	const T& operator [] (unsigned int i) const { return this->at(i); 	}
};

/*********************************************************************
**
** operators
**
*********************************************************************/

/// compare two colors of same type
template <typename T, ColorModel cm, AlphaModel am>
bool operator == (const color<T,cm,am>& c1, const color<T,cm,am>& c2) {
	for (unsigned int i=0; i<color<T,cm,am>::nr_components; ++i)
		if (c1[i] != c2[i])
			return false;
	return true;
}
/// pre multiply with color
template <typename T1, ColorModel cm1, AlphaModel am1, typename T2, ColorModel cm2, AlphaModel am2>
color<typename type::func::promote<T1,T2>::type,cm1,am1> operator * (const color<T1,cm1,am1>& c1, const color<T2,cm2,am2>& c2) {
	color<typename type::func::promote<T1,T2>::type,cm1,am1> res(c1);
	res *= c2;
	return res;
}
/// pre multiply with scalar
template <typename T1, typename T2, ColorModel cm2, AlphaModel am2>
color<typename type::func::promote<T1,T2>::type,cm2,am2> operator * (const T1& c1, const color<T2,cm2,am2>& c2) {
	color<typename type::func::promote<T1,T2>::type,cm2,am2> res(c2);
	res *= c1;
	return res;
}
/// add color
template <typename T1, ColorModel cm1, AlphaModel am1, typename T2, ColorModel cm2, AlphaModel am2>
color<typename type::func::promote<T1,T2>::type,cm1,am1> operator + (const color<T1,cm1,am1>& c1, const color<T2,cm2,am2>& c2) {
	color<typename type::func::promote<T1,T2>::type,cm1,am1> res(c1);
	res += c2;
	return res;
}
/// stream out color
template <typename T1, ColorModel cm1, AlphaModel am1>
std::ostream& operator << (std::ostream& os, const color<T1,cm1,am1>& c) {
	os << c[0];
	for (unsigned int i=1; i<color<T1,cm1,am1>::nr_components; ++i)
		os << " " << c[i];
	return os;
}
/// stream in color
template <typename T1, ColorModel cm1, AlphaModel am1>
std::istream& operator >> (std::istream& is, color<T1,cm1,am1>& c) {
	is >> c[0];
	for (unsigned int i=1; i<color<T1,cm1,am1>::nr_components; ++i)
		is >> c[i];
	return is;
}
/// stream out unsigned char color
template <ColorModel cm, AlphaModel am>
std::ostream& operator << (std::ostream& os, const color<unsigned char,cm,am>& c) {
	os << (int)c[0];
	for (unsigned int i=1; i<color<unsigned char,cm,am>::nr_components; ++i)
		os << " " << (int)c[i];
	return os;
}
/// stream in unsigned char color
template <ColorModel cm, AlphaModel am>
std::istream& operator >> (std::istream& is, color<unsigned char,cm,am>& c) {
	int tmp;
	is >> tmp; 
	c[0] = (unsigned char)tmp;
	for (unsigned int i=1; i<color<unsigned char,cm,am>::nr_components; ++i) {
		is >> tmp;
		c[i] = (unsigned char)tmp;
	}
	return is;
}

/*********************************************************************
**
** functions
**
*********************************************************************/

/// linear interpolate two colors, returns (1-t)*c1 + t*c2
template <typename T, ColorModel cm, AlphaModel am>
const color<T,cm,am> lerp(const color<T,cm,am>& c1, const color<T,cm,am>& c2, T t) {
	return ((T)1 - t) * c1 + t * c2;
}
/// special pow function for colors with RGB color model using integral types, alpha model is ignored
/// components are converted to type T2 in range [0,1] before applying the pow function
/// to ensure correct handling of integral component types like uint8_t
template <typename T1, typename T2, AlphaModel am,
	typename std::enable_if<std::is_integral<T1>::value, bool>::type = true,
	typename std::enable_if<std::is_floating_point<T2>::value, bool>::type = true>
const color<T1,RGB,am> pow(const color<T1,RGB,am>& c, T2 e) {
	constexpr T2 m = static_cast<T2>(std::numeric_limits<T1>::max());
	color<T1,RGB,am> x = c;
	for(unsigned int i=0; i<color<T1,RGB,am>::nr_color_components; ++i)
		x[i] = static_cast<T1>(std::pow(static_cast<T2>(c[i]) / m, e) * m);
	return x;
}
/// pow function for colors with RGB color model, alpha model is ignored
template<typename T, AlphaModel am, typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
const color<T,RGB,am> pow(const color<T,RGB,am>& c, T e) {
	color<T,RGB,am> x = c;
	for(unsigned int i=0; i<color<T,RGB,am>::nr_color_components; ++i)
		x[i] = std::pow(x[i], e);
	return x;
}

} // namespace media

/// @name Predefined Types
/// @{

/// declare rgb color type with 32 bit components
typedef cgv::media::color<float, cgv::media::RGB> rgb;
/// declare rgba color type with 32 bit components
typedef cgv::media::color<float, cgv::media::RGB, cgv::media::OPACITY> rgba;
/// declare rgb color type with 8 bit components
typedef cgv::media::color<cgv::type::uint8_type, cgv::media::RGB> rgb8;
/// declare rgba color type with 8 bit components
typedef cgv::media::color<cgv::type::uint8_type, cgv::media::RGB, cgv::media::OPACITY> rgba8;

/// @}

} // namespace cgv
