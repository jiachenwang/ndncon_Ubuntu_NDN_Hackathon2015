// Copyright (C) 2003, 2008 Fernando Luis Cacciola Carballal.
// Copyright (C) 2014 Andrzej Krzemienski.
//
// Use, modification, and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/optional for documentation.
//
// You are welcome to contact the author at:
//  fernando_cacciola@hotmail.com
//
// Revisions:
// 27 Apr 2008 (improved swap) Fernando Cacciola, Niels Dekker, Thorsten Ottosen
// 05 May 2014 (Added move semantics) Andrzej Krzemienski
//
#ifndef NDNBOOST_OPTIONAL_OPTIONAL_FLC_19NOV2002_HPP
#define NDNBOOST_OPTIONAL_OPTIONAL_FLC_19NOV2002_HPP

#include <new>
#include <algorithm>
#include <iosfwd>

#include <ndnboost/config.hpp>
#include <ndnboost/assert.hpp>
#include <ndnboost/core/explicit_operator_bool.hpp>
#include <ndnboost/optional/bad_optional_access.hpp>
#include <ndnboost/static_assert.hpp>
#include <ndnboost/throw_exception.hpp>
#include <ndnboost/type.hpp>
#include <ndnboost/type_traits/alignment_of.hpp>
#include <ndnboost/type_traits/has_nothrow_constructor.hpp>
#include <ndnboost/type_traits/type_with_alignment.hpp>
#include <ndnboost/type_traits/remove_const.hpp>
#include <ndnboost/type_traits/remove_reference.hpp>
#include <ndnboost/type_traits/decay.hpp>
#include <ndnboost/type_traits/is_base_of.hpp>
#include <ndnboost/type_traits/is_lvalue_reference.hpp>
#include <ndnboost/type_traits/is_nothrow_move_assignable.hpp>
#include <ndnboost/type_traits/is_nothrow_move_constructible.hpp>
#include <ndnboost/type_traits/is_reference.hpp>
#include <ndnboost/type_traits/is_rvalue_reference.hpp>
#include <ndnboost/type_traits/is_same.hpp>
#include <ndnboost/mpl/if.hpp>
#include <ndnboost/mpl/bool.hpp>
#include <ndnboost/mpl/not.hpp>
#include <ndnboost/detail/reference_content.hpp>
#include <ndnboost/move/utility.hpp>
#include <ndnboost/none.hpp>
#include <ndnboost/utility/addressof.hpp>
#include <ndnboost/utility/compare_pointees.hpp>
#include <ndnboost/utility/enable_if.hpp>
#include <ndnboost/utility/in_place_factory.hpp>
#include <ndnboost/utility/swap.hpp>



#include <ndnboost/optional/optional_fwd.hpp>

#if NDNBOOST_WORKAROUND(NDNBOOST_INTEL_CXX_VERSION,<=700)
// AFAICT only Intel 7 correctly resolves the overload set
// that includes the in-place factory taking functions,
// so for the other icc versions, in-place factory support
// is disabled
#define NDNBOOST_OPTIONAL_NO_INPLACE_FACTORY_SUPPORT
#endif

#if NDNBOOST_WORKAROUND(__BORLANDC__, <= 0x551)
// BCB (5.5.1) cannot parse the nested template struct in an inplace factory.
#define NDNBOOST_OPTIONAL_NO_INPLACE_FACTORY_SUPPORT
#endif

#if !defined(NDNBOOST_OPTIONAL_NO_INPLACE_FACTORY_SUPPORT) \
    && NDNBOOST_WORKAROUND(__BORLANDC__, NDNBOOST_TESTED_AT(0x581) )
// BCB (up to 5.64) has the following bug:
//   If there is a member function/operator template of the form
//     template<class Expr> mfunc( Expr expr ) ;
//   some calls are resolved to this even if there are other better matches.
//   The effect of this bug is that calls to converting ctors and assignments
//   are incrorrectly sink to this general catch-all member function template as shown above.
#define NDNBOOST_OPTIONAL_WEAK_OVERLOAD_RESOLUTION
#endif

#if defined(__GNUC__) && !defined(__INTEL_COMPILER)
// GCC since 3.3 has may_alias attribute that helps to alleviate optimizer issues with
// regard to violation of the strict aliasing rules. The optional< T > storage type is marked
// with this attribute in order to let the compiler know that it will alias objects of type T
// and silence compilation warnings.
#define NDNBOOST_OPTIONAL_DETAIL_USE_ATTRIBUTE_MAY_ALIAS
#endif

// Daniel Wallin discovered that bind/apply.hpp badly interacts with the apply<>
// member template of a factory as used in the optional<> implementation.
// He proposed this simple fix which is to move the call to apply<> outside
// namespace ndnboost.
namespace ndnboost_optional_detail
{
  template <class T, class Factory>
  inline void construct(Factory const& factory, void* address)
  {
    factory.NDNBOOST_NESTED_TEMPLATE apply<T>(address);
  }
}


namespace ndnboost {

class in_place_factory_base ;
class typed_in_place_factory_base ;

// This forward is needed to refer to namespace scope swap from the member swap
template<class T> void swap ( optional<T>& x, optional<T>& y );

namespace optional_detail {
// This local class is used instead of that in "aligned_storage.hpp"
// because I've found the 'official' class to ICE BCB5.5
// when some types are used with optional<>
// (due to sizeof() passed down as a non-type template parameter)
template <class T>
class aligned_storage
{
    // Borland ICEs if unnamed unions are used for this!
    union
    // This works around GCC warnings about breaking strict aliasing rules when casting storage address to T*
#if defined(NDNBOOST_OPTIONAL_DETAIL_USE_ATTRIBUTE_MAY_ALIAS)
    __attribute__((__may_alias__))
#endif
    dummy_u
    {
        char data[ sizeof(T) ];
        NDNBOOST_DEDUCED_TYPENAME type_with_alignment<
          ::ndnboost::alignment_of<T>::value >::type aligner_;
    } dummy_ ;

