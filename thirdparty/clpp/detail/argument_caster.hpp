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

#ifndef CLPP_DETAIL_ARGUMENT_CASTER_HPP
#define CLPP_DETAIL_ARGUMENT_CASTER_HPP

#include "misc.hpp"

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/lexical_cast.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

/// \namespace clpp::detail
/// \brief Details of realization.
namespace detail {

/// \struct argument_caster
/// \brief Cast type of user's function argument (corresponding to parameter with value).
struct argument_caster {
public:
    void call_func_with_inputed_arg( const any& arg
                                     , const std::string& name
                                     , const std::string& value ) const {
        if      ( argument_must_be_bool( arg ) )               { call_with_bool_arg(               arg, name, value ); }
        else if ( argument_must_be_char( arg ) )               { call_with_char_arg(               arg, name, value ); }
        else if ( argument_must_be_signed_char( arg ) )        { call_with_signed_char_arg(        arg, name, value ); }
        else if ( argument_must_be_unsigned_char( arg ) )      { call_with_unsigned_char_arg(      arg, name, value ); }
        else if ( argument_must_be_int( arg ) )                { call_with_int_arg(                arg, name, value ); }
        else if ( argument_must_be_signed_int( arg ) )         { call_with_signed_int_arg(         arg, name, value ); }
        else if ( argument_must_be_unsigned_int( arg ) )       { call_with_unsigned_int_arg(       arg, name, value ); }
        else if ( argument_must_be_short_int( arg ) )          { call_with_short_int_arg(          arg, name, value ); }
        else if ( argument_must_be_long_int( arg ) )           { call_with_long_int_arg(           arg, name, value ); }
        else if ( argument_must_be_unsigned_long_int( arg ) )  { call_with_unsigned_long_int_arg(  arg, name, value ); }
        else if ( argument_must_be_unsigned_short_int( arg ) ) { call_with_unsigned_short_int_arg( arg, name, value ); }
        else if ( argument_must_be_signed_long_int( arg ) )    { call_with_signed_long_int_arg(    arg, name, value ); }
        else if ( argument_must_be_signed_short_int( arg ) )   { call_with_signed_short_int_arg(   arg, name, value ); }
        else if ( argument_must_be_float( arg ) )              { call_with_float_arg(              arg, name, value ); }
        else if ( argument_must_be_double( arg ) )             { call_with_double_arg(             arg, name, value ); }
        else if ( argument_must_be_long_double( arg ) )        { call_with_long_double_arg(        arg, name, value ); }
        else if ( argument_must_be_string( arg ) )             { call_with_string_arg(             arg, name, value ); }
        else {
            notify_about_unsupported_type( name );
        }
    }

    void call_func_with_default_arg( const any& arg ) const {
        if      ( argument_must_be_bool( arg ) )                { call_with_bool_arg(               arg ); }
        else if ( argument_must_be_char( arg ) )                { call_with_char_arg(               arg ); }
        else if ( argument_must_be_signed_char( arg ) )         { call_with_signed_char_arg(        arg ); }
        else if ( argument_must_be_unsigned_char( arg ) )       { call_with_unsigned_char_arg(      arg ); }
        else if ( argument_must_be_int( arg ) )                 { call_with_int_arg(                arg ); }
        else if ( argument_must_be_signed_int( arg ) )          { call_with_signed_int_arg(         arg ); }
        else if ( argument_must_be_unsigned_int( arg ) )        { call_with_unsigned_int_arg(       arg ); }
        else if ( argument_must_be_short_int( arg ) )           { call_with_short_int_arg(          arg ); }
        else if ( argument_must_be_long_int( arg ) )            { call_with_long_int_arg(           arg ); }
        else if ( argument_must_be_unsigned_long_int( arg ) )   { call_with_unsigned_long_int_arg(  arg ); }
        else if ( argument_must_be_unsigned_short_int( arg ) )  { call_with_unsigned_short_int_arg( arg ); }
        else if ( argument_must_be_signed_long_int( arg ) )     { call_with_signed_long_int_arg(    arg ); }
        else if ( argument_must_be_signed_short_int( arg ) )    { call_with_signed_short_int_arg(   arg ); }
        else if ( argument_must_be_float( arg ) )               { call_with_float_arg(              arg ); }
        else if ( argument_must_be_double( arg ) )              { call_with_double_arg(             arg ); }
        else if ( argument_must_be_long_double( arg ) )         { call_with_long_double_arg(        arg ); }
        else if ( argument_must_be_string( arg ) )              { call_with_string_arg(             arg ); }
    }

