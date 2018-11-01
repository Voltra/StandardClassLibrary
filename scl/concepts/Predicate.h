#pragma once

#ifdef SCL_CPP17
//#include <scl/concepts/require.h>
#include <scl/macros.h>
#include <scl/tools/meta/constexpr_assert.h>
#include <scl/concepts/RegularInvocable.h>
#include <scl/concepts/Boolean.h>

namespace scl{
	namespace concepts{
		/**
		 * Predicate concept, a type F is a predicate with Args if it is RegularInvocable and its result is Boolean
		 * @tparam F being the function type
		 * @tparam Args being the arguents' types
		 */
		template <class F, class... Args>
		struct Predicate{
			constexpr operator bool() const{
				using namespace scl::tools;
				return META::constexpr_assert<
					RegularInvocable<F, Args...>{}
					&& Boolean<meta::invoke_result_t<F, Args...>>{}
				>();
			}
		};
	}
}
#endif
