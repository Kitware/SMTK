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
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/model/EntityTypeBits.h"

#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include <iostream>

int main(int argc, char *argv[])
{
  int status = 0;
  {
  if (argc != 2)
    {
    std::cerr << "Usage: " << argv[0] << " filename\n";
    return -1;
    }
  smtk::attribute::System system;
  std::cout << "System Created\n";
  // Lets add some analyses
  std::set<std::string> analysis;
  analysis.insert("Flow");
  analysis.insert("General");
  analysis.insert("Time");
  system.defineAnalysis("CFD Flow", analysis);
  analysis.clear();

  analysis.insert("Flow");
  analysis.insert("Heat");
  analysis.insert("General");
  analysis.insert("Time");
  system.defineAnalysis("CFD Flow with Heat Transfer", analysis);
  analysis.clear();

  analysis.insert("Constituent");
  analysis.insert("General");
  analysis.insert("Time");
  system.defineAnalysis("Constituent Transport", analysis);
  analysis.clear();

  // Lets create an attribute to represent an expression
  smtk::attribute::DefinitionPtr expDef = system.createDefinition("ExpDef");
  expDef->setBriefDescription("Sample Expression");
  expDef->setDetailedDescription("Sample Expression for testing\nThere is not much here!");
  smtk::attribute::StringItemDefinitionPtr eitemdef =
    expDef->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("Expression String");
  smtk::attribute::StringItemDefinitionPtr eitemdef2 =
    expDef->addItemDefinition<smtk::attribute::StringItemDefinition>("Aux String");
  eitemdef->setDefaultValue("sample");

  smtk::attribute::DefinitionPtr base = system.createDefinition("BaseDef");
  // Lets add some item definitions
  smtk::attribute::IntItemDefinitionPtr iitemdef =
    base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("TEMPORAL");
  iitemdef->setLabel("Time");
  iitemdef->addDiscreteValue(0, "Seconds");
  iitemdef->addDiscreteValue(1, "Minutes");
  iitemdef->addDiscreteValue(2, "Hours");
  iitemdef->addDiscreteValue(3, "Days");
  iitemdef->setDefaultDiscreteIndex(0);
  iitemdef->addCategory("Time");
  iitemdef =
    base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("IntItem2");
  iitemdef->setDefaultValue(10);
  iitemdef->addCategory("Heat");

  smtk::attribute::DefinitionPtr def1 = system.createDefinition("Derived1", "BaseDef");
  def1->setAssociationMask(smtk::model::MODEL_ENTITY); // belongs on model
   // Lets add some item definitions
  smtk::attribute::DoubleItemDefinitionPtr ditemdef =
    def1->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("DoubleItem1");
  // Allow this one to hold an expression
  ditemdef->addCategory("Veg");
  ditemdef->setExpressionDefinition(expDef);
  // Check to make sure we can use expressions
  if (!ditemdef->allowsExpressions())
    {
    std::cout << "ERROR - Item Def does not allow expressions\n";
    status = -1;
    }
  ditemdef =
    def1->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("DoubleItem2");
  //Lets add some children
  ditemdef->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("Child1");
  ditemdef->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("Child2");
  ditemdef->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("Child3");
  ditemdef->addCategory("Constituent");
  ditemdef->addDiscreteValue(0, "A");
  ditemdef->addDiscreteValue(1, "B");
  ditemdef->addDiscreteValue(2, "C");

  ditemdef->addConditionalItem("A", "Child1");
  ditemdef->addConditionalItem("A", "Child3");
  ditemdef->addConditionalItem("B", "Child3");
  ditemdef->addConditionalItem("B", "Child2");
  ditemdef->setDefaultDiscreteIndex(0);

  smtk::attribute::VoidItemDefinitionPtr vdef =
    def1->addItemDefinition<smtk::attribute::VoidItemDefinitionPtr>("VoidItem");
  vdef->setIsOptional(true);
  vdef->setLabel("Option 1");


  smtk::attribute::DefinitionPtr def2 = system.createDefinition("Derived2", "Derived1");
  def2->setAssociationMask(smtk::model::VOLUME);
   // Lets add some item definitions
  smtk::attribute::StringItemDefinitionPtr sitemdef =
    def2->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("StringItem1");
  sitemdef->setIsMultiline(true);
  sitemdef->addCategory("Flow");
  sitemdef =
    def2->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("StringItem2");
  sitemdef->setDefaultValue("Default");
  sitemdef->addCategory("General");
  smtk::attribute::DirectoryItemDefinitionPtr dirdef =
    def2->addItemDefinition<smtk::attribute::DirectoryItemDefinitionPtr>("DirectoryItem");
  dirdef->setShouldExist(true);
  dirdef->setShouldBeRelative(true);
  smtk::attribute::FileItemDefinitionPtr fdef =
    def2->addItemDefinition<smtk::attribute::FileItemDefinitionPtr>("FileItem");
  fdef->setShouldBeRelative(true);
  smtk::attribute::GroupItemDefinitionPtr gdef1, gdef =
    def2->addItemDefinition<smtk::attribute::GroupItemDefinitionPtr>("GroupItem");
  gdef->addItemDefinition<smtk::attribute::FileItemDefinitionPtr>("File1");
  gdef1 = gdef->addItemDefinition<smtk::attribute::GroupItemDefinitionPtr>("SubGroup");
  sitemdef =
    gdef1->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("GroupString");
  sitemdef->setDefaultValue("Something Cool");
  sitemdef->addCategory("General");
  sitemdef->addCategory("Flow");

  // Add in a Attribute definition with a reference to another attribute
  smtk::attribute::DefinitionPtr attrefdef = system.createDefinition("AttributeReferenceDef");
  smtk::attribute::RefItemDefinitionPtr aritemdef =
    attrefdef->addItemDefinition<smtk::attribute::RefItemDefinitionPtr>("BaseDefItem");
  aritemdef->setNumberOfRequiredValues(1);
  aritemdef->setCommonValueLabel("A reference to another attribute");
  aritemdef->setAttributeDefinition(base);

  // Process Categories
  system.updateCategories();
  // Lets test creating an attribute by passing in the expression definition explicitly
  smtk::attribute::AttributePtr expAtt = system.createAttribute("Exp1", expDef);
  smtk::attribute::AttributePtr att = system.createAttribute("testAtt", "Derived2");
  if (!att)
    {
    std::cout << "ERROR: Attribute testAtt not created\n";
    status = -1;
    }

  // Lets Test out the conditional items
  smtk::attribute::ValueItemPtr vitem =
    smtk::dynamic_pointer_cast<smtk::attribute::ValueItem>(att->find("DoubleItem2"));

  smtk::io::Logger logger;
  if (!vitem)
    {
    smtkErrorMacro(logger, "Error: Can't find DoubleItem2");
    }
  else
    {
    // Should Have 2 children Child1 and Child3 by default
    if (vitem->numberOfActiveChildrenItems() != 2)
      {
      smtkErrorMacro(logger, "Error:DoubleItem2 = A has " << vitem->numberOfActiveChildrenItems()
                     << " active children");
      }
    else
      {
      if (vitem->activeChildItem(0)->name() != "Child1")
        {
        smtkErrorMacro(logger, "Error:DoubleItem2 = A 0th child is " << vitem->activeChildItem(0)->name());
        }
      if (vitem->activeChildItem(1)->name() != "Child3")
        {
        smtkErrorMacro(logger, "Error:DoubleItem2 = A 0th child is " << vitem->activeChildItem(1)->name());
        }
      }
    }
  // Lets Change it to be using "B"
  vitem->setDiscreteIndex(1);
  if (vitem->numberOfActiveChildrenItems() != 2)
    {
    smtkErrorMacro(logger, "Error:DoubleItem2 has " << vitem->numberOfActiveChildrenItems()
                   << " active children");
    }
  else
    {
    if (vitem->activeChildItem(0)->name() != "Child3")
      {
      smtkErrorMacro(logger, "Error:DoubleItem2 = B 0th child is " << vitem->activeChildItem(0)->name());
      }
    if (vitem->activeChildItem(1)->name() != "Child2")
      {
      smtkErrorMacro(logger, "Error:DoubleItem2 = B 0th child is " << vitem->activeChildItem(1)->name());
      }
    }
  // Lets Change it to be using "C"
  vitem->setDiscreteIndex(1);
  if (vitem->numberOfActiveChildrenItems() != 0)
    {
    smtkErrorMacro(logger, "Error:DoubleItem2 = C has " << vitem->numberOfActiveChildrenItems()
                   << " active children");
    }

    smtk::attribute::ItemPtr item;

  // Find the expression enabled item
  item = att->item(2);
  vitem = smtk::dynamic_pointer_cast<smtk::attribute::ValueItem>(item);
  smtk::io::AttributeWriter writer;
  if (writer.write(system, argv[1],logger))
    {
    std::cerr << "Errors encountered creating Attribute File:\n";
    std::cerr << logger.convertToString();
    status = -1;
    }
  std::cout << "System destroyed\n";
  }
  return status;
}
