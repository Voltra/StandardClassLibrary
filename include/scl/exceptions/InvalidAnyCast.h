#pragma once

#include <scl/exceptions/InvalidCast.h>
#include <scl/macros.h>

namespace scl{
	namespace exceptions{
		/**
		 * Exception class used when attempting to cast an Any to the wrong type
		 */
		class InvalidAnyCast : public InvalidCast{
			public:
				using InvalidCast::InvalidCast;
				realConst(char*) type() const noexcept override{
					return "scl::exceptions::InvalidAnyCast";
				}

				InvalidAnyCast() : InvalidAnyCast("Tried to cast Any to an unsupported type"){
				}
		};
	}
}