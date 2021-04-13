//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <iostream>
#include <sstream>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/Logger.h"
#include "smtk/io/attributeUtils.h"
using namespace smtk::io;
using namespace smtk::attribute;

namespace
{
bool checkGroupItem(GroupItemPtr gitem, int numGroups)
{
  if (gitem->numberOfGroups() != numGroups)
  {
    std::cerr << "Group has " << gitem->numberOfGroups() << " instead of " << numGroups
              << std::endl;
    return false;
  }

  auto sitem = std::dynamic_pointer_cast<StringItem>(gitem->item(0, 0));
  if (sitem->value(0) != "a")
  {
    std::cerr << "String Item (0,0) is " << sitem->value(0) << " should be a\n";
    return false;
  }
  auto iitem = std::dynamic_pointer_cast<IntItem>(gitem->item(0, 1));
  if ((iitem->value(0) != 1) || (iitem->value(1) != 2) || (iitem->value(2) != 3))
  {
    std::cerr << "Int Item (0,1) is (" << iitem->value(0) << ", " << iitem->value(1) << ", "
              << iitem->value(2) << ") should be (1, 2, 3)\n";
    return false;
  }
  auto ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(0, 2));
  if ((ditem->value(0) != 10.1) || (ditem->value(1) != 20.202))
  {
    std::cerr << "Double Item (0,2) is (" << ditem->value(0) << ", " << ditem->value(1)
              << ") should be (10.1, 20.202)\n";
    return false;
  }

  sitem = std::dynamic_pointer_cast<StringItem>(gitem->item(1, 0));
  if (sitem->value(0) != "b")
  {
    std::cerr << "String Item (1,0) is " << sitem->value(0) << " should be b\n";
    return false;
  }
  iitem = std::dynamic_pointer_cast<IntItem>(gitem->item(1, 1));

  if ((iitem->value(0) != 10) || (iitem->value(2) != 30) || iitem->isSet(1))
  {
    std::cerr << "Int Item (1,1) is (" << iitem->value(0) << ", " << iitem->value(1) << ", "
              << iitem->value(2) << ") should be (10, unset, 30)\n";
    return false;
  }
  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(1, 2));
  if ((ditem->value(0) != 101.111) || (ditem->value(1) != 123.456))
  {
    std::cerr << "Double Item (1,2) is (" << ditem->value(0) << ", " << ditem->value(1)
              << ") should be (101.111,  123.456)\n";
    return false;
  }

  if (numGroups == 2)
  {
    return true;
  }
  sitem = std::dynamic_pointer_cast<StringItem>(gitem->item(2, 0));
  if (sitem->value(0) != "a")
  {
    std::cerr << "String Item (2,0) is " << sitem->value(0) << " should be a\n";
    return false;
  }
  iitem = std::dynamic_pointer_cast<IntItem>(gitem->item(2, 1));
  if ((iitem->value(0) != 1) || (iitem->value(1) != 2) || (iitem->value(2) != 3))
  {
    std::cerr << "Int Item (2,1) is (" << iitem->value(0) << ", " << iitem->value(1) << ", "
              << iitem->value(2) << ") should be (1, 2, 3)\n";
    return false;
  }
  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(2, 2));
  if ((ditem->value(0) != 10.1) || (ditem->value(1) != 20.202))
  {
    std::cerr << "Double Item (2,2) is (" << ditem->value(0) << ", " << ditem->value(1)
              << ") should be (10.1, 20.202)\n";
    return false;
  }

  sitem = std::dynamic_pointer_cast<StringItem>(gitem->item(3, 0));
  if (sitem->value(0) != "b")
  {
    std::cerr << "String Item (3,0) is " << sitem->value(0) << " should be b\n";
    return false;
  }
  iitem = std::dynamic_pointer_cast<IntItem>(gitem->item(3, 1));

  if ((iitem->value(0) != 10) || (iitem->value(2) != 30) || iitem->isSet(1))
  {
    std::cerr << "Int Item (3,1) is (" << iitem->value(0) << ", " << iitem->value(1) << ", "
              << iitem->value(2) << ") should be (10, unset, 30)\n";
    return false;
  }
  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(3, 2));
  if ((ditem->value(0) != 101.111) || (ditem->value(1) != 123.456))
  {
    std::cerr << "Double Item (3,2) is (" << ditem->value(0) << ", " << ditem->value(1)
              << ") should be (101.111,  123.456)\n";
    return false;
  }
  return true;
}
} // namespace