    void store_default_value( const any& arg
                              , const any& value
                              , const std::string& name ) const {
        if      ( argument_must_be_bool( arg ) )               { store_bool_value(               arg, value, name ); }
        else if ( argument_must_be_char( arg ) )               { store_char_value(               arg, value, name ); }
        else if ( argument_must_be_signed_char( arg ) )        { store_signed_char_value(        arg, value, name ); }
        else if ( argument_must_be_unsigned_char( arg ) )      { store_unsigned_char_value(      arg, value, name ); }
        else if ( argument_must_be_int( arg ) )                { store_int_value(                arg, value, name ); }
        else if ( argument_must_be_signed_int( arg ) )         { store_signed_int_value(         arg, value, name ); }
        else if ( argument_must_be_unsigned_int( arg ) )       { store_unsigned_int_value(       arg, value, name ); }
        else if ( argument_must_be_short_int( arg ) )          { store_short_int_value(          arg, value, name ); }
        else if ( argument_must_be_long_int( arg ) )           { store_long_int_value(           arg, value, name ); }
        else if ( argument_must_be_unsigned_long_int( arg ) )  { store_unsigned_long_int_value(  arg, value, name ); }
        else if ( argument_must_be_unsigned_short_int( arg ) ) { store_unsigned_short_int_value( arg, value, name ); }
        else if ( argument_must_be_signed_long_int( arg ) )    { store_signed_long_int_value(    arg, value, name ); }
        else if ( argument_must_be_signed_short_int( arg ) )   { store_signed_short_int_value(   arg, value, name ); }
        else if ( argument_must_be_float( arg ) )              { store_float_value(              arg, value, name ); }
        else if ( argument_must_be_double( arg ) )             { store_double_value(             arg, value, name ); }
        else if ( argument_must_be_long_double( arg ) )        { store_long_double_value(        arg, value, name ); }
        else if ( argument_must_be_string( arg ) )             { store_string_value(             arg, value, name ); }
        else {
            notify_about_unsupported_type( name );
        }
    }
private:
    bool argument_must_be_bool( const any& arg ) const                  { return typeid( b_arg_p ) == arg.type(); }
    bool argument_must_be_char( const any& arg ) const                  { return typeid( c_arg_p ) == arg.type(); }
    bool argument_must_be_signed_char( const any& arg ) const           { return typeid( sc_arg_p ) == arg.type(); }
    bool argument_must_be_unsigned_char( const any& arg ) const         { return typeid( uc_arg_p ) == arg.type(); }
    bool argument_must_be_int( const any& arg ) const                   { return typeid( i_arg_p ) == arg.type(); }
    bool argument_must_be_signed_int( const any& arg ) const            { return typeid( si_arg_p ) == arg.type(); }
    bool argument_must_be_unsigned_int( const any& arg ) const          { return typeid( ui_arg_p ) == arg.type(); }
    bool argument_must_be_short_int( const any& arg ) const             { return typeid( shi_arg_p ) == arg.type(); }
    bool argument_must_be_long_int( const any& arg ) const              { return typeid( li_arg_p ) == arg.type(); }
    bool argument_must_be_unsigned_long_int( const any& arg ) const     { return typeid( uli_arg_p ) == arg.type(); }
    bool argument_must_be_unsigned_short_int( const any& arg ) const    { return typeid( ushi_arg_p ) == arg.type(); }
    bool argument_must_be_signed_long_int( const any& arg ) const       { return typeid( sli_arg_p ) == arg.type(); }
    bool argument_must_be_signed_short_int( const any& arg ) const      { return typeid( sshi_arg_p ) == arg.type(); }
    bool argument_must_be_float( const any& arg ) const                 { return typeid( f_arg_p ) == arg.type(); }
    bool argument_must_be_double( const any& arg ) const                { return typeid( d_arg_p ) == arg.type(); }
    bool argument_must_be_long_double( const any& arg ) const           { return typeid( ld_arg_p ) == arg.type(); }
public:
    bool argument_must_be_string( const any& arg ) const                { return typeid( s_arg_p ) == arg.type(); }
private:
    void call_with_bool_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        b_arg_p p = boost::any_cast< b_arg_p >( arg );
        call< bool >( value, name, p );
    }

