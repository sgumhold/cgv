#include "image_view.h"
#include <algorithm>

using namespace cgv::type;

namespace cgv {
	namespace data {

template <typename T, typename S>
void sub_sample_2_2(const const_data_view& src, data_view& dest)
{
	size_t w = src.get_format()->get_width();
	size_t h = src.get_format()->get_height();
	size_t W = dest.get_format()->get_width();
	size_t H = dest.get_format()->get_height();
	
	size_t dx = src.get_step_size(1)/sizeof(T);
	size_t dy = src.get_step_size(0)/sizeof(T);
	const T* p0 = src.get_ptr<T>();
	const T* p1 = p0+dy;
	
	T* q = dest.get_ptr<T>();
	size_t Dx = dest.get_step_size(1)/sizeof(T);
	size_t Dy = dest.get_step_size(0)/sizeof(T);
	for (size_t i=0; i<H;++i) {
		for (size_t j=0; j<W;++j) {
			for (size_t k=0; k<dx; ++k) {
				*q = (T) ( ( (S)p0[0] + p1[0] + p0[dx] + p1[dx] ) / 4 );
				++q;
				++p0;
				++p1;
			}
			p0 += dx;
			p1 += dx;
		}
		p0 += 2*dy - 2*W*dx;
		p1 += 2*dy - 2*W*dx;
		q += Dy - W*Dx;
	}
}
template <typename T>
void copy_sub_view(const const_data_view& src, ptrdiff_t x, ptrdiff_t y, ptrdiff_t w, ptrdiff_t h,
				   data_view& dst, ptrdiff_t X, ptrdiff_t Y)
{ 
	ptrdiff_t max_w = src.get_format()->get_width() - x;
	ptrdiff_t max_h = src.get_format()->get_height() - y;
	ptrdiff_t max_W = dst.get_format()->get_width() - X;
	ptrdiff_t max_H = dst.get_format()->get_height() - Y;
	if (w == -1)
		w = max_w;
	if (h == -1)
		h = max_h;

	w = std::min(max_w,max_W);
	h = std::min(max_h,max_H);

	size_t dx = src.get_step_size(1)/sizeof(T);
	size_t dy = src.get_step_size(0)/sizeof(T);
	const T* p = src.get_ptr<T>()+x*dx+y*dy;
	
	size_t Dx = dst.get_step_size(1)/sizeof(T);
	size_t Dy = dst.get_step_size(0)/sizeof(T);
	T* q = dst.get_ptr<T>()+X*Dx+Y*Dy;
	for (size_t i=0; i<size_t(h);++i) {
		for (size_t j=0; j<size_t(w);++j) {
			for (size_t k=0; k<dx;++k) {
				*q = *p;
				++q;
				++p;
			}
		}
		p += dy - w*dx;
		q += Dy - w*Dx;
	}
}


image_view::image_view()
{
}

void image_view::create(const std::string& fmt_dcr, ptrdiff_t w, ptrdiff_t h)
{
	dv.~data_view();
	df.set_data_format(fmt_dcr);
	if (w != -1)
		df.set_width(w);
	if (h != -1)
		df.set_height(h);
	new (&dv) data_view(&df);
}

void* image_view::ref_void(size_t i, size_t j)
{
	return dv.get_ptr<unsigned char>()+dv.get_step_size(1)*i+dv.get_step_size(0)*j;
}

const void* image_view::get_void(size_t i, size_t j) const
{
	return dv.get_ptr<unsigned char>()+dv.get_step_size(1)*i+dv.get_step_size(0)*j;
}

void image_view::clear(double v)
{
	size_t n = df.get_nr_entries()*df.get_nr_components();
	switch (df.get_component_type()) {
	case TI_INT8 :  
		std::fill(dv.get_ptr<int8_type>(), dv.get_ptr<int8_type>()+n, (int8_type)(v*127));
		break;
	case TI_INT16 :
		std::fill(dv.get_ptr<int16_type>(), dv.get_ptr<int16_type>()+n, (int16_type)(v*32767));
		break;
	case TI_INT32 :
		std::fill(dv.get_ptr<int32_type>(), dv.get_ptr<int32_type>()+n, (int32_type)(v*2147483647));
		break;
	case TI_INT64 :
		std::fill(dv.get_ptr<int64_type>(), dv.get_ptr<int64_type>()+n, (int64_type)(v*double(9223372036854775807)));
		break;
	case TI_UINT8 :
		std::fill(dv.get_ptr<uint8_type>(), dv.get_ptr<uint8_type>()+n, (uint8_type)(v*255));
		break;
	case TI_UINT16 :
		std::fill(dv.get_ptr<uint16_type>(), dv.get_ptr<uint16_type>()+n, (uint16_type)(v*65535));
		break;
	case TI_UINT32 :
		std::fill(dv.get_ptr<uint32_type>(), dv.get_ptr<uint32_type>()+n, (uint32_type)(v*4294967295));
		break;
	case TI_UINT64 :
		std::fill(dv.get_ptr<uint64_type>(), dv.get_ptr<uint64_type>()+n, (uint64_type)(v*double(18446744073709551615lu)));
		break;
	case TI_FLT32 :
		std::fill(dv.get_ptr<flt32_type>(), dv.get_ptr<flt32_type>()+n, (flt32_type)v);
		break;
	case TI_FLT64 : 
		std::fill(dv.get_ptr<flt64_type>(), dv.get_ptr<flt64_type>()+n, (flt64_type)v);
		break;
	}
}
void image_view::copy_rectangle(const image_view& src, ptrdiff_t X, ptrdiff_t Y, ptrdiff_t x, ptrdiff_t y, ptrdiff_t w, ptrdiff_t h)
{
	switch (df.get_component_type()) {
	case TI_INT8   : copy_sub_view<int8_type>(src.dv, x, y, w, h, dv, X, Y); break;
	case TI_INT16  : copy_sub_view<int16_type>(src.dv, x, y, w, h, dv, X, Y); break;
	case TI_INT32  : copy_sub_view<int32_type>(src.dv, x, y, w, h, dv, X, Y); break;
	case TI_INT64  : copy_sub_view<int64_type>(src.dv, x, y, w, h, dv, X, Y); break;
	case TI_UINT8  : copy_sub_view<uint8_type>(src.dv, x, y, w, h, dv, X, Y); break;
	case TI_UINT16 : copy_sub_view<uint16_type>(src.dv, x, y, w, h, dv, X, Y); break;
	case TI_UINT32 : copy_sub_view<uint32_type>(src.dv, x, y, w, h, dv, X, Y); break;
	case TI_UINT64 : copy_sub_view<uint64_type>(src.dv, x, y, w, h, dv, X, Y); break;
	case TI_FLT32  : copy_sub_view<flt32_type>(src.dv, x, y, w, h, dv, X, Y); break;
	case TI_FLT64  : copy_sub_view<flt64_type>(src.dv, x, y, w, h, dv, X, Y); break;
	}
}
void image_view::sub_sample()
{
	data_format df1(df);
	df1.set_width(df1.get_width()/2);
	df1.set_height(df1.get_height()/2);
	data_view dv1(&df1);
	switch (df.get_component_type()) {
	case TI_INT8 :  
		sub_sample_2_2<int8_type,int16_type>(const_data_view(dv),dv1);
		break;
	case TI_INT16 :
		sub_sample_2_2<int16_type,int32_type>(const_data_view(dv),dv1);
		break;
	case TI_INT32 :
		sub_sample_2_2<int32_type,int64_type>(const_data_view(dv),dv1);
		break;
	case TI_INT64 :
		sub_sample_2_2<int64_type,int64_type>(const_data_view(dv),dv1);
		break;
	case TI_UINT8 :
		sub_sample_2_2<uint8_type,uint16_type>(const_data_view(dv),dv1);
		break;
	case TI_UINT16 :
		sub_sample_2_2<uint16_type,uint32_type>(const_data_view(dv),dv1);
		break;
	case TI_UINT32 :
		sub_sample_2_2<uint32_type,uint64_type>(const_data_view(dv),dv1);
		break;
	case TI_UINT64 :
		sub_sample_2_2<uint64_type,uint64_type>(const_data_view(dv),dv1);
		break;
	case TI_FLT32 :
		sub_sample_2_2<flt32_type,flt32_type>(const_data_view(dv),dv1);
		break;
	case TI_FLT64 : 
		sub_sample_2_2<flt64_type,flt64_type>(const_data_view(dv),dv1);
		break;
	}
	df = df1;
	dv = dv1;
	dv.set_format(&df);
}

	}
}
