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
  slctk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  slctk::AttributeDefinitionPtr def = manager.createDefinition("testDef");
  if (def != NULL)
    {
    std::cout << "Definition testDef created\n";
    }
  else
    {
    std::cout << "ERROR: Definition testDef not created\n";
    status = -1;
    }
  // Lets add some item definitions
  slctk::IntegerItemDefinitionPtr icompdef(new slctk::attribute::IntegerItemDefinition("IntComp1"));
  def->addItemDefinition(icompdef);
  slctk::IntegerItemDefinitionPtr icompdef2(new slctk::attribute::IntegerItemDefinition("IntComp2"));
  icompdef2->setDefaultValue(10);
  def->addItemDefinition(icompdef2);
  slctk::AttributeDefinitionPtr def1 = manager.createDefinition("testDef");
  if (def1 == NULL)
    {
    std::cout << "Duplicated definition testDef not created\n";
    }
  else
    {
    std::cout << "ERROR: Duplicated definition testDef created\n";
    status = -1;
    }
  slctk::AttributePtr att = manager.createAttribute("testAtt", "testDef");
  if (att != NULL)
    {
    std::cout << "Attribute testAtt created\n";
    }
  else
    {
    std::cout << "ERROR: Attribute testAtt not created\n";
    status = -1;
    }

  if (att->numberOfItems() != 2)
    {
    std::cout << "ERROR: attribute has " << att->numberOfItems() << " items not 2\n";
    status = -1;
    }
  else if (att->item(0)->name() != "IntComp1")
    {
    std::cout << "ERROR: Attribute's 0th item is named " << att->item(0)->name()
              << " not IntComp1\n";
     status = -1;
   }
  else if (att->item(1)->name() != "IntComp2")
    {
    std::cout << "Error Attribute's 1st item is named " << att->item(0)->name()
              << " not IntComp2\n";
    status = -1;
    }
  else
    {
    slctk::IntegerItemPtr icptr;
    icptr = slctk::dynamicCastPointer<slctk::attribute::IntegerItem>(att->item(0));
    std::cout << "Found IntComp1 - value = " << icptr->valueAsString(" %d") << std::endl;
    icptr = slctk::dynamicCastPointer<slctk::attribute::IntegerItem>(att->item(1));
    std::cout << "Found IntComp2 - value = " << icptr->valueAsString(" %d") << std::endl;
    }

  slctk::AttributePtr att1 = manager.createAttribute("testAtt", "testDef");
  if (att1 == NULL)
    {
    std::cout << "Duplicate Attribute testAtt not created\n";
    }
  else
    {
    std::cout << "ERROR: Duplicate Attribute testAtt  created\n";
    status = -1;
    }

  std::cout << "Manager destroyed\n";
  }
  return status;
}
