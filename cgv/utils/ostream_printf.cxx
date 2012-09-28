#include "ostream_printf.h"

#include <stdlib.h>

using namespace std;

namespace cgv {
	namespace utils {

//------------------------------------------------------------------------
void ostream_printf::parseFormatString()
{
  printStaticData();
  
  /* convert  printf and iso format */
  int i = m_pos;

  int width = 0;
  int precision = 6;
  int flags = 0;
  char fill = ' ';
  bool alternate = false;

  while (m_format[i] != 0) {
    i++;
    
    /* these flags can run through multiple characters */
    bool more = true;

    while (more) {
      switch(m_format[i]) {
        case '+':
          flags |= ios::showpos;
          break;
        case '-':
          flags |= ios::left;
          break;
        case '0':
          flags |= ios::internal;
          fill = '0';
          break;
        case '#':
          alternate = true;
          break;
        default:
          more = false;
          break;
      }
      if (more)
        i++;
    }

    /* width specifier */
    if ( isdigit(m_format[i]) ) {
      width = atoi(m_format+i);
      do{ i++; }while( isdigit(m_format[i]) );
    }

    /* output precision */
    if (m_format[i] == '.') {
      i++;
      precision = atoi(m_format+i);
      do{ i++; }while( isdigit(m_format[i]) );
    }

    /* type based settings */
    bool format_ok = true;

    switch(m_format[i]) {
      case 'p':
        break;
      case 's':
        break;
      case 'c':
      case 'C':
         break;
      case 'i':
      case 'u':
      case 'd':
        flags |= ios::dec;
        break;
      case 'x':
        flags |= ios::hex;
        if (alternate) flags |= ios::showbase;
        break;
      case 'X':
        flags |= ios::hex | ios::uppercase;
        if (alternate) flags |= ios::showbase;
        break;
      case 'o':
        flags |= ios::hex;
        if (alternate) flags |= ios::showbase;
        break;
      case 'f':
        flags |= ios::fixed;
        if (alternate) flags |= ios::showpoint;
        break;
      case 'e':
        flags |= ios::scientific;
        if (alternate) flags |= ios::showpoint;
        break;
      case 'E':
        flags |= ios::scientific | ios::uppercase;
        if (alternate) flags |= ios::showpoint;
        break;
      case 'g':
        if (alternate) flags |= ios::showpoint;
        break;
      case 'G':
        flags |= ios::uppercase;
        if (alternate) flags |= ios::showpoint;
        break;
      default:
        /* if we encountered an unknown type specifier, skip current
           format string and try parsing next one... */
        format_ok = false; 
        break;
    }

    /* if formatting string was recognized set stream options*/
    if (format_ok) {
      m_stream.unsetf(ios::adjustfield | ios::basefield | ios::floatfield);
		m_stream.setf((std::ios_base::fmtflags)flags);
      m_stream.width(width);
      m_stream.precision(precision);
      m_stream.fill(fill);
      break;
    }
  }
  
  /* skip type specifier and set formatting string position */
  m_pos = i+1;
}
//------------------------------------------------------------------------
void ostream_printf::printStaticData()
{
  while ( m_format[m_pos] != '\0' )
  {
    if ( m_format[m_pos] == '%' )
    {
      if ( m_format[m_pos+1] == '%' )
        m_pos++;
      else break;
    }
    m_stream << m_format[m_pos];
    m_pos++;
  }
}
	}
}