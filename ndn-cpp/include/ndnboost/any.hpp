// See http://www.boost.org/libs/any for Documentation.

#ifndef NDNBOOST_ANY_INCLUDED
#define NDNBOOST_ANY_INCLUDED

#if defined(_MSC_VER)
# pragma once
#endif

// what:  variant type ndnboost::any
// who:   contributed by Kevlin Henney,
//        with features contributed and bugs found by
//        Antony Polukhin, Ed Brey, Mark Rodgers, 
//        Peter Dimov, and James Curran
// when:  July 2001, April 2013 - May 2013

#include <algorithm>
#include <typeinfo>

#include "ndnboost/config.hpp"
#include <ndnboost/type_traits/remove_reference.hpp>
#include <ndnboost/type_traits/decay.hpp>
#include <ndnboost/type_traits/add_reference.hpp>
#include <ndnboost/type_traits/is_reference.hpp>
#include <ndnboost/type_traits/is_const.hpp>
#include <ndnboost/throw_exception.hpp>
#include <ndnboost/static_assert.hpp>
#include <ndnboost/utility/enable_if.hpp>
#include <ndnboost/type_traits/is_same.hpp>
#include <ndnboost/type_traits/is_const.hpp>

// See ndnboost/python/type_id.hpp
// TODO: add NDNBOOST_TYPEID_COMPARE_BY_NAME to config.hpp
# if defined(__GNUC__) \
 || defined(_AIX) \
 || (   defined(__sgi) && defined(__host_mips)) \
 || (defined(__hpux) && defined(__HP_aCC)) \
 || (defined(linux) && defined(__INTEL_COMPILER) && defined(__ICC))
#  define NDNBOOST_AUX_ANY_TYPE_ID_NAME
#include <cstring>
# endif 

namespace ndnboost
{
    class any
    {
    public: // structors

        any() NDNBOOST_NOEXCEPT
          : content(0)
        {
        }

        template<typename ValueType>
        any(const ValueType & value)
          : content(new holder<NDNBOOST_DEDUCED_TYPENAME decay<const ValueType>::type>(value))
        {
        }

        any(const any & other)
          : content(other.content ? other.content->clone() : 0)
        {
        }

#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
        // Move constructor
        any(any&& other) NDNBOOST_NOEXCEPT
          : content(other.content)
        {
            other.content = 0;
        }

        // Perfect forwarding of ValueType
        template<typename ValueType>
        any(ValueType&& value
            , typename ndnboost::disable_if<ndnboost::is_same<any&, ValueType> >::type* = 0 // disable if value has type `any&`
            , typename ndnboost::disable_if<ndnboost::is_const<ValueType> >::type* = 0) // disable if value has type `const ValueType&&`
          : content(new holder< typename decay<ValueType>::type >(static_cast<ValueType&&>(value)))
        {
        }
#endif

        ~any() NDNBOOST_NOEXCEPT
        {
            delete content;
        }

    public: // modifiers

        any & swap(any & rhs) NDNBOOST_NOEXCEPT
        {
            std::swap(content, rhs.content);
            return *this;
        }


#ifdef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
        template<typename ValueType>
        any & operator=(const ValueType & rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }

        any & operator=(any rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }

#else 
        any & operator=(const any& rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }

        // move assignement
        any & operator=(any&& rhs) NDNBOOST_NOEXCEPT
        {
            rhs.swap(*this);
            any().swap(rhs);
            return *this;
        }

        // Perfect forwarding of ValueType
        template <class ValueType>
        any & operator=(ValueType&& rhs)
        {
            any(static_cast<ValueType&&>(rhs)).swap(*this);
            return *this;
        }
#endif

    public: // queries

        bool empty() const NDNBOOST_NOEXCEPT
        {
            return !content;
        }

        void clear() NDNBOOST_NOEXCEPT
        {
            any().swap(*this);
        }

        const std::type_info & type() const NDNBOOST_NOEXCEPT
        {
            return content ? content->type() : typeid(void);
        }

#ifndef NDNBOOST_NO_MEMBER_TEMPLATE_FRIENDS
    private: // types
#else
    public: // types (public so any_cast can be non-friend)
#endif

        class placeholder
        {
        public: // structors

            virtual ~placeholder()
            {
            }

        public: // queries

            virtual const std::type_info & type() const NDNBOOST_NOEXCEPT = 0;

            virtual placeholder * clone() const = 0;

        };

        template<typename ValueType>
        class holder : public placeholder
        {
        public: // structors

