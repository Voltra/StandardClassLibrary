#pragma once

#include "Throwable.h"
#include "../macros.h"
#include <stdexcept>
#include <utility>

namespace scl{
	namespace exceptions{
		/**
		 * Base class for exceptions that could never be checked at compile-time
		 */
		class RuntimeException : public Throwable, public std::runtime_error{
			protected:
				/**
				 * @var reason
				 * The error message that comes with this Exception
				 */
				realConst(char*) _reason = "";

			public:
				/**
				 * Construct an scl::exceptions::Exception from an error message
				 * @tparam T being the reason's type
				 * @param reason being the error message
				 */
				template <class T>
				RuntimeException(T&& reason) : _reason{std::forward<T>(reason)} {
				}

				/**
				 * Retrieve the reason
				 * @return the reason as a const char* const
				 */
				realConst(char*) reason() const override{
					return this->_reason;
				}

				/**
				 * Override that uses the user defined reason
				 * @return the reason
				 */
				const char* what() const noexcept override{
					return this->reason();
				}

				/**
				 * A string describing the name of the exception class
				 * @return the fully qualified name of this class
				 */
				realConst(char*) type() const noexcept override{
					return "scl::exceptions::RuntimeException";
				}
		};
	}
}