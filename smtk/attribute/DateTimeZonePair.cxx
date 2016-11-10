//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/DateTimeZonePair.h"
#include "cJSON.h"

namespace smtk {
  namespace attribute {

//----------------------------------------------------------------------------
DateTimeZonePair::DateTimeZonePair() : m_datetime(), m_timezone()
{
}

//----------------------------------------------------------------------------
DateTime DateTimeZonePair::dateTime() const
{
  return m_datetime;
}

//----------------------------------------------------------------------------
TimeZone DateTimeZonePair::timeZone() const
{
  return m_timezone;
}

//----------------------------------------------------------------------------
void DateTimeZonePair::setDateTime(const DateTime& dt)
{
  m_datetime = dt;
}

//----------------------------------------------------------------------------
void DateTimeZonePair::setTimeZone(const TimeZone& tz)
{
  m_timezone = tz;
}

//----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const DateTimeZonePair& dtz)
{
  // Generate json output string
  cJSON *outputJson = cJSON_CreateObject();

  // Add "datetime" value (string)
  std::string dtString = dtz.dateTime().serialize();
  std::cout << "datetime " << dtString << std::endl;
  cJSON *dtJson = cJSON_CreateString(dtString.c_str());
  cJSON_AddItemToObject(outputJson, "datetime", dtJson);

  // Add optional "timezone" value (object)
  TimeZone tz = dtz.timeZone();
  if (tz.isSet())
    {
    std::cout << "timezone region \"" << tz.region() << "\""
              << ", posix: " << tz.posixString() << std::endl;

    // Region string takes precedence over posix
    std::string regionString = tz.region();
    if (!regionString.empty())
      {
      cJSON *regionJson = cJSON_CreateString(regionString.c_str());
      cJSON_AddItemToObject(outputJson, "timezone-region", regionJson);
      }
    else
      {
      cJSON *posixJson = cJSON_CreateString(tz.posixString().c_str());
      cJSON_AddItemToObject(outputJson, "timezone-posix", posixJson);
      }

    }

  // Convert to string and write out
  os << cJSON_PrintUnformatted(outputJson);

  cJSON_Delete(outputJson);
  return os;
}

//----------------------------------------------------------------------------
std::istream& operator>>(std::istream& is, DateTimeZonePair& dtz)
{
  // Todo reset the input dtz

  // Input string uses json format
  std::string inputText;
  is >> inputText;
  cJSON *inputJson = cJSON_Parse(inputText.c_str());
  if (!inputJson || inputJson->type != cJSON_Object)
    {
    cJSON_Delete(inputJson);
    std::cerr << "Missing or invalid DateTimeZonePair object: "
              << inputText << std::endl;
    return is;
    }

  // Extract datetime string
  cJSON *dtJson = cJSON_GetObjectItem(inputJson, "datetime");
  if (!dtJson || dtJson->type != cJSON_String)
    {
    std::cerr << "Missing or invalid DateTime string: "
              << inputText << std::endl;
    cJSON_Delete(inputJson);
    return is;
    }

  std::string dtString = dtJson->valuestring;
  if (!dtString.empty())
    {
    DateTime dt = dtz.dateTime();
    dt.deserialize(dtString);
    dtz.setDateTime(dt);
    }

  // Extract optional timezone objects
  cJSON *regionJson = cJSON_GetObjectItem(inputJson, "timezone-region");
  cJSON *ptzJson = cJSON_GetObjectItem(inputJson, "timezone-posix");
  TimeZone tz = dtz.timeZone();

  // First check for "region" item
  if (regionJson && regionJson->type == cJSON_String)
    {
    std::string regionString = regionJson->valuestring;
    tz.setRegion(regionString);
    dtz.setTimeZone(tz);
    }
  else if (ptzJson && ptzJson->type == cJSON_String)
    {
    std::string ptzString = ptzJson->valuestring;
    tz.setPosixString(ptzString);
    dtz.setTimeZone(tz);
    }

  cJSON_Delete(inputJson);
  return is;
}

//----------------------------------------------------------------------------
bool DateTimeZonePair::operator==(const DateTimeZonePair& dtz) const
{
  return this->m_datetime == dtz.m_datetime;
}

//----------------------------------------------------------------------------
bool DateTimeZonePair::operator<(const DateTimeZonePair& dtz) const
{
  return this->m_datetime < dtz.m_datetime;
}

//----------------------------------------------------------------------------
bool DateTimeZonePair::operator>(const DateTimeZonePair& dtz) const
{
  return this->m_datetime > dtz.m_datetime;
}

//----------------------------------------------------------------------------

  } // namespace attribute
} // namespace smtk