  public:

#if defined(NDNBOOST_OPTIONAL_DETAIL_USE_ATTRIBUTE_MAY_ALIAS)
    void const* address() const { return &dummy_; }
    void      * address()       { return &dummy_; }
#else
    void const* address() const { return dummy_.data; }
    void      * address()       { return dummy_.data; }
#endif
} ;

template<class T>
struct types_when_isnt_ref
{
  typedef T const& reference_const_type ;
  typedef T &      reference_type ;
#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
  typedef T &&     rval_reference_type ;
  typedef T &&     reference_type_of_temporary_wrapper;
#ifdef NDNBOOST_MOVE_OLD_RVALUE_REF_BINDING_RULES
  // GCC 4.4 has support for an early draft of rvalue references. The conforming version below
  // causes warnings about returning references to a temporary.
  static T&& move(T&& r) { return r; }
#else
  static rval_reference_type move(reference_type r) { return ndnboost::move(r); }
#endif
#endif
  typedef T const* pointer_const_type ;
  typedef T *      pointer_type ;
  typedef T const& argument_type ;
} ;

template<class T>
struct types_when_is_ref
{
  typedef NDNBOOST_DEDUCED_TYPENAME remove_reference<T>::type raw_type ;

  typedef raw_type&  reference_const_type ;
  typedef raw_type&  reference_type ;
#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
  typedef NDNBOOST_DEDUCED_TYPENAME remove_const<raw_type>::type&& rval_reference_type ;
  typedef raw_type&  reference_type_of_temporary_wrapper;
  static reference_type move(reference_type r) { return r; }
#endif
  typedef raw_type*  pointer_const_type ;
  typedef raw_type*  pointer_type ;
  typedef raw_type&  argument_type ;
} ;

template <class To, class From>
void prevent_binding_rvalue_ref_to_optional_lvalue_ref()
{
#ifndef NDNBOOST_OPTIONAL_ALLOW_BINDING_TO_RVALUES
  NDNBOOST_STATIC_ASSERT_MSG(
    !ndnboost::is_lvalue_reference<To>::value || !ndnboost::is_rvalue_reference<From>::value, 
    "binding rvalue references to optional lvalue references is disallowed");
#endif    
}

struct optional_tag {} ;

template<class T>
class optional_base : public optional_tag
{
  private :

    typedef
#if !NDNBOOST_WORKAROUND(__BORLANDC__, NDNBOOST_TESTED_AT(0x564))
    NDNBOOST_DEDUCED_TYPENAME
#endif
    ::ndnboost::detail::make_reference_content<T>::type internal_type ;

    typedef aligned_storage<internal_type> storage_type ;

    typedef types_when_isnt_ref<T> types_when_not_ref ;
    typedef types_when_is_ref<T>   types_when_ref   ;

    typedef optional_base<T> this_type ;

  protected :

    typedef T value_type ;

    typedef mpl::true_  is_reference_tag ;
    typedef mpl::false_ is_not_reference_tag ;

    typedef NDNBOOST_DEDUCED_TYPENAME is_reference<T>::type is_reference_predicate ;

  public:
    typedef NDNBOOST_DEDUCED_TYPENAME mpl::if_<is_reference_predicate,types_when_ref,types_when_not_ref>::type types ;

  protected:
    typedef NDNBOOST_DEDUCED_TYPENAME types::reference_type       reference_type ;
    typedef NDNBOOST_DEDUCED_TYPENAME types::reference_const_type reference_const_type ;
#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    typedef NDNBOOST_DEDUCED_TYPENAME types::rval_reference_type  rval_reference_type ;
    typedef NDNBOOST_DEDUCED_TYPENAME types::reference_type_of_temporary_wrapper reference_type_of_temporary_wrapper ;
#endif
    typedef NDNBOOST_DEDUCED_TYPENAME types::pointer_type         pointer_type ;
    typedef NDNBOOST_DEDUCED_TYPENAME types::pointer_const_type   pointer_const_type ;
    typedef NDNBOOST_DEDUCED_TYPENAME types::argument_type        argument_type ;

    // Creates an optional<T> uninitialized.
    // No-throw
    optional_base()
      :
      m_initialized(false) {}

    // Creates an optional<T> uninitialized.
    // No-throw
    optional_base ( none_t )
      :
      m_initialized(false) {}

    // Creates an optional<T> initialized with 'val'.
    // Can throw if T::T(T const&) does
    optional_base ( argument_type val )
      :
      m_initialized(false)
    {
      construct(val);
    }

#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // move-construct an optional<T> initialized from an rvalue-ref to 'val'.
    // Can throw if T::T(T&&) does
    optional_base ( rval_reference_type val )
      :
      m_initialized(false)
    {
      construct( ndnboost::move(val) );
    }
#endif

    // Creates an optional<T> initialized with 'val' IFF cond is true, otherwise creates an uninitialzed optional<T>.
    // Can throw if T::T(T const&) does
    optional_base ( bool cond, argument_type val )
      :
      m_initialized(false)
    {
      if ( cond )
        construct(val);
    }

    // Creates a deep copy of another optional<T>
    // Can throw if T::T(T const&) does
    optional_base ( optional_base const& rhs )
      :
      m_initialized(false)
    {
      if ( rhs.is_initialized() )
        construct(rhs.get_impl());
    }

#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // Creates a deep move of another optional<T>
    // Can throw if T::T(T&&) does
    optional_base ( optional_base&& rhs )
      :
      m_initialized(false)
    {
      if ( rhs.is_initialized() )
        construct( ndnboost::move(rhs.get_impl()) );
    }
#endif

#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES

    template<class Expr, class PtrExpr>
    explicit optional_base ( Expr&& expr, PtrExpr const* tag )
      :
      m_initialized(false)
    {
      construct(ndnboost::forward<Expr>(expr),tag);
    }

#else
    // This is used for both converting and in-place constructions.
    // Derived classes use the 'tag' to select the appropriate
    // implementation (the correct 'construct()' overload)
    template<class Expr>
    explicit optional_base ( Expr const& expr, Expr const* tag )
      :
      m_initialized(false)
    {
      construct(expr,tag);
    }

#endif


    // No-throw (assuming T::~T() doesn't)
    ~optional_base() { destroy() ; }

