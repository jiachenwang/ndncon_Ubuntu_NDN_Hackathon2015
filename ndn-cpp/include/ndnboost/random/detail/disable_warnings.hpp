/* boost random/detail/disable_warnings.hpp header file
 *
 * Copyright Steven Watanabe 2009
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 *
 * $Id$
 *
 */

// No #include guard.  This header is intended to be included multiple times.

#include <ndnboost/config.hpp>

#ifdef NDNBOOST_MSVC
#pragma warning(push)
#pragma warning(disable:4512)
#pragma warning(disable:4127)
#pragma warning(disable:4724)
#endif

#if defined(NDNBOOST_GCC) && NDNBOOST_GCC >= 40600
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlogical-op"
#endif