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

#ifndef CLPP_DETAIL_VALUES_SEMANTIC_HPP
#define CLPP_DETAIL_VALUES_SEMANTIC_HPP

#include "common_checker.hpp"
#include "../parameter.hpp"
#include "../parameter_parts_extractor.hpp"
#include "../argument_caster.hpp"

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/assign.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

/// \namespace clpp::detail
/// \brief Details of realization.
namespace detail {

using namespace boost::assign;

void check_ipv4_validity( const std::string& address, const std::string& parameter_name ) {
    try {
        boost::asio::ip::address_v4 a( boost::asio::ip::address_v4::from_string( address ) );
    } catch ( const std::exception& /* exc */ ) {
        const std::string what_happened = lib_prefix()
                                          + "Semantic error: parameter '" + parameter_name + "'"
                                          + " has invalid IPv4 value '" + address + "'!";
        throw std::invalid_argument( what_happened );
    }
}

void check_ipv6_validity( const std::string& address, const std::string& parameter_name ) {
    try {
        boost::asio::ip::address_v6 a( boost::asio::ip::address_v6::from_string( address ) );
    } catch ( const std::exception& /* exc */ ) {
        const std::string what_happened = lib_prefix()
                                          + "Semantic error: parameter '" + parameter_name + "'"
                                          + " has invalid IPv6 value '" + address + "'!";
        throw std::invalid_argument( what_happened );
    }
}

void check_ip_validity( const std::string& address, const std::string& parameter_name ) {
    try {
        check_ipv4_validity( address, parameter_name );
    } catch ( const std::exception& /* exc */ ) {
        try {
            check_ipv6_validity( address, parameter_name );
        } catch ( const std::exception& /* exc */ ) {
            const std::string what_happened = lib_prefix()
                                              + "Semantic error: parameter '" + parameter_name + "'"
                                              + " has invalid value '" + address + "' (not IPv4, not IPv6)!";
            throw std::invalid_argument( what_happened );
        }
    }
}

void check_path_existence( const std::string& path, const std::string& parameter_name ) {
  if ( !boost::filesystem::exists( path ) ) {
        const std::string what_happened = lib_prefix()
                                          + "Semantic error: parameter '" + parameter_name + "'"
                                          + " has invalid path value '" + path + "' (no such path)!";
        throw std::invalid_argument( what_happened );
    } else {}
}

void check_email_validity( const std::string& email, const std::string& parameter_name ) {
    const std::string what_happened = lib_prefix()
                                      + "Semantic error: parameter '" + parameter_name + "'"
                                      + " has invalid e-mail value '" + email + "'!";
    if ( !boost::contains( email, "@" ) || !boost::contains( email, "." ) ) {
        throw std::invalid_argument( what_happened );
    } else {}

    const size_t at_quantity = size_t( std::count( email.begin(), email.end(), '@' ) );
    if ( at_quantity > 1 ) {
        throw std::invalid_argument( what_happened );
    } else {}

    const size_t at_position        = email.find_first_of( '@' );
    const size_t last_dot_position  = email.find_last_of( '.' );
    const size_t last_char_position = email.size() - 1;
    if ( 0 == at_position
         || last_char_position == last_dot_position
         || ( last_dot_position - at_position ) == 1
         || at_position > last_dot_position ) {
        throw std::invalid_argument( what_happened );
    } else {}
}

typedef common_checker< parameters, parameter_parts_extractor, std::string > checker;

/// \class values_semantic_checker
/// \brief Parameter's values semantic checker.
///
/// Checks semantic of parameters that registered with function 'check_semantic()'.
class values_semantic_checker : public checker {
    typedef boost::function< void ( const std::string& /* arg */
                                    , const std::string& /* parameter name */ ) >
            check_func;

    typedef std::map
                <
                    const value_semantic
                    , const check_func
                >
            checkers;
public:
    values_semantic_checker( const parameters&                  registered_parameters
                             , const parameter_parts_extractor& extractor
                             , const std::string&               name_value_separator ) :
            checker( registered_parameters, extractor, name_value_separator ) {
        insert( semantic_checkers )( path,  check_path_existence )
                                   ( ipv4,  check_ipv4_validity )
                                   ( ipv6,  check_ipv6_validity )
                                   ( ip,    check_ip_validity )
                                   ( email, check_email_validity )
                                   ;
    }
private:
    checkers semantic_checkers;
    argument_caster caster;
public:
    void check( const str_storage& inputed_parameters ) const {
      str_storage all_names = collect_names_of_all_registered_parameters();
        check_semantic_of_inputed_values( inputed_parameters, all_names );
        check_semantic_of_default_values( all_names );
    }
private:
    str_storage collect_names_of_all_registered_parameters() const {
        str_storage all_names;
      BOOST_FOREACH ( const parameter& registered_parameter, registered_parameters ) {
        all_names += registered_parameter.short_name;
      }
        return all_names;
    }

    void check_semantic_of_inputed_values( const str_storage& inputed_parameters
                                           , str_storage& all_names ) const {
        BOOST_FOREACH ( const std::string& inputed_parameter, inputed_parameters ) {
        if ( this_is_parameter_with_value( inputed_parameter ) ) {
          const std::string name  = extractor.extract_name_from( inputed_parameter );
                const std::string value = extractor.extract_value_from( inputed_parameter );
          parameter_const_it it = std::find( registered_parameters.begin()
                                                   , registered_parameters.end()
                                                   , name );
                if ( registered_parameters.end() != it ) {
                    check_semantic_of_inputed_value( *it, value );
                    delete_element( all_names, name );
                } else {}
        } else {}
      }
    }

    bool this_is_parameter_with_value( const std::string& inputed_parameter ) const {
        return boost::contains( inputed_parameter, name_value_separator );
    }

    void check_semantic_of_inputed_value( const parameter& registered_parameter
                                          , const std::string& value ) const {
        const value_semantic semantic = registered_parameter.semantic;
        if ( no_semantic != semantic ) {
            const std::string name = registered_parameter.short_name;
            semantic_checkers.at( semantic )( value, name );
    } else {}
    }
private:
    void check_semantic_of_default_values( const str_storage& all_names ) const {
        BOOST_FOREACH ( const parameter& registered_parameter, registered_parameters ) {
        str_const_it it = std::find( all_names.begin()
                                         , all_names.end()
                                         , registered_parameter.short_name );
        if ( all_names.end() != it ) {
                check_semantic_of_default_value( registered_parameter );
        } else {}
      }
    }

    void check_semantic_of_default_value( const parameter& registered_parameter ) const {
        const value_semantic semantic = registered_parameter.semantic;
        if ( need_to_check_semantic( registered_parameter, semantic ) ) {
      s_arg_p p = boost::any_cast< s_arg_p >( registered_parameter.for_arg );
      semantic_checkers.at( registered_parameter.semantic )( p->default_value, registered_parameter.short_name );
        } else {}
    }

    bool need_to_check_semantic( const parameter& registered_parameter
                                 , const value_semantic& semantic ) const {
        return registered_parameter.has_default_value()
               && ( no_semantic != semantic )
               && caster.argument_must_be_string( registered_parameter )
               ;
    }
};

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_VALUES_SEMANTIC_HPP
