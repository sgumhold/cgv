#include "line_break.h"

namespace cgv {
	namespace os {
		std::istream& safe_getline(std::istream& is, std::string& line)
		{
			line.clear();
			std::istream::sentry se(is, true);
			std::streambuf* sb = is.rdbuf();

			for (;;) {
				int c = sb->sbumpc();
				switch (c) {
				case '\n':
					return is;
				case '\r':
					if (sb->sgetc() == '\n')
						sb->sbumpc();
					return is;
				case std::streambuf::traits_type::eof():
					// Also handle the case when the last line has no line ending
					if (line.empty())
						is.setstate(std::ios::eofbit);
					return is;
				default:
					line += (char)c;
				}
			}
		}
	}
}
