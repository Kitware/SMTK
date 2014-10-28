// C++ Command line parameters parser.
//
// Copyright (C) Denis Shevchenko, 2010.
// shev.denis @ gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// See http://www.opensource.org/licenses/mit-license.php
//
// http://clp-parser.sourceforge.net/

#ifndef CLPP_DETAIL_ARGUMENT_HOLDER_HPP
#define CLPP_DETAIL_ARGUMENT_HOLDER_HPP

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <string>

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

/// \namespace clpp::detail
/// \brief Details of realization.
namespace detail {

/// \struct argument_holder
/// \brief Argument's info holder.
///
/// Hold ptr to function with arguments (corresponding to parameter with value),
/// and default value for it (if defined).
template< typename ArgType >
struct argument_holder {
    typedef boost::function< void ( const ArgType& /* value */ ) >
            user_function_with_arg;
public:
  explicit argument_holder( void (*fn)( const ArgType& ) ) :
            func_with_arg( fn ) {}

  explicit argument_holder( void (*fn)( ArgType ) ) :
            func_with_arg( fn ) {}

  template< typename Object >
  explicit argument_holder( Object* obj, void ( Object::*fn )( const ArgType& ) ) :
            func_with_arg( boost::bind( fn, obj, _1 ) ) {}

  template< typename Object >
  explicit argument_holder( Object* obj, void ( Object::*fn )( const ArgType& ) const ) :
            func_with_arg( boost::bind( fn, obj, _1 ) ) {}

  template< typename Object >
  explicit argument_holder( Object* obj, void ( Object::*fn )( ArgType ) ) :
        func_with_arg( boost::bind( fn, obj, _1 ) ) {}

    template< typename Object >
  explicit argument_holder( Object* obj, void ( Object::*fn )( ArgType ) const ) :
        func_with_arg( boost::bind( fn, obj, _1 ) ) {}
public:
    user_function_with_arg  func_with_arg;
    ArgType           default_value;
};

typedef boost::shared_ptr< argument_holder< bool > >                b_arg_p;

typedef boost::shared_ptr< argument_holder< char > >                c_arg_p;
typedef boost::shared_ptr< argument_holder< signed char > >         sc_arg_p;
typedef boost::shared_ptr< argument_holder< unsigned char > >       uc_arg_p;
typedef boost::shared_ptr< argument_holder< const char* > >         cca_arg_p;

typedef boost::shared_ptr< argument_holder< int > >           i_arg_p;
typedef boost::shared_ptr< argument_holder< signed int > >      si_arg_p;
typedef boost::shared_ptr< argument_holder< unsigned int > >    ui_arg_p;
typedef boost::shared_ptr< argument_holder< short int > >           shi_arg_p;
typedef boost::shared_ptr< argument_holder< long int > >          li_arg_p;
typedef boost::shared_ptr< argument_holder< unsigned long int > >   uli_arg_p;
typedef boost::shared_ptr< argument_holder< unsigned short int > >  ushi_arg_p;
typedef boost::shared_ptr< argument_holder< signed long int > >   sli_arg_p;
typedef boost::shared_ptr< argument_holder< signed short int > >  sshi_arg_p;

typedef boost::shared_ptr< argument_holder< float > >               f_arg_p;
typedef boost::shared_ptr< argument_holder< double > >              d_arg_p;
typedef boost::shared_ptr< argument_holder< long double > >         ld_arg_p;

typedef boost::shared_ptr< argument_holder< std::string > >       s_arg_p;

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_ARGUMENT_HOLDER_HPP
