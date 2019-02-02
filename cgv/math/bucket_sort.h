#pragma once

#include <vector>

namespace cgv {
	namespace math {
		/// compute a permutation perm with bucket_sort over index typed keys that a limited to nr_keys, optionally provide an initial permutation
		template <typename key_type, typename idx_type>
		void bucket_sort(const std::vector<key_type>& keys, size_t nr_keys, std::vector<idx_type>& perm, std::vector<idx_type>* perm_in_ptr = 0)
		{
			// prepare links and lookup table
			std::vector<idx_type> links(keys.size(), idx_type(-1));
			std::vector<idx_type> lookup(nr_keys, idx_type(-1));
			// sort indices into lookup table
			idx_type i;
			for (i = 0; i < keys.size(); ++i) {
				key_type k = keys[perm_in_ptr ? perm_in_ptr->at(i) : i];
				idx_type j = lookup[k];
				if (j != idx_type(-1))
					links[i] = j;
				lookup[k] = i;
			}
			// construct permutation
			perm.resize(keys.size());
			i = idx_type(keys.size());
			for (key_type k = key_type(nr_keys); k > 0; ) {
				idx_type j = lookup[--k];
				while (j != idx_type(-1)) {
					perm[--i] = perm_in_ptr ? perm_in_ptr->at(j) : j;
					j = links[j];
				}
			}
		}
	}
}