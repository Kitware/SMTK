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
std::string DateTimeZonePair::serialize() const
{
  // Generate json output string
  cJSON *outputJson = cJSON_CreateObject();

  // Add "datetime" value (string)
  std::string dtString = m_datetime.serialize();
  std::cout << "datetime " << dtString << std::endl;
  cJSON *dtJson = cJSON_CreateString(dtString.c_str());
  cJSON_AddItemToObject(outputJson, "datetime", dtJson);

  // Add optional "timezone" value (object)
  if (m_timezone.isSet())
    {
    std::cout << "timezone region \"" << m_timezone.region() << "\""
              << ", posix: " << m_timezone.posixString() << std::endl;

    // Region string takes precedence over posix
    std::string regionString = m_timezone.region();
    if (!regionString.empty())
      {
      cJSON *regionJson = cJSON_CreateString(regionString.c_str());
      cJSON_AddItemToObject(outputJson, "timezone-region", regionJson);
      }
    else
      {
      cJSON *posixJson = cJSON_CreateString(m_timezone.posixString().c_str());
      cJSON_AddItemToObject(outputJson, "timezone-posix", posixJson);
      }

    }

  // Convert to string
  std::string outputString = cJSON_PrintUnformatted(outputJson);

  cJSON_Delete(outputJson);
  return outputString;
}

//----------------------------------------------------------------------------
bool DateTimeZonePair::deserialize(const std::string& content)
{
  cJSON *inputJson = cJSON_Parse(content.c_str());
  if (!inputJson || inputJson->type != cJSON_Object)
    {
    cJSON_Delete(inputJson);
    std::cerr << "Missing or invalid DateTimeZonePair object: "
              << content << std::endl;
    return false;
    }

  // Extract datetime string
  cJSON *dtJson = cJSON_GetObjectItem(inputJson, "datetime");
  if (!dtJson || dtJson->type != cJSON_String)
    {
    std::cerr << "Missing or invalid DateTime string: "
              << content << std::endl;
    cJSON_Delete(inputJson);
    return false;
    }

  std::string dtString = dtJson->valuestring;
  if (!dtString.empty())
    {
    m_datetime.deserialize(dtString);
    }

  // Extract optional timezone objects
  cJSON *regionJson = cJSON_GetObjectItem(inputJson, "timezone-region");
  cJSON *ptzJson = cJSON_GetObjectItem(inputJson, "timezone-posix");

  // First check for "region" item
  if (regionJson && regionJson->type == cJSON_String)
    {
    std::string regionString = regionJson->valuestring;
    m_timezone.setRegion(regionString);
    }
  else if (ptzJson && ptzJson->type == cJSON_String)
    {
    std::string ptzString = ptzJson->valuestring;
    m_timezone.setPosixString(ptzString);
    }

  cJSON_Delete(inputJson);
  return true;
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
std::ostream& operator<<(std::ostream& os, const DateTimeZonePair& dtz)
{
  // Convert to string and write out
  std::string dtzString = dtz.serialize();
  os << dtzString;
  return os;
}

//----------------------------------------------------------------------------
std::istream& operator>>(std::istream& is, DateTimeZonePair& dtz)
{
  // Todo reset the input dtz

  // Input string uses json format
  std::string inputText;
  is >> inputText;
  dtz.deserialize(inputText);
  return is;
}

//----------------------------------------------------------------------------

  } // namespace attribute
} // namespace smtk