    void call_with_char_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        c_arg_p p = boost::any_cast< c_arg_p >( arg );
        call< char >( value, name, p );
    }

    void call_with_signed_char_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        sc_arg_p p = boost::any_cast< sc_arg_p >( arg );
        call< signed char >( value, name, p );
    }

    void call_with_unsigned_char_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        uc_arg_p p = boost::any_cast< uc_arg_p >( arg );
        call< unsigned char >( value, name, p );
    }

    void call_with_int_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        i_arg_p p = boost::any_cast< i_arg_p >( arg );
        call< int >( value, name, p );
    }

    void call_with_signed_int_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        si_arg_p p = boost::any_cast< si_arg_p >( arg );
        call< signed int >( value, name, p );
    }

    void call_with_unsigned_int_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        ui_arg_p p = boost::any_cast< ui_arg_p >( arg );
        call< unsigned int >( value, name, p );
    }

    void call_with_short_int_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        shi_arg_p p = boost::any_cast< shi_arg_p >( arg );
        call< short int >( value, name, p );
    }

    void call_with_long_int_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        li_arg_p p = boost::any_cast< li_arg_p >( arg );
        call< long int >( value, name, p );
    }

    void call_with_unsigned_long_int_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        uli_arg_p p = boost::any_cast< uli_arg_p >( arg );
        call< unsigned long int >( value, name, p );
    }

    void call_with_unsigned_short_int_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        ushi_arg_p p = boost::any_cast< ushi_arg_p >( arg );
        call< unsigned short int >( value, name, p );
    }

    void call_with_signed_long_int_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        sli_arg_p p = boost::any_cast< sli_arg_p >( arg );
        call< signed long int >( value, name, p );
    }

    void call_with_signed_short_int_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        sshi_arg_p p = boost::any_cast< sshi_arg_p >( arg );
        call< signed short int >( value, name, p );
    }

    void call_with_float_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        f_arg_p p = boost::any_cast< f_arg_p >( arg );
        call< float >( value, name, p );
    }

    void call_with_double_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        d_arg_p p = boost::any_cast< d_arg_p >( arg );
        call< double >( value, name, p );
    }

    void call_with_long_double_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        ld_arg_p p = boost::any_cast< ld_arg_p >( arg );
        call< long double >( value, name, p );
    }

    void call_with_string_arg(
            const any& arg, const std::string& name, const std::string& value ) const {
        s_arg_p p = boost::any_cast< s_arg_p >( arg );
        call< std::string >( value, name, p );
    }
private:
    template
    <
        typename ArgType
        , typename ArgHolderType
    >
    void call( const std::string&    inputed_value
               , const std::string&  parameter_name
               , ArgHolderType&      holder ) const {
        ArgType argument;
        try {
            argument = boost::lexical_cast< ArgType >( inputed_value );
        } catch ( const std::exception& /* exc */ ) {
            notify_about_error_type< ArgType >( parameter_name );
        }
        holder->func_with_arg( argument );
    }
