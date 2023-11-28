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
	void* ref_void(int i, int j);
	const void* get_void(int i, int j) const;
public:
	image_view();
	int get_width() const { return df.get_width(); }
	int get_height() const { return df.get_height(); }
	data_view& ref_data_view() { return dv; }
	void create(const std::string& fmt_dcr, int w=-1, int h=-1);
	void clear(double v);
	
	/*
	bool read(const std::string& fn);
	bool write(const std::string& fn) const;
	*/

	template <typename T>
	T& ref(int i, int j) { return *((T*)ref_void(i,j)); }
	template <typename T>
	const T& get(int i, int j) const { return *((const T*)get_void(i,j)); }
	template <typename T>
	T* ptr(int i, int j) { return (T*)ref_void(i,j); }
	void copy_rectangle(const image_view& src, int X = 0, int Y = 0, int x = 0, int y = 0, int w = -1, int h = -1);
	void sub_sample();
};

	}
}

#include <cgv/config/lib_end.h>