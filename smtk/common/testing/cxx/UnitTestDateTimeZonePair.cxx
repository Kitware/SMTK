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
#include "smtk/common/DateTimeZonePair.h"
#include "smtk/common/TimeZone.h"

#include "smtk/common/testing/cxx/helpers.h"
#include <sstream>

namespace sc = smtk::common;
namespace
{

void verifyNotSet()
{
  std::cout << "verifyNotSet()" << std::endl;
  sc::DateTimeZonePair dtzOut;
  std::stringstream ss;
  ss << dtzOut;
  std::cout << "NotSet: " << ss.str() << std::endl;

  sc::DateTimeZonePair dtzIn1;
  ss.seekg(0);
  ss >> dtzIn1;
  sc::DateTime dtIn1 = dtzIn1.dateTime();
  test(!dtIn1.isSet(), "Failed to show datetime as not set");

  sc::TimeZone tzIn1 = dtzIn1.timeZone();
  test(!tzIn1.isSet(), "Failed to show timezone as not set");
}

void verifyParsers()
{
  std::cout << "verifyParsers()" << std::endl;
  // Using DateTime::deserialize():
  sc::DateTimeZonePair dtzInvalid;
  dtzInvalid.deserialize("");
  test(!dtzInvalid.dateTime().isSet(), "Failed to detect empty (invalid) iso string");
  test(!dtzInvalid.deserialize("20160231"), "failed to detect invalid json");

  // Other forms are tested in UnitTestDateTime
  sc::DateTimeZonePair dtzValid;
  test(
    !!dtzValid.deserialize(
      "{\"datetime\":\"20220620T000000\",\"timezone-region\":\"America/New_York\"}"),
    "Failed to deserialize valid json DateTimeZonePair");
  test(
    !!dtzValid.dateTime().isSet() && !!dtzValid.timeZone().isSet(),
    "Failed to return true for isSet()");
}

void verifyNoTimeZone()
{
  std::cout << "verifyNoTimeZone()" << std::endl;
  sc::DateTimeZonePair dtzOut;

  sc::DateTime dt;
  dt.setComponents(2016, 11, 8, 16, 28, 2, 321);
  dtzOut.setDateTime(dt);

  std::stringstream ss;
  ss << dtzOut;
  std::cout << "NoTimeZone: " << ss.str() << std::endl;

  sc::DateTimeZonePair dtzIn1;
  ss.seekg(0);
  ss >> dtzIn1;
  sc::DateTime dtIn1 = dtzIn1.dateTime();

  int yr, month, day, hr, minute, sec, msec;
  dtIn1.components(yr, month, day, hr, minute, sec, msec);
  bool match1 = (yr == 2016) && (month == 11) && (day == 8) && (hr == 16) && (minute == 28) &&
    (sec == 2) && (msec == 321);
  test(match1, "Failed to set DateTime when no TimeZone specified");

  sc::TimeZone tzIn1 = dtzIn1.timeZone();
  test(!tzIn1.isSet(), "Failed to show timezone as unSet()");
}

void verifyTimeZoneOnly()
{
  std::cout << "verifyTimeZoneOnly()" << std::endl;
  sc::DateTimeZonePair dtzOut;

  sc::TimeZone tz;
  tz.setRegion("Pacific/Honolulu");
  dtzOut.setTimeZone(tz);

  std::stringstream ss;
  ss << dtzOut;
  std::cout << "TimeZoneOnly:" << ss.str() << std::endl;

  sc::DateTimeZonePair dtzIn1;
  ss.seekg(0);
  ss >> dtzIn1;
  sc::DateTime dtIn1 = dtzIn1.dateTime();
  test(!dtIn1.isSet(), "Failed to show datetime/TimeZoneOnly as set");

  sc::TimeZone tzIn1 = dtzIn1.timeZone();
  test(!!tzIn1.isSet(), "Failed to show timezone/TimeZoneOnly as set");
}

void verifyUTC()
{
  std::cout << "verifyUTC()" << std::endl;
  sc::DateTimeZonePair dtzOut;

  sc::DateTime dt;
  dt.setComponents(1999, 12, 31, 23, 59, 59, 999);
  dtzOut.setDateTime(dt);

  sc::TimeZone tz;
  tz.setUTC();
  dtzOut.setTimeZone(tz);

  std::stringstream ss;
  ss << dtzOut;
  std::cout << "UTC TimeZone:" << ss.str() << std::endl;

  sc::DateTimeZonePair dtzIn1;
  ss.seekg(0);
  ss >> dtzIn1;
  sc::DateTime dtIn1 = dtzIn1.dateTime();

  int yr, month, day, hr, minute, sec, msec;
  dtIn1.components(yr, month, day, hr, minute, sec, msec);
  bool match1 = (yr == 1999) && (month == 12) && (day == 31) && (hr == 23) && (minute == 59) &&
    (sec == 59) && (msec == 999);
  test(match1, "Failed to set DateTime when UTC specified");

  sc::TimeZone tzIn1 = dtzIn1.timeZone();
  test(!!tzIn1.isSet(), "Failed to show UTC timezone as set()");
  test(!!tzIn1.isUTC(), "Failed to return true for isUTC()");
}

void verifyRegionTimeZone()
{
  std::cout << "verifyRegionTimeZone()" << std::endl;
  sc::DateTimeZonePair dtzOut;

  sc::DateTime dt;
  dt.setComponents(2000, 1, 2, 3, 4, 5, 6);
  dtzOut.setDateTime(dt);

  sc::TimeZone tz;
  tz.setRegion("Europe/London");
  dtzOut.setTimeZone(tz);

  std::stringstream ss;
  ss << dtzOut;
  std::cout << "RegionTimeZone:" << ss.str() << std::endl;

  sc::DateTimeZonePair dtzIn1;
  ss.seekg(0);
  ss >> dtzIn1;
  sc::DateTime dtIn1 = dtzIn1.dateTime();

  int yr, month, day, hr, minute, sec, msec;
  dtIn1.components(yr, month, day, hr, minute, sec, msec);
  bool match1 = (yr == 2000) && (month == 1) && (day == 2) && (hr == 3) && (minute == 4) &&
    (sec == 5) && (msec == 6);
  test(match1, "Failed to set DateTime when RegionTimeZone specified");

  sc::TimeZone tzIn1 = dtzIn1.timeZone();
  test(!!tzIn1.isSet(), "Failed to show region timezone as set()");
  test(tzIn1.region() == "Europe/London", "Failed to return correct time zone region");
}

void verifyPosixTimeZone()
{
  std::cout << "verifyPosixTimeZone()" << std::endl;
  sc::DateTimeZonePair dtzOut;

  sc::DateTime dt;
  dt.setComponents(1776, 7, 4, 12, 0, 1, 500);
  dtzOut.setDateTime(dt);

  sc::TimeZone tz;
  tz.setPosixString("PST-8");
  dtzOut.setTimeZone(tz);

  // Serialize
  std::stringstream ss;
  ss << dtzOut;
  std::cout << "PosixTimeZone:" << ss.str() << std::endl;

  // Deserialize
  sc::DateTimeZonePair dtzIn1;
  ss.seekg(0);
  ss >> dtzIn1;
  sc::DateTime dtIn1 = dtzIn1.dateTime();

  int yr, month, day, hr, minute, sec, msec;
  dtIn1.components(yr, month, day, hr, minute, sec, msec);
  bool match1 = (yr == 1776) && (month == 7) && (day == 4) && (hr == 12) && (minute == 0) &&
    (sec == 1) && (msec == 500);
  test(match1, "Failed to set DateTime when PosixTimeZone specified");

  sc::TimeZone tzIn1 = dtzIn1.timeZone();
  test(!!tzIn1.isSet(), "Failed to show posix timezone as set()");
  std::cout << "posix string: " << tzIn1.posixString() << std::endl;
  test(
    tzIn1.posixString() == "PST-08", // note it's -08 NOT -8
    "Failed to return correct posix time zone");
}

} // end namespace

int UnitTestDateTimeZonePair(int /*unused*/, char** const /*unused*/)
{
  verifyNotSet();
  verifyParsers();
  verifyTimeZoneOnly();
  verifyNoTimeZone();
  verifyUTC();
  verifyRegionTimeZone();
  verifyPosixTimeZone();
  return 0;
}
