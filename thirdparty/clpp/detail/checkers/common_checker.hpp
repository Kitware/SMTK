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

#ifndef CLPP_DETAIL_COMMON_CHECKER_HPP
#define CLPP_DETAIL_COMMON_CHECKER_HPP

#include "../types.hpp"

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

/// \namespace clpp::detail
/// \brief Details of realization.
namespace detail {

/// \struct common_checker
/// \brief Superclass for all checkers.
///
/// Need for polymorphic use of checkers. Parameterized for "file independence".
template
<
    typename Parameters
    , typename Extractor
    , typename Separator
>
struct common_checker {
    common_checker( const Parameters&  _registered_parameters
                    , const Extractor& _extractor
                    , const Separator& _name_value_separator ) :
            registered_parameters( _registered_parameters )
            , extractor( _extractor )
            , name_value_separator( _name_value_separator ) {}
public:
    const Parameters& registered_parameters;
    const Extractor&  extractor;
    const Separator&  name_value_separator;
public:
    virtual void check( const str_storage& inputed_parameters ) const = 0;
};

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_COMMON_CHECKER_HPP