    // Assigns from another optional<T> (deep-copies the rhs value)
    void assign ( optional_base const& rhs )
    {
      if (is_initialized())
      {
        if ( rhs.is_initialized() )
             assign_value(rhs.get_impl(), is_reference_predicate() );
        else destroy();
      }
      else
      {
        if ( rhs.is_initialized() )
          construct(rhs.get_impl());
      }
    }
    
#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // Assigns from another optional<T> (deep-moves the rhs value)
    void assign ( optional_base&& rhs )
    {
      if (is_initialized())
      {
        if ( rhs.is_initialized() )
             assign_value(ndnboost::move(rhs.get_impl()), is_reference_predicate() );
        else destroy();
      }
      else
      {
        if ( rhs.is_initialized() )
          construct(ndnboost::move(rhs.get_impl()));
      }
    }
#endif 

    // Assigns from another _convertible_ optional<U> (deep-copies the rhs value)
    template<class U>
    void assign ( optional<U> const& rhs )
    {
      if (is_initialized())
      {
        if ( rhs.is_initialized() )
             assign_value(static_cast<value_type>(rhs.get()), is_reference_predicate() );
        else destroy();
      }
      else
      {
        if ( rhs.is_initialized() )
          construct(static_cast<value_type>(rhs.get()));
      }
    }

#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // move-assigns from another _convertible_ optional<U> (deep-moves from the rhs value)
    template<class U>
    void assign ( optional<U>&& rhs )
    {
      typedef NDNBOOST_DEDUCED_TYPENAME optional<U>::rval_reference_type ref_type;
      if (is_initialized())
      {
        if ( rhs.is_initialized() )
             assign_value(static_cast<ref_type>(rhs.get()), is_reference_predicate() );
        else destroy();
      }
      else
      {
        if ( rhs.is_initialized() )
          construct(static_cast<ref_type>(rhs.get()));
      }
    }
#endif
    
    // Assigns from a T (deep-copies the rhs value)
    void assign ( argument_type val )
    {
      if (is_initialized())
           assign_value(val, is_reference_predicate() );
      else construct(val);
    }
    
#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // Assigns from a T (deep-moves the rhs value)
    void assign ( rval_reference_type val )
    {
      if (is_initialized())
           assign_value( ndnboost::move(val), is_reference_predicate() );
      else construct( ndnboost::move(val) );
    }
#endif

    // Assigns from "none", destroying the current value, if any, leaving this UNINITIALIZED
    // No-throw (assuming T::~T() doesn't)
    void assign ( none_t ) NDNBOOST_NOEXCEPT { destroy(); }

#ifndef NDNBOOST_OPTIONAL_NO_INPLACE_FACTORY_SUPPORT

#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    template<class Expr, class ExprPtr>
    void assign_expr ( Expr&& expr, ExprPtr const* tag )
    {
      if (is_initialized())
        assign_expr_to_initialized(ndnboost::forward<Expr>(expr),tag);
      else construct(ndnboost::forward<Expr>(expr),tag);
    }
#else
    template<class Expr>
    void assign_expr ( Expr const& expr, Expr const* tag )
    {
      if (is_initialized())
        assign_expr_to_initialized(expr,tag);
      else construct(expr,tag);
    }
#endif

#endif

  public :

    // **DEPPRECATED** Destroys the current value, if any, leaving this UNINITIALIZED
    // No-throw (assuming T::~T() doesn't)
    void reset() NDNBOOST_NOEXCEPT { destroy(); }

    // **DEPPRECATED** Replaces the current value -if any- with 'val'
    void reset ( argument_type val ) { assign(val); }

    // Returns a pointer to the value if this is initialized, otherwise,
    // returns NULL.
    // No-throw
    pointer_const_type get_ptr() const { return m_initialized ? get_ptr_impl() : 0 ; }
    pointer_type       get_ptr()       { return m_initialized ? get_ptr_impl() : 0 ; }

    bool is_initialized() const { return m_initialized ; }

  protected :

    void construct ( argument_type val )
     {
       ::new (m_storage.address()) internal_type(val) ;
       m_initialized = true ;
     }
     
#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    void construct ( rval_reference_type val )
     {
       ::new (m_storage.address()) internal_type( types::move(val) ) ;
       m_initialized = true ;
     }
#endif


#if (!defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES) && (!defined NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
    // Constructs in-place
    // upon exception *this is always uninitialized
    template<class... Args>
    void emplace_assign ( Args&&... args )
     {
       destroy();
       ::new (m_storage.address()) internal_type( ndnboost::forward<Args>(args)... );
       m_initialized = true ;
     }
#elif (!defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES)
    template<class Arg>
    void emplace_assign ( Arg&& arg )
     {
       destroy();
       ::new (m_storage.address()) internal_type( ndnboost::forward<Arg>(arg) );
       m_initialized = true ;
     }
#else
    template<class Arg>
    void emplace_assign ( const Arg& arg )
     {
       destroy();
       ::new (m_storage.address()) internal_type( arg );
       m_initialized = true ;
     }
     
     template<class Arg>
    void emplace_assign ( Arg& arg )
     {
       destroy();
       ::new (m_storage.address()) internal_type( arg );
       m_initialized = true ;
     }
#endif

#ifndef NDNBOOST_OPTIONAL_NO_INPLACE_FACTORY_SUPPORT

#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // Constructs in-place using the given factory
    template<class Expr>
    void construct ( Expr&& factory, in_place_factory_base const* )
     {
       NDNBOOST_STATIC_ASSERT ( ::ndnboost::mpl::not_<is_reference_predicate>::value ) ;
       ndnboost_optional_detail::construct<value_type>(factory, m_storage.address());
       m_initialized = true ;
     }

    // Constructs in-place using the given typed factory
    template<class Expr>
    void construct ( Expr&& factory, typed_in_place_factory_base const* )
     {
       NDNBOOST_STATIC_ASSERT ( ::ndnboost::mpl::not_<is_reference_predicate>::value ) ;
       factory.apply(m_storage.address()) ;
       m_initialized = true ;
     }

    template<class Expr>
    void assign_expr_to_initialized ( Expr&& factory, in_place_factory_base const* tag )
     {
       destroy();
       construct(factory,tag);
     }

    // Constructs in-place using the given typed factory
    template<class Expr>
    void assign_expr_to_initialized ( Expr&& factory, typed_in_place_factory_base const* tag )
     {
       destroy();
       construct(factory,tag);
     }

#else
    // Constructs in-place using the given factory
    template<class Expr>
    void construct ( Expr const& factory, in_place_factory_base const* )
     {
       NDNBOOST_STATIC_ASSERT ( ::ndnboost::mpl::not_<is_reference_predicate>::value ) ;
       ndnboost_optional_detail::construct<value_type>(factory, m_storage.address());
       m_initialized = true ;
     }

