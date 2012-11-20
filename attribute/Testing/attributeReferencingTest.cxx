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
  int status = 0;
  {
  slctk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  // Lets create an attribute to represent an expression
  slctk::AttributeDefinitionPtr expDef = manager.createDefinition("ExpDef");
  slctk::StringItemDefinitionPtr eitemdef = 
    expDef->addItemDefinition<slctk::StringItemDefinitionPtr>("Expression String");
  slctk::StringItemDefinitionPtr eitemdef2 =
    expDef->addItemDefinition<slctk::attribute::StringItemDefinition>("Aux String");
  eitemdef->setDefaultValue("sample");

  slctk::AttributeDefinitionPtr base = manager.createDefinition("BaseDef");
  // Lets add some item definitions
  slctk::DoubleItemDefinitionPtr ditemdef = 
    base->addItemDefinition<slctk::DoubleItemDefinitionPtr>("DoubleItem1");
  // Allow this one to hold an expression
  ditemdef->setExpressionDefinition(expDef);

  // Lets test creating an attribute by passing in the expression definition explicitly
  slctk::AttributePtr expAtt1 = manager.createAttribute("Exp1", expDef);
  slctk::AttributePtr expAtt2 = manager.createAttribute("Exp2", expDef);
  slctk::AttributePtr att = manager.createAttribute("testAtt1", "BaseDef");
  slctk::AttributePtr att1 = manager.createAttribute("testAtt2", "BaseDef");
  slctk::AttributePtr att2 = manager.createAttribute("testAtt3", "BaseDef");
  
  
  slctk::ValueItemPtr vitem;
  slctk::AttributeItemPtr item;
  slctk::dynamicCastPointer<slctk::attribute::ValueItem>(att->item(0))->setExpression(expAtt1);
  slctk::dynamicCastPointer<slctk::attribute::ValueItem>(att1->item(0))->setExpression(expAtt1);
  slctk::dynamicCastPointer<slctk::attribute::ValueItem>(att2->item(0))->setExpression(expAtt2);

  // Lets see what attributes are being referenced
  std::vector<slctk::AttributeItemPtr> refs;
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

  slctk::dynamicCastPointer<slctk::attribute::ValueItem>(att2->item(0))->setExpression(expAtt1);
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
