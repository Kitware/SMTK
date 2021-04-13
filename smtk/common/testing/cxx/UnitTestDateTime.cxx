//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/DateTime.h"
#include "smtk/common/TimeZone.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace
{

void verifyTimeZones()
{
  smtk::common::TimeZone tz;
  test(!tz.isSet(), "Failed to recognize unset TimeZone");
  test(tz.setRegion("America/New_York"), "Failed to set TimeZone region");
  test(!tz.setRegion("Invalid"), "Failed to recognize invalid TimeZone region");
}

void verifyConstructors()
{
  smtk::common::DateTime dtEmpty;
  test(!dtEmpty.isSet(), "Failed to recognize invalid state");
}

void verifyParsers()
{
  // Using DateTime::deserialize():
  smtk::common::DateTime dtInvalid;
  dtInvalid.deserialize("");
  test(!dtInvalid.isSet(), "Failed to detect empty (invalid) iso string");
  test(!dtInvalid.deserialize("20160231"), "failed to detect invalid date");

  smtk::common::DateTime dtValid;
  const char* validCases[] = { "20161026T141559",
                               "Failed to parse iso string",
                               "20161026T141559.123456+0200",
                               "Failed to parse iso string with tz offset",
                               "20161026T141559Z",
                               "Failed to parse iso string with Z suffix" };
  for (std::size_t i = 0; i < sizeof(validCases) / sizeof(const char*); i += 2)
  {
    test(!!dtValid.deserialize(validCases[i]), validCases[i + 1]);
    test(!!dtValid.isSet(), "Failed to return true for isSet()");
  }

  smtk::common::DateTime dtNotSet;
  test(dtNotSet.deserialize("not-a-date-time"), "Failed to parse not-a-date-time");
  test(!dtNotSet.isSet(), "Failed to return isSet() false");

  // Using DateTime::parseBoostFormat():
  smtk::common::DateTime dt;
  dt.parseBoostFormat("2002-01-20 23:59:59.000");
  test(!!dt.isSet(), "Failed to parse basic string");
  dt.parseBoostFormat("2002-01-20 23:59:59.000-02:00");
  test(!!dt.isSet(), "Failed to parse string with offset");

  // Unfortunately, boost doesn't correctly parse date-only strings
  dt.parseBoostFormat("2002-01-20");
  test(!dt.isSet(), "Failed to show that parsing date only dont work");
}

void verifySetGets()
{
  // Initialize
  smtk::common::DateTime dt;
  dt.deserialize("20161028T145400.123456");

  int yr, month, day, hr, minute, sec, msec;

  // Read back
  test(dt.components(yr, month, day, hr, minute, sec, msec), "Failed to get components");
  test(yr == 2016, "Failed to get year component");
  test(month == 10, "Failed to get month component");
  test(day == 28, "Failed to get day component");
  test(hr == 14, "Failed to get hour component");
  test(minute == 54, "Failed to get minute component");
  test(sec == 0, "Failed to get second component");
  test(msec == 123, "Failed to get millisecond component");

  // Set new datetime and read back
  int yr2 = 2017;
  int month2 = 2;
  int day2 = 15;
  int hr2 = 8;
  int minute2 = 15;
  int sec2 = 0;
  int msec2 = 456;
  test(dt.setComponents(yr2, month2, day2, hr2, minute2, sec2, msec2), "Failed to set components");
  dt.components(yr, month, day, hr, minute, sec, msec),
    test(yr == yr2, "Failed to get year component");
  test(month == month2, "Failed to get month component");
  test(day == day2, "Failed to get day component");
  test(hr == hr2, "Failed to get hour component");
  test(minute == minute2, "Failed to get minute component");
  test(sec == sec2, "Failed to get second component");
  test(msec == msec2, "Failed to get millisecond component");

  // Set with time zone
  smtk::common::TimeZone tzCST;
  tzCST.setPosixString("CST-6");
  test(
    dt.setComponents(tzCST, yr2, month2, day2, hr2, minute2, sec2, msec2),
    "Failed to setComponents() with time zone");
  dt.components(yr, month, day, hr, minute, sec, msec),
    // Returned hour should be ahead by 6
    test(hr == hr2 + 6, "Failed to get hour component for tzCST (set)");

  // Get with time zone
  smtk::common::TimeZone tzPST;
  tzPST.setPosixString("PST-8");
  test(
    dt.components(tzPST, yr, month, day, hr, minute, sec, msec),
    "Failed components() with time zone");
  std::cout << "hr: " << hr << ",  hr2: " << hr2 << std::endl;
  // Returned hour should be behind by 2 (plus 6 minus 8)
  test(hr == hr2 - 8 + 6, "Failed to get hour component for tzPST (get)");
}

} // end namespace

int UnitTestDateTime(int /*unused*/, char** const /*unused*/)
{
  verifyTimeZones();
  verifyConstructors();
  verifyParsers();
  verifySetGets();
  return 0;
}
