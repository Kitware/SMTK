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

#ifndef CLPP_DETAIL_PARAMETERS_EXISTENCE_HPP
#define CLPP_DETAIL_PARAMETERS_EXISTENCE_HPP

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

/// \class parameters_existence_checker
/// \brief Parameter's existence checker.
///
/// Checks existence of registered parameters.
class parameters_existence_checker : public checker {
public:
    parameters_existence_checker( const parameters&                     registered_parameters
                                  , const parameter_parts_extractor&    extractor
                                  , const std::string&                  name_value_separator ) :
            checker( registered_parameters, extractor, name_value_separator ) {}
public:
    void check( const str_storage& inputed_parameters ) const {
        if ( registered_parameters.empty() && !inputed_parameters.empty() ) {
            notify_about_parameters_absence();
        } else {}
    }
private:
    void notify_about_parameters_absence() const {
        const std::string what_happened = lib_prefix() + "You inputs some parameter(s), but no one registered!";
        throw std::runtime_error( what_happened );
    }
};

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_PARAMETERS_EXISTENCE_HPP
