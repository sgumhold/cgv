#pragma once
@exclude <cgv/config/ppp.ppp>
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
@for (i=1; i<3*N_ARG; ++i) @{
//-----------------------------------------------------------------------------
template <@["class T0";",";"class T".(i-1)]>
inline void oprintf(std::ostream& stream_, const char fmt_[],@["T0 t0";",";"T".(i-1)." t".(i-1)])
{
  ostream_printf print(stream_,fmt_);
  print << @["t0";" << ";"t".(i-1)];
}
@}

	}
}

#include <cgv/config/lib_end.h>