#pragma once

namespace cgv {
	namespace type {
		namespace ctrl {
			/// the if traits selects a type from a given condition type which should be cond::true_type or cond::false_type
			template <bool condition_type, typename then_type, typename else_type> 
			struct if_ {
				typedef then_type type;
			};
			template <typename then_type, typename else_type> 
			struct if_<false,then_type,else_type> {
				typedef else_type type;
			};
		}
	}
}