    // Constructs in-place using the given typed factory
    template<class Expr>
    void construct ( Expr const& factory, typed_in_place_factory_base const* )
     {
       NDNBOOST_STATIC_ASSERT ( ::ndnboost::mpl::not_<is_reference_predicate>::value ) ;
       factory.apply(m_storage.address()) ;
       m_initialized = true ;
     }

    template<class Expr>
    void assign_expr_to_initialized ( Expr const& factory, in_place_factory_base const* tag )
     {
       destroy();
       construct(factory,tag);
     }

    // Constructs in-place using the given typed factory
    template<class Expr>
    void assign_expr_to_initialized ( Expr const& factory, typed_in_place_factory_base const* tag )
     {
       destroy();
       construct(factory,tag);
     }
#endif

#endif

#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // Constructs using any expression implicitly convertible to the single argument
    // of a one-argument T constructor.
    // Converting constructions of optional<T> from optional<U> uses this function with
    // 'Expr' being of type 'U' and relying on a converting constructor of T from U.
    template<class Expr>
    void construct ( Expr&& expr, void const* )
    {
      new (m_storage.address()) internal_type(ndnboost::forward<Expr>(expr)) ;
      m_initialized = true ;
    }

    // Assigns using a form any expression implicitly convertible to the single argument
    // of a T's assignment operator.
    // Converting assignments of optional<T> from optional<U> uses this function with
    // 'Expr' being of type 'U' and relying on a converting assignment of T from U.
    template<class Expr>
    void assign_expr_to_initialized ( Expr&& expr, void const* )
    {
      assign_value(ndnboost::forward<Expr>(expr), is_reference_predicate());
    }
#else
    // Constructs using any expression implicitly convertible to the single argument
    // of a one-argument T constructor.
    // Converting constructions of optional<T> from optional<U> uses this function with
    // 'Expr' being of type 'U' and relying on a converting constructor of T from U.
    template<class Expr>
    void construct ( Expr const& expr, void const* )
     {
       new (m_storage.address()) internal_type(expr) ;
       m_initialized = true ;
     }

    // Assigns using a form any expression implicitly convertible to the single argument
    // of a T's assignment operator.
    // Converting assignments of optional<T> from optional<U> uses this function with
    // 'Expr' being of type 'U' and relying on a converting assignment of T from U.
    template<class Expr>
    void assign_expr_to_initialized ( Expr const& expr, void const* )
     {
       assign_value(expr, is_reference_predicate());
     }

#endif

#ifdef NDNBOOST_OPTIONAL_WEAK_OVERLOAD_RESOLUTION
    // BCB5.64 (and probably lower versions) workaround.
    //   The in-place factories are supported by means of catch-all constructors
    //   and assignment operators (the functions are parameterized in terms of
    //   an arbitrary 'Expr' type)
    //   This compiler incorrectly resolves the overload set and sinks optional<T> and optional<U>
    //   to the 'Expr'-taking functions even though explicit overloads are present for them.
    //   Thus, the following overload is needed to properly handle the case when the 'lhs'
    //   is another optional.
    //
    // For VC<=70 compilers this workaround dosen't work becasue the comnpiler issues and error
    // instead of choosing the wrong overload
    //
#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // Notice that 'Expr' will be optional<T> or optional<U> (but not optional_base<..>)
    template<class Expr>
    void construct ( Expr&& expr, optional_tag const* )
     {
       if ( expr.is_initialized() )
       {
         // An exception can be thrown here.
         // It it happens, THIS will be left uninitialized.
         new (m_storage.address()) internal_type(types::move(expr.get())) ;
         m_initialized = true ;
       }
     }
#else
    // Notice that 'Expr' will be optional<T> or optional<U> (but not optional_base<..>)
    template<class Expr>
    void construct ( Expr const& expr, optional_tag const* )
     {
       if ( expr.is_initialized() )
       {
         // An exception can be thrown here.
         // It it happens, THIS will be left uninitialized.
         new (m_storage.address()) internal_type(expr.get()) ;
         m_initialized = true ;
       }
     }
#endif
#endif // defined NDNBOOST_OPTIONAL_WEAK_OVERLOAD_RESOLUTION

    void assign_value ( argument_type val, is_not_reference_tag ) { get_impl() = val; }
    void assign_value ( argument_type val, is_reference_tag     ) { construct(val); }
#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    void assign_value ( rval_reference_type val, is_not_reference_tag ) { get_impl() = static_cast<rval_reference_type>(val); }
    void assign_value ( rval_reference_type val, is_reference_tag     ) { construct( static_cast<rval_reference_type>(val) ); }
#endif

    void destroy()
    {
      if ( m_initialized )
        destroy_impl(is_reference_predicate()) ;
    }

    reference_const_type get_impl() const { return dereference(get_object(), is_reference_predicate() ) ; }
    reference_type       get_impl()       { return dereference(get_object(), is_reference_predicate() ) ; }

    pointer_const_type get_ptr_impl() const { return cast_ptr(get_object(), is_reference_predicate() ) ; }
    pointer_type       get_ptr_impl()       { return cast_ptr(get_object(), is_reference_predicate() ) ; }

  private :

    // internal_type can be either T or reference_content<T>
#if defined(NDNBOOST_OPTIONAL_DETAIL_USE_ATTRIBUTE_MAY_ALIAS)
    // This workaround is supposed to silence GCC warnings about broken strict aliasing rules
    internal_type const* get_object() const
    {
        union { void const* ap_pvoid; internal_type const* as_ptype; } caster = { m_storage.address() };
        return caster.as_ptype;
    }
    internal_type *      get_object()
    {
        union { void* ap_pvoid; internal_type* as_ptype; } caster = { m_storage.address() };
        return caster.as_ptype;
    }
#else
    internal_type const* get_object() const { return static_cast<internal_type const*>(m_storage.address()); }
    internal_type *      get_object()       { return static_cast<internal_type *>     (m_storage.address()); }
#endif

