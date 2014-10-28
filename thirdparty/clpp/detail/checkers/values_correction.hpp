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

#ifndef CLPP_DETAIL_VALUES_CORRECTION_HPP
#define CLPP_DETAIL_VALUES_CORRECTION_HPP

#include "common_checker.hpp"
#include "../parameter.hpp"
#include "../parameter_parts_extractor.hpp"

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

/// \namespace clpp::detail
/// \brief Details of realization.
namespace detail {

typedef common_checker< parameters, parameter_parts_extractor, std::string > checker;

/// \class parameters_values_checker
/// \brief Parameters values checker.
///
/// Checks parameter's values common correction:
/// - if parameter registered as parameter without value, it cannot have value,
/// - if parameter registered as parameter with value, it must have value.
class parameters_values_checker : public checker {
public:
    parameters_values_checker( const parameters&                    registered_parameters
                               , const parameter_parts_extractor&   extractor
                               , const std::string&                 name_value_separator ) :
            checker( registered_parameters, extractor, name_value_separator ) {}
public:
    void check( const str_storage& inputed_parameters ) const {
        BOOST_FOREACH ( const std::string& inputed_parameter, inputed_parameters ) {
        const std::string name  = extractor.extract_name_from( inputed_parameter );
        if ( this_is_parameter_with_value( inputed_parameter ) ) {
                check_parameter_with_value( name, inputed_parameter );
        } else {
            check_parameter_without_value( name );
        }
      }
    }
private:
    bool this_is_parameter_with_value( const std::string& inputed_parameter ) const {
        return boost::contains( inputed_parameter, name_value_separator );
    }
private:
    void check_parameter_with_value( const std::string& name, const std::string& inputed_parameter ) const {
        parameter_const_it it = std::find( registered_parameters.begin()
                                           , registered_parameters.end()
                                           , name );
        if ( registered_parameters.end() != it ) {
            const std::string value = extractor.extract_value_from( inputed_parameter );
            if ( !it->func_without_arg.empty() ) {
                notify_about_parameter_with_unexpected_value( name, value );
      } else if ( value.empty() ) {
          notify_about_parameter_with_missing_value( name );
      }
        } else {}
    }

    void notify_about_parameter_with_unexpected_value( const std::string& name
                                                       , const std::string& value ) const {
        const std::string what_happened = lib_prefix()
                                          + "Parameter '" + name + "' inputed with value '" + value + "', "
                                          + "but registered without value!";
    throw std::logic_error( what_happened );
    }

    void notify_about_parameter_with_missing_value( const std::string& name ) const {
        const std::string what_happened = lib_prefix() + "Parameter '" + name
                                  + "' registered with value, but value is missing!";
    throw std::logic_error( what_happened );
    }
private:
    void check_parameter_without_value( const std::string& name ) const {
        parameter_const_it it = std::find( registered_parameters.begin()
                                           , registered_parameters.end()
                                           , name );
        if ( registered_parameters.end() != it ) {
            if ( it->func_without_arg.empty() ) {
                notify_about_parameter_with_missing_value( name );
      } else {}
        } else{}
    }
};

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_VALUES_CORRECTION_HPP
