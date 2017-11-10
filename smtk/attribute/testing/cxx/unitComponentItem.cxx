//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ComponentItemDefinition.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::attribute;

int unitComponentItem(int, char** const)
{
  // Instantiate definition
  auto compDef = ComponentItemDefinition::New("comp-def");
  smtkTest(!!compDef, "Failed to instantiate ComponentItemDefinition");
  smtkTest(compDef->type() == Item::ComponentType, "Failed to return COMPONENT as definition type");

  // Instantiate att collection, attdef, & attribute
  CollectionPtr collection = Collection::create();
  DefinitionPtr attDef = collection->createDefinition("test-component");
  attDef->addItemDefinition(compDef);
  AttributePtr att = collection->createAttribute(attDef);

  ItemPtr item = att->find("comp-def");
  smtkTest(!!item, "Failed to find Item");
  smtkTest(item->type() == Item::ComponentType, "Failed to return COMPONENT as item type");

  ComponentItemPtr compItem = att->findComponent("comp-def");
  smtkTest(!!compItem, "Failed to find ComponentItem");
  smtkTest(
    compItem->type() == Item::ComponentType, "Failed to return COMPONENT as ComponentItem type");
  smtkTest(!compItem->isSet(), "isSet() failed to return false for default item");

  return 0;
}
