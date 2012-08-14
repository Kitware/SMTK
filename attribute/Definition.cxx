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


#include "attribute/Definition.h"

#include "attribute/Attribute.h"
#include "attribute/Cluster.h"
#include "attribute/Component.h"
#include "attribute/ComponentDefinition.h"
#include <iostream>

using namespace slctk::attribute; 

//----------------------------------------------------------------------------
Definition::Definition(const std::string &myType, 
                       slctk::AttributeClusterPtr myCluster) : m_cluster(myCluster)
{
  this->m_type = myType;
  this->m_version = 0;
  this->m_isNodal = false;
  this->m_advanceLevel = 0;
  this->m_isUnique = true;
  this->m_isRequired = false;
  this->m_associationMask = 0;
}

//----------------------------------------------------------------------------
Definition::~Definition()
{
  std::cout << "Deleting Definition " << this->m_type << std::endl;
}
//----------------------------------------------------------------------------
slctk::attribute::Manager *Definition::manager() const
{
  if (this->m_cluster.lock())
    {
    return this->m_cluster.lock()->manager();
    }
  return NULL;
}
//----------------------------------------------------------------------------
slctk::AttributeDefinitionPtr Definition::baseDefinition() const
{
  if (this->m_cluster.lock() && this->m_cluster.lock()->parent())
    {
    return this->m_cluster.lock()->parent()->definition();
    }
  return slctk::AttributeDefinitionPtr();
}
//----------------------------------------------------------------------------
bool Definition::isA(slctk::ConstAttributeDefinitionPtr targetDef) const
{
  // Walk up the inheritence tree until we either hit the root or
  // encounter this definition
  const Definition *def = this;
  for (def = this; def != NULL; def = def->baseDefinition().get())
    {
    if (def == targetDef.get())
      {
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool Definition::conflicts(slctk::AttributeDefinitionPtr def) const
{
  // 2 definitions conflict if their inheritance tree intersects and isUnique is
  // is true within the intersection
  // ASSUMING isUnique has been set consistantly first verify that both definitions
  // are suppose to be unique
  if (!(this->isUnique() && def->isUnique()))
    {
    return false;
    }
  // Test the trivial case that they are the same definition
  if (this == def.get())
    {
    return true;
    }

  // Get the most "basic" definition that is unique
   slctk::ConstAttributeDefinitionPtr baseDef = this->findIsUniqueBaseClass();
  // See if the other definition is derived from this base defintion.
  // If it is not then we know there is no conflict
  return def->isA(baseDef);
}
//----------------------------------------------------------------------------
slctk::ConstAttributeDefinitionPtr Definition::findIsUniqueBaseClass() const
{
  const Definition *uDef = this, *def;
  while (1)
    {
    def = uDef->baseDefinition().get();
    if ((def == NULL) || (!def->isUnique()))
      {
      return slctk::ConstAttributeDefinitionPtr(uDef);
      }
    uDef = def;
    }
}
//----------------------------------------------------------------------------
bool 
Definition::canBeAssociated(slctk::ModelEntity *entity,
                            std::vector<Attribute *>*conflicts) const
{
  // TO DO - Need to pull in Model Entity class to do this
  // Procedure:
  // 1. Determine if the definition can be applied to the model entity - this
  // involves getting its type and calling the appropriate associatesWith method.
  // In the case of a boundary condition set or domain set we need to look at the
  // model's dimension to call the appropriate method. - return false if it can't.
  // 2. Get a list of attributes on the entity and call conflicts method with each
  // definition.  All conflicting attributes gets added to the list.
  return false;
}
//----------------------------------------------------------------------------
void Definition::buildAttribute(AttributePtr att) const
{
  // If there is a super definition have it prep the attribute and add its components
  const Definition *bdef = this->baseDefinition().get();
  if (bdef)
    {
    bdef->buildAttribute(att);
    }
  else
    {
    // This is the "base definition" so first we should make sure the attribute 
    // is "empty" of components
    att->removeAllComponents();
    }

  // Next - for each component definition we have build and add the appropriate
  // component to the attribute
  slctk::AttributeComponentPtr comp;
  std::size_t i, n = this->m_componentDefs.size();
  for (i = 0; i < n; i++)
    {
    comp = this->m_componentDefs[i]->buildComponent();
    comp->setDefinition(this->m_componentDefs[i]);
    att->addComponent(comp);
    }
}
//----------------------------------------------------------------------------
bool Definition::isMemberOf(const std::vector<std::string> &catagories) const
{
  std::size_t i, n = catagories.size();
  for (i = 0; i < n; i++)
    {
    if (this->isMemberOf(catagories[i]))
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool Definition::addComponentDefinition(slctk::AttributeComponentDefinitionPtr cdef)
{
  // First see if there is a component by the same name
  if (this->findComponentPosition(cdef->name()) >= 0)
    {
    return false;
    }
  std::size_t n = this->m_componentDefs.size();
  this->m_componentDefs.push_back(cdef);
  this->m_componentDefPositions[cdef->name()] = n;
  return true;
}
//----------------------------------------------------------------------------
