#include "date_time.h"
#include <time.h>

#ifdef _WIN32
#pragma warning(disable:4996)
#endif

namespace cgv {
	namespace utils {

time::time(unsigned char _h, unsigned _min, unsigned char _sec) 
	: h(_h), minutes(_min), sec(_sec) 
{
}


date::date(unsigned short _year, unsigned char _month, unsigned char _day) 
	: year(_year), month(_month), day(_day) 
{
}

date_time::date_time(const time& t, const date& d) 
	: date(d), time(t)
{
}
date_time::date_time(const date& d) 
	: date(d) 
{
}


time_t convert_to_time(const date_time& dt)
{
	tm t;
	t.tm_sec = dt.sec;
   t.tm_min = dt.minutes;
	t.tm_hour = dt.h;   
	t.tm_mday = dt.day-1;   
	t.tm_mon = dt.month-1;    
	t.tm_year = dt.year;   
//	t.tm_wday;   
//	t.tm_yday;   
	return mktime(&t);
}

date_time convert_to_date_time(time_t ti)
{
	const tm& t = *gmtime(&ti);
	return date_time(time(t.tm_hour,t.tm_min,t.tm_sec),date(t.tm_year,t.tm_mon+1,t.tm_mday+1));
}

/// compute the difference of two points in time in seconds
long date_time::operator - (const date_time& dt) const
{
	return (long)convert_to_time(*this)-(long)convert_to_time(dt);
}

/// add given number of seconds to the point in time
date_time date_time::operator + (long secs) const
{
	return convert_to_date_time(convert_to_time(*this)+secs);
}

/// subtract given number of seconds to the point in time
date_time date_time::operator - (long secs) const
{
	return convert_to_date_time(convert_to_time(*this)-secs);
}

date_time now()
{
	return convert_to_date_time(::time(NULL));
}

std::ostream& operator << (std::ostream& os, const time& T)
{
	return os << int(T.h) << ':' << int(T.minutes) << ':' << int(T.sec);
}

std::ostream& operator << (std::ostream& os, const date& D)
{
	return os << int(D.day) << '.' << int(D.month) << '.' << int(D.year);
}

std::ostream& operator << (std::ostream& os, const date_time& DT)
{
	return os << static_cast<const time&>(DT) << ' ' << static_cast<const date&>(DT);
}

	}
}
