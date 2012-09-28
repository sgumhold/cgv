#pragma once

#include "lib_begin.h"

namespace cgv {
	namespace utils {

struct CGV_API time
{
	unsigned char h;
	unsigned char minutes;
	unsigned char sec;
	time(unsigned char _h = 0, unsigned _min = 0, unsigned char _sec = 0);
};

struct CGV_API date
{
	unsigned short year;
	unsigned char month;
	unsigned char day;
	date(unsigned short _year, unsigned char _month = 0, unsigned char _day = 0);
};

struct CGV_API date_time : public date, public time
{
	date_time(const time& t, const date& d);
	date_time(const date& d);
	/// compute the difference of two points in time in seconds
	long operator - (const date_time& dt) const;
	/// add given number of seconds to the point in time
	date_time operator + (long secs) const;
	/// subtract given number of seconds to the point in time
	date_time operator - (long secs) const;
};

extern CGV_API date_time now();

	}
}

#include <cgv/config/lib_end.h>