    // reference_content<T> lacks an implicit conversion to T&, so the following is needed to obtain a proper reference.
    reference_const_type dereference( internal_type const* p, is_not_reference_tag ) const { return *p ; }
    reference_type       dereference( internal_type*       p, is_not_reference_tag )       { return *p ; }
    reference_const_type dereference( internal_type const* p, is_reference_tag     ) const { return p->get() ; }
    reference_type       dereference( internal_type*       p, is_reference_tag     )       { return p->get() ; }

#if NDNBOOST_WORKAROUND(__BORLANDC__, NDNBOOST_TESTED_AT(0x581))
    void destroy_impl ( is_not_reference_tag ) { get_ptr_impl()->internal_type::~internal_type() ; m_initialized = false ; }
#else
    void destroy_impl ( is_not_reference_tag ) { get_ptr_impl()->~T() ; m_initialized = false ; }
#endif

    void destroy_impl ( is_reference_tag     ) { m_initialized = false ; }

    // If T is of reference type, trying to get a pointer to the held value must result in a compile-time error.
    // Decent compilers should disallow conversions from reference_content<T>* to T*, but just in case,
    // the following olverloads are used to filter out the case and guarantee an error in case of T being a reference.
    pointer_const_type cast_ptr( internal_type const* p, is_not_reference_tag ) const { return p ; }
    pointer_type       cast_ptr( internal_type *      p, is_not_reference_tag )       { return p ; }
    pointer_const_type cast_ptr( internal_type const* p, is_reference_tag     ) const { return &p->get() ; }
    pointer_type       cast_ptr( internal_type *      p, is_reference_tag     )       { return &p->get() ; }

    bool m_initialized ;
    storage_type m_storage ;
} ;

} // namespace optional_detail

template<class T>
class optional : public optional_detail::optional_base<T>
{
    typedef optional_detail::optional_base<T> base ;

  public :

    typedef optional<T> this_type ;

    typedef NDNBOOST_DEDUCED_TYPENAME base::value_type           value_type ;
    typedef NDNBOOST_DEDUCED_TYPENAME base::reference_type       reference_type ;
    typedef NDNBOOST_DEDUCED_TYPENAME base::reference_const_type reference_const_type ;
#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    typedef NDNBOOST_DEDUCED_TYPENAME base::rval_reference_type  rval_reference_type ;
    typedef NDNBOOST_DEDUCED_TYPENAME base::reference_type_of_temporary_wrapper reference_type_of_temporary_wrapper ;
#endif
    typedef NDNBOOST_DEDUCED_TYPENAME base::pointer_type         pointer_type ;
    typedef NDNBOOST_DEDUCED_TYPENAME base::pointer_const_type   pointer_const_type ;
    typedef NDNBOOST_DEDUCED_TYPENAME base::argument_type        argument_type ;

    // Creates an optional<T> uninitialized.
    // No-throw
    optional() NDNBOOST_NOEXCEPT : base() {}

    // Creates an optional<T> uninitialized.
    // No-throw
    optional( none_t none_ ) NDNBOOST_NOEXCEPT : base(none_) {}

    // Creates an optional<T> initialized with 'val'.
    // Can throw if T::T(T const&) does
    optional ( argument_type val ) : base(val) {}

#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // Creates an optional<T> initialized with 'move(val)'.
    // Can throw if T::T(T &&) does
    optional ( rval_reference_type val ) : base( ndnboost::forward<T>(val) ) 
      {optional_detail::prevent_binding_rvalue_ref_to_optional_lvalue_ref<T, rval_reference_type>();}
#endif

    // Creates an optional<T> initialized with 'val' IFF cond is true, otherwise creates an uninitialized optional.
    // Can throw if T::T(T const&) does
    optional ( bool cond, argument_type val ) : base(cond,val) {}

    // NOTE: MSVC needs templated versions first

    // Creates a deep copy of another convertible optional<U>
    // Requires a valid conversion from U to T.
    // Can throw if T::T(U const&) does
    template<class U>
    explicit optional ( optional<U> const& rhs )
      :
      base()
    {
      if ( rhs.is_initialized() )
        this->construct(rhs.get());
    }
    
#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // Creates a deep move of another convertible optional<U>
    // Requires a valid conversion from U to T.
    // Can throw if T::T(U&&) does
    template<class U>
    explicit optional ( optional<U> && rhs )
      :
      base()
    {
      if ( rhs.is_initialized() )
        this->construct( ndnboost::move(rhs.get()) );
    }
#endif

#ifndef NDNBOOST_OPTIONAL_NO_INPLACE_FACTORY_SUPPORT
    // Creates an optional<T> with an expression which can be either
    //  (a) An instance of InPlaceFactory (i.e. in_place(a,b,...,n);
    //  (b) An instance of TypedInPlaceFactory ( i.e. in_place<T>(a,b,...,n);
    //  (c) Any expression implicitly convertible to the single type
    //      of a one-argument T's constructor.
    //  (d*) Weak compilers (BCB) might also resolved Expr as optional<T> and optional<U>
    //       even though explicit overloads are present for these.
    // Depending on the above some T ctor is called.
    // Can throw if the resolved T ctor throws.
#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES


  template<class Expr>
  explicit optional ( Expr&& expr, 
                      NDNBOOST_DEDUCED_TYPENAME ndnboost::disable_if_c<
                        (ndnboost::is_base_of<optional_detail::optional_tag, NDNBOOST_DEDUCED_TYPENAME ndnboost::decay<Expr>::type>::value) || 
                        ndnboost::is_same<NDNBOOST_DEDUCED_TYPENAME ndnboost::decay<Expr>::type, none_t>::value >::type* = 0 
  ) 
    : base(ndnboost::forward<Expr>(expr),ndnboost::addressof(expr)) 
    {optional_detail::prevent_binding_rvalue_ref_to_optional_lvalue_ref<T, Expr&&>();}

#else
    template<class Expr>
    explicit optional ( Expr const& expr ) : base(expr,ndnboost::addressof(expr)) {}
#endif // !defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES
#endif // !defined NDNBOOST_OPTIONAL_NO_INPLACE_FACTORY_SUPPORT

