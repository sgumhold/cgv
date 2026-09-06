#pragma once

#include <type_traits>

namespace cgv {
	namespace type {
		namespace traits {
			/** The is_instance_of trait provides a constant value equal to true if the type T is an instance of template class Template.
				Only Template types taking type arguments only can be used. Template types taking non-template arguments are not supported. */

			template<class T, template<class...> class Template>
			/*inline*/ constexpr bool is_instance_of_v = std::false_type{};

			template<template<class...> class T, class ...TemplateArgs>
			/*inline*/ constexpr bool is_instance_of_v<T<TemplateArgs...>, T> = std::true_type{};

			template<class T, template<class...> class Template>
			struct is_instance_of : std::bool_constant<is_instance_of_v<T, Template>> {};
		}
	}
}