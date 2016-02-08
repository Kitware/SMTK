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

int main()
{
  int status = 0;
  {
  smtk::attribute::System system;
  std::cout << "System Created\n";
  smtk::attribute::DefinitionPtr def = system.createDefinition("testDef");
  if (def)
    {
    std::cout << "Definition testDef created\n";
    }
  else
    {
    std::cout << "ERROR: Definition testDef not created\n";
    status = -1;
    }
  // Lets add some item definitions
  smtk::attribute::IntItemDefinitionPtr icompdef = smtk::attribute::IntItemDefinition::New("IntComp1");
  icompdef->setAdvanceLevel(1);
  def->addItemDefinition(icompdef);
  smtk::attribute::IntItemDefinitionPtr icompdef2 = smtk::attribute::IntItemDefinition::New("IntComp2");
  icompdef2->setDefaultValue(10);
  icompdef2->setAdvanceLevel(0, 3);
  icompdef2->setAdvanceLevel(1, 4);

  def->addItemDefinition(icompdef2);
  smtk::attribute::DefinitionPtr def1 = system.createDefinition("testDef");
  if (!def1)
    {
    std::cout << "Duplicated definition testDef not created\n";
    }
  else
    {
    std::cout << "ERROR: Duplicated definition testDef created\n";
    status = -1;
    }
  smtk::attribute::AttributePtr att = system.createAttribute("testAtt", "testDef");
  if (att)
    {
    std::cout << "Attribute testAtt created\n";
    }
  else
    {
    std::cout << "ERROR: Attribute testAtt not created\n";
    status = -1;
    }

  if (att->numberOfItems() != 2)
    {
    std::cout << "ERROR: attribute has " << att->numberOfItems() << " items not 2\n";
    status = -1;
    }
  else if (att->item(0)->name() != "IntComp1")
    {
    std::cout << "ERROR: Attribute's 0th item is named " << att->item(0)->name()
              << " not IntComp1\n";
     status = -1;
   }
  else if (att->item(1)->name() != "IntComp2")
    {
    std::cout << "Error Attribute's 1st item is named " << att->item(0)->name()
              << " not IntComp2\n";
    status = -1;
    }
  else
    {
    smtk::attribute::IntItemPtr icptr;
    icptr = smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(att->item(0));
    // Lets make sure the advance level information is correct! - first the
    // item should be using the definition's advance level information
    if ((!icptr->usingDefinitionAdvanceLevel(0)) || (!icptr->usingDefinitionAdvanceLevel(1)))
      {
      std::cout << "ERROR: IntComp1 did not return the proper usingDefinitionAdvanceLevel Information\n";
      std::cout <<"\tusingDefinitionAdvanceLevel(0) = " << icptr->usingDefinitionAdvanceLevel(0)
                <<" usingDefinitionAdvanceLevel(1) = " << icptr->usingDefinitionAdvanceLevel(1) <<"\n";
      status = -1;
      }
    else
      {
      std::cout << "IntComp1 did return the proper usingDefinitionAdvanceLevel Information\n";
      }
    if ((icptr->advanceLevel(0) != 1) || (icptr->advanceLevel(1) != 1))
      {
      std::cout << "ERROR: IntComp1 did not return the proper advanceLevel Information\n";
      std::cout <<"\tadvanceLevel(0) = " << icptr->advanceLevel(0)
                <<" advanceLevel(1) = " << icptr->advanceLevel(1) <<"\n";
      status = -1;
      }
    else
      {
      std::cout << "IntComp1 did return the proper advanceLevel Information\n";
      }

    //Lets try changing the read and write values
    icptr->setAdvanceLevel(0, 10);
    icptr->setAdvanceLevel(1, 20);
    if (!(icptr->usingDefinitionAdvanceLevel(0)) && icptr->usingDefinitionAdvanceLevel(1))
      {
      std::cout << "ERROR: Pass 2: IntComp1 did not return the proper usingDefinitionAdvanceLevel Information\n";
      std::cout <<"\tusingDefinitionAdvanceLevel(0) = " << icptr->usingDefinitionAdvanceLevel(0)
                <<" usingDefinitionAdvanceLevel(1) = " << icptr->usingDefinitionAdvanceLevel(1) <<"\n";
      status = -1;
      }
    else
      {
      std::cout << "Pass2: IntComp1 did return the proper usingDefinitionAdvanceLevel Information\n";
      }
    if ((icptr->advanceLevel(0) != 10) || (icptr->advanceLevel(1) != 20))
      {
      std::cout << "ERROR: IntComp1 did not return the proper advanceLevel Information\n";
      std::cout <<"\tadvanceLevel(0) = " << icptr->advanceLevel(0)
                <<" advanceLevel(1) = " << icptr->advanceLevel(1) <<"\n";
      status = -1;
      }
    else
      {
      std::cout << "IntComp1 did return the proper advanceLevel Information\n";
      }

    std::cout << "Found IntComp1 - value = " << icptr->valueAsString() << std::endl;
    icptr = smtk::dynamic_pointer_cast<smtk::attribute::IntItem>(att->item(1));
    if ((icptr->advanceLevel(0) != 3) || (icptr->advanceLevel(1) != 4))
      {
      std::cout << "ERROR: IntComp2 did not return the proper advanceLevel Information\n";
      std::cout <<"\tadvanceLevel(0) = " << icptr->advanceLevel(0)
                <<" advanceLevel(1) = " << icptr->advanceLevel(1) <<"\n";
      status = -1;
      }
    else
      {
      std::cout << "IntComp2 did return the proper advanceLevel Information\n";
      }

    std::cout << "Found IntComp2 - value = " << icptr->valueAsString() << std::endl;
    }

  smtk::attribute::AttributePtr att1 = system.createAttribute("testAtt", "testDef");
  if (!att1)
    {
    std::cout << "Duplicate Attribute testAtt not created\n";
    }
  else
    {
    std::cout << "ERROR: Duplicate Attribute testAtt  created\n";
    status = -1;
    }

  //Lets test the RootName feature
  att1 = system.createAttribute("testDef");
  if (att1->name() != "testDef-0")
    {
    std::cout << "Name = " << att1->name() << "\n";
    std::cout << "ERROR: Creating Attribute without name test Failed\n";
    status = -1;
    }
  else
    {
    std::cout << "Created attribute with default name testDef-0\n";
    }
  def->setRootName("Foo");
  att1 = system.createAttribute("testDef");
  if (att1->name() != "Foo-0")
    {
    std::cout << "Name = " << att1->name() << "\n";
    std::cout << "ERROR: Creating Attribute using RootName test Failed\n";
    status = -1;
    }
  else
    {
    std::cout << "Created attribute with RootName name Foo-0\n";
    }
  
  std::cout << "System destroyed\n";
  }
  return status;
}
