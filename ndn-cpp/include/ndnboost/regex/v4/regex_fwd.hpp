/*
 *
 * Copyright (c) 1998-2002
 * John Maddock
 *
 * Use, modification and distribution are subject to the 
 * Boost Software License, Version 1.0. (See accompanying file 
 * LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */

 /*
  *   LOCATION:    see http://www.boost.org for most recent version.
  *   FILE         regex_fwd.cpp
  *   VERSION      see <ndnboost/version.hpp>
  *   DESCRIPTION: Forward declares ndnboost::basic_regex<> and
  *                associated typedefs.
  */

#ifndef NDNBOOST_REGEX_FWD_HPP_INCLUDED
#define NDNBOOST_REGEX_FWD_HPP_INCLUDED

#ifndef NDNBOOST_REGEX_CONFIG_HPP
#include <ndnboost/regex/config.hpp>
#endif

//
// define NDNBOOST_REGEX_NO_FWD if this
// header doesn't work!
//
#ifdef NDNBOOST_REGEX_NO_FWD
#  ifndef NDNBOOST_RE_REGEX_HPP
#     include <ndnboost/regex.hpp>
#  endif
#else

namespace ndnboost{

template <class charT>
class cpp_regex_traits;
template <class charT>
struct c_regex_traits;
template <class charT>
class w32_regex_traits;

#ifdef NDNBOOST_REGEX_USE_WIN32_LOCALE
template <class charT, class implementationT = w32_regex_traits<charT> >
struct regex_traits;
#elif defined(NDNBOOST_REGEX_USE_CPP_LOCALE)
template <class charT, class implementationT = cpp_regex_traits<charT> >
struct regex_traits;
#else
template <class charT, class implementationT = c_regex_traits<charT> >
struct regex_traits;
#endif

template <class charT, class traits = regex_traits<charT> >
class basic_regex;

typedef basic_regex<char, regex_traits<char> > regex;
#ifndef NDNBOOST_NO_WREGEX
typedef basic_regex<wchar_t, regex_traits<wchar_t> > wregex;
#endif

} // namespace ndnboost

#endif  // NDNBOOST_REGEX_NO_FWD

#endif




