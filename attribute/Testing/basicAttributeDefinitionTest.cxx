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
#include "attribute/IntegerComponent.h"
#include "attribute/IntegerComponentDefinition.h"
#include "attribute/DoubleComponent.h"
#include "attribute/DoubleComponentDefinition.h"
#include "attribute/StringComponent.h"
#include "attribute/StringComponentDefinition.h"
#include <iostream>

int main()
{
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
    }
  // Lets add some component definitions
  slctk::IntegerComponentDefinitionPtr icompdef(new slctk::attribute::IntegerComponentDefinition("IntComp1", 0));
  def->addComponentDefinition(icompdef);
  slctk::IntegerComponentDefinitionPtr icompdef2(new slctk::attribute::IntegerComponentDefinition("IntComp2", 0));
  icompdef2->setDefaultValue(10);
  def->addComponentDefinition(icompdef2);
  slctk::AttributeDefinitionPtr def1 = manager.createDefinition("testDef");
  if (def1 == NULL)
    {
    std::cout << "Duplicated definition testDef not created\n";
    }
  else
    {
    std::cout << "ERROR: Duplicated definition testDef created\n";
    }
  slctk::AttributePtr att = manager.createAttribute("testAtt", "testDef");
  if (att != NULL)
    {
    std::cout << "Attribute testAtt created\n";
    }
  else
    {
    std::cout << "ERROR: Attribute testAtt not created\n";
    }

  if (att->numberOfComponents() != 2)
    {
    std::cout << "ERROR: attribute has " << att->numberOfComponents() << " components not 2\n";
    }
  else if (att->component(0)->name() != "IntComp1")
    {
    std::cout << "Attribute's 0th component is named " << att->component(0)->name()
              << " not IntComp1\n";
    }
  else if (att->component(1)->name() != "IntComp2")
    {
    std::cout << "Attribute's 1st component is named " << att->component(0)->name()
              << " not IntComp2\n";
    }
  else
    {
    slctk::IntegerComponentPtr icptr;
    icptr = slctk::dynamicCastPointer<slctk::attribute::IntegerComponent>(att->component(0));
    std::cout << "Found IntComp1 - value = " << icptr->valueAsString(" %d") << std::endl;
    icptr = slctk::dynamicCastPointer<slctk::attribute::IntegerComponent>(att->component(1));
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
    }

  std::cout << "Manager destroyed\n";
  }
}