    // Creates a deep copy of another optional<T>
    // Can throw if T::T(T const&) does
    optional ( optional const& rhs ) : base( static_cast<base const&>(rhs) ) {}

#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
	// Creates a deep move of another optional<T>
	// Can throw if T::T(T&&) does
	optional ( optional && rhs ) 
	  NDNBOOST_NOEXCEPT_IF(::ndnboost::is_nothrow_move_constructible<T>::value)
	  : base( ndnboost::move(rhs) ) 
	{}

#endif
   // No-throw (assuming T::~T() doesn't)
    ~optional() {}

#if !defined(NDNBOOST_OPTIONAL_NO_INPLACE_FACTORY_SUPPORT) && !defined(NDNBOOST_OPTIONAL_WEAK_OVERLOAD_RESOLUTION)
    // Assigns from an expression. See corresponding constructor.
    // Basic Guarantee: If the resolved T ctor throws, this is left UNINITIALIZED
#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES

    template<class Expr>
    NDNBOOST_DEDUCED_TYPENAME ndnboost::disable_if_c<
      ndnboost::is_base_of<optional_detail::optional_tag, NDNBOOST_DEDUCED_TYPENAME ndnboost::decay<Expr>::type>::value || 
        ndnboost::is_same<NDNBOOST_DEDUCED_TYPENAME ndnboost::decay<Expr>::type, none_t>::value,
      optional&
    >::type 
    operator= ( Expr&& expr )
      {
        optional_detail::prevent_binding_rvalue_ref_to_optional_lvalue_ref<T, Expr&&>();
        this->assign_expr(ndnboost::forward<Expr>(expr),ndnboost::addressof(expr));
        return *this ;
      }

#else
    template<class Expr>
    optional& operator= ( Expr const& expr )
      {
        this->assign_expr(expr,ndnboost::addressof(expr));
        return *this ;
      }
#endif // !defined  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
#endif // !defined(NDNBOOST_OPTIONAL_NO_INPLACE_FACTORY_SUPPORT) && !defined(NDNBOOST_OPTIONAL_WEAK_OVERLOAD_RESOLUTION)

    // Copy-assigns from another convertible optional<U> (converts && deep-copies the rhs value)
    // Requires a valid conversion from U to T.
    // Basic Guarantee: If T::T( U const& ) throws, this is left UNINITIALIZED
    template<class U>
    optional& operator= ( optional<U> const& rhs )
      {
        this->assign(rhs);
        return *this ;
      }
      
#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // Move-assigns from another convertible optional<U> (converts && deep-moves the rhs value)
    // Requires a valid conversion from U to T.
    // Basic Guarantee: If T::T( U && ) throws, this is left UNINITIALIZED
    template<class U>
    optional& operator= ( optional<U> && rhs )
      {
        this->assign(ndnboost::move(rhs));
        return *this ;
      }
#endif

    // Assigns from another optional<T> (deep-copies the rhs value)
    // Basic Guarantee: If T::T( T const& ) throws, this is left UNINITIALIZED
    //  (NOTE: On BCB, this operator is not actually called and left is left UNMODIFIED in case of a throw)
    optional& operator= ( optional const& rhs )
      {
        this->assign( static_cast<base const&>(rhs) ) ;
        return *this ;
      }

#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // Assigns from another optional<T> (deep-moves the rhs value)
    optional& operator= ( optional && rhs ) 
	  NDNBOOST_NOEXCEPT_IF(::ndnboost::is_nothrow_move_constructible<T>::value && ::ndnboost::is_nothrow_move_assignable<T>::value)
      {
        this->assign( static_cast<base &&>(rhs) ) ;
        return *this ;
      }
#endif

    // Assigns from a T (deep-copies the rhs value)
    // Basic Guarantee: If T::( T const& ) throws, this is left UNINITIALIZED
    optional& operator= ( argument_type val )
      {
        this->assign( val ) ;
        return *this ;
      }

#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    // Assigns from a T (deep-moves the rhs value)
    optional& operator= ( rval_reference_type val )
      {
        optional_detail::prevent_binding_rvalue_ref_to_optional_lvalue_ref<T, rval_reference_type>();
        this->assign( ndnboost::move(val) ) ;
        return *this ;
      }
#endif

    // Assigns from a "none"
    // Which destroys the current value, if any, leaving this UNINITIALIZED
    // No-throw (assuming T::~T() doesn't)
    optional& operator= ( none_t none_ ) NDNBOOST_NOEXCEPT
      {
        this->assign( none_ ) ;
        return *this ;
      }
      
#if (!defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES) && (!defined NDNBOOST_NO_CXX11_VARIADIC_TEMPLATES)
    // Constructs in-place
    // upon exception *this is always uninitialized
    template<class... Args>
    void emplace ( Args&&... args )
     {
       this->emplace_assign( ndnboost::forward<Args>(args)... );
     }
#elif (!defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES)
    template<class Arg>
    void emplace ( Arg&& arg )
     {
       this->emplace_assign( ndnboost::forward<Arg>(arg) );
     }
#else
    template<class Arg>
    void emplace ( const Arg& arg )
     {
       this->emplace_assign( arg );
     }
     
    template<class Arg>
    void emplace ( Arg& arg )
     {
       this->emplace_assign( arg );
     }
#endif

    void swap( optional & arg )
	  NDNBOOST_NOEXCEPT_IF(::ndnboost::is_nothrow_move_constructible<T>::value && ::ndnboost::is_nothrow_move_assignable<T>::value)
      {
        // allow for Koenig lookup
        ndnboost::swap(*this, arg);
      }


    // Returns a reference to the value if this is initialized, otherwise,
    // the behaviour is UNDEFINED
    // No-throw
    reference_const_type get() const { NDNBOOST_ASSERT(this->is_initialized()) ; return this->get_impl(); }
    reference_type       get()       { NDNBOOST_ASSERT(this->is_initialized()) ; return this->get_impl(); }

    // Returns a copy of the value if this is initialized, 'v' otherwise
    reference_const_type get_value_or ( reference_const_type v ) const { return this->is_initialized() ? get() : v ; }
    reference_type       get_value_or ( reference_type       v )       { return this->is_initialized() ? get() : v ; }

    // Returns a pointer to the value if this is initialized, otherwise,
    // the behaviour is UNDEFINED
    // No-throw
    pointer_const_type operator->() const { NDNBOOST_ASSERT(this->is_initialized()) ; return this->get_ptr_impl() ; }
    pointer_type       operator->()       { NDNBOOST_ASSERT(this->is_initialized()) ; return this->get_ptr_impl() ; }

