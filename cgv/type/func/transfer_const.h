#pragma once 

namespace cgv {
	namespace type {
		namespace func {
			/** return const S if T is const and S otherwise */
			template <typename T, typename S>
			struct transfer_const
			{
				typedef S type;
			};
			// specialize for const types
			template <typename T, typename S>
			struct transfer_const<const T, S>
			{
				typedef const S type;
			};
		}
	}
}
