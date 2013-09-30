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
#include "smtk/attribute/AttributeDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/XmlV1StringWriter.h"

#include <iostream>

int main()
{
  int status = 0;
  {
  smtk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  // Lets create an attribute to represent an expression
  smtk::AttributeDefinitionPtr expDef = manager.createAttributeDefinition("ExpDef");
  expDef->setBriefDescription("Sample Expression");
  expDef->setDetailedDescription("Sample Expression for testing\nThere is not much here!");
  smtk::StringItemDefinitionPtr eitemdef = 
    expDef->addItemDefinition<smtk::StringItemDefinitionPtr>("Expression String");
  smtk::StringItemDefinitionPtr eitemdef2 =
    expDef->addItemDefinition<smtk::attribute::StringItemDefinition>("Aux String");
  eitemdef->setDefaultValue("sample");

  smtk::AttributeDefinitionPtr base = manager.createAttributeDefinition("BaseDef");
  // Lets add some item definitions
  smtk::IntItemDefinitionPtr iitemdef = 
    base->addItemDefinition<smtk::IntItemDefinitionPtr>("IntItem1");
  iitemdef->addCategory("Flow");
  iitemdef = 
    base->addItemDefinition<smtk::IntItemDefinitionPtr>("IntItem2");
  iitemdef->setDefaultValue(10);
  iitemdef->addCategory("Heat");

  smtk::AttributeDefinitionPtr def1 = manager.createAttributeDefinition("Derived1", "BaseDef");
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
  ditemdef->addCategory("Constituent");

  smtk::AttributeDefinitionPtr def2 = manager.createAttributeDefinition("Derived2", "Derived1");
   // Lets add some item definitions
  smtk::StringItemDefinitionPtr sitemdef = 
    def2->addItemDefinition<smtk::StringItemDefinitionPtr>("StringItem1");
  sitemdef->addCategory("Flow");
  sitemdef = 
    def2->addItemDefinition<smtk::StringItemDefinitionPtr>("StringItem2");
  sitemdef->setDefaultValue("Default");
  sitemdef->addCategory("General");

  // Process Categories
  manager.updateCategories();
  // Lets see what categories the attribute definitions think they are
  if (expDef->numberOfCategories())
    {
    const std::set<std::string> &categories = expDef->categories();
    std::set<std::string>::const_iterator it;
    std::cout << "ERROR: ExpDef's categories: ";
    for (it = categories.begin(); it != categories.end(); it++)
      {
      std::cout << "\""<< (*it) << "\" ";
      }
    std::cout << "\n";
    }
  else
    {
    std::cout << "ExpDef has no categories\n";
    }
  if (def2->numberOfCategories())
    {
    const std::set<std::string> &categories = def2->categories();
    std::set<std::string>::const_iterator it;
    std::cout << "Def2's categories: ";
    for (it = categories.begin(); it != categories.end(); it++)
      {
      std::cout << "\""<< (*it) << "\" ";
      }
    std::cout << "\n";
    }
  else
    {
    std::cout << "ERROR: Def2 has no categories!\n";
    }
  // Lets test creating an attribute by passing in the expression definition explicitly
  smtk::AttributePtr expAtt = manager.createAttribute("Exp1", expDef);
  smtk::AttributePtr att = manager.createAttribute("testAtt", "Derived2");
  if (att != NULL)
    {
    std::cout << "Attribute testAtt created\n";
    }
  else
    {
    std::cout << "ERROR: Attribute testAtt not created\n";
    status = -1;
    }

  smtk::ValueItemPtr vitem;
  smtk::AttributeItemPtr item;

  // Find the expression enabled item
  item = att->item(2);
  vitem = smtk::dynamicCastPointer<smtk::attribute::ValueItem>(item);
  if (vitem->allowsExpressions())
    {
    vitem->setExpression(expAtt);
    std::cout << "Expression Set on " << vitem->name() << "\n";
    }
  else
    {
    std::cout << "ERROR: Can not set expression on " << vitem->name() << "\n";
    status = -1;
    }
  
  int i, n = att->numberOfItems();
  std::cout << "Items of testAtt:\n";
  for (i = 0; i < n; i++)
    {
    item = att->item(i);
    std::cout << "\t" << item->name() << " Type = " << smtk::attribute::Item::type2String(item->type()) << ", ";
    vitem = smtk::dynamicCastPointer<smtk::attribute::ValueItem>(item);
    if (vitem != NULL)
      {
      if (vitem->isExpression())
        {
        std::cout << " using Expression: " << vitem->expression()->name() << "\n";
        }
      else
        {
        std::cout << " Value = " << vitem->valueAsString() << "\n";
        }
      }
    }
  smtk::attribute::XmlV1StringWriter writer(manager);
  std::cout << writer.convertToString() << std::endl;
  std::cout << "Manager destroyed\n";
  }
  return status;
}
