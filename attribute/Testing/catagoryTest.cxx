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

#include "attribute/Manager.h"
#include "attribute/Definition.h"
#include "attribute/Attribute.h"
#include "attribute/IntItem.h"
#include "attribute/IntItemDefinition.h"
#include "attribute/DoubleItem.h"
#include "attribute/DoubleItemDefinition.h"
#include "attribute/StringItem.h"
#include "attribute/StringItemDefinition.h"
#include "attribute/XmlV1StringWriter.h"

#include <iostream>

int main()
{
  int status;
  {
  slctk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  // Lets create an attribute to represent an expression
  slctk::AttributeDefinitionPtr expDef = manager.createDefinition("ExpDef");
  expDef->setBriefDescription("Sample Expression");
  expDef->setDetailedDescription("Sample Expression for testing\nThere is not much here!");
  slctk::StringItemDefinitionPtr eitemdef = 
    expDef->addItemDefinition<slctk::StringItemDefinitionPtr>("Expression String");
  slctk::StringItemDefinitionPtr eitemdef2 =
    expDef->addItemDefinition<slctk::attribute::StringItemDefinition>("Aux String");
  eitemdef->setDefaultValue("sample");

  slctk::AttributeDefinitionPtr base = manager.createDefinition("BaseDef");
  // Lets add some item definitions
  slctk::IntItemDefinitionPtr iitemdef = 
    base->addItemDefinition<slctk::IntItemDefinitionPtr>("IntItem1");
  iitemdef->addCatagory("Flow");
  iitemdef = 
    base->addItemDefinition<slctk::IntItemDefinitionPtr>("IntItem2");
  iitemdef->setDefaultValue(10);
  iitemdef->addCatagory("Heat");

  slctk::AttributeDefinitionPtr def1 = manager.createDefinition("Derived1", "BaseDef");
   // Lets add some item definitions
  slctk::DoubleItemDefinitionPtr ditemdef = 
    def1->addItemDefinition<slctk::DoubleItemDefinitionPtr>("DoubleItem1");
  // Allow this one to hold an expression
  ditemdef->addCatagory("Veg");
  ditemdef->setExpressionDefinition(expDef);
  // Check to make sure we can use expressions
  if (!ditemdef->allowsExpressions())
    {
    std::cout << "ERROR - Item Def does not allow expressions\n";
    status = -1;
    }
  ditemdef = 
    def1->addItemDefinition<slctk::DoubleItemDefinitionPtr>("DoubleItem2");
  ditemdef->setDefaultValue(-35.2);
  ditemdef->addCatagory("Constituent");

  slctk::AttributeDefinitionPtr def2 = manager.createDefinition("Derived2", "Derived1");
   // Lets add some item definitions
  slctk::StringItemDefinitionPtr sitemdef = 
    def2->addItemDefinition<slctk::StringItemDefinitionPtr>("StringItem1");
  sitemdef->addCatagory("Flow");
  sitemdef = 
    def2->addItemDefinition<slctk::StringItemDefinitionPtr>("StringItem2");
  sitemdef->setDefaultValue("Default");
  sitemdef->addCatagory("General");

  // Process Catagories
  manager.updateCatagories();
  // Lets see what catagories the attribute definitions think they are
  if (expDef->numberOfCatagories())
    {
    const std::set<std::string> &catagories = expDef->catagories();
    std::set<std::string>::const_iterator it;
    std::cout << "ERROR: ExpDef's catagories: ";
    for (it = catagories.begin(); it != catagories.end(); it++)
      {
      std::cout << "\""<< (*it) << "\" ";
      }
    std::cout << "\n";
    }
  else
    {
    std::cout << "ExpDef has no catagories\n";
    }
  if (def2->numberOfCatagories())
    {
    const std::set<std::string> &catagories = def2->catagories();
    std::set<std::string>::const_iterator it;
    std::cout << "Def2's catagories: ";
    for (it = catagories.begin(); it != catagories.end(); it++)
      {
      std::cout << "\""<< (*it) << "\" ";
      }
    std::cout << "\n";
    }
  else
    {
    std::cout << "ERROR: Def2 has no catagories!\n";
    }
  // Lets test creating an attribute by passing in the expression definition explicitly
  slctk::AttributePtr expAtt = manager.createAttribute("Exp1", expDef);
  slctk::AttributePtr att = manager.createAttribute("testAtt", "Derived2");
  if (att != NULL)
    {
    std::cout << "Attribute testAtt created\n";
    }
  else
    {
    std::cout << "ERROR: Attribute testAtt not created\n";
    status = -1;
    }

  slctk::ValueItemPtr vitem;
  slctk::AttributeItemPtr item;

  // Find the expression enabled item
  item = att->item(2);
  vitem = slctk::dynamicCastPointer<slctk::attribute::ValueItem>(item);
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
    std::cout << "\t" << item->name() << " Type = " << slctk::attribute::Item::type2String(item->type()) << ", ";
    vitem = slctk::dynamicCastPointer<slctk::attribute::ValueItem>(item);
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
  slctk::attribute::XmlV1StringWriter writer(manager);
  std::cout << writer.convertToString() << std::endl;
  std::cout << "Manager destroyed\n";
  }
  return status;
}
