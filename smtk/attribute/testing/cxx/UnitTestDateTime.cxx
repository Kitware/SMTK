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

#include "smtk/common/testing/cxx/helpers.h"

//----------------------------------------------------------------------------
void verifyTimeZones()
{
  smtk::attribute::TimeZone tz;
  test(!tz.isSet(), "Failed to recognize unset TimeZone");
  test(tz.setRegion("America/New_York"), "Failed to set TimeZone region");
  test(!tz.setRegion("Invalid"), "Failed to recognize invalid TimeZone region");
}

//----------------------------------------------------------------------------
void verify_constructors()
{
  smtk::attribute::DateTime dtEmpty;
  test(!dtEmpty.isSet(), "Failed to recognize invalid state");
}

//----------------------------------------------------------------------------
void verify_parsers()
{
  // Using DateTime::parse():
  smtk::attribute::DateTime dtInvalid;
  dtInvalid.parse("");
  test(!dtInvalid.isSet(), "Failed to detect empty (invalid) iso string");
  test(!dtInvalid.parse("20160231"), "failed to detect invalid date");

  smtk::attribute::DateTime dtValid;
  const char *validCases[] = {
    "20161026T141559", "Failed to parse iso string",
    "20161026T141559.123456+0200", "Failed to parse iso string with tz offset",
    "20161026T141559Z",  "Failed to parse iso string with Z suffix"
  };
  for (std::size_t i = 0; i < sizeof(validCases)/sizeof(const char *); i += 2)
    {
    test(!!dtValid.parse(validCases[i]), validCases[i+1]);
    test(!!dtValid.isSet(), "Failed to return true for isSet()");
    }

  // Using DateTime::parseBoostFormat():
  smtk::attribute::DateTime dt;
  dt.parseBoostFormat("2002-01-20 23:59:59.000");
  test(!!dt.isSet(), "Failed to parse basic string");
  dt.parseBoostFormat("2002-01-20 23:59:59.000-02:00");
  test(!!dt.isSet(), "Failed to parse string with offset");

  // Unfortunately, boost doesn't correctly parse date-only strings
  dt.parseBoostFormat("2002-01-20");
  test(!dt.isSet(), "Failed to show that parsing date only dont work");
}

//----------------------------------------------------------------------------
void verify_setGets()
{
  // Initialize
  smtk::attribute::DateTime dt;
  dt.parse("20161028T145400.123456");

  int yr, month, day, hr, minute, sec, msec;

  // Read back
  test(
    dt.getComponents(yr, month, day, hr, minute, sec, msec),
    "Failed to get components");
  test(yr     == 2016, "Failed to get year component");
  test(month  == 10, "Failed to get month component");
  test(day    == 28, "Failed to get day component");
  test(hr     == 14, "Failed to get hour component");
  test(minute == 54, "Failed to get minute component");
  test(sec    == 0, "Failed to get second component");
  test(msec   == 123, "Failed to get millisecond component");

  // Set new datetime and read back
  int yr2 = 2017;
  int month2 = 2;
  int day2 = 15;
  int hr2 = 8;
  int minute2 = 15;
  int sec2 = 0;
  int msec2 = 456;
  test(
    dt.setComponents(yr2, month2, day2, hr2, minute2, sec2, msec2),
    "Failed to set components");
  dt.getComponents(yr, month, day, hr, minute, sec, msec),
  test(yr == yr2, "Failed to get year component");
  test(month == month2, "Failed to get month component");
  test(day == day2, "Failed to get day component");
  test(hr == hr2, "Failed to get hour component");
  test(minute == minute2, "Failed to get minute component");
  test(sec == sec2, "Failed to get second component");
  test(msec == msec2, "Failed to get millisecond component");
}

//----------------------------------------------------------------------------
int UnitTestDateTime(int, char** const)
{
  verifyTimeZones();
  verify_constructors();
  verify_parsers();
  verify_setGets();
  return 0;
}
