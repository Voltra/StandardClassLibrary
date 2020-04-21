#pragma once

namespace scl{
	namespace tools{
		/**
		 * @namespace scl::tools::make
		 * Group of factory functions
		 */
		namespace make{}
	}
}

#include <scl/tools/make/from.h>
#include <scl/tools/make/ptr.h>
#include <scl/tools/make/unique.h>
#include <scl/tools/make/shared.h>
#include <scl/tools/make/atomic.h>
#include <scl/tools/make/promise.h>
#include <scl/tools/make/optional.h>
#include <scl/tools/make/result.h>
#include <scl/tools/make/any.h>
#include <scl/tools/make/mutexed.h>