    // Returns a reference to the value if this is initialized, otherwise,
    // the behaviour is UNDEFINED
    // No-throw
#ifndef NDNBOOST_NO_CXX11_REF_QUALIFIERS
    reference_const_type operator *() const& { return this->get() ; }
    reference_type       operator *() &      { return this->get() ; }
    reference_type_of_temporary_wrapper operator *() && { return ndnboost::move(this->get()) ; }
#else
    reference_const_type operator *() const { return this->get() ; }
    reference_type       operator *()       { return this->get() ; }
#endif // !defined NDNBOOST_NO_CXX11_REF_QUALIFIERS

#ifndef NDNBOOST_NO_CXX11_REF_QUALIFIERS
    reference_const_type value() const&
      { 
        if (this->is_initialized())
          return this->get() ;
        else
          throw_exception(bad_optional_access());
      }
      
    reference_type value() &
      { 
        if (this->is_initialized())
          return this->get() ;
        else
          throw_exception(bad_optional_access());
      }
      
    reference_type_of_temporary_wrapper value() &&
      { 
        if (this->is_initialized())
          return ndnboost::move(this->get()) ;
        else
          throw_exception(bad_optional_access());
      }

#else 
    reference_const_type value() const
      { 
        if (this->is_initialized())
          return this->get() ;
        else
          throw_exception(bad_optional_access());
      }
      
    reference_type value()
      { 
        if (this->is_initialized())
          return this->get() ;
        else
          throw_exception(bad_optional_access());
      }
#endif


#ifndef NDNBOOST_NO_CXX11_REF_QUALIFIERS
    template <class U>
    value_type value_or ( U&& v ) const&
      { 
        if (this->is_initialized())
          return get();
        else
          return ndnboost::forward<U>(v);
      }
    
    template <class U>
    value_type value_or ( U&& v ) && 
      { 
        if (this->is_initialized())
          return ndnboost::move(get());
        else
          return ndnboost::forward<U>(v);
      }
#elif !defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES
    template <class U>
    value_type value_or ( U&& v ) const 
      {
        if (this->is_initialized())
          return get();
        else
          return ndnboost::forward<U>(v);
      }
#else
    template <class U>
    value_type value_or ( U const& v ) const 
      { 
        if (this->is_initialized())
          return get();
        else
          return v;
      }
#endif


#ifndef NDNBOOST_NO_CXX11_REF_QUALIFIERS
    template <typename F>
    value_type value_or_eval ( F f ) const&
      {
        if (this->is_initialized())
          return get();
        else
          return f();
      }
      
    template <typename F>
    value_type value_or_eval ( F f ) &&
      {
        if (this->is_initialized())
          return ndnboost::move(get());
        else
          return f();
      }
#else
    template <typename F>
    value_type value_or_eval ( F f ) const
      {
        if (this->is_initialized())
          return get();
        else
          return f();
      }
#endif
      
    bool operator!() const NDNBOOST_NOEXCEPT { return !this->is_initialized() ; }
    
