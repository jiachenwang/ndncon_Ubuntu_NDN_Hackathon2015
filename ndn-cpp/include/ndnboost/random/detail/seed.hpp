/* boost random/detail/seed.hpp header file
 *
 * Copyright Steven Watanabe 2009
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org for most recent version including documentation.
 *
 * $Id$
 */

#ifndef NDNBOOST_RANDOM_DETAIL_SEED_HPP
#define NDNBOOST_RANDOM_DETAIL_SEED_HPP

#include <ndnboost/config.hpp>

// Sun seems to have trouble with the use of SFINAE for the
// templated constructor.  So does Borland.
#if !defined(NDNBOOST_NO_SFINAE) && !defined(__SUNPRO_CC) && !defined(__BORLANDC__)

#include <ndnboost/utility/enable_if.hpp>
#include <ndnboost/type_traits/is_arithmetic.hpp>

namespace ndnboost {
namespace random {
namespace detail {

template<class T>
struct disable_seed : ndnboost::disable_if<ndnboost::is_arithmetic<T> > {};

template<class Engine, class T>
struct disable_constructor : disable_seed<T> {};

template<class Engine>
struct disable_constructor<Engine, Engine> {};

#define NDNBOOST_RANDOM_DETAIL_GENERATOR_CONSTRUCTOR(Self, Generator, gen) \
    template<class Generator>                                           \
    explicit Self(Generator& gen, typename ::ndnboost::random::detail::disable_constructor<Self, Generator>::type* = 0)

#define NDNBOOST_RANDOM_DETAIL_GENERATOR_SEED(Self, Generator, gen)    \
    template<class Generator>                                       \
    void seed(Generator& gen, typename ::ndnboost::random::detail::disable_seed<Generator>::type* = 0)

#define NDNBOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR(Self, SeedSeq, seq)    \
    template<class SeedSeq>                                             \
    explicit Self(SeedSeq& seq, typename ::ndnboost::random::detail::disable_constructor<Self, SeedSeq>::type* = 0)

#define NDNBOOST_RANDOM_DETAIL_SEED_SEQ_SEED(Self, SeedSeq, seq)   \
    template<class SeedSeq>                                     \
    void seed(SeedSeq& seq, typename ::ndnboost::random::detail::disable_seed<SeedSeq>::type* = 0)

#define NDNBOOST_RANDOM_DETAIL_ARITHMETIC_CONSTRUCTOR(Self, T, x)  \
    explicit Self(const T& x)

#define NDNBOOST_RANDOM_DETAIL_ARITHMETIC_SEED(Self, T, x) \
    void seed(const T& x)
}
}
}

#else

#include <ndnboost/type_traits/is_arithmetic.hpp>
#include <ndnboost/mpl/bool.hpp>

#define NDNBOOST_RANDOM_DETAIL_GENERATOR_CONSTRUCTOR(Self, Generator, gen) \
    Self(Self& other) { *this = other; }                                \
    Self(const Self& other) { *this = other; }                          \
    template<class Generator>                                           \
    explicit Self(Generator& gen) {                                     \
        boost_random_constructor_impl(gen, ::ndnboost::is_arithmetic<Generator>());\
    }                                                                   \
    template<class Generator>                                           \
    void boost_random_constructor_impl(Generator& gen, ::ndnboost::mpl::false_)

#define NDNBOOST_RANDOM_DETAIL_GENERATOR_SEED(Self, Generator, gen)    \
    template<class Generator>                                       \
    void seed(Generator& gen) {                                     \
        boost_random_seed_impl(gen, ::ndnboost::is_arithmetic<Generator>());\
    }\
    template<class Generator>\
    void boost_random_seed_impl(Generator& gen, ::ndnboost::mpl::false_)

#define NDNBOOST_RANDOM_DETAIL_SEED_SEQ_CONSTRUCTOR(Self, SeedSeq, seq)    \
    Self(Self& other) { *this = other; }                                \
    Self(const Self& other) { *this = other; }                          \
    template<class SeedSeq>                                             \
    explicit Self(SeedSeq& seq) {                                       \
        boost_random_constructor_impl(seq, ::ndnboost::is_arithmetic<SeedSeq>());\
    }                                                                   \
    template<class SeedSeq>                                             \
    void boost_random_constructor_impl(SeedSeq& seq, ::ndnboost::mpl::false_)

#define NDNBOOST_RANDOM_DETAIL_SEED_SEQ_SEED(Self, SeedSeq, seq)           \
    template<class SeedSeq>                                             \
    void seed(SeedSeq& seq) {                                           \
        boost_random_seed_impl(seq, ::ndnboost::is_arithmetic<SeedSeq>()); \
    }                                                                   \
    template<class SeedSeq>                                             \
    void boost_random_seed_impl(SeedSeq& seq, ::ndnboost::mpl::false_)

#define NDNBOOST_RANDOM_DETAIL_ARITHMETIC_CONSTRUCTOR(Self, T, x)  \
    explicit Self(const T& x) { boost_random_constructor_impl(x, ::ndnboost::mpl::true_()); }\
    void boost_random_constructor_impl(const T& x, ::ndnboost::mpl::true_)

#define NDNBOOST_RANDOM_DETAIL_ARITHMETIC_SEED(Self, T, x) \
    void seed(const T& x) { boost_random_seed_impl(x, ::ndnboost::mpl::true_()); }\
    void boost_random_seed_impl(const T& x, ::ndnboost::mpl::true_)

#endif

#endif
