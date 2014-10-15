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

#ifndef CLPP_DETAIL_PARAMETERS_REDUNDANCY_HPP
#define CLPP_DETAIL_PARAMETERS_REDUNDANCY_HPP

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

/// \class parameters_redundancy_checker
/// \brief Parameter's redundancy checker.
///
/// Check redundancy of inputed parameters.
/// Assumed that each of the parameters can be inputed only once,
/// so quantity of inputed parameters cannot be greater than
/// quantity of registered parameters.
class parameters_redundancy_checker : public checker {
public:
    parameters_redundancy_checker( const parameters&                    registered_parameters
                                   , const parameter_parts_extractor&   extractor
                                   , const std::string&                 name_value_separator ) :
            checker( registered_parameters, extractor, name_value_separator ) {}
public:
    void check( const str_storage& inputed_parameters ) const {
        const size_t inputed_parameters_quantity = inputed_parameters.size();
        const size_t registered_parameters_quantity = registered_parameters.size();
        if ( inputed_parameters_quantity > registered_parameters_quantity ) {
            notify_about_parameters_redundancy( inputed_parameters_quantity, registered_parameters_quantity );
        } else {}
    }
private:
    void notify_about_parameters_redundancy( size_t   inputed_parameters_quantity
                                             , size_t registered_parameters_quantity ) const {
        o_stream what_happened;
        what_happened << lib_prefix()
                      << "You inputs " << inputed_parameters_quantity << " parameters, "
                      << "but only " << registered_parameters_quantity << " registered!";
        throw std::runtime_error( what_happened.str() );
    }
};

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_PARAMETERS_REDUNDANCY_HPP
