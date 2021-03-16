//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;
using namespace smtk::common;
using namespace smtk;

int unitItemBlocks(int /*unused*/, char* /*unused*/ [])
{
  // Read in the test configurations files
  bool status = true;
  std::string attFile;
  attFile = SMTK_DATA_DIR;
  attFile += "/attribute/attribute_collection/ItemBlockTest.sbt";
  io::AttributeReader reader;
  io::Logger logger;
  auto attRes = attribute::Resource::create();
  reader.read(attRes, attFile, logger);
  if (logger.hasErrors())
  {
    std::cerr << "Errors Generated when reading SBT file :\n" << logger.convertToString();
  }

  DefinitionPtr t1 = attRes->findDefinition("Type1");
  if (t1 == nullptr)
  {
    std::cerr << "Could not find Definition Type1\n";
    status = false;
  }
  else
  {
    // t1 should be of categories Solid Mechanics or Fluid Flow but not Heat Transfer
    if (!t1->categories().passes("Solid Mechanics"))
    {
      std::cerr << "Type1 does not pass Solid Mechanics\n";
      status = false;
    }
    if (!t1->categories().passes("Fluid Flow"))
    {
      std::cerr << "Type1 does not pass Fluid Flow\n";
      status = false;
    }
    if (t1->categories().passes("Heat Transfer"))
    {
      std::cerr << "Type1 should not have passed Heat Transfer\n";
      status = false;
    }
    if (t1->numberOfItemDefinitions() != 4)
    {
      std::cerr << "Type1 does not contain the correct items"
                << " found: ";
      int numIDefs = (int)t1->numberOfItemDefinitions();
      for (int i = 0; i < numIDefs; ++i)
      {
        std::cerr << "\"" << t1->itemDefinition(i)->name() << "\" ";
      }
      status = false;
    }
    else
    {
      auto item = t1->itemDefinition(0);
      if (item->name() != "foo")
      {
        std::cerr << "Type1[0] should be foo - found: " << item->name() << std::endl;
        status = false;
      }
      item = t1->itemDefinition(1);
      if (item->name() != "s1")
      {
        std::cerr << "Type1[1] should be s1 - found: " << item->name() << std::endl;
        status = false;
      }
      else if (!item->categories().passes("Solid Mechanics"))
      {
        std::cerr << "Type1[1] did not pass Solid Mechanics\n";
        status = false;
      }
      else if (!item->categories().passes("Fluid Flow"))
      {
        std::cerr << "Type1[1] did not pass Fluid Flow\n";
        status = false;
      }
      item = t1->itemDefinition(2);
      if (item->name() != "i1")
      {
        std::cerr << "Type1[2] should be i1 - found: " << item->name() << std::endl;
        status = false;
      }
      item = t1->itemDefinition(3);
      if (item->name() != "bar")
      {
        std::cerr << "Type1[3] should be bar - found: " << item->name() << std::endl;
        status = false;
      }
    }
  }

  DefinitionPtr t2 = attRes->findDefinition("Type2");
  if (t2 == nullptr)
  {
    std::cerr << "Could not find Definition Type2\n";
    status = false;
  }
  else
  {
    // t2 should be of categories Heat Transfer or Fluid Flow but not Solid Mechanics
    if (!t2->categories().passes("Solid Mechanics"))
    {
      std::cerr << "Type2 does not pass Solid Mechanics\n";
      status = false;
    }
    if (t2->categories().passes("Fluid Flow"))
    {
      std::cerr << "Type2 should not have passed Fluid Flow\n";
      status = false;
    }
    if (!t2->categories().passes("Heat Transfer"))
    {
      std::cerr << "Type2 does not pass Solid Mechanics\n";
      status = false;
    }
    if (t2->numberOfItemDefinitions() != 3)
    {
      std::cerr << "Type1 does not contain the correct items"
                << " found: ";
      int numIDefs = (int)t2->numberOfItemDefinitions();
      for (int i = 0; i < numIDefs; ++i)
      {
        std::cerr << "\"" << t2->itemDefinition(i)->name() << "\" ";
      }
      status = false;
    }
    else
    {
      auto item = t2->itemDefinition(0);
      if (item->name() != "s1")
      {
        std::cerr << "Type2[0] should be s1 - found: " << item->name() << std::endl;
        status = false;
      }
      else if (!item->categories().passes("Heat Transfer"))
      {
        std::cerr << "Type2[0] did not pass Heat Transfer\n";
        status = false;
      }
      else if (!item->categories().passes("Solid Mechanics"))
      {
        std::cerr << "Type2[0] did not pass Solid Mechanics\n";
        status = false;
      }
      item = t2->itemDefinition(1);
      if (item->name() != "i1")
      {
        std::cerr << "Type2[1] should be i1 - found: " << item->name() << std::endl;
        status = false;
      }
      item = t2->itemDefinition(2);
      if (item->name() != "str2")
      {
        std::cerr << "Type2[2] should be str2 - found: " << item->name() << std::endl;
        status = false;
      }
    }
  }

  if (status)
  {
    std::cerr << "All Tests Passed!\n";
    return 0;
  }
  return -1;
}
