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

	/// start scoping OpenGL commands for time measurement
	void begin_scope() const;
	/// close the scope of OpenGL commands included in the time measurement
	void end_scope() const;
	/// collect the measured time, blocking until the query object is ready and the time can be retrieved
	/// elapsed time is given in nanoseconds (1 second = 10^9 nanoseconds)
	double collect() const;

	/// close the scope of OpenGL commands included in the time measurement and retrieve the duration immediately,
	/// blocking until the query object is ready and the time can be retrieved
	inline double end_scope_and_collect() const {
		end_scope();
		return collect();
	}

	/// begin the time measurement
	[[deprecated("Use begin_scope() instead, which does the exact same thing, but is named more clearly and fits better with the new way of using gl_time_query.")]]
	void begin() const;
	/// end the time measurement and return elapsed time in nanoseconds (1 second = 10^9 nanoseconds)
	[[deprecated("This method blocks the calling thread until the scoped commands finish executing. Use end_scope() and collect() instead.")]]
	double end() const;
};

		} // namespace cgv
	} // namespace render
} // namespace gl

#include <cgv/config/lib_end.h>
