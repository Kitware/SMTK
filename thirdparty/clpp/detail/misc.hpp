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

#ifndef CLPP_DETAIL_MISC_HPP
#define CLPP_DETAIL_MISC_HPP

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

inline std::string lib_prefix() { return "[CLPP] "; }

/// \enum value_semantic
/// \brief Value semantic types (for check).
enum value_semantic {
    no_semantic     /*!< Default value. */
  , path        /*!< Path semantic check. */
  , ipv4        /*!< IPv4 semantic check. */
    , ipv6              /*!< IPv6 semantic check. */
    , ip                /*!< IP semantic check. */
    , email             /*!< E-mail semantic check. */
};

/// \namespace clpp::detail
/// \brief Details of realization.
namespace detail {

template< typename T >
inline std::string to_str( const T& t ) {
    std::string as_string;
    try {
        as_string = boost::lexical_cast< std::string >( t );
    } catch ( const std::exception& /* exc */ ) {
        std::string what_happened = lib_prefix() + "Value cannot be present as std::string!";
        throw std::invalid_argument( what_happened );
    }
    return as_string;
}

template
<
    typename STLContainer
    , typename Element
>
inline void delete_element( STLContainer& cont, const Element& element ) {
    cont.erase( std::remove( cont.begin(), cont.end(), element )
        , cont.end() );
}

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_MISC_HPP
