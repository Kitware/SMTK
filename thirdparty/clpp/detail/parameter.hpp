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

#ifndef CLPP_DETAIL_PARAMETER_HPP
#define CLPP_DETAIL_PARAMETER_HPP

#include "argument_holder.hpp"
#include "argument_caster.hpp"
#include "checkers/all.hpp"
#include "misc.hpp"

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <set>

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

/// \namespace clpp::detail
/// \brief Details of realization.
namespace detail {

/// \class parameter
/// \brief Command line parameter.
///
/// Presents one command line parameter with all options.
class parameter {
    typedef boost::function< void () >  user_func_without_arg;
    typedef std::set< size_t >          orders_storage;
public:
    explicit parameter( const std::string&   short_name
                        , const std::string& full_name
                        , void ( *fn )() ) :
            func_without_arg( fn ) {
        init( short_name, full_name );
    }

    template< typename ArgType >
    explicit parameter( const std::string&   short_name
                        , const std::string& full_name
                        , void ( *fn )( const ArgType& ) ) :
            for_arg( boost::make_shared< argument_holder<ArgType> >( fn ) ) {
        init( short_name, full_name );
    }

    template< typename ArgType >
    explicit parameter( const std::string&   short_name
                        , const std::string& full_name
                        , void ( *fn )( ArgType ) ) :
            for_arg( boost::make_shared< argument_holder<ArgType> >( fn ) ) {
        init( short_name, full_name );
    }

    template< typename ObjectType >
    explicit parameter( const std::string&   short_name
                        , const std::string& full_name
                        , ObjectType* obj
                        , void ( ObjectType::*fn )() ) :
            func_without_arg( boost::bind( fn, obj ) ) {
        init( short_name, full_name );
    }

    template< typename ObjectType >
    explicit parameter( const std::string&   short_name
                        , const std::string& full_name
                        , ObjectType* obj
                        , void ( ObjectType::*fn )() const ) :
            func_without_arg( boost::bind( fn, obj ) ) {
        init( short_name, full_name );
    }

    template
    <
      typename ObjectType
      , typename ArgType
    >
    explicit parameter( const std::string&   short_name
                        , const std::string& full_name
                        , ObjectType* obj
                        , void ( ObjectType::*fn )( const ArgType& ) ) :
            for_arg( boost::make_shared< argument_holder<ArgType> >( obj, fn ) ) {
        init( short_name, full_name );
    }

    template
    <
      typename ObjectType
      , typename ArgType
    >
    explicit parameter( const std::string&   short_name
                        , const std::string& full_name
                        , ObjectType* obj
                        , void ( ObjectType::*fn )( const ArgType& ) const ) :
            for_arg( boost::make_shared< argument_holder<ArgType> >( obj, fn ) ) {
        init( short_name, full_name );
    }

    template
    <
      typename ObjectType
      , typename ArgType
    >
    explicit parameter( const std::string&   short_name
                        , const std::string& full_name
                        , ObjectType* obj
                        , void ( ObjectType::*fn )( ArgType ) ) :
            for_arg( boost::make_shared< argument_holder<ArgType> >( obj, fn ) ) {
        init( short_name, full_name );
    }

