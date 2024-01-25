#pragma once

#include <cgv/media/axis_aligned_box.h>

namespace stream_vis {

	template<typename T>
	class streaming_aabb_base
	{
	public:
		virtual ~streaming_aabb_base() {}

		virtual void add_samples_base(const T* data_ptr, size_t count) = 0;
		virtual void put_aabb(T* aabb_ptr) const = 0;
	};

	template<typename T, cgv::type::uint32_type N>
	class streaming_aabb_brute_force : public streaming_aabb_base<T>
	{
	protected:
		size_t nr_samples;
		size_t window_size;
		std::vector<cgv::math::fvec<T, N> > sample_cache;
		mutable cgv::media::axis_aligned_box<T, N> aabb;
		mutable bool outofdate;
	public:
		streaming_aabb_brute_force(size_t _window_size)
		{
			nr_samples = 0;
			sample_cache.reserve(_window_size);
			window_size = _window_size;
			aabb.invalidate();
			outofdate = false;
		}
		void add_samples(const cgv::math::fvec<T, N>* sample_ptr, size_t count)
		{
			for (size_t i = 0; i < count; ++i) {
				if (sample_cache.size() < window_size) {
					sample_cache.push_back(sample_ptr[i]);
					aabb.add_point(sample_ptr[i]);
				}
				else {
					sample_cache[nr_samples % window_size] = sample_ptr[i];
					outofdate = true;
				}
				++nr_samples;
			}
		}
		cgv::media::axis_aligned_box<T, N> get_aabb() const
		{
			if (outofdate) {
				aabb.invalidate();
				for (const auto& s : sample_cache)
					aabb.add_point(s);
				outofdate = false;
			}
			return aabb;
		}
		void add_samples_base(const T* data_ptr, size_t count)
		{
			add_samples(reinterpret_cast<const cgv::math::fvec<T, N>*>(data_ptr), count);
		}
		void put_aabb(T* aabb_ptr) const
		{
			*reinterpret_cast<cgv::media::axis_aligned_box<T, N>*>(aabb_ptr) = get_aabb();
		}
	};

	template<typename T, cgv::type::uint32_type N>
	class streaming_aabb_block_based : public streaming_aabb_base<T>
	{
	protected:
		size_t nr_samples;
		size_t window_size;
		size_t block_size;
		size_t cache_size;
		std::vector<cgv::media::axis_aligned_box<T, N>> sample_cache;
		mutable cgv::media::axis_aligned_box<T, N> aabb;
	public:
		streaming_aabb_block_based(size_t _window_size, size_t _block_size)
		{
			nr_samples = 0;
			block_size = _block_size;
			cache_size = _window_size / _block_size;
			if (cache_size * _block_size < _window_size)
				++cache_size;
			sample_cache.reserve(cache_size);
			window_size = _window_size;
			aabb.invalidate();
		}
		void add_samples(const cgv::math::fvec<T, N>* sample_ptr, size_t count)
		{
			for (size_t i = 0; i < count; ++i) {
				aabb.add_point(sample_ptr[i]);
				++nr_samples;
				// if a block is finished
				if (nr_samples % block_size == 0) {
					// incorporate aabb into all cached aabbs
					for (auto& box : sample_cache) // TODO: amortize this cost over sample iterations
						box.add_axis_aligned_box(aabb);
					// add aabb additionally at the end to the cache
					if (sample_cache.size() < cache_size)
						sample_cache.push_back(aabb);
					else
						sample_cache[(nr_samples / block_size) % cache_size] = aabb;
				}
			}
		}
		cgv::media::axis_aligned_box<T, N> get_aabb() const
		{
			if (sample_cache.empty())
				return aabb;
			cgv::media::axis_aligned_box<T, N> res = aabb;
			if (sample_cache.size() < cache_size)
				res.add_axis_aligned_box(sample_cache.front());
			else
				res.add_axis_aligned_box(sample_cache[(nr_samples / block_size) % cache_size]);
			return res;
		}
		void add_samples_base(const T* data_ptr, size_t count)
		{
			add_samples(reinterpret_cast<const cgv::math::fvec<T, N>*>(data_ptr), count);
		}
		void put_aabb(T* aabb_ptr) const
		{
			*reinterpret_cast<cgv::media::axis_aligned_box<T, N>*>(aabb_ptr) = get_aabb();
		}
	};


}