int main(int argc, char* argv[])
{

  if (argc != 2)
  {
    std::cerr << "Usage: groupItemCSVTest cvsFile" << std::endl;
    return 1;
  }

  std::string csvFile(argv[1]);
  Logger logger;
  logger.setFlushToStderr(false);
  bool passed = true;

  smtk::attribute::ResourcePtr resource = smtk::attribute::Resource::create();
  // Lets create a definition with 2 group items
  auto aDef = resource->createDefinition("A");
  auto gdef = aDef->addItemDefinition<GroupItemDefinition>("g0");
  gdef->setIsExtensible(true);
  auto sdef = gdef->addItemDefinition<StringItemDefinition>("s0");
  auto idef = gdef->addItemDefinition<IntItemDefinition>("i0");
  idef->setNumberOfRequiredValues(3);
  auto ddef = gdef->addItemDefinition<DoubleItemDefinition>("d0");
  ddef->setNumberOfRequiredValues(2);
  // Lets create a couple of groups that the CSV importer can't support
  gdef = aDef->addItemDefinition<GroupItemDefinition>("g1");
  gdef->setIsExtensible(true);
  sdef = gdef->addItemDefinition<StringItemDefinition>("s1");
  sdef->setIsOptional(true); // importer can't deal with optional items
  idef = gdef->addItemDefinition<IntItemDefinition>("i1");
  idef->setNumberOfRequiredValues(3);
  ddef = gdef->addItemDefinition<DoubleItemDefinition>("d1");
  ddef->setNumberOfRequiredValues(2);

  gdef = aDef->addItemDefinition<GroupItemDefinition>("g2");
  gdef->setIsExtensible(true);
  sdef = gdef->addItemDefinition<StringItemDefinition>("s2");
  idef = gdef->addItemDefinition<IntItemDefinition>("i2");
  idef->setNumberOfRequiredValues(3);
  idef->setIsExtensible(true); // importer can't deal with extensible children
  ddef = gdef->addItemDefinition<DoubleItemDefinition>("d2");
  ddef->setNumberOfRequiredValues(2);

  resource->finalizeDefinitions();

  auto att = resource->createAttribute("a", aDef);

  // Lets try reading into the first group item
  auto gitem = std::dynamic_pointer_cast<GroupItem>(att->item(0));
  if (!importFromCSV(*gitem, csvFile, logger, false, ",", "#"))
  {
    std::cerr << "Import into Group Item 0 - FAILED\n";
    passed = false;
  }
  else
  {
    if (!checkGroupItem(gitem, 2))
    {
      std::cerr << "Checking Group Item 0 after first import - FAILED\n";
      passed = false;
    }
    else
    {
      std::cerr << "Checking Group Item 0 after first import - PASSED\n";
    }
  }

  // Lets try it again - using overwrite
  if (!importFromCSV(*gitem, csvFile, logger, false, ",", "#"))
  {
    std::cerr << "Import into Group Item 0 (Overwrite Pass)- FAILED\n";
    passed = false;
  }
  else
  {
    if (!checkGroupItem(gitem, 2))
    {
      std::cerr << "Checking Group Item 0 after overwrite import - FAILED\n";
      passed = false;
    }
    else
    {
      std::cerr << "Checking Group Item 0 after overwrite import - PASSED\n";
    }
  }

  // Lets try it again - using append
  if (!importFromCSV(*gitem, csvFile, logger, true, ",", "#"))
  {
    std::cerr << "Import into Group Item 0 (Append Pass)- FAILED\n";
    passed = false;
  }
  else
  {
    if (!checkGroupItem(gitem, 4))
    {
      std::cerr << "Checking Group Item 0 after append import - FAILED\n";
      passed = false;
    }
    else
    {
      std::cerr << "Checking Group Item 0 after append import - PASSED\n";
    }
  }

  gitem = std::dynamic_pointer_cast<GroupItem>(att->item(1));
  if (importFromCSV(*gitem, csvFile, logger, false, ",", "#"))
  {
    std::cerr << "Import into Group Item 1 Succeeded - FAILED\n";
    passed = false;
  }

  gitem = std::dynamic_pointer_cast<GroupItem>(att->item(2));
  if (importFromCSV(*gitem, csvFile, logger, false, ",", "#"))
  {
    std::cerr << "Import into Group Item 2 Succeeded - FAILED\n";
    passed = false;
  }

  // Lets verify that the other groups fail
  if (passed)
  {
    std::cerr << "Test Passed!\n";
    return 0;
  }
  return -1;
}
