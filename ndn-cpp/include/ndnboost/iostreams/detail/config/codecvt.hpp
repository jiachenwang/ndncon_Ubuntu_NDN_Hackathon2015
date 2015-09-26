// (C) Copyright 2008 CodeRage, LLC (turkanis at coderage dot com)
// (C) Copyright 2003-2007 Jonathan Turkanis
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.

#ifndef NDNBOOST_IOSTREAMS_DETAIL_CONFIG_CODECVT_HPP_INCLUDED
#define NDNBOOST_IOSTREAMS_DETAIL_CONFIG_CODECVT_HPP_INCLUDED

#include <ndnboost/config.hpp>
#include <ndnboost/detail/workaround.hpp>
#include <ndnboost/iostreams/detail/config/wide_streams.hpp>
#include <cstddef>

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif       

//------------------Support for codecvt with user-defined state types---------//

#if defined(__MSL_CPP__) || defined(__LIBCOMO__) || \
    NDNBOOST_WORKAROUND(_STLPORT_VERSION, <= 0x450) \
    /**/
# define NDNBOOST_IOSTREAMS_NO_PRIMARY_CODECVT_DEFINITION
#endif

#if defined(__GLIBCPP__) || defined(__GLIBCXX__) || \
    NDNBOOST_WORKAROUND(_STLPORT_VERSION, > 0x450) \
    /**/
# define NDNBOOST_IOSTREAMS_EMPTY_PRIMARY_CODECVT_DEFINITION
#endif

//------------------Check for codecvt ctor taking a reference count-----------//

#if NDNBOOST_WORKAROUND(__MWERKS__, NDNBOOST_TESTED_AT(0x3205)) || \
    NDNBOOST_WORKAROUND(_STLPORT_VERSION, < 0x461) \
    /**/
# define NDNBOOST_IOSTREAMS_NO_CODECVT_CTOR_FROM_SIZE_T
#endif

//------------------Normalize codecvt::length---------------------------------//

#if !defined(__MSL_CPP__) && !defined(__LIBCOMO__) && \
    (!defined(NDNBOOST_RWSTD_VER) || NDNBOOST_RWSTD_VER < 0x04010300) && \
    (!defined(__MACH__) || !defined(__INTEL_COMPILER))
    /**/
# define NDNBOOST_IOSTREAMS_CODECVT_CV_QUALIFIER const
#else
# define NDNBOOST_IOSTREAMS_CODECVT_CV_QUALIFIER
#endif

//------------------Check for codecvt::max_length-----------------------------//

#if NDNBOOST_WORKAROUND(_STLPORT_VERSION, < 0x461)
# define NDNBOOST_IOSTREAMS_NO_CODECVT_MAX_LENGTH
#endif
                    
//------------------Put mbstate_t and codecvt in std--------------------------//

#ifndef NDNBOOST_IOSTREAMS_NO_LOCALE
# include <locale>
#endif

// From Robert Ramey's version of utf8_codecvt_facet.
namespace std { 

#if defined(__LIBCOMO__)
    using ::mbstate_t;
#elif defined(NDNBOOST_DINKUMWARE_STDLIB) && !defined(__BORLANDC__)
    using ::mbstate_t;
#elif defined(__SGI_STL_PORT)
#elif defined(NDNBOOST_NO_STDC_NAMESPACE)
    using ::codecvt;
    using ::mbstate_t;
#endif

} // End namespace std.

#endif // #ifndef NDNBOOST_IOSTREAMS_DETAIL_CONFIG_CODECVT_HPP_INCLUDED
