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

#ifndef CLPP_DETAIL_UNNAMED_PARAMETERS_HANDLER_HPP
#define CLPP_DETAIL_UNNAMED_PARAMETERS_HANDLER_HPP

#include "parameter.hpp"
#include "parameter_parts_extractor.hpp"

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

/// \namespace clpp::detail
/// \brief Details of realization.
namespace detail {

/// \class unnamed_parameters_handler
/// \brief Unnamed parameters handler.
///
/// Handle unnamed inputed parameters.
/// Assumed that unnamed parameter registered with 'order()' function.
class unnamed_parameters_handler {
public:
    unnamed_parameters_handler( const parameters&                     _registered_parameters
                                , const parameter_parts_extractor&    _extractor
                                , const std::string&                  _name_value_separator ) :
            registered_parameters( _registered_parameters )
            , extractor( _extractor )
            , name_value_separator( _name_value_separator ) {}
private:
    const parameters&                registered_parameters;
    const parameter_parts_extractor& extractor;
    const std::string&               name_value_separator;
public:
    void operator()( str_storage& inputed_parameters ) const {
        if ( no_registered_unnamed_parameters() ) {
            return;
        } else {}

        for ( size_t i = 0; i < inputed_parameters.size(); ++i ) {
            std::string& inputed_parameter = inputed_parameters[i];
            if ( this_is_unnamed_parameter( inputed_parameter ) ) {
                handle_unnamed_parameter( i, inputed_parameter );
            } else {}
        }
    }
private:
    bool no_registered_unnamed_parameters() const {
        return parameter::orders.empty();
    }

    bool this_is_unnamed_parameter( const std::string& inputed_parameter ) const {
        return !boost::contains( inputed_parameter, name_value_separator )
               && this_is_not_name( inputed_parameter );
    }

    bool this_is_not_name( const std::string& inputed_parameter ) const {
        parameter_const_it it = std::find( registered_parameters.begin()
                                           , registered_parameters.end()
                                           , inputed_parameter );
        return registered_parameters.end() == it;
    }

    void handle_unnamed_parameter( size_t inputed_parameter_index, std::string& inputed_parameter ) const {
        const size_t inputed_parameter_serial_number = inputed_parameter_index + 1;
        parameter_const_it it = std::find( registered_parameters.begin()
                                           , registered_parameters.end()
                                           , inputed_parameter_serial_number );
        if ( registered_parameters.end() != it ) {
            const std::string value_of_unnamed_parameter = inputed_parameter;
            inputed_parameter = it->short_name + name_value_separator + value_of_unnamed_parameter;
        } else {
            notify_about_missing_of_parameter_with_serial_number( inputed_parameter
                                                                  , inputed_parameter_serial_number );
        }
    }

    void notify_about_missing_of_parameter_with_serial_number( const std::string& inputed_parameter
                                                               , size_t serial_number ) const {
        o_stream what_happened;
        what_happened << lib_prefix()
                      << "You inputed unnamed parameter '" << inputed_parameter << "' "
                      << "with order number " << serial_number << ", "
                      << "but there is no parameter with such order number!";
        throw std::runtime_error( what_happened.str() );
    }
};

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_UNNAMED_PARAMETERS_HANDLER_HPP
