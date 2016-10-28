//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/DateTime.h"
#include "smtk/attribute/TimeZone.h"

#ifndef SHIBOKEN_SKIP
SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/date_time/gregorian/gregorian.hpp"
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
SMTK_THIRDPARTY_POST_INCLUDE
#endif

#include <exception>

namespace smtk {
  namespace attribute {

//----------------------------------------------------------------------------
/// Default constructor creates invalid ptime
DateTime::DateTime()
  : m_ptime()
{
}

//----------------------------------------------------------------------------
bool DateTime::setComponents(
  int yr, int month, int day,
  int hr, int min, int sec, int msec,
  TimeZone *timeZone)
{
  // Construct date & time_duration components
  boost::gregorian::date ptimeDate(yr, month, day);
  // Cannot use main time_duration constructor because fractional_seconds
  // argument is ambiguous. Use milliseconds constructor instead.
  int msecTotal = 1000*(hr*3600 + min*60 + sec) + msec;
  boost::posix_time::time_duration ptimeTime =
    boost::posix_time::milliseconds(msecTotal);

  // Convert depending on timeZone input
  if (timeZone)
    {
    // If time zone specified, convert to UTC
    boost::local_time::local_date_time local(
      ptimeDate,
      ptimeTime,
      timeZone->boostPointer(),
      boost::local_time::local_date_time::NOT_DATE_TIME_ON_ERROR);
    this->m_ptime = local.utc_time();
    // std::cout << "local: " << local
    //           << "\nm_ptime: " << this->m_ptime << std::endl;
    }
  else
    {
    // If no tz, use as is
    this->m_ptime = boost::posix_time::ptime(ptimeDate, ptimeTime);
    }

  return this->isSet();
}

//----------------------------------------------------------------------------
bool DateTime::components(
  int& yr, int& month, int& day,
  int& hr, int& min, int& sec, int& msec,
  TimeZone *timeZone) const
{
  if (!this->isSet())
    {
    return false;
    }

  if (timeZone)
    {
    // Convert to local time
    boost::local_time::local_date_time local(
      this->m_ptime, timeZone->boostPointer());

    // Convert date
    boost::gregorian::date localDate = local.date();
    yr = localDate.year();
    month = localDate.month();
    day = localDate.day();

    // Convert time
    boost::posix_time::ptime ptimeLocal = local.local_time();
    boost::posix_time::time_duration workingTime = ptimeLocal.time_of_day();
    std::cout << "m_data " << this->m_ptime << std::endl;
    std::cout << "workingTime " << workingTime << std::endl;
    hr = workingTime.hours();
    min = workingTime.minutes();
    sec = workingTime.seconds();
    msec = workingTime.total_milliseconds() - 1000*workingTime.total_seconds();

    return true;
    }

  // else use ptime
  boost::gregorian::date ptimeDate = this->m_ptime.date();
  yr = ptimeDate.year();
  month = ptimeDate.month();
  day = ptimeDate.day();

  boost::posix_time::time_duration ptimeTime = this->m_ptime.time_of_day();
  std::cout << "m_data " << this->m_ptime << std::endl;
  std::cout << "ptimeTime " << ptimeTime << std::endl;
  hr = ptimeTime.hours();
  min = ptimeTime.minutes();
  sec = ptimeTime.seconds();
  msec = ptimeTime.total_milliseconds() - 1000*ptimeTime.total_seconds();

  return true;
}

//----------------------------------------------------------------------------
bool DateTime::isSet() const
{
  return !this->m_ptime.is_special();
}

//----------------------------------------------------------------------------
// Parse input string in canonical format only: YYYYMMDDThhmmss[.zzzzzz]
bool DateTime::parse(const std::string& ts)
{
  try
    {
    this->m_ptime = boost::posix_time::from_iso_string(ts);
    }
  catch (std::exception& e)
    {
#ifndef NDEBUG
    std::cerr << "exception: " << e.what() << std::endl;
#endif
    this->m_ptime = boost::posix_time::not_a_date_time;
    return false;
    }

  return this->isSet();
}

//----------------------------------------------------------------------------
/// Parse string using boost time_from_string(), NOT ISO compliant
bool DateTime::parseBoostFormat(const std::string& ts)
{
  try
    {
    this->m_ptime = boost::posix_time::time_from_string(ts);
    }
  catch (std::exception& e)
    {
#ifndef NDEBUG
    std::cerr << "exception: " << e.what() << std::endl;
#endif
    this->m_ptime = boost::posix_time::not_a_date_time;
    return false;
    }

  return this->isSet();
}

  }  // namespace attribute
}  // namespace smtk
