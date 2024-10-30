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
#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/Resource.h"
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
    typedef smtk::attribute::DoubleItem DoubleItem;
    typedef smtk::attribute::ValueItem ValueItem;
    typedef smtk::attribute::Item AttItem;

    smtk::attribute::ResourcePtr resptr = smtk::attribute::Resource::create();
    smtk::attribute::Resource& resource(*resptr.get());
    std::cout << "Resource Created\n";

    // let's create definitions to represent 2 types of expressions
    smtk::attribute::DefinitionPtr expDef = resource.createDefinition("ExpDef");
    smtk::attribute::StringItemDefinitionPtr eitemdef = StringItemDef::New("Expression String");
    expDef->addItemDefinition(eitemdef);

    smtk::attribute::DefinitionPtr expDef1 = resource.createDefinition("ExpDef1");
    smtk::attribute::StringItemDefinitionPtr eitemdef1 = StringItemDef::New("Expression String");
    expDef1->addItemDefinition(eitemdef1);

    smtk::attribute::DefinitionPtr base = resource.createDefinition("BaseDef");
    // let's add some item definitions
    smtk::attribute::IntItemDefinitionPtr iitemdef = IntItemDef::New("IntItem1");
    base->addItemDefinition(iitemdef);
    smtk::attribute::IntItemDefinitionPtr iitemdef2 = IntItemDef::New("IntItem2");
    iitemdef2->setDefaultValue(10);
    base->addItemDefinition(iitemdef2);

    smtk::attribute::DefinitionPtr def1 = resource.createDefinition("Derived1", "BaseDef");
    // let's add some item definitions
    smtk::attribute::DoubleItemDefinitionPtr ditemdef = DoubleItemDef::New("DoubleItem1");
    // Allow this one to hold an expression
    ditemdef->setExpressionType(expDef->type());
    // Check to make sure we can use expressions
    if (!ditemdef->allowsExpressions())
    {
      std::cout << "ERROR - Item Def does not allow expressions\n";
      status = -1;
    }
    def1->addItemDefinition(ditemdef);

    smtk::attribute::DoubleItemDefinitionPtr ditemdef1a = DoubleItemDef::New("DoubleItem1a");
    // Allow this one to hold 2 expressions types
    auto expInfo = ditemdef1a->expressionInformation();
    expInfo->setAcceptsEntries(
      smtk::common::typeName<smtk::attribute::Resource>(),
      smtk::attribute::Resource::createAttributeQuery(expDef->type()),
      true);
    expInfo->setAcceptsEntries(
      smtk::common::typeName<smtk::attribute::Resource>(),
      smtk::attribute::Resource::createAttributeQuery(expDef1->type()),
      true);
    // Check to make sure we can use expressions
    if (!ditemdef1a->allowsExpressions())
    {
      std::cout << "ERROR - Item Def 1a does not allow expressions\n";
      status = -1;
    }
    // Check to make sure that expression type comes back as empty since
    // that API only works when there is only 1 acceptable type of expressions
    if (!ditemdef1a->expressionType().empty())
    {
      std::cout << "ERROR - Item Def 1a 's expression type is " << ditemdef1a->expressionType()
                << " but should have been an empty string!\n";
      status = -1;
    }
    def1->addItemDefinition(ditemdef1a);
    smtk::attribute::DoubleItemDefinitionPtr ditemdef2 = DoubleItemDef::New("DoubleItem2");
    ditemdef2->setDefaultValue(-35.2);
    def1->addItemDefinition(ditemdef2);

    smtk::attribute::DefinitionPtr def2 = resource.createDefinition("Derived2", "Derived1");
    // let's add some item definitions
    smtk::attribute::StringItemDefinitionPtr sitemdef = StringItemDef::New("StringItem1");
    def1->addItemDefinition(sitemdef);
    smtk::attribute::StringItemDefinitionPtr sitemdef2 = StringItemDef::New("StringItem2");
    sitemdef2->setDefaultValue("Default");
    def1->addItemDefinition(sitemdef2);

    // let's test creating an attribute by passing in the expression definition explicitly
    smtk::attribute::AttributePtr expAtt = resource.createAttribute("Exp", expDef);
    smtk::attribute::AttributePtr expAtt1 = resource.createAttribute("Exp1", expDef1);
    smtk::attribute::AttributePtr att = resource.createAttribute("testAtt", "Derived2");
    if (att)
    {
      std::cout << "Attribute testAtt created\n";
    }
    else
    {
      std::cout << "ERROR: Attribute testAtt not created\n";
      status = -1;
    }

    smtk::attribute::ValueItemPtr vitem, vitem1;
    smtk::attribute::ItemPtr item;

    // Find the expression enabled items
    item = att->item(2);
    vitem = smtk::dynamic_pointer_cast<ValueItem>(item);
    vitem1 = smtk::dynamic_pointer_cast<ValueItem>(att->item(3));

    // Test the item that can only deal with one type of expression
    if (vitem->allowsExpressions())
    {
      // By default the item should not be an expression
      if (!vitem->isExpression())
      {
        std::cout << "Item " << vitem->name() << " does not have an expression\n";
      }
      else
      {
        std::cout << "Error: Item " << vitem->name() << " does have an expression\n";
        status = -1;
      }
      // let's first try setting it to an expression it doesn't support
      if (!vitem->setExpression(expAtt1))
      {
        std::cout << vitem->name() << " did not accept expression " << expAtt1->name()
                  << " which is correct\n";
      }
      else
      {
        std::cout << "Error: " << vitem->name() << " did accept expression " << expAtt1->name()
                  << " which is not correct\n";
        status = -1;
      }

      if (vitem->setExpression(expAtt))
      {
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
      }
      else
      {
        std::cout << "Could not set expression Exp on " << vitem->name() << std::endl;
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

    // Test the item that can support 2 types of expressions
    if (vitem1->allowsExpressions())
    {
      // By default the item should not be an expression
      if (!vitem1->isExpression())
      {
        std::cout << "Item " << vitem1->name() << " does not have an expression\n";
      }
      else
      {
        std::cout << "Error: Item " << vitem1->name() << " does have an expression\n";
        status = -1;
      }
      // This item should allow both types of expressions
      if (vitem1->setExpression(expAtt))
      {
        std::cout << "Expression: " << expAtt->name() << " Set on " << vitem1->name() << "\n";
        if (vitem1->isExpression())
        {
          std::cout << "After setting an expression (" << expAtt->name() << "), Item "
                    << vitem1->name() << " does have an expression\n";
        }
        else
        {
          std::cout << "Error: After setting an expression (" << expAtt->name() << "), Item "
                    << vitem1->name() << " does not have an expression\n";
          status = -1;
        }
      }
      else
      {
        std::cout << "Could not set expression: " << expAtt->name() << " on " << vitem1->name()
                  << std::endl;
        status = -1;
      }
      if (vitem1->setExpression(expAtt1))
      {
        std::cout << "Expression: " << expAtt1->name() << " Set on " << vitem1->name() << "\n";
        if (vitem1->isExpression())
        {
          std::cout << "After setting an expression (" << expAtt1->name() << "), Item "
                    << vitem1->name() << " does have an expression\n";
        }
        else
        {
          std::cout << "Error: After setting an expression (" << expAtt1->name() << "), Item "
                    << vitem1->name() << " does not have an expression\n";
          status = -1;
        }
      }
      else
      {
        std::cout << "Could not set expression: " << expAtt1->name() << vitem1->name() << std::endl;
        status = -1;
      }
      smtk::attribute::DoubleItemPtr ditem = smtk::dynamic_pointer_cast<DoubleItem>(vitem1);
      ditem->setValue(10.0);
      if (!vitem1->isExpression())
      {
        std::cout << "After setting a value, Item " << vitem1->name()
                  << " does not have an expression\n";
      }
      else
      {
        std::cout << "Error: After setting a value , Item " << vitem1->name()
                  << " does have an expression\n";
        status = -1;
      }
    }
    else
    {
      std::cout << "ERROR: Can not set expression on " << vitem1->name() << "\n";
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
            case AttItem::DoubleType:
            case AttItem::IntType:
              std::cout << " Value = " << vitem->valueAsString() << std::endl;
              break;
            case AttItem::StringType:
              std::cout << " String Val = " << vitem->valueAsString() << std::endl;
              break;
            default:
              break;
          }
        }
      }
    }
    std::cout << "Resource destroyed\n";
  }
  return status;
}
