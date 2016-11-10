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
#include <boost/date_time/posix_time/posix_time.hpp>

#include <sstream>

namespace smtk {
  namespace attribute {

boost::local_time::tz_database TimeZone::s_database;
bool TimeZone::s_databaseLoaded = false;

//----------------------------------------------------------------------------
TimeZone::TimeZone()
  : m_boostTimeZone(0)
{
}

//----------------------------------------------------------------------------
bool TimeZone::isSet() const
{
  return !!m_boostTimeZone;
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

  this->m_boostTimeZone = s_database.time_zone_from_region(region);
  if (!this->isSet())
    {
    this->m_region.clear();
    return false;
    }

  // else
  this->m_region = region;
  return true;
}

//----------------------------------------------------------------------------
std::string TimeZone::region() const
{
  return this->m_region;
}

//----------------------------------------------------------------------------
bool TimeZone::setPosixString(const std::string& posixTimeZoneString)
{
  try
    {
    boost::local_time::time_zone_ptr tz(
      new boost::local_time::posix_time_zone(posixTimeZoneString));
    this->m_boostTimeZone = tz;
    }
  catch (std::exception& e)
    {
#ifndef NDEBUG
    std::cerr << "exception: " << e.what() << std::endl;
#endif
    boost::local_time::time_zone_ptr tzNull(
      new boost::local_time::posix_time_zone(""));
    this->m_boostTimeZone = tzNull;
    return false;
    }

  this->m_region.clear();
  return this->isSet();
}

//----------------------------------------------------------------------------
std::string TimeZone::posixString() const
{
  return this->m_boostTimeZone->to_posix_string();
}

//----------------------------------------------------------------------------
std::string TimeZone::stdZoneName() const
{
  if (this->m_boostTimeZone)
    {
    return this->m_boostTimeZone->std_zone_name();
    }
  // else
  return std::string();
}


//----------------------------------------------------------------------------
std::string TimeZone::stdZoneAbbreviation() const
{
  if (this->m_boostTimeZone)
    {
    return this->m_boostTimeZone->std_zone_abbrev();
    }
  // else
  return std::string();
}

//----------------------------------------------------------------------------
std::string TimeZone::dstZoneName() const
{
  if (this->m_boostTimeZone)
    {
    return this->m_boostTimeZone->dst_zone_name();
    }
  // else
  return std::string();
}

//----------------------------------------------------------------------------
std::string TimeZone::dstZoneAbbreviation() const
{
  if (this->m_boostTimeZone)
    {
    return this->m_boostTimeZone->dst_zone_abbrev();
    }
  // else
  return std::string();
}

//----------------------------------------------------------------------------
bool TimeZone::hasDST() const
{
  if (this->m_boostTimeZone)
    {
    return this->m_boostTimeZone->has_dst();
    }
  // else
  return false;
}

//----------------------------------------------------------------------------
bool TimeZone::utcOffset(int& hours, int& minutes) const
{
  if (this->m_boostTimeZone)
    {
    boost::posix_time::time_duration delta =
      this->m_boostTimeZone->base_utc_offset();
    hours = delta.hours();
    minutes = delta.minutes();
    return true;
    }
  // else
  return false;
}

//----------------------------------------------------------------------------
bool TimeZone::dstShift(int& hours, int& minutes) const
{
  if (this->m_boostTimeZone)
    {
    boost::posix_time::time_duration delta =
      this->m_boostTimeZone->dst_offset();
    hours = delta.hours();
    minutes = delta.minutes();
    return true;
    }
  // else
  return false;
}

//----------------------------------------------------------------------------
const boost::local_time::time_zone_ptr TimeZone::boostPointer() const
{
  return this->m_boostTimeZone;
}

//----------------------------------------------------------------------------

  } // namespace attribute
} // namespace smtk

