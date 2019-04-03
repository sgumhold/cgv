#pragma once

#include <iostream>
#include <string>

#include <cgv/utils/lib_begin.h>

namespace cgv {
	namespace utils {

///////////////////////////////////////////////////////////////////////////////
// ostream_printf
///////////////////////////////////////////////////////////////////////////////
class CGV_API ostream_printf 
{
public:
  ostream_printf(std::ostream& stream_, const char fmt_[])
    : m_stream(stream_)
    , m_format(fmt_)
    , m_pos(0)
  {
    /* print out initial static text data */
    printStaticData();
  }

  ~ostream_printf()
  {
    /* print out remaining static text data */
    printStaticData();

    /* entire formatting string was not used, too few arguments */
//    invariant( m_format[m_pos] == '\0' );
  }

  inline ostream_printf& operator<<(bool  value_)                   { parseFormatString(); m_stream << value_; return *this; }
  inline ostream_printf& operator<<(void* value_)                   { parseFormatString(); m_stream << value_; return *this; }

  inline ostream_printf& operator<<(char      value_)               { parseFormatString(); m_stream << value_; return *this; }
  inline ostream_printf& operator<<(short     value_)               { parseFormatString(); m_stream << value_; return *this; }
  inline ostream_printf& operator<<(int       value_)               { parseFormatString(); m_stream << value_; return *this; }
  inline ostream_printf& operator<<(long      value_)               { parseFormatString(); m_stream << value_; return *this; }
  inline ostream_printf& operator<<(long long value_)               { parseFormatString(); m_stream << value_; return *this; }

  inline ostream_printf& operator<<(unsigned char      value_)      { parseFormatString(); m_stream << value_; return *this; }
  inline ostream_printf& operator<<(unsigned short     value_)      { parseFormatString(); m_stream << value_; return *this; }
  inline ostream_printf& operator<<(unsigned int       value_)      { parseFormatString(); m_stream << value_; return *this; }
  inline ostream_printf& operator<<(unsigned long      value_)      { parseFormatString(); m_stream << value_; return *this; }
  inline ostream_printf& operator<<(unsigned long long value_)      { parseFormatString(); m_stream << value_; return *this; }

  inline ostream_printf& operator<<(float  value_)                  { parseFormatString(); m_stream << value_; return *this; }
  inline ostream_printf& operator<<(double value_)                  { parseFormatString(); m_stream << value_; return *this; }

  inline ostream_printf& operator<<(const char*           value_)   { parseFormatString(); m_stream << value_; return *this; }
  inline ostream_printf& operator<<(const unsigned char*  value_)   { parseFormatString(); m_stream << value_; return *this; }
     
protected:

  /* set stream formattings options */
  void parseFormatString();

  /* output static text data inside formatting string */
  void printStaticData();

private:

  /* disable assignment operator and copy constructor */
  ostream_printf( ostream_printf& temp_ );
  ostream_printf& operator=( ostream_printf& temp_ );

  std::ostream& m_stream;
  const char*   m_format;
  int           m_pos;
};

///////////////////////////////////////////////////////////////////////////////
//  oprintf
///////////////////////////////////////////////////////////////////////////////
inline void oprintf(std::ostream& stream_, const char fmt_[])
{
  ostream_printf print(stream_,fmt_);
}

//-----------------------------------------------------------------------------
template <class T0>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0)
{
  ostream_printf print(stream_,fmt_);
  print << t0;
}

//-----------------------------------------------------------------------------
template <class T0,class T1>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11,T12 t12)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11,T12 t12,T13 t13)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11,T12 t12,T13 t13,T14 t14)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11,T12 t12,T13 t13,T14 t14,T15 t15)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11,T12 t12,T13 t13,T14 t14,T15 t15,T16 t16)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11,T12 t12,T13 t13,T14 t14,T15 t15,T16 t16,T17 t17)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11,T12 t12,T13 t13,T14 t14,T15 t15,T16 t16,T17 t17,T18 t18)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11,T12 t12,T13 t13,T14 t14,T15 t15,T16 t16,T17 t17,T18 t18,T19 t19)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11,T12 t12,T13 t13,T14 t14,T15 t15,T16 t16,T17 t17,T18 t18,T19 t19,T20 t20)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19 << t20;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11,T12 t12,T13 t13,T14 t14,T15 t15,T16 t16,T17 t17,T18 t18,T19 t19,T20 t20,T21 t21)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19 << t20 << t21;
}

//-----------------------------------------------------------------------------
template <class T0,class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18,class T19,class T20,class T21,class T22>
inline void oprintf(std::ostream& stream_, const char fmt_[],T0 t0,T1 t1,T2 t2,T3 t3,T4 t4,T5 t5,T6 t6,T7 t7,T8 t8,T9 t9,T10 t10,T11 t11,T12 t12,T13 t13,T14 t14,T15 t15,T16 t16,T17 t17,T18 t18,T19 t19,T20 t20,T21 t21,T22 t22)
{
  ostream_printf print(stream_,fmt_);
  print << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10 << t11 << t12 << t13 << t14 << t15 << t16 << t17 << t18 << t19 << t20 << t21 << t22;
}


	}
}

#include <cgv/config/lib_end.h>