private:
    void call_with_bool_arg( const any& arg ) const {
        b_arg_p p = boost::any_cast< b_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_char_arg( const any& arg ) const {
        c_arg_p p = boost::any_cast< c_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_signed_char_arg( const any& arg ) const {
        sc_arg_p p = boost::any_cast< sc_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_unsigned_char_arg( const any& arg ) const {
        uc_arg_p p = boost::any_cast< uc_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_int_arg( const any& arg ) const {
        i_arg_p p = boost::any_cast< i_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_signed_int_arg( const any& arg ) const {
        si_arg_p p = boost::any_cast< si_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_unsigned_int_arg( const any& arg ) const {
        ui_arg_p p = boost::any_cast< ui_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_short_int_arg( const any& arg ) const {
        shi_arg_p p = boost::any_cast< shi_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_long_int_arg( const any& arg ) const {
        li_arg_p p = boost::any_cast< li_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_unsigned_long_int_arg( const any& arg ) const {
        uli_arg_p p = boost::any_cast< uli_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_unsigned_short_int_arg( const any& arg ) const {
        ushi_arg_p p = boost::any_cast< ushi_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_signed_long_int_arg( const any& arg ) const {
        sli_arg_p p = boost::any_cast< sli_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_signed_short_int_arg( const any& arg ) const {
        sshi_arg_p p = boost::any_cast< sshi_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_float_arg( const any& arg ) const {
        f_arg_p p = boost::any_cast< f_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_double_arg( const any& arg ) const {
        d_arg_p p = boost::any_cast< d_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_long_double_arg( const any& arg ) const {
        ld_arg_p p = boost::any_cast< ld_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }

    void call_with_string_arg( const any& arg ) const {
        s_arg_p p = boost::any_cast< s_arg_p >( arg );
        p->func_with_arg( p->default_value );
    }
private:
    void store_bool_value( const any& arg
                           , const any& inputed_value
                           , const std::string& parameter_name ) const {
        b_arg_p p = boost::any_cast< b_arg_p >( arg );
        store< bool >( inputed_value, p, parameter_name );
    }

    void store_char_value( const any& arg
                           , const any& inputed_value
                           , const std::string& parameter_name ) const {
        c_arg_p p = boost::any_cast< c_arg_p >( arg );
        store< char >( inputed_value, p, parameter_name );
    }

    void store_signed_char_value( const any& arg
                                  , const any& inputed_value
                                  , const std::string& parameter_name ) const {
        sc_arg_p p = boost::any_cast< sc_arg_p >( arg );
        store< signed char >( inputed_value, p, parameter_name );
    }

    void store_unsigned_char_value( const any& arg
                                    , const any& inputed_value
                                    , const std::string& parameter_name ) const {
        uc_arg_p p = boost::any_cast< uc_arg_p >( arg );
        store< unsigned char >( inputed_value, p, parameter_name );
    }

    void store_int_value( const any& arg
                          , const any& inputed_value
                          , const std::string& parameter_name ) const {
        i_arg_p p = boost::any_cast< i_arg_p >( arg );
        store< int >( inputed_value, p, parameter_name );
    }

    void store_signed_int_value( const any& arg
                                 , const any& inputed_value
                                 , const std::string& parameter_name ) const {
        si_arg_p p = boost::any_cast< si_arg_p >( arg );
        store< signed int >( inputed_value, p, parameter_name );
    }

    void store_unsigned_int_value( const any& arg
                                   , const any& inputed_value
                                   , const std::string& parameter_name ) const {
        ui_arg_p p = boost::any_cast< ui_arg_p >( arg );
        store< unsigned int >( inputed_value, p, parameter_name );
    }

    void store_short_int_value( const any& arg
                                , const any& inputed_value
                                , const std::string& parameter_name ) const {
        shi_arg_p p = boost::any_cast< shi_arg_p >( arg );
        store< short int >( inputed_value, p, parameter_name );
    }

    void store_long_int_value( const any& arg
                               , const any& inputed_value
                               , const std::string& parameter_name ) const {
        li_arg_p p = boost::any_cast< li_arg_p >( arg );
        store< long int >( inputed_value, p, parameter_name );
    }

    void store_unsigned_short_int_value( const any& arg
                                         , const any& inputed_value
                                         , const std::string& parameter_name ) const {
        ushi_arg_p p = boost::any_cast< ushi_arg_p >( arg );
        store< unsigned short int >( inputed_value, p, parameter_name );
    }

    void store_unsigned_long_int_value( const any& arg
                                        , const any& inputed_value
                                        , const std::string& parameter_name ) const {
        uli_arg_p p = boost::any_cast< uli_arg_p >( arg );
        store< unsigned long int >( inputed_value, p, parameter_name );
    }

    void store_signed_short_int_value( const any& arg
                                       , const any& inputed_value
                                       , const std::string& parameter_name ) const {
        sshi_arg_p p = boost::any_cast< sshi_arg_p >( arg );
        store< signed short int >( inputed_value, p, parameter_name );
    }

    void store_signed_long_int_value( const any& arg
                                      , const any& inputed_value
                                      , const std::string& parameter_name ) const {
        sli_arg_p p = boost::any_cast< sli_arg_p >( arg );
        store< signed long int >( inputed_value, p, parameter_name );
    }

    void store_float_value( const any& arg
                            , const any& inputed_value
                            , const std::string& parameter_name ) const {
        f_arg_p p = boost::any_cast< f_arg_p >( arg );
        store< float >( inputed_value, p, parameter_name );
    }

    void store_double_value( const any& arg
                             , const any& inputed_value
                             , const std::string& parameter_name ) const {
        d_arg_p p = boost::any_cast< d_arg_p >( arg );
        store< double >( inputed_value, p, parameter_name );
    }

    void store_long_double_value( const any& arg
                                  , const any& inputed_value
                                  , const std::string& parameter_name ) const {
        ld_arg_p p = boost::any_cast< ld_arg_p >( arg );
        store< long double >( inputed_value, p, parameter_name );
    }

    void store_string_value( const any& arg
                             , const any& inputed_value
                             , const std::string& parameter_name ) const {
        s_arg_p p = boost::any_cast< s_arg_p >( arg );
        store< std::string >( inputed_value, p, parameter_name );
    }
private:
    template
    <
        typename ArgType
        , typename ArgHolderType
    >
    void store( const any& inputed_value, ArgHolderType& holder, const std::string& parameter_name ) const {
        if ( typeid( ArgType ) == inputed_value.type() ) {
            holder->default_value = boost::any_cast< ArgType >( inputed_value );
        } else {
            notify_about_error_type_of_default_value< ArgType >( parameter_name );
        }
    }
private:
    template< typename ArgType >
    void notify_about_error_type( const std::string& parameter_name ) const {
        const std::string what_happened = lib_prefix()
                                          + "Value's type for parameter '" + parameter_name
                                          + "' must be <" + get_type_identifier< ArgType >() + ">!"
                                          ;
        throw std::invalid_argument( what_happened );
    }

    template< typename ArgType >
    void notify_about_error_type_of_default_value( const std::string& parameter_name ) const {
        const std::string what_happened = lib_prefix()
                                          + "Default value's type for parameter '" + parameter_name
                                          + "' must be <" + get_type_identifier< ArgType >() + ">!"
                                          ;
        throw std::invalid_argument( what_happened );
    }

    template< typename ArgType >
    std::string get_type_identifier() const {
        std::string identifier;

        if      ( typeid( ArgType ) == typeid( bool ) )                { identifier = "bool"; }
        else if ( typeid( ArgType ) == typeid( char ) )                { identifier = "char"; }
        else if ( typeid( ArgType ) == typeid( signed char ) )         { identifier = "signed char"; }
        else if ( typeid( ArgType ) == typeid( unsigned char ) )       { identifier = "unsigned char"; }
        else if ( typeid( ArgType ) == typeid( int ) )                 { identifier = "int"; }
        else if ( typeid( ArgType ) == typeid( signed int ) )          { identifier = "signed int"; }
        else if ( typeid( ArgType ) == typeid( unsigned int ) )        { identifier = "unsigned int"; }
        else if ( typeid( ArgType ) == typeid( short int ) )           { identifier = "short int"; }
        else if ( typeid( ArgType ) == typeid( long int ) )            { identifier = "long int"; }
        else if ( typeid( ArgType ) == typeid( unsigned long int ) )   { identifier = "unsigned long int"; }
        else if ( typeid( ArgType ) == typeid( unsigned short int ) )  { identifier = "unsigned short int"; }
        else if ( typeid( ArgType ) == typeid( signed long int ) )     { identifier = "signed long int"; }
        else if ( typeid( ArgType ) == typeid( signed short int ) )    { identifier = "signed short int"; }
        else if ( typeid( ArgType ) == typeid( float ) )               { identifier = "float"; }
        else if ( typeid( ArgType ) == typeid( double ) )              { identifier = "double"; }
        else if ( typeid( ArgType ) == typeid( long double ) )         { identifier = "long double"; }
        else if ( typeid( ArgType ) == typeid( std::string ) )         { identifier = "std::string"; }

        return identifier;
    }

    void notify_about_unsupported_type( const std::string& parameter_name ) const {
        const std::string what_happened = "Parameter '"
                                          + parameter_name
                                          + "' registered with function having argument's unsupported type!";
        throw std::invalid_argument( what_happened );
    }
};

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_ARGUMENT_CASTER_HPP
