#pragma once

//#include <cgv/render/context.h>
//#include <cgv_gl/gl/gl.h>
//
//#include "sl.h"
//#include "utils.h"
//#include "compute_kernel.h"

namespace cgv {
namespace gpgpu {

template<typename T>
class double_buffer_wrapper {
public:
	double_buffer_wrapper(T* first, T* second) : _first(first), _second(second) {}

	T* first() { return _first; }
	T* second() { return _second; }

	void swap() { std::swap(_first, _second); }
private:
	T* _first = nullptr;
	T* _second = nullptr;
};

} // namespace gpgpu
} // namespace cgv
