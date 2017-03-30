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
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/System.h"
#include <iostream>

int main()
{
  int status = 0;
  {
    typedef smtk::attribute::IntItemDefinition IntItemDef;
    typedef smtk::attribute::DoubleItemDefinition DoubleItemDef;
    typedef smtk::attribute::StringItemDefinition StringItemDef;
    typedef smtk::attribute::DoubleItem DoubleItem;
    typedef smtk::attribute::ValueItem ValueItem;
    typedef smtk::attribute::Item AttItem;

    smtk::attribute::System system;
    std::cout << "System Created\n";
    // Lets create an attribute to represent an expression
    smtk::attribute::DefinitionPtr expDef = system.createDefinition("ExpDef");
    smtk::attribute::StringItemDefinitionPtr eitemdef = StringItemDef::New("Expression String");
    expDef->addItemDefinition(eitemdef);

    smtk::attribute::DefinitionPtr base = system.createDefinition("BaseDef");
    // Lets add some item definitions
    smtk::attribute::IntItemDefinitionPtr iitemdef = IntItemDef::New("IntItem1");
    base->addItemDefinition(iitemdef);
    smtk::attribute::IntItemDefinitionPtr iitemdef2 = IntItemDef::New("IntItem2");
    iitemdef2->setDefaultValue(10);
    base->addItemDefinition(iitemdef2);

    smtk::attribute::DefinitionPtr def1 = system.createDefinition("Derived1", "BaseDef");
    // Lets add some item definitions
    smtk::attribute::DoubleItemDefinitionPtr ditemdef = DoubleItemDef::New("DoubleItem1");
    // Allow this one to hold an expression
    ditemdef->setExpressionDefinition(expDef);
    // Check to make sure we can use expressions
    if (!ditemdef->allowsExpressions())
    {
      std::cout << "ERROR - Item Def does not allow expressions\n";
      status = -1;
    }
    def1->addItemDefinition(ditemdef);
    smtk::attribute::DoubleItemDefinitionPtr ditemdef2 = DoubleItemDef::New("DoubleItem2");
    ditemdef2->setDefaultValue(-35.2);
    def1->addItemDefinition(ditemdef2);

    smtk::attribute::DefinitionPtr def2 = system.createDefinition("Derived2", "Derived1");
    // Lets add some item definitions
    smtk::attribute::StringItemDefinitionPtr sitemdef = StringItemDef::New("StringItem1");
    def1->addItemDefinition(sitemdef);
    smtk::attribute::StringItemDefinitionPtr sitemdef2 = StringItemDef::New("StringItem2");
    sitemdef2->setDefaultValue("Default");
    def1->addItemDefinition(sitemdef2);

    // Lets test creating an attribute by passing in the expression definition explicitly
    smtk::attribute::AttributePtr expAtt = system.createAttribute("Exp1", expDef);
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

    smtk::attribute::ValueItemPtr vitem;
    smtk::attribute::ItemPtr item;

    // Find the expression enabled item
    item = att->item(2);
    vitem = smtk::dynamic_pointer_cast<ValueItem>(item);
    if (vitem->allowsExpressions())
    {
      // By default the item shouls not be an expression
      if (!vitem->isExpression())
      {
        std::cout << "Item " << vitem->name() << " does not have an expression\n";
      }
      else
      {
        std::cout << "Error: Item " << vitem->name() << " does have an expression\n";
        status = -1;
      }
      vitem->setExpression(expAtt);
      std::cout << "Expression Set on " << vitem->name() << "\n";
      if (vitem->isExpression())
      {
        std::cout << "After setting an expression, Item " << vitem->name()
                  << " does have an expression\n";
      }
      else
      {
        std::cout << "Error: After setting an expression, Item " << vitem->name()
                  << " does not have an expression\n";
        status = -1;
      }
      smtk::attribute::DoubleItemPtr ditem = smtk::dynamic_pointer_cast<DoubleItem>(vitem);
      ditem->setValue(10.0);
      if (!vitem->isExpression())
      {
        std::cout << "After setting a value, Item " << vitem->name()
                  << " does not have an expression\n";
      }
      else
      {
        std::cout << "Error: After setting a value , Item " << vitem->name()
                  << " does have an expression\n";
        status = -1;
      }
    }
    else
    {
      std::cout << "ERROR: Can not set expression on " << vitem->name() << "\n";
      status = -1;
    }

    int i, n = static_cast<int>(att->numberOfItems());

    // Checking Default checks
    vitem = smtk::dynamic_pointer_cast<ValueItem>(att->item(0));
    if (vitem->isUsingDefault(0) || vitem->isUsingDefault())
    {
      std::cout << "Failed first Default Test!\n";
      status = -1;
    }
    else
    {
      std::cout << "Passed First Default Test\n";
    }

    vitem = smtk::dynamic_pointer_cast<ValueItem>(att->item(1));
    if (!(vitem->isUsingDefault(0) && vitem->isUsingDefault()))
    {
      std::cout << "Failed second Default Test!\n";
      status = -1;
    }
    else
    {
      std::cout << "Passed Second Default Test\n";
    }

    std::cout << "Items of testAtt:\n";
    for (i = 0; i < n; i++)
    {
      item = att->item(i);
      std::cout << "\t" << item->name() << " Type = " << AttItem::type2String(item->type()) << ", ";
      vitem = smtk::dynamic_pointer_cast<ValueItem>(item);
      if (vitem)
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
              std::cout << " Value = " << vitem->valueAsString() << std::endl;
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
    std::cout << "System destroyed\n";
  }
  return status;
}
