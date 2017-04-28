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
    smtk::attribute::SystemPtr sysptr = smtk::attribute::System::create();
    smtk::attribute::System& system(*sysptr.get());
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
    smtk::attribute::DoubleItemDefinitionPtr ditemdef =
      base->addItemDefinition<smtk::attribute::DoubleItemDefinitionPtr>("DoubleItem1");
    // Allow this one to hold an expression
    ditemdef->setExpressionDefinition(expDef);

    // Lets test creating an attribute by passing in the expression definition explicitly
    smtk::attribute::AttributePtr expAtt1 = system.createAttribute("Exp1", expDef);
    smtk::attribute::AttributePtr expAtt2 = system.createAttribute("Exp2", expDef);
    smtk::attribute::AttributePtr att = system.createAttribute("testAtt1", "BaseDef");
    smtk::attribute::AttributePtr att1 = system.createAttribute("testAtt2", "BaseDef");
    smtk::attribute::AttributePtr att2 = system.createAttribute("testAtt3", "BaseDef");

    smtk::attribute::ValueItemPtr vitem;
    smtk::attribute::ItemPtr item;
    smtk::dynamic_pointer_cast<smtk::attribute::ValueItem>(att->item(0))->setExpression(expAtt1);
    smtk::dynamic_pointer_cast<smtk::attribute::ValueItem>(att1->item(0))->setExpression(expAtt1);
    smtk::dynamic_pointer_cast<smtk::attribute::ValueItem>(att2->item(0))->setExpression(expAtt2);

    // Lets see what attributes are being referenced
    std::vector<smtk::attribute::ItemPtr> refs;
    std::size_t i;
    expAtt1->references(refs);
    std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
    for (i = 0; i < refs.size(); i++)
    {
      std::cout << "\tAtt:" << refs[i]->attribute()->name()
                << " Item:" << refs[i]->owningItem()->name() << "\n";
    }

    expAtt2->references(refs);
    std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
    for (i = 0; i < refs.size(); i++)
    {
      std::cout << "\tAtt:" << refs[i]->attribute()->name()
                << " Item:" << refs[i]->owningItem()->name() << "\n";
    }

    system.removeAttribute(att1);
    att1.reset(); // Should delete att1
    std::cout << "testAtt1 deleted\n";
    expAtt1->references(refs);
    std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
    for (i = 0; i < refs.size(); i++)
    {
      std::cout << "\tAtt:" << refs[i]->attribute()->name()
                << " Item:" << refs[i]->owningItem()->name() << "\n";
    }

    expAtt2->references(refs);
    std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
    for (i = 0; i < refs.size(); i++)
    {
      std::cout << "\tAtt:" << refs[i]->attribute()->name()
                << " Item:" << refs[i]->owningItem()->name() << "\n";
    }

    smtk::dynamic_pointer_cast<smtk::attribute::ValueItem>(att2->item(0))->setExpression(expAtt1);
    std::cout << "testAtt3 now using Exp2\n";

    expAtt1->references(refs);
    std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
    for (i = 0; i < refs.size(); i++)
    {
      std::cout << "\tAtt:" << refs[i]->attribute()->name()
                << " Item:" << refs[i]->owningItem()->name() << "\n";
    }

    expAtt2->references(refs);
    std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
    for (i = 0; i < refs.size(); i++)
    {
      std::cout << "\tAtt:" << refs[i]->attribute()->name()
                << " Item:" << refs[i]->owningItem()->name() << "\n";
    }

    std::cout << "System destroyed\n";
  }
  return status;
}
