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

#ifndef CLPP_DETAIL_USER_FUNCTIONS_CALLER_HPP
#define CLPP_DETAIL_USER_FUNCTIONS_CALLER_HPP

#include "parameter.hpp"
#include "parameter_parts_extractor.hpp"
#include "argument_caster.hpp"

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/assign.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <iostream>

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

/// \namespace clpp::detail
/// \brief Details of realization.
namespace detail {

using namespace boost::assign;

/// \class user_functions_caller
/// \brief User's function caller.
///
/// Calls user's functions corresponding to registered parameters.
///
/// Exception occurs in the user's functions are NOT handled (take care of this yourself).
class user_functions_caller {
public:
    user_functions_caller( const parameters&                    _registered_parameters
                           , const parameter_parts_extractor&   _extractor
                           , const std::string&                 _name_value_separator ) :
            registered_parameters( _registered_parameters )
            , extractor( _extractor )
            , name_value_separator( _name_value_separator ) {}
private:
    const parameters&                registered_parameters;
    const parameter_parts_extractor& extractor;
    const std::string&               name_value_separator;
    argument_caster                  caster;
public:
    void call( const str_storage& inputed_parameters ) const {
      str_storage all_names = collect_names_of_all_registered_parameters();
        call_functions_for_parameters_without_values( inputed_parameters, all_names );
        call_functions_for_parameters_with_inputed_values( inputed_parameters, all_names );
        call_functions_for_parameters_with_default_values( all_names );
    }
private:
    str_storage collect_names_of_all_registered_parameters() const {
        str_storage all_names;
      BOOST_FOREACH ( const parameter& registered_parameter, registered_parameters ) {
        all_names += registered_parameter.short_name;
      }
        return all_names;
    }
private:
    void call_functions_for_parameters_without_values( const str_storage&  inputed_parameters
                                                       , str_storage&      all_names ) const {
        BOOST_FOREACH ( const std::string& inputed_parameter, inputed_parameters ) {
            if ( this_is_parameter_without_value( inputed_parameter ) ) {
                const std::string name = inputed_parameter;
                parameter_const_it it = std::find( registered_parameters.begin()
                                                   , registered_parameters.end()
                                                   , name );
                if ( registered_parameters.end() != it ) {
                    if ( !it->func_without_arg.empty() ) {
                        it->func_without_arg();
                    } else {}
                    delete_element( all_names, it->short_name );
                } else {}
            }
        }
    }

    void call_functions_for_parameters_with_inputed_values( const str_storage&  inputed_parameters
                                                            , str_storage&      all_names ) const {
        BOOST_FOREACH ( const std::string& inputed_parameter, inputed_parameters ) {
            if ( this_is_parameter_with_value( inputed_parameter ) ) {
                const std::string name  = extractor.extract_name_from( inputed_parameter );
                parameter_const_it it = std::find( registered_parameters.begin()
                                                   , registered_parameters.end()
                                                   , name );
                if ( registered_parameters.end() != it ) {
                    if ( !it->func_without_arg.empty() ) {
                        it->func_without_arg();
                    } else {
                        const std::string value = extractor.extract_value_from( inputed_parameter );
                        call_func_with_arg( *it, value );
                    }
                    delete_element( all_names, it->short_name );
                } else {}
            }
        }
    }

    bool this_is_parameter_with_value( const std::string& inputed_parameter ) const {
        return boost::contains( inputed_parameter, name_value_separator );
    }

    bool this_is_parameter_without_value( const std::string& inputed_parameter ) const {
        return !boost::contains( inputed_parameter, name_value_separator );
    }

    void call_func_with_arg( const parameter& registered_parameter
                             , const std::string& inputed_value ) const {
        const any& for_arg = registered_parameter.for_arg;
        const std::string name = registered_parameter.short_name;
        caster.call_func_with_inputed_arg( for_arg, name, inputed_value );
    }
private:
    void call_functions_for_parameters_with_default_values( const str_storage& all_names ) const {
        BOOST_FOREACH ( const parameter& registered_parameter, registered_parameters ) {
        str_const_it it = std::find( all_names.begin()
                                         , all_names.end()
                                         , registered_parameter.short_name );
            if ( all_names.end() != it ) {
                call_func_with_default_arg( registered_parameter );
            } else {}
      }
    }

    void call_func_with_default_arg( const parameter& registered_parameter ) const {
        if ( registered_parameter.has_default_value() && registered_parameter.func_without_arg.empty() ) {
            const any& for_arg = registered_parameter.for_arg;
            caster.call_func_with_default_arg( for_arg );
        } else {}
    }
};

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_USER_FUNCTIONS_CALLER_HPP
