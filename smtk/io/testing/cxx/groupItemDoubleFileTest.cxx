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

  auto ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(0, 0));
  if (ditem->value(0) != 41.00)
  {
    std::cerr << "Double Item (0,0) is " << ditem->value(0) << " should be 41.00\n";
    return false;
  }

  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(0, 1));
  if (ditem->value(0) != 60.39)
  {
    std::cerr << "Double Item (0,1) is " << ditem->value(0) << " should be 60.39\n";
    return false;
  }

  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(0, 2));
  if (ditem->value(0) != 39.35)
  {
    std::cerr << "Double Item (0,2) is " << ditem->value(0) << " should be 39.35\n";
    return false;
  }

  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(0, 3));
  if (ditem->value(0) != 7621.56)
  {
    std::cerr << "Double Item (0,3) is " << ditem->value(0) << " should be 7621.56\n";
    return false;
  }

  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(0, 4));
  if (ditem->value(0) != 37.41)
  {
    std::cerr << "Double Item (0,4) is " << ditem->value(0) << " should be 37.41\n";
    return false;
  }

  std::size_t last = gitem->numberOfGroups() - 1;
  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(last, 0));
  if (ditem->value(0) != 1321)
  {
    std::cerr << "Double Item (" << last << ",0) is " << ditem->value(0) << " should be 1321\n";
    return false;
  }

  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(last, 1));
  if (ditem->value(0) != 39.8642)
  {
    std::cerr << "Double Item (" << last << ",1) is " << ditem->value(0) << " should be 39.8642\n";
    return false;
  }

  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(last, 2));
  if (ditem->value(0) != 900.80)
  {
    std::cerr << "Double Item (" << last << ",2) is " << ditem->value(2) << " should be 900.80\n";
    return false;
  }

  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(last, 3));
  if (ditem->value(0) != 7246.24)
  {
    std::cerr << "Double Item (" << last << ",3) is " << ditem->value(3) << " should be 7246.24\n";
    return false;
  }

  ditem = std::dynamic_pointer_cast<DoubleItem>(gitem->item(last, 4));
  if (ditem->value(0) != 900.80)
  {
    std::cerr << "Double Item (" << last << ",4) is " << ditem->value(4) << " should be 900.80\n";
    return false;
  }

  return true;
}
}

int main(int argc, char* argv[])
{

  if (argc != 2)
  {
    std::cerr << "Usage: groupItemDoubleFileTest cvsFile" << std::endl;
    return 1;
  }

  std::string doubleFile(argv[1]);
  Logger logger;
  logger.setFlushToStderr(false);
  bool passed = true;

  smtk::attribute::ResourcePtr resource = smtk::attribute::Resource::create();
  // Lets create a definition with 2 group items
  auto aDef = resource->createDefinition("A");
  auto gdef = aDef->addItemDefinition<GroupItemDefinition>("g0");
  gdef->setIsExtensible(true);
  auto ddef = gdef->addItemDefinition<DoubleItemDefinition>("T");
  ddef = gdef->addItemDefinition<DoubleItemDefinition>("K");
  ddef = gdef->addItemDefinition<DoubleItemDefinition>("Density");
  ddef = gdef->addItemDefinition<DoubleItemDefinition>("Enthalpy");
  ddef = gdef->addItemDefinition<DoubleItemDefinition>("Extra");
  // Lets create a couple of groups that the Double File importer can't support
  gdef = aDef->addItemDefinition<GroupItemDefinition>("g1");
  gdef->setIsExtensible(true);
  ddef = gdef->addItemDefinition<DoubleItemDefinition>("d1");
  ddef->setIsOptional(true); // importer can't deal with optional items

  gdef = aDef->addItemDefinition<GroupItemDefinition>("g2");
  gdef->setIsExtensible(true);
  ddef = gdef->addItemDefinition<DoubleItemDefinition>("d1");
  ddef->setNumberOfRequiredValues(3);
  ddef->setIsExtensible(true); // importer can't deal with extensible children

  resource->finalizeDefinitions();

  auto att = resource->createAttribute("a", aDef);

  // Lets try reading into the first group item
  auto gitem = std::dynamic_pointer_cast<GroupItem>(att->item(0));
  if (!importFromDoubleFile(*gitem, doubleFile, logger, false, ",", "#"))
  {
    std::cerr << "Import into Group Item 0 - FAILED\n";
    passed = false;
  }
  else
  {
    if (!checkGroupItem(gitem, 33))
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
  if (!importFromDoubleFile(*gitem, doubleFile, logger, false, ",", "#"))
  {
    std::cerr << "Import into Group Item 0 (Overwrite Pass)- FAILED\n";
    passed = false;
  }
  else
  {
    if (!checkGroupItem(gitem, 33))
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
  if (!importFromDoubleFile(*gitem, doubleFile, logger, true, ",", "#"))
  {
    std::cerr << "Import into Group Item 0 (Append Pass)- FAILED\n";
    passed = false;
  }
  else
  {
    if (!checkGroupItem(gitem, 66))
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
  if (importFromDoubleFile(*gitem, doubleFile, logger, false, ",", "#"))
  {
    std::cerr << "Import into Group Item 1 Succeeded - FAILED\n";
    passed = false;
  }

  gitem = std::dynamic_pointer_cast<GroupItem>(att->item(2));
  if (importFromDoubleFile(*gitem, doubleFile, logger, false, ",", "#"))
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
