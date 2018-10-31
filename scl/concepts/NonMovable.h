#pragma once

#include <scl/tools/meta/type_check.h>

namespace scl{
	namespace concepts{
		/**
		 * NonMovable concept, a type is non movable if it is neither move constructible nor move assignable
		 * @tparam T being the type to check against
		 */
		template <class T>
		struct NonMovable{
			constexpr operator bool() const{
				using namespace scl::tools;
				static_assert(!meta::is_move_constructible<T>(), "T is move constructible");
				static_assert(!meta::is_move_assignable<T>(), "T is move assignable");
				return true;
			}
		};
	}
}