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

#include "smtk/common/testing/cxx/helpers.h"

//----------------------------------------------------------------------------
void verify_constructors()
{
  smtk::attribute::DateTime dtEmpty;
  test(!dtEmpty.isValid(), "Failed to recognize invalid state");
}

//----------------------------------------------------------------------------
void verify_parsers()
{
  smtk::attribute::DateTime dtInvalid;
  dtInvalid.parseIsoString("");
  test(!dtInvalid.isValid(), "Failed to detect empty (invalid) iso string");

  smtk::attribute::DateTime dtIso;
  dtIso.parseIsoString("20161026T141559");
  test(!!dtIso.isValid(), "Failed to parse iso string");

  test(
    !!dtIso.parseIsoString("20161026T141559.123456+0200"),
    "failed to parse iso string with tz offset");

  smtk::attribute::DateTime dtDate;
  dtDate.parseString("2002-01-20 23:59:59.000");
  test(!!dtIso.isValid(), "Failed to parse date string");
}

//----------------------------------------------------------------------------
int UnitTestDateTime(int, char** const)
{
  verify_constructors();
  verify_parsers();
  return 0;
}
