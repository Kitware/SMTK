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

#ifndef CLPP_DETAIL_INCORRECT_PARAMETERS_HPP
#define CLPP_DETAIL_INCORRECT_PARAMETERS_HPP

#include "common_checker.hpp"
#include "../parameter.hpp"
#include "../parameter_parts_extractor.hpp"

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/assign.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

/// \namespace clpp::detail
/// \brief Details of realization.
namespace detail {

typedef common_checker< parameters, parameter_parts_extractor, std::string > checker;

using namespace boost::assign;

/// \class incorrect_parameters_checker
/// \brief Incorrect parameters checker.
///
/// Checks existence of incorrect parameters.
/// Assumed that only registered parameters can be inputed.
class incorrect_parameters_checker : public checker {
public:
    incorrect_parameters_checker( const parameters&                     registered_parameters
                                  , const parameter_parts_extractor&    extractor
                                  , const std::string&                  name_value_separator ) :
            checker( registered_parameters, extractor, name_value_separator ) {}
public:
    void check( const str_storage& inputed_parameters ) const {
        str_storage names_of_incorrect_parameters;
        collect_names_of_incorrect_parameters_if_such_exists( inputed_parameters, names_of_incorrect_parameters );
        if ( !names_of_incorrect_parameters.empty() ) {
            notify_about_incorrect_parameters( names_of_incorrect_parameters );
        } else {}
    }
private:
    void collect_names_of_incorrect_parameters_if_such_exists( const str_storage& inputed_parameters
                                                               , str_storage& names_of_incorrect_parameters ) const {
        BOOST_FOREACH ( const std::string& inputed_parameter, inputed_parameters ) {
            const std::string name = extractor.extract_name_from( inputed_parameter );
            parameter_const_it it = std::find( registered_parameters.begin()
                                               , registered_parameters.end()
                                               , name );
            if ( registered_parameters.end() == it ) {
                names_of_incorrect_parameters += name;
            } else {}
        }
    }

    void notify_about_incorrect_parameters( const str_storage& names_of_incorrect_parameters ) const {
        std::string what_happened = lib_prefix() + "Parameter ";

        BOOST_FOREACH ( const std::string& name, names_of_incorrect_parameters ) {
          what_happened += "'" + name + "', ";
        }
        boost::erase_last( what_happened, ", " );

        if ( 1 == names_of_incorrect_parameters.size() ) {
            what_happened += " is incorrect (no such parameter)!";
        } else {
            boost::replace_first( what_happened, "Parameter", "Parameters" );
            what_happened += " are incorrect (no such parameters)!";
        }

        throw std::runtime_error( what_happened );
    }
};

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_INCORRECT_PARAMETERS_HPP
