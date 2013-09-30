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
#include <iostream>

int main()
{
  int status = 0;
  {
  typedef smtk::attribute::IntItemDefinition IntItemDef;
  typedef smtk::attribute::DoubleItemDefinition DoubleItemDef;
  typedef smtk::attribute::StringItemDefinition StringItemDef;
  typedef smtk::attribute::ValueItem ValueItem;
  typedef smtk::attribute::Item AttItem;

  smtk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  // Lets create an attribute to represent an expression
  smtk::AttributeDefinitionPtr expDef = manager.createAttributeDefinition("ExpDef");
  smtk::StringItemDefinitionPtr eitemdef = StringItemDef::New("Expression String");
  expDef->addItemDefinition(eitemdef);

  smtk::AttributeDefinitionPtr base = manager.createAttributeDefinition("BaseDef");
  // Lets add some item definitions
  smtk::IntItemDefinitionPtr iitemdef = IntItemDef::New("IntItem1");
  base->addItemDefinition(iitemdef);
  smtk::IntItemDefinitionPtr iitemdef2 = IntItemDef::New("IntItem2");
  iitemdef2->setDefaultValue(10);
  base->addItemDefinition(iitemdef2);

  smtk::AttributeDefinitionPtr def1 = manager.createAttributeDefinition("Derived1", "BaseDef");
   // Lets add some item definitions
  smtk::DoubleItemDefinitionPtr ditemdef = DoubleItemDef::New("DoubleItem1");
  // Allow this one to hold an expression
  ditemdef->setExpressionDefinition(expDef);
  // Check to make sure we can use expressions
  if (!ditemdef->allowsExpressions())
    {
    std::cout << "ERROR - Item Def does not allow expressions\n";
    status = -1;
    }
  def1->addItemDefinition(ditemdef);
  smtk::DoubleItemDefinitionPtr ditemdef2 = DoubleItemDef::New("DoubleItem2");
  ditemdef2->setDefaultValue(-35.2);
  def1->addItemDefinition(ditemdef2);

  smtk::AttributeDefinitionPtr def2 = manager.createAttributeDefinition("Derived2", "Derived1");
   // Lets add some item definitions
  smtk::StringItemDefinitionPtr sitemdef = StringItemDef::New("StringItem1");
  def1->addItemDefinition(sitemdef);
  smtk::StringItemDefinitionPtr sitemdef2 = StringItemDef::New("StringItem2");
  sitemdef2->setDefaultValue("Default");
  def1->addItemDefinition(sitemdef2);

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
  vitem = smtk::dynamicCastPointer<ValueItem>(item);
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
    std::cout << "\t" << item->name() << " Type = " << AttItem::type2String(item->type()) << ", ";
    vitem = smtk::dynamicCastPointer<ValueItem>(item);
    if (vitem != NULL)
      {
      if (vitem->isExpression())
        {
        std::cout << " using Expression: " << vitem->expression()->name() << "\n";
        }
      else
        {
        switch (vitem->type())
          {
          case AttItem::DOUBLE:
          case AttItem::INT:
            std::cout << " Value = "  << vitem->valueAsString() << std::endl;
            break;
          case AttItem::STRING:
            std::cout << " String Val = " << vitem->valueAsString() << std::endl;
            break;
          default:
            break;
          }
        }
      }
    }
  std::cout << "Manager destroyed\n";
  }
  return status;
}
