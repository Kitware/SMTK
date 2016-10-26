//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_DateTime_h
#define __smtk_attribute_DateTime_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"
#include "smtk/common/CompilerInformation.h"

#ifndef SHIBOKEN_SKIP
SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/date_time/posix_time/ptime.hpp>
SMTK_THIRDPARTY_POST_INCLUDE
#endif

#include <iostream>
#include <string>

namespace smtk {
  namespace attribute {

//.NAME DateTime - A DateTime represention generally based on ISO 8601
//.SECTION Description
// This class uses Boost's DateTime class to implement smtk::attribute::DateTime.
class SMTKCORE_EXPORT DateTime
{
public:
  DateTime();

  bool isValid() const;
  bool parseIsoString(const std::string& ts);
  bool parseString(const std::string& ts);

protected:
  // Implemented using Boost's DateTime library.
  boost::posix_time::ptime m_data;

};

  } // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_DateTime_h
