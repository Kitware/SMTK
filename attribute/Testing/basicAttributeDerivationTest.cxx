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
#include "attribute/IntegerItem.h"
#include "attribute/IntegerItemDefinition.h"
#include "attribute/DoubleItem.h"
#include "attribute/DoubleItemDefinition.h"
#include "attribute/StringItem.h"
#include "attribute/StringItemDefinition.h"
#include <iostream>

int main()
{
  int status = 0;
  {
  typedef slctk::attribute::IntegerItemDefinition IntCompDef;
  typedef slctk::attribute::DoubleItemDefinition DoubleCompDef;
  typedef slctk::attribute::StringItemDefinition StringCompDef;
  typedef slctk::attribute::ValueItem ValueComp;
  typedef slctk::attribute::Item AttComp;

  slctk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  slctk::AttributeDefinitionPtr base = manager.createDefinition("BaseDef");
  // Lets add some item definitions
  slctk::IntegerItemDefinitionPtr icompdef(new IntCompDef("IntComp1"));
  base->addItemDefinition(icompdef);
  slctk::IntegerItemDefinitionPtr icompdef2(new IntCompDef("IntComp2"));
  icompdef2->setDefaultValue(10);
  base->addItemDefinition(icompdef2);

  slctk::AttributeDefinitionPtr def1 = manager.createDefinition("Derived1", "BaseDef");
   // Lets add some item definitions
  slctk::DoubleItemDefinitionPtr dcompdef(new DoubleCompDef("DoubleComp1"));
  def1->addItemDefinition(dcompdef);
  slctk::DoubleItemDefinitionPtr dcompdef2(new DoubleCompDef("DoubleComp2"));
  dcompdef2->setDefaultValue(-35.2);
  def1->addItemDefinition(dcompdef2);

  slctk::AttributeDefinitionPtr def2 = manager.createDefinition("Derived2", "Derived1");
   // Lets add some item definitions
  slctk::StringItemDefinitionPtr scompdef(new StringCompDef("StringComp1"));
  def1->addItemDefinition(scompdef);
  slctk::StringItemDefinitionPtr scompdef2(new StringCompDef("StringComp2"));
  scompdef2->setDefaultValue("Default");
  def1->addItemDefinition(scompdef2);

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

  slctk::ValueItemPtr vcomp;
  slctk::AttributeItemPtr comp;

  int i, n = att->numberOfItems();
  std::cout << "Items of testAtt:\n";
  for (i = 0; i < n; i++)
    {
    comp = att->item(i);
    std::cout << "\t" << comp->name() << " Type = " << AttComp::type2String(comp->type()) << ", ";
    vcomp = std::tr1::dynamic_pointer_cast<ValueComp>(comp);
    if (vcomp != NULL)
      {
      switch (vcomp->type())
        {
        case AttComp::DOUBLE:
          std::cout << " Value = " << vcomp->valueAsString("%g") << "\n";
          break;
        case AttComp::INTEGER:
          std::cout << " Value = " << vcomp->valueAsString("%d") << "\n";
          break;
        case AttComp::STRING:
          std::cout << vcomp->valueAsString(" String Val = %s") << "\n";
          break;
        default:
          break;
        }
      }
    }
  std::cout << "Manager destroyed\n";
  }
  return status;
}
