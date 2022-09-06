#pragma once

#include <iostream>
#include <string>

#include <cgv/signal/signal.h>

namespace cgv {
	namespace signal {

/// simple implementation of a streambuf that sends all written text to the write signal that is referenced from the possessing stream
struct callback_streambuf : public std::streambuf
{
	cgv::signal::signal<const std::string&>& write;

	callback_streambuf(cgv::signal::signal<const std::string&>& _write) : write(_write) 
	{
	}
	std::streambuf *setbuf(char_type *buf, std::streamsize n)
	{
		setp(buf,buf+n-1);
		return this;
	}
	int_type overflow(int_type c)
	{
		if (c != std::char_traits<char>::eof()) {
			*pptr() = c;
			pbump(1);
		}
		int num = static_cast<int>(pptr()-pbase());
		std::string text(pbase(),num);
		write(text);
        pbump(-num);
		return c;
	}
	int sync()
	{
		int num = static_cast<int>(pptr()-pbase());
		std::string text(pbase(),num);
		write(text);
        pbump(-num);
        return num;
	}
};

/// connect to the write signal of the callback stream in order to process all text written to the stream
class callback_stream : public std::ostream
{
protected:
	callback_streambuf buf;
	char buffer[256];
public:
	/// signal to which all text written to the stream is sent
	cgv::signal::signal<const std::string&> write;
	/// constructor sets the stream buffer of the stream to the callback_streambuf
	callback_stream() : std::ostream(0), buf(write)
	{
		buf.setbuf(buffer,256);
		init(&buf);
	}
};

	}
}
