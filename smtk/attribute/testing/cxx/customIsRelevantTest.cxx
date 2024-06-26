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
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include <iostream>

using namespace smtk::attribute;

bool customIsRelevant(
  const Item* item,
  bool includeCategories,
  bool includeReadAccess,
  unsigned int readAccessLevel)
{
  // This function will add an extra relevance constraint indicating that
  // this item is relevant only if item b's value is between 0 and 10 inclusive

  // find Item b
  if (item)
  {
    auto att = item->attribute();
    if (att)
    {
      auto itemB = att->findAs<DoubleItem>("b");
      if (itemB)
      {
        if (!itemB->isSet())
        {
          // Item b has not value
          return false;
        }
        double val = itemB->value();
        if ((val < 0) || (val > 10))
        {
          return false;
        }
      }
    }
  }
  return item->defaultIsRelevant(includeCategories, includeReadAccess, readAccessLevel);
}

bool customEnumIsRelevant(
  const Item* item,
  int enumIndex,
  bool includeCategories,
  const std::set<std::string>& testCategories,
  bool includeReadAccess,
  unsigned int readAccessLevel)
{
  // This function will add an extra relevance constraint indicating that
  // the "alpha" value is relevant only if item b's value is between 0 and 10 inclusive

  const auto def = std::dynamic_pointer_cast<const ValueItemDefinition>(item->definition());
  if (enumIndex == 0) // Alpha's index
  {
    // find Item b
    auto att = item->attribute();
    if (att)
    {
      auto itemB = att->findAs<DoubleItem>("b");
      if (itemB)
      {
        if (!itemB->isSet())
        {
          // Item b has not value
          return false;
        }
        double val = itemB->value();
        if ((val < 0) || (val > 10))
        {
          return false;
        }
      }
    }
  }
  return def->defaultIsEnumRelevant(
    enumIndex, includeCategories, testCategories, includeReadAccess, readAccessLevel);
}

int main()
{
  int status = 0;

  smtk::attribute::ResourcePtr resource = Resource::create();
  std::cerr << "Resource Created\n";

  // Lets create an attribute with 2 Double Items
  smtk::attribute::DefinitionPtr attDef = resource->createDefinition("Test");
  auto adef = attDef->addItemDefinition<DoubleItemDefinitionPtr>("a");
  auto bdef = attDef->addItemDefinition<DoubleItemDefinitionPtr>("b");
  auto cdef = attDef->addItemDefinition<IntItemDefinitionPtr>("c");

  // Lets set some discrete values for Item c
  cdef->addDiscreteValue(1, "alpha");
  cdef->addDiscreteValue(2, "beta");

  auto att = resource->createAttribute("testAtt", attDef);
  if (att)
  {
    std::cerr << "Attribute testAtt created\n";
  }
  else
  {
    std::cerr << "ERROR: Attribute testAtt not created\n";
    status = -1;
  }

  // By default, if we ignore categories and read access, the attribute and its two items
  // should be considered relevant
  if (att->isRelevant(false, false))
  {
    std::cerr << "Initial Test: testAtt is Relevant\n";
  }
  else
  {
    std::cerr << "ERROR: Initial Test: testAtt is NOT Relevant\n";
    status = -1;
  }

  // Lets grab the discrete item
  auto discreteItem = std::dynamic_pointer_cast<IntItem>(att->item(2));
  for (int i = 0; i < 3; i++)
  {
    if (att->item(i)->isRelevant(false, false))
    {
      std::cerr << "Initial Test: Item " << att->item(i)->name() << " is Relevant\n";
    }
    else
    {
      std::cerr << "ERROR: Initial Test: Item " << att->item(i)->name() << " is NOT Relevant\n";
      status = -1;
    }
  }

  // Lets test the discrete Item's enums - if we ignore categories and read access there should be 2
  auto relevantEnums = discreteItem->relevantEnums(false, false, 0);
  if (relevantEnums.size() != 2)
  {
    std::cerr << "ERROR: Initial Test: Item " << discreteItem->name()
              << " has incorrect relevant enums values : ";
    status = -1;
  }
  else
  {
    std::cerr << "Initial Test: Item " << discreteItem->name()
              << " has correct relevant enum values : ";
  }

  for (const auto& estring : relevantEnums)
  {
    std::cerr << estring << " ";
  }
  std::cerr << std::endl;

  // Now lets give Item a the custom relevance function
  att->item(0)->setCustomIsRelevant(customIsRelevant);

  // Let give Item c's definition a custom enum relevance function
  cdef->setCustomEnumIsRelevant(customEnumIsRelevant);

  // Since b is not currently set, a should not be relevant and discreteItem should have only one relevant enum
  if (!att->item(0)->isRelevant(false, false))
  {
    std::cerr << "Pass Item b not set: Item " << att->item(0)->name() << " is not Relevant\n";
  }
  else
  {
    std::cerr << "ERROR: Pass Item b not set: Item " << att->item(0)->name() << " IS Relevant\n";
    status = -1;
  }

  relevantEnums = discreteItem->relevantEnums(false, false, 0);
  if (!((relevantEnums.size() == 1) && (relevantEnums[0] == "beta")))
  {
    std::cerr << "ERROR: Item b not set: Item " << discreteItem->name()
              << " has incorrect relevant enums values : ";
    status = -1;
  }
  else
  {
    std::cerr << "Initial Item b not set: Item " << discreteItem->name()
              << " has correct relevant enum values : ";
  }

  for (const auto& estring : relevantEnums)
  {
    std::cerr << estring << " ";
  }
  std::cerr << std::endl;

  // now lets set b to 5 which should make a relevant as well as both of the discreteItem's enums
  auto itemB = att->findAs<DoubleItem>("b");
  itemB->setValue(5.0);

  if (att->item(0)->isRelevant(false, false))
  {
    std::cerr << "Pass Item b set to 5: Item " << att->item(0)->name() << " is Relevant\n";
  }
  else
  {
    std::cerr << "ERROR: Pass Item b set to 5: Item " << att->item(0)->name()
              << " is NOT Relevant\n";
    status = -1;
  }

  relevantEnums = discreteItem->relevantEnums(false, false, 0);
  if (relevantEnums.size() != 2)
  {
    std::cerr << "ERROR: Item b set to 5: Item " << discreteItem->name()
              << " has incorrect relevant enums values : ";
    status = -1;
  }
  else
  {
    std::cerr << "Initial Item b set to 5: Item " << discreteItem->name()
              << " has correct relevant enum values : ";
  }

  for (const auto& estring : relevantEnums)
  {
    std::cerr << estring << " ";
  }
  std::cerr << std::endl;

  return status;
}
