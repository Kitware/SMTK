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
#include <iostream>

int main()
{
  int status;
  {
  typedef slctk::attribute::IntItemDefinition IntItemDef;
  typedef slctk::attribute::DoubleItemDefinition DoubleItemDef;
  typedef slctk::attribute::StringItemDefinition StringItemDef;
  typedef slctk::attribute::ValueItem ValueItem;
  typedef slctk::attribute::Item AttItem;

  slctk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  // Lets create an attribute to represent an expression
  slctk::AttributeDefinitionPtr expDef = manager.createDefinition("ExpDef");
  slctk::StringItemDefinitionPtr eitemdef(new StringItemDef("Expression String"));
  expDef->addItemDefinition(eitemdef);

  slctk::AttributeDefinitionPtr base = manager.createDefinition("BaseDef");
  // Lets add some item definitions
  slctk::IntItemDefinitionPtr iitemdef(new IntItemDef("IntItem1"));
  base->addItemDefinition(iitemdef);
  slctk::IntItemDefinitionPtr iitemdef2(new IntItemDef("IntItem2"));
  iitemdef2->setDefaultValue(10);
  base->addItemDefinition(iitemdef2);

  slctk::AttributeDefinitionPtr def1 = manager.createDefinition("Derived1", "BaseDef");
   // Lets add some item definitions
  slctk::DoubleItemDefinitionPtr ditemdef(new DoubleItemDef("DoubleItem1"));
  // Allow this one to hold an expression
  ditemdef->setExpressionDefinition(expDef);
  // Check to make sure we can use expressions
  if (!ditemdef->allowsExpressions())
    {
    std::cout << "ERROR - Item Def does not allow expressions\n";
    status = -1;
    }
  def1->addItemDefinition(ditemdef);
  slctk::DoubleItemDefinitionPtr ditemdef2(new DoubleItemDef("DoubleItem2"));
  ditemdef2->setDefaultValue(-35.2);
  def1->addItemDefinition(ditemdef2);

  slctk::AttributeDefinitionPtr def2 = manager.createDefinition("Derived2", "Derived1");
   // Lets add some item definitions
  slctk::StringItemDefinitionPtr sitemdef(new StringItemDef("StringItem1"));
  def1->addItemDefinition(sitemdef);
  slctk::StringItemDefinitionPtr sitemdef2(new StringItemDef("StringItem2"));
  sitemdef2->setDefaultValue("Default");
  def1->addItemDefinition(sitemdef2);

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
  vitem = slctk::dynamicCastPointer<ValueItem>(item);
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
    vitem = slctk::dynamicCastPointer<ValueItem>(item);
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
            std::cout << " Value = " << vitem->valueAsString("%g") << "\n";
            break;
          case AttItem::INT:
            std::cout << " Value = " << vitem->valueAsString("%d") << "\n";
            break;
          case AttItem::STRING:
            std::cout << vitem->valueAsString(" String Val = %s") << "\n";
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
