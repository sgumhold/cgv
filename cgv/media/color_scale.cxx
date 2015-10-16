#include "color_scale.h"

namespace cgv {
	namespace media {

/// compute an rgb color according to the selected color scale
color<float,RGB> color_scale(double v, ColorScale cs)
{
	switch (cs) {
	case CS_TEMPERATURE :
		if (v < 1.0/3)
			return color<float,RGB>((float)(3*v),0,0);
		if (v < 2.0/3)
			return color<float,RGB>(1,(float)(3*v)-1,0);
		return color<float,RGB>(1,1,(float)(3*v)-2);
	case CS_HUE :
		return color<float,RGB>(color<float,HLS>((float)v,0.5f,1));
	case CS_HUE_LUMINANCE :
		return color<float,RGB>(color<float,HLS>((float)v,(float)v,1));
	}
	return color<float,RGB>(0,0,0);
}

	}
}
