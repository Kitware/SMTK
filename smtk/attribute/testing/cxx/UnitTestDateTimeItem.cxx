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
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

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

//----------------------------------------------------------------------------
void verifySerialize()
{
  // Instantiate writer
  smtk::io::AttributeWriter writer;
  smtk::io::Logger logger;
  std::string contents;

  // Instantiate att system, attdef, & attribute to write out
  sa::System outputSystem;
  sa::DefinitionPtr attDef = outputSystem.createDefinition("test-att");
  sa::DateTimeItemDefinitionPtr dtDef = sa::DateTimeItemDefinition::New(
    "datetime");
  dtDef->setDisplayFormat("dd-MMM-yyyy  h:mm:ss.zzz AP");
  dtDef->setUseTimeZone(false);
  dtDef->setEnableCalendarPopup(false);

  // Set default value
  sa::DateTimeZonePair dtz;
  sa::DateTime dt;
  dt.setComponents(2016, 11, 10, 14, 18, 0, 0);
  dtz.setDateTime(dt);
  sa::TimeZone tz;
  tz.setRegion("America/Chicago");
  dtz.setTimeZone(tz);
  dtDef->setDefaultValue(dtz);

  attDef->addItemDefinition(dtDef);
  sa::AttributePtr att = outputSystem.createAttribute(attDef);

  // Write to string
  bool writeError = writer.writeContents(outputSystem, contents, logger);
  if (writeError)
    {
    std::string reason = logger.convertToString(true);
    test(false, reason);
    }

  std::cout << "File contents" << "\n" << contents << std::endl;

  // Read back
  sa::System inputSystem;
  smtk::io::AttributeReader reader;
  bool readError = reader.readContents(inputSystem, contents, logger);
  if (readError)
    {
    std::string reason = logger.convertToString(true);
    test(false, reason);
    }

  // Check item-definition contents
  sa::DefinitionPtr inputDef = inputSystem.findDefinition("test-att");
  test(!!inputDef, "Failed to read back definition");
  test(
    inputDef->numberOfItemDefinitions() == 1,
    "Wrong number of item definitions read back");
  int i = inputDef->findItemPosition("datetime");
  sa::ItemDefinitionPtr inputItemDef = inputDef->itemDefinition(i);
  test(!!inputItemDef, "Failed to read back \"datetime\" ItemDefinition");
  sa::DateTimeItemDefinitionPtr inputDateTimeItemDef =
    smtk::dynamic_pointer_cast<sa::DateTimeItemDefinition>(inputItemDef);
  test(!!inputDateTimeItemDef, "Failed to read back DateTimeItemDefinition");
  test(
    inputDateTimeItemDef->displayFormat() == "dd-MMM-yyyy  h:mm:ss.zzz AP",
    "Failed to read back definition display format");
  test(
    !inputDateTimeItemDef->useTimeZone(),
    "Failed to read back use-time-zone setting");
  test(
    !inputDateTimeItemDef->useCalendarPopup(),
    "Failed to read back enable-calendar-popup setting");
  test(inputDateTimeItemDef->hasDefault(), "Failed to read back default value");

  // Check item contents
  sa::AttributePtr inputAtt = inputSystem.findAttribute(att->name());
  test(!!inputAtt, "Failed to read back attribute");
  sa::DateTimeItemPtr inputDateTimeItem = inputAtt->findDateTime("datetime");
  test(!!inputDateTimeItem, "Failed to find datetime item");
  // Must explicitly set to default
  inputDateTimeItem->setToDefault(0);
  test(inputDateTimeItem->isSet(), "Failed to read back that default value is set");

  sa::DateTimeZonePair dtzSet = inputDateTimeItem->value(0);
  //std::cout << dtzSet << std::endl;

  sa::DateTime dtSet = dtzSet.dateTime();
  int yr=-1, month=-1, day=-1, hr=-1, minute=-1, sec=-1, msec=-1;
  dtSet.components(yr, month, day, hr, minute, sec, msec);
  bool match = (2016 == yr) && (11 == month) && (10 == day) &&
    (14 == hr) && (18 == minute) && (0 == sec) && (0 == msec);
  test(match, "Failed to set attribute item DateTime");

  sa::TimeZone tzSet = dtzSet.timeZone();
  test(
    tzSet.region() == "America/Chicago",
    "Failed to set attribute item TimeZone");
}

}  // end namespace

//----------------------------------------------------------------------------
int UnitTestDateTimeItem(int, char** const)
{
  verifyDefault();
  verifySerialize();
  return 0;
}
