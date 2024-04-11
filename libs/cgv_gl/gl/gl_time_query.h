#pragma once

#include <cgv/render/texture.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {
		namespace gl {

/** OpenGL helper class to perform time measurements. */
class CGV_API gl_time_query
{
private:
	context* ctx_ptr = nullptr;
	unsigned int query = 0;

public:
	/// a string that contains the last error, which is only set by the init method
	mutable std::string last_error;

	/// construct an uninitialized time query
	gl_time_query();
	/// destruct the query
	~gl_time_query();
	/// initialize the time query by generating the query object, return success
	bool init(context& ctx);
	/// deinitialize the query
	void destruct(context& ctx);
	/// check whether the time query has been initialized, i.e. the init method has been called successfully before
	bool is_initialized() const;
	/// begin the time measurement
	void begin();
	/// end the time measurement and return elapsed time in nanoseconds (1 second = 10^9 nanoseconds)
	double end();
};

		} // namespace cgv
	} // namespace render
} // namespace gl

#include <cgv/config/lib_end.h>
