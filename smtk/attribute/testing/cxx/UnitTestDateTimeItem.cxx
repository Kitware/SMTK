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

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/System.h"
#include "smtk/common/DateTime.h"
#include "smtk/common/DateTimeZonePair.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace sa = smtk::attribute;
namespace sc = smtk::common;
namespace
{

void verifyDefault()
{
  // Instantiate item definition
  sa::DateTimeItemDefinitionPtr dtDef = sa::DateTimeItemDefinition::New("dt-def");
  test(!!dtDef, "Failed to instantiate DateTimeItemDefinition");
  test(dtDef->type() == sa::Item::DATE_TIME, "Failed to return DATE_TIME as definition type");

  // Instantiate att system, attdef, & attribute
  sa::System system;
  sa::DefinitionPtr attDef = system.createDefinition("test-datetime");
  attDef->addItemDefinition(dtDef);
  sa::AttributePtr att = system.createAttribute(attDef);

  sa::ItemPtr item = att->find("dt-def");
  test(!!item, "Failed to find Item");
  test(item->type() == sa::Item::DATE_TIME, "Failed to return DATE_TIME as item type");

  sa::DateTimeItemPtr dtItem = att->findDateTime("dt-def");
  test(!!dtItem, "Failed to find DateTimeItem");
  test(dtItem->type() == sa::Item::DATE_TIME, "Failed to return DATE_TIME as DateTimeItem type");
  test(!dtItem->isSet(), "isSet() failed to return false for default item");

  // Set default value
  smtk::common::DateTimeZonePair dtz;

  smtk::common::DateTime dt;
  dt.setComponents(2016, 11, 5, 13, 41, 33, 555);
  dtz.setDateTime(dt);

  smtk::common::TimeZone tz;
  tz.setRegion("America/New_York");
  dtz.setTimeZone(tz);

  dtDef->setDefaultValue(dtz);
  sa::AttributePtr attSet = system.createAttribute(attDef);
  sa::DateTimeItemPtr dtItemSet = att->findDateTime("dt-def");
  // Must explicitly set to default
  dtItemSet->setToDefault(0);
  test(dtItem->isSet(), "isSet() failed to return true for default item");
  smtk::common::DateTimeZonePair dtzSet = dtItemSet->value(0);
  //std::cout << dtzSet << std::endl;
  smtk::common::DateTime dtSet = dtzSet.dateTime();
  int yr = -1, month = -1, day = -1, hr = -1, minute = -1, sec = -1, msec = -1;
  dtSet.components(yr, month, day, hr, minute, sec, msec);
  bool match = (2016 == yr) && (11 == month) && (5 == day) && (13 == hr) && (41 == minute) &&
    (33 == sec) && (555 == msec);
  test(match, "Failed to read back components");
}

void verifySerialize()
{
  // Instantiate writer
  smtk::io::AttributeWriter writer;
  writer.setMaxFileVersion(); // (need version 3 or later)
  smtk::io::Logger logger;
  std::string contents;

  // Instantiate att system, attdef, & attribute to write out
  sa::System outputSystem;
  sa::DefinitionPtr attDef = outputSystem.createDefinition("test-att");

  // First DateTimeItemDefinition
  sa::DateTimeItemDefinitionPtr dt1Def = sa::DateTimeItemDefinition::New("dt1");
  dt1Def->setDisplayFormat("dd-MMM-yyyy  h:mm:ss.zzz AP");
  dt1Def->setUseTimeZone(false);
  dt1Def->setEnableCalendarPopup(false);

  // Set default value
  smtk::common::DateTimeZonePair dtz;
  smtk::common::DateTime dt;
  dt.setComponents(2016, 11, 10, 14, 18, 0, 0);
  dtz.setDateTime(dt);
  smtk::common::TimeZone tz;
  tz.setRegion("America/Chicago");
  dtz.setTimeZone(tz);
  dt1Def->setDefaultValue(dtz);

  attDef->addItemDefinition(dt1Def);

  // Second DateTimeItemDefinition
  sa::DateTimeItemDefinitionPtr dt2Def = sa::DateTimeItemDefinition::New("dt2");
  attDef->addItemDefinition(dt2Def);

  // Group item w/3rd DateTime
  sa::GroupItemDefinitionPtr groupDef = sa::GroupItemDefinition::New("group");
  sa::DateTimeItemDefinitionPtr dt3Def = sa::DateTimeItemDefinition::New("dt3");
  groupDef->addItemDefinition(dt3Def);
  attDef->addItemDefinition(groupDef);

  // Discrete item w/child 4th DateTime
  sa::IntItemDefinitionPtr discreteDef = sa::IntItemDefinition::New("discrete");
  discreteDef->addDiscreteValue(0, "Without DateTime");
  discreteDef->addDiscreteValue(1, "With DateTime");
  sa::DateTimeItemDefinitionPtr dt4Def = sa::DateTimeItemDefinition::New("dt4");
  discreteDef->addChildItemDefinition(dt4Def);
  discreteDef->addConditionalItem("With DateTime", "dt4");
  attDef->addItemDefinition(discreteDef);

  // Instantiate attribute
  sa::AttributePtr att = outputSystem.createAttribute(attDef);

  // Get first item and set to the default value
  sa::DateTimeItemPtr dt1Item = att->findDateTime("dt1");
  dt1Item->setToDefault(0);

  // Set contents of 2nd item
  sa::DateTimeItemPtr dt2Item = att->findDateTime("dt2");
  smtk::common::DateTime dt2;
  dt2.deserialize("19690720T201800");
  smtk::common::TimeZone tz2;
  tz2.setUTC();
  smtk::common::DateTimeZonePair dtz2;
  dtz2.setDateTime(dt2);
  dtz2.setTimeZone(tz2);
  dt2Item->setValue(dtz2);

  // Write to string
  bool writeError = writer.writeContents(outputSystem, contents, logger);
  if (writeError)
  {
    std::string reason = logger.convertToString(true);
    test(false, reason);
  }

  std::cout << "File contents"
            << "\n"
            << contents << std::endl;

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
  test(inputDef->numberOfItemDefinitions() == 4, "Wrong number of item definitions read back");
  int i1 = inputDef->findItemPosition("dt1");
  sa::ItemDefinitionPtr inputItemDef = inputDef->itemDefinition(i1);
  test(!!inputItemDef, "Failed to read back \"dt1\" ItemDefinition");
  sa::DateTimeItemDefinitionPtr dt1InputDef =
    smtk::dynamic_pointer_cast<sa::DateTimeItemDefinition>(inputItemDef);
  test(!!dt1InputDef, "Failed to read back DateTimeItemDefinition");
  test(dt1InputDef->displayFormat() == "dd-MMM-yyyy  h:mm:ss.zzz AP",
    "Failed to read back definition display format");
  test(!dt1InputDef->useTimeZone(), "Failed to read back use-time-zone setting");
  test(!dt1InputDef->useCalendarPopup(), "Failed to read back enable-calendar-popup setting");
  test(dt1InputDef->hasDefault(), "Failed to read back default value");

  // Check attribute
  sa::AttributePtr inputAtt = inputSystem.findAttribute(att->name());
  test(!!inputAtt, "Failed to read back attribute");

  // Check first DateTimeItem
  sa::DateTimeItemPtr dt1ItemInput = inputAtt->findDateTime("dt1");
  test(!!dt1ItemInput, "Failed to find datetime item");
  test(dt1ItemInput->isSet(), "Failed to read back that default value is set");

  smtk::common::DateTimeZonePair dtz1Input = dt1ItemInput->value(0);
  //std::cout << dtz1Input << std::endl;

  smtk::common::DateTime dt1Input = dtz1Input.dateTime();
  int yr = -1, month = -1, day = -1, hr = -1, minute = -1, sec = -1, msec = -1;
  dt1Input.components(yr, month, day, hr, minute, sec, msec);
  bool match1 = (2016 == yr) && (11 == month) && (10 == day) && (14 == hr) && (18 == minute) &&
    (0 == sec) && (0 == msec);
  test(match1, "Failed to set 1st item's DateTime");

  smtk::common::TimeZone tz1Input = dtz1Input.timeZone();
  test(tz1Input.region() == "America/Chicago", "Failed to set attribute item TimeZone");

  // Check 2nd DateTimeItem
  sa::DateTimeItemPtr dt2ItemInput = inputAtt->findDateTime("dt2");
  smtk::common::DateTimeZonePair dtz2Input = dt2ItemInput->value(0);

  smtk::common::DateTime dt2Input = dtz2Input.dateTime();
  yr = -1, month = -1, day = -1, hr = -1, minute = -1, sec = -1, msec = -1;
  dt2Input.components(yr, month, day, hr, minute, sec, msec);
  bool match2 = (1969 == yr) && (7 == month) && (20 == day) && (20 == hr) && (18 == minute) &&
    (0 == sec) && (0 == msec);
  test(match2, "Failed to set 2nd item's DateTime");

  smtk::common::TimeZone tz2Input = dtz2Input.timeZone();
  test(tz2Input.region().empty(), "Failed to clear region value for 2nd item's TimeZone");
  test(tz2Input.isUTC(), "Failed to set TimeZone to UTC");

  // Check 3rd DateTime (parented by group)
  sa::GroupItemPtr groupItemInput = inputAtt->findGroup("group");
  test(!!groupItemInput, "Failed to find GroupItem");
  sa::DateTimeItemPtr dt3ItemInput = groupItemInput->findAs<sa::DateTimeItem>("dt3");
  test(!!dt3ItemInput, "Failed to find DateTimeItem as child of GroupItem");

  // Check 4th DateTime (conditional child)
  sa::IntItemPtr discreteItemInput = inputAtt->findInt("discrete");
  test(!!discreteItemInput, "Failed to find discrete IntItem");
  test(discreteItemInput->setDiscreteIndex(0), "Invalid discrete index 0");
  test(discreteItemInput->numberOfActiveChildrenItems() == 0,
    "Wrong number of active children; should be 0");
  test(discreteItemInput->setDiscreteIndex(1), "Invalid discrete index 1");
  test(discreteItemInput->numberOfActiveChildrenItems() == 1,
    "Wrong number of active children; should be 1");
  sa::ItemPtr activeChild = discreteItemInput->activeChildItem(0);
  test(activeChild->type() == sa::Item::DATE_TIME, "Active child not DateTime");
}

} // end namespace

int UnitTestDateTimeItem(int, char** const)
{
  verifyDefault();
  verifySerialize();
  return 0;
}
