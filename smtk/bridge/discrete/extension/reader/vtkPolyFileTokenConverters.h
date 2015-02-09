//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtkdiscrete_vtkPolyFileTokenConverters_h
#define __smtkdiscrete_vtkPolyFileTokenConverters_h

#include "vtkObjectBase.h"
#include <iostream>
#include <sstream>
#include <string>

#if defined(_MSC_VER)
#define strtoll _strtoi64
#endif

namespace smtk {
  namespace bridge {
    namespace discrete {

class DoubleConverter
{
public:
  typedef double type;
  static double bad_value()
    {
    return std::numeric_limits<double>::quiet_NaN();
    }
  static double convert(const std::string& token, bool& ok)
    {
    const char* ptr = token.c_str();
    char* endptr = NULL;
    double v = strtod(ptr, &endptr);
    if (ptr == endptr)
      {
      ok = false;
      return bad_value();
      }
    ok = true;
    return v;
    }
};

class Int32Converter
{
public:
  typedef vtkTypeInt32 type;
  static vtkTypeInt32 bad_value()
    {
    return std::numeric_limits<vtkTypeInt32>::min();
    }
  static vtkTypeInt32 convert(const std::string& token, bool& ok)
    {
    const char* ptr = token.c_str();
    char* endptr = NULL;
    vtkTypeInt32 v = strtoll(ptr, &endptr, 10);
    if (ptr == endptr)
      {
      ok = false;
      return bad_value();
      }
    ok = true;
    return v;
    }
};
    } // namespace discrete
  } // namespace bridge
} // namespace smtk

#endif // __vtkPolyFileTokenConverters_h
