#pragma once

#include "data_view.h"

#include "lib_begin.h"

namespace cgv {
	namespace data {

class CGV_API image_view
{
public:
	data_format df;
	data_view dv;
	void* ref_void(size_t i, size_t j);
	const void* get_void(size_t i, size_t j) const;
public:
	image_view();
	size_t get_width() const { return df.get_width(); }
	size_t get_height() const { return df.get_height(); }
	data_view& ref_data_view() { return dv; }
	void create(const std::string& fmt_dcr, std::ptrdiff_t w=-1, std::ptrdiff_t h=-1);
	void clear(double v);
	template <typename T>
	T& ref(size_t i, size_t j) { return *((T*)ref_void(i,j)); }
	template <typename T>
	const T& get(size_t i, size_t j) const { return *((const T*)get_void(i,j)); }
	template <typename T>
	T* ptr(size_t i, size_t j) { return (T*)ref_void(i,j); }
	void copy_rectangle(const image_view& src, std::ptrdiff_t X = 0, std::ptrdiff_t Y = 0, std::ptrdiff_t x = 0, std::ptrdiff_t y = 0, std::ptrdiff_t w = -1, std::ptrdiff_t h = -1);
	void sub_sample();
};

	}
}

#include <cgv/config/lib_end.h>