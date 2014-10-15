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

#ifndef CLPP_DETAIL_TYPES_HPP
#define CLPP_DETAIL_TYPES_HPP

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <boost/any.hpp>
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

#include <vector>
#include <sstream>

/// \namespace clpp
/// \brief Main namespace of library.
namespace clpp {

/// \namespace clpp::detail
/// \brief Details of realization.
namespace detail {

typedef std::vector< std::string >  str_storage;
typedef str_storage::iterator       str_it;
typedef str_storage::const_iterator str_const_it;

typedef std::ostringstream          o_stream;

typedef boost::any                  any;

} // namespace detail
} // namespace clpp

#endif // CLPP_DETAIL_TYPES_HPP