    template
    <
      typename ObjectType
      , typename ArgType
    >
    explicit parameter( const std::string&   short_name
                        , const std::string& full_name
                        , ObjectType* obj
                        , void ( ObjectType::*fn )( ArgType ) const ) :
            for_arg( boost::make_shared< argument_holder<ArgType> >( obj, fn ) ) {
        init( short_name, full_name );
    }
private:
    void init( const std::string& _short_name, const std::string& _full_name ) {
        short_name           = _short_name;
        full_name            = _full_name;
        is_necessary         = false;
        is_has_default_value = false;
        semantic             = no_semantic;
        order_number         = 0;
    }
public:
    std::string             short_name;
    std::string             full_name;
  bool                is_necessary;
  bool              is_has_default_value;
  any                   for_arg;
    user_func_without_arg func_without_arg;
  value_semantic          semantic;
    int                     order_number;
    static orders_storage   orders;
public:
    bool has_default_value() const { return is_has_default_value; }
    bool it_is_necessary() const { return is_necessary; }
public:
  parameter& check_semantic( const value_semantic& _semantic ) {
        check_semantic_validity( _semantic );
      semantic = _semantic;
      return *this;
  }
private:
    void check_semantic_validity( const value_semantic& _semantic ) const {
        if ( _semantic < no_semantic || _semantic > email ) {
            const std::string what_happened = lib_prefix()
                    + "Incorrect value of semantic, supports only 'path', 'ipv4', 'ipv6', 'ip' and 'email'!";
            throw std::invalid_argument( what_happened );
        } else {}
    }
public:
  parameter& necessary() {
    check_parameter_default_value_existence();
    is_necessary = true;
    return *this;
  }
private:
    void check_validity_of_string_default_value( const any& value ) const {
        if ( typeid( std::string ) == value.type() ) {
            const std::string value_as_string = boost::any_cast< std::string >( value );
            if ( boost::contains( value_as_string, " " ) ) {
                const std::string what_happened = lib_prefix()
                                                  + "Invalid default parameter's value '" + short_name
                                      + "': it shouldn't contains space(s)!";
              throw std::invalid_argument( what_happened );
            } else {}
        } else {}
    }

    void check_parameter_necessity() const {
        if ( is_necessary ) {
      const std::string what_happened = lib_prefix()
                                              + "Parameter '" + short_name
                                        + "' defined as necessary, it cannot have default value!";
      throw std::logic_error( what_happened );
    } else {}
    }

    void check_parameter_default_value_existence() const {
        if ( is_has_default_value ) {
      const std::string what_happened = lib_prefix()
                                              + "Parameter '" + short_name
                                        + "' already have default value, it cannot be necessary!";
      throw std::logic_error( what_happened );
    } else {}
    }
private:
    argument_caster caster;
public:
  parameter& default_value( const any& value ) {
      check_parameter_necessity();
    check_validity_of_string_default_value( value );
    is_has_default_value = true;
        caster.store_default_value( for_arg, value, short_name );
    return *this;
  }
private:
template< typename ExpectedType >
    ExpectedType check_type_of_default_value( const any& value ) const {
        ExpectedType default_value;
        if ( typeid( default_value ) == value.type() ) {
      default_value = boost::any_cast< ExpectedType >( value );
    } else {
      const std::string what_happened = lib_prefix()
                                              + "Incorrect default value's type of parameter '"
                                              + short_name + "'!";
      throw std::invalid_argument( what_happened );
    }
        return default_value;
    }
public:
  /// \brief Define parameter's default value.
  /// Overloaded for const char* arguments,
  /// because boost::any cannot be initialized by array.
  parameter& default_value( const char* value ) {
    return default_value( std::string( value ) );
  }
public:
    parameter& order( int _order_number ) {
        order_number = _order_number;
        check_order_validity();
        check_order_uniqueness();
        return *this;
    }
private:
    void check_order_validity() const {
        if ( order_number < 1 ) {
            o_stream what_happened;
            what_happened << lib_prefix()
                          << "Invalid order number '" << order_number << "' "
                          << "(parameter '" << short_name << "'), "
                          << "it cannot be less than 1!";
            throw std::invalid_argument( what_happened.str() );
        } else {}
    }

    void check_order_uniqueness() {
        const size_t current_orders_quantity = orders.size();
        orders.insert( order_number );
        const size_t new_orders_quantity = orders.size();
        if ( current_orders_quantity == new_orders_quantity ) {
            const std::string what_happened = lib_prefix() + "Parameter's order number must be unique!";
            throw std::logic_error( what_happened );
        } else {}
    }
public:
    bool operator==( const std::string& parameter_name ) const {
      return parameter_name == short_name || parameter_name == full_name;
    }

    bool operator==( int _order_number ) const {
        return _order_number == order_number;
    }
};

parameter::orders_storage parameter::orders;

} // namespace detail

typedef detail::parameter           parameter;
typedef std::vector< parameter >    parameters;
typedef parameters::iterator        parameter_it;
typedef parameters::const_iterator  parameter_const_it;

} // namespace clpp

#endif // CLPP_DETAIL_PARAMETER_HPP
