#pragma once

#include "sort_algorithm.h"
#include "radix_sort_4x.h"
#include "radix_sort_onesweep.h"

namespace cgv {
namespace gpgpu {

enum class SortImplementation {
	Default,			// Will choose the fastest implementation that is supported by the current GPU
	RadixSort4x,		// A default radix sort working on 2 bits simultaneously; compatible with most GPUs
	RadixSortOnesweep	// A faster radix sort that needs a decently modern GPU with support for the KHR subgroup extensions and will only work on Nvidia hardware
};

struct sort_factory {
	static std::unique_ptr<sort_algorithm> create(const cgv::render::context& ctx, SortImplementation implementation) {
		switch(implementation) {
		case SortImplementation::RadixSort4x:
			return std::make_unique<radix_sort_4x>();
		case SortImplementation::RadixSortOnesweep:
			return std::make_unique<radix_sort_onesweep>();
		default:
			// Use the onesweep implementation if the GPU supports the necessary extensions.
			// Currently only Nvidia GPUs are supported because they use the required warp size of 32 threads.
			if(ctx.get_gpu_vendor_id() == cgv::render::GPUVendorID::GPU_VENDOR_NVIDIA) {
				if(GLEW_KHR_shader_subgroup) {
					GLint subgroup_size = 0;
					glGetIntegerv(GL_SUBGROUP_SIZE_KHR, &subgroup_size);
					GLint features = 0;
					glGetIntegerv(GL_SUBGROUP_SUPPORTED_FEATURES_KHR, &features);

					if(subgroup_size == 32 &&
						features & GL_SUBGROUP_FEATURE_BASIC_BIT_KHR &&
						features & GL_SUBGROUP_FEATURE_BALLOT_BIT_KHR &&
						features & GL_SUBGROUP_FEATURE_ARITHMETIC_BIT_KHR)
						return std::make_unique<radix_sort_onesweep>();
				}
			}
			// Else default to the 4x version.
			return std::make_unique<radix_sort_4x>();
		}
	}
};

namespace detail {

class sort_interface_helper {
public:
	std::string get_type_name() const { return _type_name; }

	bool is_initialized() const { return _sort && _sort->is_initialized(); }

	void destruct(const cgv::render::context& ctx) {
		if(_sort) {
			_sort->destruct(ctx);
			_sort.reset(nullptr);
		}
	}

	bool resize(cgv::render::context& ctx, size_t size) {
		return _sort->resize(ctx, size);
	}

	error get_last_error() const {
		if(_sort)
			_sort->get_last_error();
		return {};
	}

protected:
	sort_interface_helper(const std::string type_name) : _type_name(type_name) {}

	bool create_interface(cgv::render::context& ctx, SortImplementation implementation) {
		_sort = sort_factory::create(ctx, implementation);
		return _sort != nullptr;
	}

	sort_algorithm* get_interface() const {
		return _sort.get();
	}

private:
	const std::string _type_name;
	std::unique_ptr<sort_algorithm> _sort;
};

} // namespace detail


namespace generic {

class sort : public cgv::gpgpu::detail::sort_interface_helper {
public:
	sort() : sort_interface_helper("sort") {}

	bool init(cgv::render::context& ctx, const sl::data_type& key_type, SortOrder order, size_t size, SortImplementation implementation = SortImplementation::Default) {
		if(sort_interface_helper::create_interface(ctx, implementation))
			return get_interface()->init(ctx, key_type, order, size);
		return false;
	}

	void dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& buffer) {
		return get_interface()->dispatch(ctx, buffer);
	}
};

class sort_by_key : public cgv::gpgpu::detail::sort_interface_helper {
public:
	sort_by_key() : sort_interface_helper("sort_by_key") {}

	bool init(cgv::render::context& ctx, const sl::data_type& key_type, const sl::data_type& value_type, SortOrder order, size_t size, SortImplementation implementation = SortImplementation::Default) {
		if(sort_interface_helper::create_interface(ctx, implementation))
			return get_interface()->init(ctx, key_type, value_type, order, size);
		return false;
	}

	void dispatch(cgv::render::context& ctx, const cgv::render::vertex_buffer& keys_buffer, const cgv::render::vertex_buffer& values_buffer) {
		return get_interface()->dispatch(ctx, keys_buffer, values_buffer);
	}
};

} // namespace generic

template<class T>
class sort : public generic::sort {
public:
	static_assert(type_representation<T>::value, "T must be representable as sl::data_type");

	using base = generic::sort;
	using base::base;

	bool init(cgv::render::context& ctx, SortOrder order, size_t size, SortImplementation implementation = SortImplementation::Default) {
		sl::data_type key_type = register_type_representation<T>();
		return base::init(ctx, key_type, order, size, implementation);
	}
};

template<class KeyT, class ValueT>
class sort_by_key : public generic::sort_by_key {
public:
	static_assert(type_representation<KeyT>::value, "KeyT must be representable as sl::data_type");
	static_assert(type_representation<ValueT>::value, "ValueT must be representable as sl::data_type");

	using base = generic::sort_by_key;
	using base::base;

	bool init(cgv::render::context& ctx, SortOrder order, size_t size, SortImplementation implementation = SortImplementation::Default) {
		sl::data_type key_type = register_type_representation<KeyT>();
		sl::data_type value_type = register_type_representation<ValueT>();
		return base::init(ctx, key_type, value_type, order, size, implementation);
	}
};

} // namespace gpgpu
} // namespace cgv
