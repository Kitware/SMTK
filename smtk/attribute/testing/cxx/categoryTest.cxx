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
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include <iostream>

int main()
{
  int status = 0;
  {
    smtk::attribute::ResourcePtr resptr = smtk::attribute::Resource::create();
    smtk::attribute::Resource& resource(*resptr.get());
    std::cout << "Resource Created\n";
    // Lets create an attribute to represent an expression
    smtk::attribute::DefinitionPtr expDef = resource.createDefinition("ExpDef");
    expDef->setBriefDescription("Sample Expression");
    expDef->setDetailedDescription("Sample Expression for testing\nThere is not much here!");
    smtk::attribute::StringItemDefinitionPtr eitemdef =
      expDef->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("Expression String");
    smtk::attribute::StringItemDefinitionPtr eitemdef2 =
      expDef->addItemDefinition<smtk::attribute::StringItemDefinition>("Aux String");
    eitemdef->setDefaultValue("sample");

    smtk::attribute::DefinitionPtr base = resource.createDefinition("BaseDef");
    // Lets add some item definitions
    smtk::attribute::IntItemDefinitionPtr iitemdef =
      base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("IntItem1");
    iitemdef->localCategories().insertInclusion("Flow");
    iitemdef = base->addItemDefinition<smtk::attribute::IntItemDefinitionPtr>("IntItem2");
    iitemdef->setDefaultValue(10);
    iitemdef->localCategories().insertInclusion("Heat");

    smtk::attribute::DefinitionPtr def1 = resource.createDefinition("Derived1", "BaseDef");
    // Lets add some item definitions
    smtk::attribute::DoubleItemDefinitionPtr ditemdef =
      def1->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("DoubleItem1");
    // Allow this one to hold an expression
    ditemdef->localCategories().insertInclusion("Veg");
    ditemdef->setExpressionDefinition(expDef);
    // Check to make sure we can use expressions
    if (!ditemdef->allowsExpressions())
    {
      std::cout << "ERROR - Item Def does not allow expressions\n";
      status = -1;
    }
    ditemdef = def1->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("DoubleItem2");
    ditemdef->setDefaultValue(-35.2);
    ditemdef->localCategories().insertInclusion("Constituent");

    smtk::attribute::DefinitionPtr def2 = resource.createDefinition("Derived2", "Derived1");
    // Lets add some item definitions
    smtk::attribute::StringItemDefinitionPtr sitemdef =
      def2->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("StringItem1");
    sitemdef->localCategories().insertInclusion("Flow");
    sitemdef = def2->addItemDefinition<smtk::attribute::StringItemDefinitionPtr>("StringItem2");
    sitemdef->setDefaultValue("Default");
    sitemdef->localCategories().insertInclusion("General");

    // Process Categories
    resource.finalizeDefinitions();
    // Lets see what categories the attribute definitions think they are
    if (expDef->categories().size())
    {
      std::cout << "ERROR: ExpDef's categories: ";
      std::cout << expDef->categories().convertToString() << std::endl;
    }
    else
    {
      std::cout << "ExpDef has no categories\n";
    }
    if (def2->categories().size())
    {
      std::cout << "Def2's categories: ";
      std::cout << def2->categories().convertToString() << std::endl;
    }
    else
    {
      std::cout << "ERROR: Def2 has no categories!\n";
    }
    // Lets test creating an attribute by passing in the expression definition explicitly
    smtk::attribute::AttributePtr expAtt = resource.createAttribute("Exp1", expDef);
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
    smtk::io::Logger logger;
    std::string contents;
    smtk::io::AttributeWriter writer;
    writer.writeContents(resptr, contents, logger);
    if (logger.hasErrors())
    {
      std::cerr << "Errors encountered creating Attribute String:\n";
      std::cerr << logger.convertToString();
    }

    std::cout << "Resource destroyed\n";
  }
  return status;
}
