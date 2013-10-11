/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/

#include "smtk/attribute/Manager.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/AttributeRefItemDefinition.h"
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
#include "smtk/util/AttributeWriter.h"
#include "smtk/util/Logger.h"

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
  smtk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  // Lets add some analyses
  std::set<std::string> analysis;
  analysis.insert("Flow");
  analysis.insert("General");
  analysis.insert("Time");
  manager.defineAnalysis("CFD Flow", analysis);
  analysis.clear();

  analysis.insert("Flow");
  analysis.insert("Heat");
  analysis.insert("General");
  analysis.insert("Time");
  manager.defineAnalysis("CFD Flow with Heat Transfer", analysis);
  analysis.clear();

  analysis.insert("Constituent");
  analysis.insert("General");
  analysis.insert("Time");
  manager.defineAnalysis("Constituent Transport", analysis);
  analysis.clear();

  // Lets create an attribute to represent an expression
  smtk::AttributeDefinitionPtr expDef = manager.createDefinition("ExpDef");
  expDef->setBriefDescription("Sample Expression");
  expDef->setDetailedDescription("Sample Expression for testing\nThere is not much here!");
  smtk::StringItemDefinitionPtr eitemdef = 
    expDef->addItemDefinition<smtk::StringItemDefinitionPtr>("Expression String");
  smtk::StringItemDefinitionPtr eitemdef2 =
    expDef->addItemDefinition<smtk::attribute::StringItemDefinition>("Aux String");
  eitemdef->setDefaultValue("sample");

  smtk::AttributeDefinitionPtr base = manager.createDefinition("BaseDef");
  // Lets add some item definitions
  smtk::IntItemDefinitionPtr iitemdef = 
    base->addItemDefinition<smtk::IntItemDefinitionPtr>("TEMPORAL");
  iitemdef->setCommonValueLabel("Time");
  iitemdef->addDiscreteValue(0, "Seconds");
  iitemdef->addDiscreteValue(1, "Minutes");
  iitemdef->addDiscreteValue(2, "Hours");
  iitemdef->addDiscreteValue(3, "Days");
  iitemdef->setDefaultDiscreteIndex(0);
  iitemdef->addCategory("Time");
  iitemdef = 
    base->addItemDefinition<smtk::IntItemDefinitionPtr>("IntItem2");
  iitemdef->setDefaultValue(10);
  iitemdef->addCategory("Heat");

  smtk::AttributeDefinitionPtr def1 = manager.createDefinition("Derived1", "BaseDef");
  def1->setAssociationMask(0x20); // belongs on domains
   // Lets add some item definitions
  smtk::DoubleItemDefinitionPtr ditemdef = 
    def1->addItemDefinition<smtk::DoubleItemDefinitionPtr>("DoubleItem1");
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
    def1->addItemDefinition<smtk::DoubleItemDefinitionPtr>("DoubleItem2");
  ditemdef->setDefaultValue(-35.2);
  ditemdef->setMinRange(-100, true);
  ditemdef->setMaxRange(125.0, false);
  ditemdef->addCategory("Constituent");
  smtk::VoidItemDefinitionPtr vdef = 
    def1->addItemDefinition<smtk::VoidItemDefinitionPtr>("VoidItem");
  vdef->setIsOptional(true);
  vdef->setLabel("Option 1");
 

  smtk::AttributeDefinitionPtr def2 = manager.createDefinition("Derived2", "Derived1");
  def2->setAssociationMask(0x7);
   // Lets add some item definitions
  smtk::StringItemDefinitionPtr sitemdef = 
    def2->addItemDefinition<smtk::StringItemDefinitionPtr>("StringItem1");
  sitemdef->setIsMultiline(true);
  sitemdef->addCategory("Flow");
  sitemdef = 
    def2->addItemDefinition<smtk::StringItemDefinitionPtr>("StringItem2");
  sitemdef->setDefaultValue("Default");
  sitemdef->addCategory("General");
  smtk::DirectoryItemDefinitionPtr dirdef =
    def2->addItemDefinition<smtk::DirectoryItemDefinitionPtr>("DirectoryItem");
  dirdef->setShouldExist(true);
  dirdef->setShouldBeRelative(true);
  smtk::FileItemDefinitionPtr fdef =
    def2->addItemDefinition<smtk::FileItemDefinitionPtr>("FileItem");
  fdef->setShouldBeRelative(true);
  smtk::GroupItemDefinitionPtr gdef1, gdef =
    def2->addItemDefinition<smtk::GroupItemDefinitionPtr>("GroupItem");
  gdef->addItemDefinition<smtk::FileItemDefinitionPtr>("File1");
  gdef1 = gdef->addItemDefinition<smtk::GroupItemDefinitionPtr>("SubGroup");
  sitemdef = 
    gdef1->addItemDefinition<smtk::StringItemDefinitionPtr>("GroupString");
  sitemdef->setDefaultValue("Something Cool");
  sitemdef->addCategory("General");
  sitemdef->addCategory("Flow");

  // Add in a Attribute definition with a reference to another attribute
  smtk::AttributeDefinitionPtr attrefdef = manager.createDefinition("AttributeReferenceDef");
  smtk::AttributeRefItemDefinitionPtr aritemdef =
    attrefdef->addItemDefinition<smtk::AttributeRefItemDefinitionPtr>("BaseDefItem");
  aritemdef->setCommonValueLabel("A reference to another attribute");
  aritemdef->setAttributeDefinition(base);

  // Process Categories
  manager.updateCategories();
  // Lets test creating an attribute by passing in the expression definition explicitly
  smtk::AttributePtr expAtt = manager.createAttribute("Exp1", expDef);
  smtk::AttributePtr att = manager.createAttribute("testAtt", "Derived2");
  if (att == NULL)
    {
    std::cout << "ERROR: Attribute testAtt not created\n";
    status = -1;
    }

  smtk::ValueItemPtr vitem;
  smtk::AttributeItemPtr item;

  // Find the expression enabled item
  item = att->item(2);
  vitem = smtk::dynamicCastPointer<smtk::attribute::ValueItem>(item);
  smtk::util::AttributeWriter writer;
  smtk::util::Logger logger;
  if (writer.write(manager, argv[1],logger))
    {
    std::cerr << "Errors encountered creating Attribute File:\n";
    std::cerr << logger.convertToString();
    }
  std::cout << "Manager destroyed\n";
  }
  return status;
}
