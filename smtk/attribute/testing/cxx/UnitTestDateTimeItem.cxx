//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/DateTimeItemDefinition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DateTime.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/System.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace sa = smtk::attribute;
namespace {

//----------------------------------------------------------------------------
void verifyDefault()
{
  // Instantiate item definition
  sa::DateTimeItemDefinitionPtr dtDef = sa::DateTimeItemDefinition::New(
    "dt-def");
  test(!!dtDef, "Failed to instantiate DateTimeItemDefinition");
  test(dtDef->type() == sa::Item::DATE_TIME,
       "Failed to return DATE_TIME as definition type");

  // Instantiate att system, attdef, & attribute
  sa::System system;
  sa::DefinitionPtr attDef = system.createDefinition("test-datetime");
  attDef->addItemDefinition(dtDef);
  sa::AttributePtr att = system.createAttribute(attDef);

  sa::ItemPtr item = att->find("dt-def");
  test(!!item, "Failed to find Item");
  test(item->type() == sa::Item::DATE_TIME,
       "Failed to return DATE_TIME as item type");

  sa::DateTimeItemPtr dtItem = att->findDateTime("dt-def");
  test(!!dtItem, "Failed to find DateTimeItem");
  test(dtItem->type() == sa::Item::DATE_TIME,
       "Failed to return DATE_TIME as DateTimeItem type");
  test(!dtItem->isSet(), "isSet() failed to return false for default item");

  // Set default value
  sa::DateTimeZonePair dtz;

  sa::DateTime dt;
  dt.setComponents(2016, 11, 5, 13, 41, 33, 555);
  dtz.setDateTime(dt);

  sa::TimeZone tz;
  tz.setRegion("America/New_York");
  dtz.setTimeZone(tz);

  dtDef->setDefaultValue(dtz);
  sa::AttributePtr attSet = system.createAttribute(attDef);
  sa::DateTimeItemPtr dtItemSet = att->findDateTime("dt-def");
  // Must explicitly set to default
  dtItemSet->setToDefault(0);
  test(dtItem->isSet(), "isSet() failed to return true for default item");
  sa::DateTimeZonePair dtzSet = dtItemSet->value(0);
  //std::cout << dtzSet << std::endl;
  sa::DateTime dtSet = dtzSet.dateTime();
  int yr=-1, month=-1, day=-1, hr=-1, minute=-1, sec=-1, msec=-1;
  dtSet.components(yr, month, day, hr, minute, sec, msec);
  bool match = (2016 == yr) && (11 == month) && (5 == day) &&
    (13 == hr) && (41 == minute) && (33 == sec) && (555 == msec);
  test(match, "Failed to read back components");
}

}  // end namespace

//----------------------------------------------------------------------------
int UnitTestDateTimeItem(int, char** const)
{
  verifyDefault();
  return 0;
}
