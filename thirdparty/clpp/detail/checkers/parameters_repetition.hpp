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

#ifndef CLPP_DETAIL_PARAMETERS_REPETITION_HPP
#define CLPP_DETAIL_PARAMETERS_REPETITION_HPP

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

/// \class parameters_repetition_checker
/// \brief Parameters repetition checker.
///
/// Checks repetition of parameter(s).
/// Assumed that each of the parameters can be inputed only once.
class parameters_repetition_checker : public checker {
public:
    parameters_repetition_checker( const parameters&                    registered_parameters
                                   , const parameter_parts_extractor&   extractor
                                   , const std::string&                 name_value_separator ) :
            checker( registered_parameters, extractor, name_value_separator ) {}
public:
    void check( const str_storage& inputed_parameters ) const {
        if ( inputed_parameters.size() < 2 ) {
            return;
        } else {}

        str_storage sorted_parameters = get_sorted_inputed_parameters( inputed_parameters );
        str_storage unique_parameters = get_unique_parameters( sorted_parameters, inputed_parameters );

        if ( sorted_parameters != unique_parameters ) {
            notify_about_parameters_repetition();
        } else {}
    }
private:
    str_storage get_sorted_inputed_parameters( const str_storage& inputed_parameters ) const {
        str_storage parameters_for_sorting( inputed_parameters.begin(), inputed_parameters.end() );
        std::sort( parameters_for_sorting.begin(), parameters_for_sorting.end() );
        return parameters_for_sorting;
    }

    str_storage get_unique_parameters( const str_storage& sorted
                                       , const str_storage& inputed_parameters ) const {
        str_storage unique_parameters( inputed_parameters.size() );
        str_const_it it = std::unique_copy( sorted.begin(), sorted.end(), unique_parameters.begin() );
        unique_parameters.resize( it - unique_parameters.begin() );
        return unique_parameters;
    }

    void notify_about_parameters_repetition() const {
        const std::string what_happened = lib_prefix() + "Parameter(s) repetition detected!";
        throw std::runtime_error( what_happened );
    }
};

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_PARAMETERS_REPETITION_HPP
