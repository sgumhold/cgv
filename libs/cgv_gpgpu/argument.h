#pragma once

#include <cgv/render/context.h>

namespace cgv {
namespace gpgpu {

struct uniform_argument {
	std::string name;
	cgv::render::type_descriptor desc;
	void* addr = nullptr;

	uniform_argument() {}

	template<typename T>
	uniform_argument(const std::string& name, T value) : name(name) {
		desc = cgv::render::element_descriptor_traits<T>::get_type_descriptor(value);
		addr = cgv::render::element_descriptor_traits<T>::get_address(value);
	}
};

using uniform_argument_list = std::vector<uniform_argument>;

} // namespace gpgpu
} // namespace cgv
