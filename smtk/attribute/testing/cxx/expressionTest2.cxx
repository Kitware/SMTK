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
    smtk::attribute::System system;
    std::cout << "System Created\n";
    // Lets create an attribute to represent an expression
    smtk::attribute::DefinitionPtr expDef = system.createDefinition("ExpDef");
    smtk::attribute::StringItemDefinitionPtr eitemdef =
      expDef->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("Expression String");
    smtk::attribute::StringItemDefinitionPtr eitemdef2 =
      expDef->addItemDefinition<smtk::attribute::StringItemDefinition>("Aux String");
    eitemdef->setDefaultValue("sample");

    smtk::attribute::DefinitionPtr base = system.createDefinition("BaseDef");
    // Lets add some item definitions
    smtk::attribute::IntItemDefinitionPtr iitemdef =
      base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("IntItem1");
    iitemdef = base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("IntItem2");
    iitemdef->setDefaultValue(10);

    smtk::attribute::DefinitionPtr def1 = system.createDefinition("Derived1", "BaseDef");
    // Lets add some item definitions
    smtk::attribute::DoubleItemDefinitionPtr ditemdef =
      def1->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("DoubleItem1");
    // Allow this one to hold an expression
    ditemdef->setExpressionDefinition(expDef);
    // Check to make sure we can use expressions
    if (!ditemdef->allowsExpressions())
    {
      std::cout << "ERROR - Item Def does not allow expressions\n";
      status = -1;
    }
    ditemdef = def1->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("DoubleItem2");
    ditemdef->setDefaultValue(-35.2);

    smtk::attribute::DefinitionPtr def2 = system.createDefinition("Derived2", "Derived1");
    // Lets add some item definitions
    smtk::attribute::StringItemDefinitionPtr sitemdef =
      def2->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("StringItem1");
    sitemdef = def2->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("StringItem2");
    sitemdef->setDefaultValue("Default");

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
    vitem = smtk::dynamic_pointer_cast<smtk::attribute::ValueItem>(item);
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

    int i, n = static_cast<int>(att->numberOfItems());
    std::cout << "Items of testAtt:\n";
    for (i = 0; i < n; i++)
    {
      item = att->item(i);
      std::cout << "\t" << item->name()
                << " Type = " << smtk::attribute::Item::type2String(item->type()) << ", ";
      vitem = smtk::dynamic_pointer_cast<smtk::attribute::ValueItem>(item);
      if (vitem)
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
    std::cout << "System destroyed\n";
  }
  return status;
}
