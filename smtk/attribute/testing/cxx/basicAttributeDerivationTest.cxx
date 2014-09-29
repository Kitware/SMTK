//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/System.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include <iostream>

std::string itemNames[] = {
  "IntComp1",
  "IntComp2",
  "DoubleComp1",
  "DoubleComp2",
  "StringComp1",
  "StringComp2"
};

int main()
{
  int status = 0;
  {
  typedef smtk::attribute::IntItemDefinition IntCompDef;
  typedef smtk::attribute::DoubleItemDefinition DoubleCompDef;
  typedef smtk::attribute::StringItemDefinition StringCompDef;
  typedef smtk::attribute::ValueItem ValueComp;
  typedef smtk::attribute::Item AttComp;

  smtk::attribute::System system;
  std::cout << "System Created\n";
  smtk::attribute::DefinitionPtr base = system.createDefinition("BaseDef");
  // Lets add some item definitions
  base->addItemDefinition<IntCompDef>(itemNames[0]);
  smtk::attribute::IntItemDefinitionPtr icompdef2 = base->addItemDefinition<IntCompDef>(itemNames[1]);
  icompdef2->setDefaultValue(10);

  smtk::attribute::DefinitionPtr def1 = system.createDefinition("Derived1", "BaseDef");
   // Lets add some item definitions
  def1->addItemDefinition<DoubleCompDef>(itemNames[2]);
  smtk::attribute::DoubleItemDefinitionPtr dcompdef2 = def1->addItemDefinition<DoubleCompDef>(itemNames[3]);
  dcompdef2->setDefaultValue(-35.2);

  smtk::attribute::DefinitionPtr def2 = system.createDefinition("Derived2", "Derived1");
   // Lets add some item definitions
  def2->addItemDefinition<StringCompDef>(itemNames[4]);
  smtk::attribute::StringItemDefinitionPtr scompdef2 = def2->addItemDefinition<StringCompDef>(itemNames[5]);
  scompdef2->setDefaultValue("Default");

  // Lets test out the find item position method
  int j, pstatus = 0;
  for (j =0; j < 6; j++)
    {
    if (def2->findItemPosition(itemNames[j]) != j)
      {
      std::cerr << "Incorrect Position Returned for " << itemNames[j]
                << ", position returned is " << def2->findItemPosition(itemNames[j])
                << ", but it should be " << j << "\n";
      pstatus = status = -1;
      }
    }
  if (!pstatus)
    {
    std::cout << "Initial Position Test Passed!\n";
    }
  else
    {
    std::cout << "Initial Position Test Failed!\n";
    }

  // Lets add a  component to the base def and verify that positions are reordered
  base->addItemDefinition<StringCompDef>("InsertStringItem");
  pstatus = 0;
  for (j =2; j < 6; j++)
    {
    if (def2->findItemPosition(itemNames[j]) != (j+1))
      {
      std::cerr << "Incorrect Position Returned for " << itemNames[j]
                << ", position returned is " << def2->findItemPosition(itemNames[j])
                << ", but it should be " << j+1 << "\n";
      pstatus = status = -1;
      }
    }

  if (!pstatus)
    {
    std::cout << "Insertion Position Test Passed!\n";
    }
  else
    {
    std::cout << "Insertion Position Test Failed!\n";
    }


  smtk::attribute::AttributePtr att = system.createAttribute("testAtt", "Derived2");
  if (att)
    {
    std::cout << "Attribute testAtt created\n";
    }
  else
    {
    std::cout << "ERROR: Attribute testAtt not created\n";
    status = -1;
    }

  smtk::attribute::ValueItemPtr vcomp;
  smtk::attribute::ItemPtr comp;

  //test the find of base def
  comp = att->find("DoubleComp1");
  if (comp)
    {
    vcomp = smtk::dynamic_pointer_cast<ValueComp>(comp);
    std::cout << " Value = "  << vcomp->valueAsString() << std::endl;
    }
  else
    {
    std::cout << "ERROR: could not find the base's item" << std::endl;
    status = -1;
    }

  int i, n = static_cast<int>(att->numberOfItems());
  std::cout << "Items of testAtt:\n";
  for (i = 0; i < n; i++)
    {
    comp = att->item(i);
    std::cout << "\t" << comp->name() << " Type = " << AttComp::type2String(comp->type()) << ", ";
    vcomp = smtk::dynamic_pointer_cast<ValueComp>(comp);
    if (vcomp)
      {
      switch (vcomp->type())
        {
        case AttComp::DOUBLE:
        case AttComp::INT:
          std::cout << " Value = "  << vcomp->valueAsString() << std::endl;
          break;
        case AttComp::STRING:
          std::cout << " String Val = " << vcomp->valueAsString() << std::endl;
          break;
        default:
          break;
        }
      }
    }
  std::cout << "System destroyed\n";
  }
  return status;
}
