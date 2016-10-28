//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/TimeZone.h"
#include "smtk/attribute/timezonespec.h"

#include <boost/date_time/local_time/local_time.hpp>

#include <sstream>

namespace smtk {
  namespace attribute {

boost::local_time::tz_database TimeZone::s_database;
bool TimeZone::s_databaseLoaded = false;

//----------------------------------------------------------------------------
TimeZone::TimeZone()
  : m_data(0)
{
}

//----------------------------------------------------------------------------
bool TimeZone::isSet() const
{
  return !!m_data;
}

//----------------------------------------------------------------------------
bool TimeZone::setRegion(const std::string& region)
{
  // Load database if needed
  if (!TimeZone::s_databaseLoaded)
    {
    std::string tzSpec(timezonespec_csv);
    std::istringstream ss(tzSpec);

    // Must discard first line (headers) before loading from stream
    std::string buff;
    std::getline(ss, buff);
    TimeZone::s_database.load_from_stream(ss);
    TimeZone::s_databaseLoaded = true;
    }

  this->m_data = s_database.time_zone_from_region(region);
  return this->isSet();
}

//----------------------------------------------------------------------------
bool TimeZone::setPosix(const std::string& posixTimeZoneString)
{
  //boost::local_time::time_zone_ptr tz(
  boost::local_time::time_zone_ptr tz(
    new boost::local_time::posix_time_zone(posixTimeZoneString));
  this->m_data = tz;
}

//----------------------------------------------------------------------------
const boost::local_time::time_zone_ptr TimeZone::boostPointer() const
{
  return this->m_data;
}

  } // namespace attribute
} // namespace smtk

