#pragma once

namespace scl{
	namespace tools{
		namespace iostream{
			/**
			 * @namespace scl::tools::iostream::log
			 * Logging utilities
			 */
			namespace log{}
		}
	}
}

#include <scl/tools/iostream/log/basic.h>
#include <scl/tools/iostream/log/prefixed.h>
#include <scl/tools/iostream/log/surrounded.h>

namespace scl{
	namespace tools{
		namespace iostream{
			namespace log{
				/**
				 * @namespace scl::tools::iostream::log::wrap
				 * Alias for scl::tools::iostream::log::surrounded
				 */
				namespace wrap = surrounded;
			}
		}
	}

	/**
	 * @namespace scl::console
	 * Alias for scl::tools::iostream::log
	 */
	namespace console = tools::iostream::log;

	/**
	 * @namespace scl::cli
	 * Alias for scl::console
	 */
	namespace cli = console;
}