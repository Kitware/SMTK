//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Resource.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;

int unitItemPath(int /*unused*/, char** const /*unused*/)
{
  // Instantiate att resource, attdef, & attribute
  int status = 0;
  ResourcePtr resource = Resource::create();
  DefinitionPtr attDef = resource->createDefinition("A");
  attDef->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("i1");
  attDef->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("i2");
  auto groupDef = attDef->addItemDefinition<smtk::attribute::GroupItemDefinitionPtr>("g1");
  groupDef->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("i3");
  groupDef->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("i4");
  groupDef->setNumberOfRequiredGroups(4);
  AttributePtr att = resource->createAttribute(attDef);

  std::size_t i, j, n = att->numberOfItems();
  smtkTest(n == 3, "Number of Items in A is " << n << " should be 3!");

  for (i = 0; i < n; i++)
  {
    auto item = att->item(i);
    if (att->itemAtPath(item->path()) != item)
    {
      std::cerr << " Failed to find Item: " << item->name() << " Item's path: " << item->path()
                << std::endl;
      status = -1;
    }
    else
    {
      std::cerr << "Item: " << item->name() << "'s path:" << item->path() << " worked!\n";
    }
  }

  // Lets checkout the Group's Items
  auto gitem = att->itemAtPathAs<smtk::attribute::GroupItem>("/g1");
  smtkTest(!!gitem, "Failed to find Group Item");
  std::cerr << "Finding Group using /g1 worked!\n";
  std::size_t numGroups = gitem->numberOfGroups();
  std::size_t numItemsPerGroup = gitem->numberOfItemsPerGroup();

  smtkTest(numGroups == 4, "Number of Groups is : " << numGroups << " should have been 4");
  smtkTest(
    numItemsPerGroup == 2,
    "Number Items with a Groups is : " << numItemsPerGroup << " should have been 2");

  for (i = 0; i < numGroups; i++)
  {
    for (j = 0; j < numItemsPerGroup; j++)
    {
      auto item = gitem->item(i, j);
      if (att->itemAtPath(item->path()) != item)
      {
        std::cerr << " Failed to find Item: " << item->name() << " Item's path: " << item->path()
                  << std::endl;
        status = -1;
      }
      else
      {
        std::cerr << "Item: " << item->name() << "'s path:" << item->path() << " worked!\n";
      }
    }
  }

  return status;
}
