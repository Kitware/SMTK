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

#ifndef CLPP_DETAIL_NECESSARY_PARAMETERS_HPP
#define CLPP_DETAIL_NECESSARY_PARAMETERS_HPP

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

/// \class necessary_parameters_checker
/// \brief Necessary parameters existence checker.
///
/// Checks existence of parameters that registered as necessary.
class necessary_parameters_checker : public checker {
public:
    necessary_parameters_checker( const parameters&                    registered_parameters
                                  , const parameter_parts_extractor&   extractor
                                  , const std::string&                 name_value_separator ) :
            checker( registered_parameters, extractor, name_value_separator ) {}
public:
    void check( const str_storage& inputed_parameters ) const {
        str_storage names_that_should_be = collect_names_of_necessary_parameters();
        remove_names_of_inputed_necessary_parameters( inputed_parameters, names_that_should_be );
        if ( !names_that_should_be.empty() ) {
            notify_about_missing_of_necessary_parameters( names_that_should_be );
        } else {}
    }
private:
    str_storage collect_names_of_necessary_parameters() const {
        str_storage names;
      BOOST_FOREACH ( const parameter& registered_parameter, registered_parameters ) {
            if ( registered_parameter.is_necessary ) {
              names += registered_parameter.short_name;
            } else {}
        }
        return names;
    }

    void remove_names_of_inputed_necessary_parameters( const str_storage& inputed_parameters
                                                       , str_storage&     names_that_should_be ) const {
        BOOST_FOREACH ( const std::string& inputed_parameter, inputed_parameters ) {
          const std::string name = extractor.extract_name_from( inputed_parameter );
            detail::delete_element( names_that_should_be, name );
        }
    }

    void notify_about_missing_of_necessary_parameters( const str_storage& names_that_should_be ) const {
        std::string what_happened = lib_prefix() + "Parameter ";

        BOOST_FOREACH ( const std::string& name, names_that_should_be ) {
          what_happened += "'" + name + "', ";
        }
        boost::erase_last( what_happened, ", " );

        if ( 1 == names_that_should_be.size() ) {
            what_happened += " is defined as necessary, but it missed!";
        } else {
            boost::replace_first( what_happened, "Parameter", "Parameters" );
            what_happened += " are defined as necessary, but they missed!";
        }

        throw std::runtime_error( what_happened );
    }
};

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_NECESSARY_PARAMETERS_HPP