            holder(const ValueType & value)
              : held(value)
            {
            }

#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
            holder(ValueType&& value)
              : held(static_cast< ValueType&& >(value))
            {
            }
#endif
        public: // queries

            virtual const std::type_info & type() const NDNBOOST_NOEXCEPT
            {
                return typeid(ValueType);
            }

            virtual placeholder * clone() const
            {
                return new holder(held);
            }

        public: // representation

            ValueType held;

        private: // intentionally left unimplemented
            holder & operator=(const holder &);
        };

#ifndef NDNBOOST_NO_MEMBER_TEMPLATE_FRIENDS

    private: // representation

        template<typename ValueType>
        friend ValueType * any_cast(any *) NDNBOOST_NOEXCEPT;

        template<typename ValueType>
        friend ValueType * unsafe_any_cast(any *) NDNBOOST_NOEXCEPT;

#else

    public: // representation (public so any_cast can be non-friend)

#endif

        placeholder * content;

    };
 
    inline void swap(any & lhs, any & rhs) NDNBOOST_NOEXCEPT
    {
        lhs.swap(rhs);
    }

    class NDNBOOST_SYMBOL_VISIBLE bad_any_cast : public std::bad_cast
    {
    public:
        virtual const char * what() const NDNBOOST_NOEXCEPT_OR_NOTHROW
        {
            return "ndnboost::bad_any_cast: "
                   "failed conversion using ndnboost::any_cast";
        }
    };

    template<typename ValueType>
    ValueType * any_cast(any * operand) NDNBOOST_NOEXCEPT
    {
        return operand && 
#ifdef NDNBOOST_AUX_ANY_TYPE_ID_NAME
            std::strcmp(operand->type().name(), typeid(ValueType).name()) == 0
#else
            operand->type() == typeid(ValueType)
#endif
            ? &static_cast<any::holder<ValueType> *>(operand->content)->held
            : 0;
    }

    template<typename ValueType>
    inline const ValueType * any_cast(const any * operand) NDNBOOST_NOEXCEPT
    {
        return any_cast<ValueType>(const_cast<any *>(operand));
    }

    template<typename ValueType>
    ValueType any_cast(any & operand)
    {
        typedef NDNBOOST_DEDUCED_TYPENAME remove_reference<ValueType>::type nonref;


        nonref * result = any_cast<nonref>(&operand);
        if(!result)
            ndnboost::throw_exception(bad_any_cast());

        // Attempt to avoid construction of a temporary object in cases when 
        // `ValueType` is not a reference. Example:
        // `static_cast<std::string>(*result);` 
        // which is equal to `std::string(*result);`
        typedef NDNBOOST_DEDUCED_TYPENAME ndnboost::mpl::if_<
            ndnboost::is_reference<ValueType>,
            ValueType,
            NDNBOOST_DEDUCED_TYPENAME ndnboost::add_reference<ValueType>::type
        >::type ref_type;

        return static_cast<ref_type>(*result);
    }

    template<typename ValueType>
    inline ValueType any_cast(const any & operand)
    {
        typedef NDNBOOST_DEDUCED_TYPENAME remove_reference<ValueType>::type nonref;
        return any_cast<const nonref &>(const_cast<any &>(operand));
    }

#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    template<typename ValueType>
    inline ValueType any_cast(any&& operand)
    {
        NDNBOOST_STATIC_ASSERT_MSG(
            ndnboost::is_rvalue_reference<ValueType&&>::value /*true if ValueType is rvalue or just a value*/
            || ndnboost::is_const< typename ndnboost::remove_reference<ValueType>::type >::value,
            "ndnboost::any_cast shall not be used for getting nonconst references to temporary objects" 
        );
        return any_cast<ValueType>(operand);
    }
#endif


    // Note: The "unsafe" versions of any_cast are not part of the
    // public interface and may be removed at any time. They are
    // required where we know what type is stored in the any and can't
    // use typeid() comparison, e.g., when our types may travel across
    // different shared libraries.
    template<typename ValueType>
    inline ValueType * unsafe_any_cast(any * operand) NDNBOOST_NOEXCEPT
    {
        return &static_cast<any::holder<ValueType> *>(operand->content)->held;
    }

    template<typename ValueType>
    inline const ValueType * unsafe_any_cast(const any * operand) NDNBOOST_NOEXCEPT
    {
        return unsafe_any_cast<ValueType>(const_cast<any *>(operand));
    }
}

// Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#endif
