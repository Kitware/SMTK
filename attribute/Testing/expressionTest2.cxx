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
  int status;
  {
  typedef slctk::attribute::IntegerComponentDefinition IntCompDef;
  typedef slctk::attribute::DoubleComponentDefinition DoubleCompDef;
  typedef slctk::attribute::StringComponentDefinition StringCompDef;
  typedef slctk::attribute::ValueComponent ValueComp;
  typedef slctk::attribute::Component AttComp;

  slctk::attribute::Manager manager;
  std::cout << "Manager Created\n";
  // Lets create an attribute to represent an expression
  slctk::AttributeDefinitionPtr expDef = manager.createDefinition("ExpDef");
  slctk::StringComponentDefinitionPtr ecompdef(new StringCompDef("Expression String"));
  expDef->addComponentDefinition(ecompdef);

  slctk::AttributeDefinitionPtr base = manager.createDefinition("BaseDef");
  // Lets add some component definitions
  slctk::IntegerComponentDefinitionPtr icompdef(new IntCompDef("IntComp1"));
  base->addComponentDefinition(icompdef);
  slctk::IntegerComponentDefinitionPtr icompdef2(new IntCompDef("IntComp2"));
  icompdef2->setDefaultValue(10);
  base->addComponentDefinition(icompdef2);

  slctk::AttributeDefinitionPtr def1 = manager.createDefinition("Derived1", "BaseDef");
   // Lets add some component definitions
  slctk::DoubleComponentDefinitionPtr dcompdef(new DoubleCompDef("DoubleComp1"));
  // Allow this one to hold an expression
  dcompdef->setExpressionDefinition(expDef);
  // Check to make sure we can use expressions
  if (!dcompdef->allowsExpressions())
    {
    std::cout << "ERROR - Component Def does not allow expressions\n";
    status = -1;
    }
  def1->addComponentDefinition(dcompdef);
  slctk::DoubleComponentDefinitionPtr dcompdef2(new DoubleCompDef("DoubleComp2"));
  dcompdef2->setDefaultValue(-35.2);
  def1->addComponentDefinition(dcompdef2);

  slctk::AttributeDefinitionPtr def2 = manager.createDefinition("Derived2", "Derived1");
   // Lets add some component definitions
  slctk::StringComponentDefinitionPtr scompdef(new StringCompDef("StringComp1"));
  def1->addComponentDefinition(scompdef);
  slctk::StringComponentDefinitionPtr scompdef2(new StringCompDef("StringComp2"));
  scompdef2->setDefaultValue("Default");
  def1->addComponentDefinition(scompdef2);

  // Lets test creating an attribute by passing in the expression definition explicitly
  slctk::AttributePtr expAtt = manager.createAttribute("Exp1", expDef);
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

  slctk::ValueComponentPtr vcomp;
  slctk::AttributeComponentPtr comp;

  // Find the expression enabled component
  comp = att->component(2);
  vcomp = std::tr1::dynamic_pointer_cast<ValueComp>(comp);
  if (vcomp->allowsExpressions())
    {
    vcomp->setExpression(expAtt);
    std::cout << "Expression Set on " << vcomp->name() << "\n";
    }
  else
    {
    std::cout << "ERROR: Can not set expression on " << vcomp->name() << "\n";
    status = -1;
    }
  
  int i, n = att->numberOfComponents();
  std::cout << "Components of testAtt:\n";
  for (i = 0; i < n; i++)
    {
    comp = att->component(i);
    std::cout << "\t" << comp->name() << " Type = " << AttComp::type2String(comp->type()) << ", ";
    vcomp = std::tr1::dynamic_pointer_cast<ValueComp>(comp);
    if (vcomp != NULL)
      {
      if (vcomp->isExpression())
        {
        std::cout << " using Expression: " << vcomp->expression()->name() << "\n";
        }
      else
        {
        std::cout << " Value = " << vcomp->valueAsString() << "\n";
        }
      }
    }
  std::cout << "Manager destroyed\n";
  }
  return status;
}