    NDNBOOST_EXPLICIT_OPERATOR_BOOL_NOEXCEPT()
} ;

#ifndef  NDNBOOST_NO_CXX11_RVALUE_REFERENCES
template<class T>
class optional<T&&>
{
  NDNBOOST_STATIC_ASSERT_MSG(sizeof(T) == 0, "Optional rvalue references are illegal.");
} ;
#endif

// Returns optional<T>(v)
template<class T>
inline
optional<T> make_optional ( T const& v  )
{
  return optional<T>(v);
}

// Returns optional<T>(cond,v)
template<class T>
inline
optional<T> make_optional ( bool cond, T const& v )
{
  return optional<T>(cond,v);
}

// Returns a reference to the value if this is initialized, otherwise, the behaviour is UNDEFINED.
// No-throw
template<class T>
inline
NDNBOOST_DEDUCED_TYPENAME optional<T>::reference_const_type
get ( optional<T> const& opt )
{
  return opt.get() ;
}

template<class T>
inline
NDNBOOST_DEDUCED_TYPENAME optional<T>::reference_type
get ( optional<T>& opt )
{
  return opt.get() ;
}

// Returns a pointer to the value if this is initialized, otherwise, returns NULL.
// No-throw
template<class T>
inline
NDNBOOST_DEDUCED_TYPENAME optional<T>::pointer_const_type
get ( optional<T> const* opt )
{
  return opt->get_ptr() ;
}

template<class T>
inline
NDNBOOST_DEDUCED_TYPENAME optional<T>::pointer_type
get ( optional<T>* opt )
{
  return opt->get_ptr() ;
}

// Returns a reference to the value if this is initialized, otherwise, the behaviour is UNDEFINED.
// No-throw
template<class T>
inline
NDNBOOST_DEDUCED_TYPENAME optional<T>::reference_const_type
get_optional_value_or ( optional<T> const& opt, NDNBOOST_DEDUCED_TYPENAME optional<T>::reference_const_type v )
{
  return opt.get_value_or(v) ;
}

template<class T>
inline
NDNBOOST_DEDUCED_TYPENAME optional<T>::reference_type
get_optional_value_or ( optional<T>& opt, NDNBOOST_DEDUCED_TYPENAME optional<T>::reference_type v )
{
  return opt.get_value_or(v) ;
}

// Returns a pointer to the value if this is initialized, otherwise, returns NULL.
// No-throw
template<class T>
inline
NDNBOOST_DEDUCED_TYPENAME optional<T>::pointer_const_type
get_pointer ( optional<T> const& opt )
{
  return opt.get_ptr() ;
}

template<class T>
inline
NDNBOOST_DEDUCED_TYPENAME optional<T>::pointer_type
get_pointer ( optional<T>& opt )
{
  return opt.get_ptr() ;
}

// Forward declaration to prevent operator safe-bool from being used.
template<class CharType, class CharTrait, class T>
std::basic_ostream<CharType, CharTrait>&
operator<<(std::basic_ostream<CharType, CharTrait>& out, optional<T> const& v);

// optional's relational operators ( ==, !=, <, >, <=, >= ) have deep-semantics (compare values).
// WARNING: This is UNLIKE pointers. Use equal_pointees()/less_pointess() in generic code instead.


//
// optional<T> vs optional<T> cases
//

template<class T>
inline
bool operator == ( optional<T> const& x, optional<T> const& y )
{ return equal_pointees(x,y); }

template<class T>
inline
bool operator < ( optional<T> const& x, optional<T> const& y )
{ return less_pointees(x,y); }

template<class T>
inline
bool operator != ( optional<T> const& x, optional<T> const& y )
{ return !( x == y ) ; }

template<class T>
inline
bool operator > ( optional<T> const& x, optional<T> const& y )
{ return y < x ; }

template<class T>
inline
bool operator <= ( optional<T> const& x, optional<T> const& y )
{ return !( y < x ) ; }

template<class T>
inline
bool operator >= ( optional<T> const& x, optional<T> const& y )
{ return !( x < y ) ; }


//
// optional<T> vs T cases
//
template<class T>
inline
bool operator == ( optional<T> const& x, T const& y )
{ return equal_pointees(x, optional<T>(y)); }

template<class T>
inline
bool operator < ( optional<T> const& x, T const& y )
{ return less_pointees(x, optional<T>(y)); }

template<class T>
inline
bool operator != ( optional<T> const& x, T const& y )
{ return !( x == y ) ; }

template<class T>
inline
bool operator > ( optional<T> const& x, T const& y )
{ return y < x ; }

template<class T>
inline
bool operator <= ( optional<T> const& x, T const& y )
{ return !( y < x ) ; }

template<class T>
inline
bool operator >= ( optional<T> const& x, T const& y )
{ return !( x < y ) ; }

//
// T vs optional<T> cases
//

template<class T>
inline
bool operator == ( T const& x, optional<T> const& y )
{ return equal_pointees( optional<T>(x), y ); }

template<class T>
inline
bool operator < ( T const& x, optional<T> const& y )
{ return less_pointees( optional<T>(x), y ); }

template<class T>
inline
bool operator != ( T const& x, optional<T> const& y )
{ return !( x == y ) ; }

template<class T>
inline
bool operator > ( T const& x, optional<T> const& y )
{ return y < x ; }

template<class T>
inline
bool operator <= ( T const& x, optional<T> const& y )
{ return !( y < x ) ; }

template<class T>
inline
bool operator >= ( T const& x, optional<T> const& y )
{ return !( x < y ) ; }


//
// optional<T> vs none cases
//

template<class T>
inline
bool operator == ( optional<T> const& x, none_t ) NDNBOOST_NOEXCEPT
{ return !x; }

template<class T>
inline
bool operator < ( optional<T> const& x, none_t )
{ return less_pointees(x,optional<T>() ); }

template<class T>
inline
bool operator != ( optional<T> const& x, none_t ) NDNBOOST_NOEXCEPT
{ return bool(x); }

template<class T>
inline
bool operator > ( optional<T> const& x, none_t y )
{ return y < x ; }

template<class T>
inline
bool operator <= ( optional<T> const& x, none_t y )
{ return !( y < x ) ; }

template<class T>
inline
bool operator >= ( optional<T> const& x, none_t y )
{ return !( x < y ) ; }

//
// none vs optional<T> cases
//

template<class T>
inline
bool operator == ( none_t , optional<T> const& y ) NDNBOOST_NOEXCEPT
{ return !y; }

template<class T>
inline
bool operator < ( none_t , optional<T> const& y )
{ return less_pointees(optional<T>() ,y); }

template<class T>
inline
bool operator != ( none_t, optional<T> const& y ) NDNBOOST_NOEXCEPT
{ return bool(y); }

template<class T>
inline
bool operator > ( none_t x, optional<T> const& y )
{ return y < x ; }

template<class T>
inline
bool operator <= ( none_t x, optional<T> const& y )
{ return !( y < x ) ; }

template<class T>
inline
bool operator >= ( none_t x, optional<T> const& y )
{ return !( x < y ) ; }

namespace optional_detail {

template<bool use_default_constructor> struct swap_selector;

template<>
struct swap_selector<true>
{
    template<class T>
    static void optional_swap ( optional<T>& x, optional<T>& y )
    {
        const bool hasX = !!x;
        const bool hasY = !!y;

        if ( !hasX && !hasY )
            return;

        if( !hasX )
            x = ndnboost::in_place();
        else if ( !hasY )
            y = ndnboost::in_place();

        // Boost.Utility.Swap will take care of ADL and workarounds for broken compilers
        ndnboost::swap(x.get(),y.get());

        if( !hasX )
            y = ndnboost::none ;
        else if( !hasY )
            x = ndnboost::none ;
    }
};

#ifndef NDNBOOST_NO_CXX11_RVALUE_REFERENCES
template<>
struct swap_selector<false>
{
    template<class T>
    static void optional_swap ( optional<T>& x, optional<T>& y ) 
    //NDNBOOST_NOEXCEPT_IF(::ndnboost::is_nothrow_move_constructible<T>::value && NDNBOOST_NOEXCEPT_EXPR(ndnboost::swap(*x, *y)))
    {
        if(x)
        {
            if (y)
            {
                ndnboost::swap(*x, *y);
            }
            else
            {
                y = ndnboost::move(*x);
                x = ndnboost::none;
            }
        }
        else
        {
            if (y)
            {
                x = ndnboost::move(*y);
                y = ndnboost::none;
            }
        }
    }
};
#else
template<>
struct swap_selector<false>
{
    template<class T>
    static void optional_swap ( optional<T>& x, optional<T>& y )
    {
        const bool hasX = !!x;
        const bool hasY = !!y;

        if ( !hasX && hasY )
        {
            x = y.get();
            y = ndnboost::none ;
        }
        else if ( hasX && !hasY )
        {
            y = x.get();
            x = ndnboost::none ;
        }
        else if ( hasX && hasY )
        {
            // Boost.Utility.Swap will take care of ADL and workarounds for broken compilers
            ndnboost::swap(x.get(),y.get());
        }
    }
};
#endif // !defined NDNBOOST_NO_CXX11_RVALUE_REFERENCES

} // namespace optional_detail

template<class T>
struct optional_swap_should_use_default_constructor : has_nothrow_default_constructor<T> {} ;

template<class T> inline void swap ( optional<T>& x, optional<T>& y )
  //NDNBOOST_NOEXCEPT_IF(::ndnboost::is_nothrow_move_constructible<T>::value && NDNBOOST_NOEXCEPT_EXPR(ndnboost::swap(*x, *y)))
{
    optional_detail::swap_selector<optional_swap_should_use_default_constructor<T>::value>::optional_swap(x, y);
}

} // namespace ndnboost

#endif
