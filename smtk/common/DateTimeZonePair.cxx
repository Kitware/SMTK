//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/DateTimeZonePair.h"
#include "nlohmann/json.hpp"
#include <cstdio> // for snprintf()
#include <exception>

#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf(buf, cnt, fmt, ...) _snprintf_s(buf, cnt, cnt, fmt, __VA_ARGS__)
#endif

namespace smtk
{
namespace common
{

DateTimeZonePair::DateTimeZonePair() = default;

DateTime DateTimeZonePair::dateTime() const
{
  return m_datetime;
}

TimeZone DateTimeZonePair::timeZone() const
{
  return m_timezone;
}

void DateTimeZonePair::setDateTime(const DateTime& dt)
{
  m_datetime = dt;
}

void DateTimeZonePair::setTimeZone(const TimeZone& tz)
{
  m_timezone = tz;
}

std::string DateTimeZonePair::serialize() const
{
  // Generate json output string
  nlohmann::json outputJson;

  // Add "datetime" value (string)
  outputJson["datetime"] = m_datetime.serialize();

  // Add optional "timezone" value (object)
  if (m_timezone.isSet())
  {
    // Check UTC then region then posix string
    std::string regionString = m_timezone.region();
    if (m_timezone.isUTC())
    {
      outputJson["timezone-utc"] = true;
    }
    else if (!regionString.empty())
    {
      outputJson["timezone-region"] = regionString;
    }
    else
    {
      outputJson["timezone-posix"] = m_timezone.posixString();
    }
  } // if (timezone is set)

  // Convert to string
  return outputJson.dump();
}

bool DateTimeZonePair::deserialize(const std::string& content)
{
  nlohmann::json inputJson;
  try
  {
    inputJson = nlohmann::json::parse(content);
  }
  catch (...)
  {
    std::cerr << "Not valid json for DateTimeZonePair object: " << content << std::endl;
    std::cerr << "  Follow the form: "
                 "{\"datetime\":\"20220620T000000\",\"timezone-region\":\"America/New_York\"}"
              << std::endl;
    return false;
  }
  if (inputJson.type() != nlohmann::json::value_t::object)
  {
    std::cerr << "Missing or invalid DateTimeZonePair object: " << content << std::endl;
    return false;
  }

  // Extract datetime string
  auto dtJson = inputJson.find("datetime");
  if (dtJson == inputJson.end())
  {
    std::cerr << "Missing or invalid DateTime string: " << content << std::endl;
    return false;
  }

  auto dtString = dtJson->get_ref<const nlohmann::json::string_t&>();
  if (!dtString.empty())
  {
    m_datetime.deserialize(dtString);
  }

  // Extract optional timezone objects
  auto utcJson = inputJson.find("timezone-utc");
  auto regionJson = inputJson.find("timezone-region");
  auto ptzJson = inputJson.find("timezone-posix");
  if (
    utcJson != inputJson.end() && utcJson->type() == nlohmann::json::value_t::boolean &&
    utcJson->get_ref<const nlohmann::json::boolean_t&>())
  {
    m_timezone.setUTC();
  }
  else if (regionJson != inputJson.end() && regionJson->type() == nlohmann::json::value_t::string)
  {
    m_timezone.setRegion(regionJson->get_ref<const nlohmann::json::string_t&>());
  }
  else if (ptzJson != inputJson.end() && ptzJson->type() == nlohmann::json::value_t::string)
  {
    m_timezone.setPosixString(ptzJson->get_ref<const nlohmann::json::string_t&>());
  }

  return true;
}

std::string DateTimeZonePair::jsonString() const
{
  // Create json string, equivalent to javascript Date.toJson() output.
  // Format is  yyyy-mm-ddThh:mm:ss.mmmZ, e.g. 2016-03-31T13:44:30.095Z.
  int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0, millisecond = 0;
  bool ok = m_datetime.components(m_timezone, year, month, day, hour, minute, second, millisecond);
  if (!ok)
  {
    // If invalid or not set, return empty string
    return std::string();
  }

  char buffer[64];
  snprintf(
    buffer,
    sizeof(buffer),
    "%4d-%02d-%02dT%02d:%02d:%02d.%03dZ",
    year,
    month,
    day,
    hour,
    minute,
    second,
    millisecond);

  std::string json = buffer;
  return json;
}

bool DateTimeZonePair::operator==(const DateTimeZonePair& dtz) const
{
  return m_datetime == dtz.m_datetime;
}

bool DateTimeZonePair::operator<(const DateTimeZonePair& dtz) const
{
  return m_datetime < dtz.m_datetime;
}

bool DateTimeZonePair::operator>(const DateTimeZonePair& dtz) const
{
  return m_datetime > dtz.m_datetime;
}

std::ostream& operator<<(std::ostream& os, const DateTimeZonePair& dtz)
{
  // Convert to string and write out
  std::string dtzString = dtz.serialize();
  os << dtzString;
  return os;
}

std::istream& operator>>(std::istream& is, DateTimeZonePair& dtz)
{
  // Todo reset the input dtz

  // Input string uses json format
  std::string inputText;
  is >> inputText;
  dtz.deserialize(inputText);
  return is;
}

} // namespace common
} // namespace smtk
