#pragma once

#include <scl/utils/Optional.h>
#include <utility>

namespace scl{
	namespace tools{
		namespace make{
			/**
			 * Creates an optional with a constructed value
			 * @tparam T being the optional's value type
			 * @tparam Args being the arguments' types
			 * @param args being the arguments passed to the constructor of the value type
			 * @return an Optional<T> constructed with the given arguments
			 */
			template <class T, class... Args>
			scl::utils::Optional<T> optional(Args&&... args){
				return T{std::forward<Args>(args)...};
			}

			/**
			 * Creates an empty optional
			 * @tparam T being the optional's value type
			 * @return an Optional<T> initialized with None
			 */
			template <class T>
			scl::utils::Optional<T> emptyOptional(){
				return scl::utils::none;
			}
		}
	}
}