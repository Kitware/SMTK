//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_common_DateTimeZonePair_h
#define __smtk_common_DateTimeZonePair_h

#include "smtk/common/DateTime.h"
#include "smtk/common/TimeZone.h"
#include <iostream>
#include <string>

namespace smtk {
  namespace common {

//.NAME DateTimeZonePair - simple container for DateTime & TimeZone objects
class SMTKCORE_EXPORT DateTimeZonePair
{
 public:
  DateTimeZonePair();

  DateTime dateTime() const;
  TimeZone timeZone() const;

  void setDateTime(const DateTime& dt);
  void setTimeZone(const TimeZone& tz);

  std::string serialize() const;
  bool deserialize(const std::string& content);
  std::string jsonString() const;

  // Relational operators
  bool operator==(const DateTimeZonePair& dt) const;
  bool operator<(const DateTimeZonePair& dt) const;
  bool operator>(const DateTimeZonePair& dt) const;

 protected:
  DateTime m_datetime;
  TimeZone m_timezone;
};

SMTKCORE_EXPORT std::ostream& operator<<(std::ostream& os, const DateTimeZonePair& dtz);
SMTKCORE_EXPORT std::istream& operator>>(std::istream& is, DateTimeZonePair& dtz);

  } // namespace common
} // namespace smtk

#endif  // __smtk_common_DateTimeZonePair_h
