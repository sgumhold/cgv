#pragma once

#include <vector>
#include <cassert>
#include <cgv/math/fvec.h>
#include <cgv/media/axis_aligned_box.h>

namespace cgv {
	namespace data {
		template <typename T>
		class quadtree
		{
		public:
			typedef cgv::math::fvec<T, 2> vec2;
			typedef cgv::media::axis_aligned_box<T, 2> box2;
			struct point_provider
			{
				virtual const vec2& get_point(int i) const = 0;
			};
		protected:
			struct node
			{
				bool is_leaf;
				int children[4];
				node() : is_leaf(true) { children[0] = children[1] = children[2] = children[3] = -1; }
				/// in case of leaf, return the number of contained points
				unsigned size() const { unsigned s = 0; while (s < 4 && children[s] != -1) ++s; return s; }
			};
			box2 box;
			const point_provider& provider;
			int root_idx;
			std::vector<node> nodes;

			const node& get_node(int ni) const { return nodes[ni]; }
				  node& ref_node(int ni)       { return nodes[ni]; }

				  bool is_leaf(int ni) const { return nodes[ni].is_leaf; }
			void add_point_to_leaf(int pi, int ni) {
				node& n = ref_node(ni);
				assert(n.is_leaf && n.size() < 4);
				n.children[n.size()] = pi;
			}
			bool split_leaf(int ni, const vec2& x) {
				if (!is_leaf(ni))
					return false;
				int c0 = (int)nodes.size();
				nodes.resize(nodes.size() + 4);
				node& n = ref_node(ni);
				for (unsigned j = 0; j < 4; ++j) {
					int pi = n.children[j];
					if (pi != -1) {
						const vec2& p = provider.get_point(pi);
						unsigned k = 0;
						if (p(0) > x(0))
							k += 1;
						if (p(1) > x(1))
							k += 2;
						add_point_to_leaf(pi, c0 + k);
					}
					n.children[j] = c0 + j;
				}
				n.is_leaf = false;
				return true;
			}
		public:
			quadtree(const point_provider& _provider, const box2& _box) : provider(_provider), box(_box) { nodes.push_back(node()); root_idx = 0; }
			bool empty() const { return is_leaf(get_root_index()) && get_node(get_root_index()).size() == 0; }
			int get_root_index() const { return root_idx; }
			bool collides(const vec2& x, T d, int ni = -1, box2 b = box2()) const {
				if (ni == -1) {
					ni = get_root_index();
					b = box;
				}
				// check for overlap
				if (x(0) + d > b.get_min_pnt()(0) && x(0) - d < b.get_max_pnt()(0) &&
					x(1) + d > b.get_min_pnt()(1) && x(1) - d < b.get_max_pnt()(1)) {
					// in leaf case compare to points
					if (is_leaf(ni)) {
						const node& n = get_node(ni);
						for (unsigned j = 0; j < 4; ++j) {
							if (n.children[j] == -1)
								break;
							if ((provider.get_point(n.children[j])-x).length() < d)
								return true;
						}
						return false;
					}
					else {
						vec2 c = b.get_center();
						const node& n = get_node(ni);
						return
							collides(x, d, n.children[0], box2(b.get_min_pnt(), c)) ||
							collides(x, d, n.children[1], box2(vec2(c(0), b.get_min_pnt()(1)), vec2(b.get_max_pnt()(0), c(1)))) ||
							collides(x, d, n.children[2], box2(vec2(b.get_min_pnt()(0), c(1)), vec2(c(0), b.get_max_pnt()(1)))) ||
							collides(x, d, n.children[3], box2(c, b.get_max_pnt()));
					}
				}
				else
					return false;
			}
			void insert(const vec2& p, int pi) {
				int ni = get_root_index();
				box2 b = box;
				while (true) {
					if (is_leaf(ni)) {
						const node& n = get_node(ni);
						if (n.size() < 4) {
							add_point_to_leaf(pi, ni);
							return;
						}
						split_leaf(ni, b.get_center());
					}
					vec2 x = b.get_center();
					int k = 0;
					if (p(0) > x(0)) {
						k += 1;
						b.ref_min_pnt()(0) = x(0);
					}
					else
						b.ref_max_pnt()(0) = x(0);

					if (p(1) > x(1)) {
						k += 2;
						b.ref_min_pnt()(1) = x(1);
					}
					else
						b.ref_max_pnt()(1) = x(1);

					ni = get_node(ni).children[k];
				}
			}
		};
	}
}