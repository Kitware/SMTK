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
  smtk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  // Lets create an attribute to represent an expression
  smtk::AttributeDefinitionPtr expDef = manager.createDefinition("ExpDef");
  smtk::StringItemDefinitionPtr eitemdef = 
    expDef->addItemDefinition<smtk::StringItemDefinitionPtr>("Expression String");
  smtk::StringItemDefinitionPtr eitemdef2 =
    expDef->addItemDefinition<smtk::attribute::StringItemDefinition>("Aux String");
  eitemdef->setDefaultValue("sample");

  smtk::AttributeDefinitionPtr base = manager.createDefinition("BaseDef");
  // Lets add some item definitions
  smtk::DoubleItemDefinitionPtr ditemdef = 
    base->addItemDefinition<smtk::DoubleItemDefinitionPtr>("DoubleItem1");
  // Allow this one to hold an expression
  ditemdef->setExpressionDefinition(expDef);

  // Lets test creating an attribute by passing in the expression definition explicitly
  smtk::AttributePtr expAtt1 = manager.createAttribute("Exp1", expDef);
  smtk::AttributePtr expAtt2 = manager.createAttribute("Exp2", expDef);
  smtk::AttributePtr att = manager.createAttribute("testAtt1", "BaseDef");
  smtk::AttributePtr att1 = manager.createAttribute("testAtt2", "BaseDef");
  smtk::AttributePtr att2 = manager.createAttribute("testAtt3", "BaseDef");
  
  
  smtk::ValueItemPtr vitem;
  smtk::AttributeItemPtr item;
  smtk::dynamicCastPointer<smtk::attribute::ValueItem>(att->item(0))->setExpression(expAtt1);
  smtk::dynamicCastPointer<smtk::attribute::ValueItem>(att1->item(0))->setExpression(expAtt1);
  smtk::dynamicCastPointer<smtk::attribute::ValueItem>(att2->item(0))->setExpression(expAtt2);

  // Lets see what attributes are being referenced
  std::vector<smtk::AttributeItemPtr> refs;
  std::size_t i;
  expAtt1->references(refs);
  std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
  for (i = 0; i < refs.size(); i++)
    {
    std::cout << "\tAtt:" << refs[i]->attribute()->name() << " Item:" << refs[i]->owningItem()->name() 
              << "\n";
    } 

  expAtt2->references(refs);
  std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
  for (i = 0; i < refs.size(); i++)
    {
    std::cout << "\tAtt:" << refs[i]->attribute()->name() << " Item:" << refs[i]->owningItem()->name() 
              << "\n";
    } 

  manager.removeAttribute(att1);
  att1.reset(); // Should delete att1
  std::cout << "testAtt1 deleted\n";
  expAtt1->references(refs);
  std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
  for (i = 0; i < refs.size(); i++)
    {
    std::cout << "\tAtt:" << refs[i]->attribute()->name() << " Item:" << refs[i]->owningItem()->name() 
              << "\n";
    } 

  expAtt2->references(refs);
  std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
  for (i = 0; i < refs.size(); i++)
    {
    std::cout << "\tAtt:" << refs[i]->attribute()->name() << " Item:" << refs[i]->owningItem()->name() 
              << "\n";
    } 

  smtk::dynamicCastPointer<smtk::attribute::ValueItem>(att2->item(0))->setExpression(expAtt1);
  std::cout << "testAtt3 now using Exp2\n";

  expAtt1->references(refs);
  std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
  for (i = 0; i < refs.size(); i++)
    {
    std::cout << "\tAtt:" << refs[i]->attribute()->name() << " Item:" << refs[i]->owningItem()->name() 
              << "\n";
    } 

  expAtt2->references(refs);
  std::cout << "Number of Items referencing expAtt1: " << refs.size() << "\n";
  for (i = 0; i < refs.size(); i++)
    {
    std::cout << "\tAtt:" << refs[i]->attribute()->name() << " Item:" << refs[i]->owningItem()->name() 
              << "\n";
    } 


  std::cout << "Manager destroyed\n";
  }
  return status;
}
