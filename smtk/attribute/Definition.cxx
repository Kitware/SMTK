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


#include "smtk/attribute/Definition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include <iostream>

using namespace smtk::attribute; 
double Definition::s_notApplicableBaseColor[4] = {0.0, 0.0, 0.0, 0.0};
double Definition::s_defaultBaseColor[4] = {1.0, 1.0, 1.0, 1.0};

//----------------------------------------------------------------------------
Definition::Definition(const std::string &myType, 
                       smtk::AttributeDefinitionPtr myBaseDef,
                       Manager *myManager)
{
  this->m_manager = myManager;
  this->m_baseDefinition = myBaseDef;
  this->m_type = myType;
  this->m_version = 0;
  this->m_isAbstract = false;
  this->m_isNodal = false;
  this->m_advanceLevel = 0;
  this->m_isUnique = true;
  this->m_isRequired = false;
  this->m_associationMask = 0;
  this->m_isNotApplicableColorSet = false;
  this->m_isDefaultColorSet = false;
}

//----------------------------------------------------------------------------
Definition::~Definition()
{
  std::cout << "Deleting Definition " << this->m_type << std::endl;
}
//----------------------------------------------------------------------------
bool Definition::isA(smtk::ConstAttributeDefinitionPtr targetDef) const
{
  // Walk up the inheritence tree until we either hit the root or
  // encounter this definition
  const Definition *def = this;
  for (def = this; def != NULL; def = def->m_baseDefinition.get())
    {
    if (def == targetDef.get())
      {
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool Definition::conflicts(smtk::AttributeDefinitionPtr def) const
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
   smtk::ConstAttributeDefinitionPtr baseDef = this->findIsUniqueBaseClass();
  // See if the other definition is derived from this base defintion.
  // If it is not then we know there is no conflict
  return def->isA(baseDef);
}
//----------------------------------------------------------------------------
smtk::ConstAttributeDefinitionPtr Definition::findIsUniqueBaseClass() const
{
  const Definition *uDef = this, *def;
  while (1)
    {
    def = uDef->m_baseDefinition.get();
    if ((def == NULL) || (!def->isUnique()))
      {
      return smtk::ConstAttributeDefinitionPtr(uDef);
      }
    uDef = def;
    }
}
//----------------------------------------------------------------------------
bool 
Definition::canBeAssociated(smtk::ModelItemPtr entity,
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
void Definition::buildAttribute(Attribute *att) const
{
  // If there is a super definition have it prep the attribute and add its items
  const Definition *bdef = this->m_baseDefinition.get();
  if (bdef)
    {
    bdef->buildAttribute(att);
    }
  else
    {
    // This is the "base definition" so first we should make sure the attribute 
    // is "empty" of items
    att->removeAllItems();
    }

  // Next - for each item definition we have build and add the appropriate
  // item to the attribute
  smtk::AttributeItemPtr comp;
  std::size_t i, j, n = this->m_itemDefs.size();
  j = att->numberOfItems();
  for (i = 0; i < n; i++, j++)
    {
    comp = this->m_itemDefs[i]->buildItem(att, j);
    comp->setDefinition(this->m_itemDefs[i]);
    att->addItem(comp);
    }
}
//----------------------------------------------------------------------------
bool Definition::isMemberOf(const std::vector<std::string> &categories) const
{
  std::size_t i, n = categories.size();
  for (i = 0; i < n; i++)
    {
    if (this->isMemberOf(categories[i]))
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool Definition::addItemDefinition(smtk::AttributeItemDefinitionPtr cdef)
{
  // First see if there is a item by the same name
  if (this->findItemPosition(cdef->name()) >= 0)
    {
    return false;
    }
  std::size_t n = this->m_itemDefs.size();
  this->m_itemDefs.push_back(cdef);
  this->m_itemDefPositions[cdef->name()] = n;
  return true;
}
//----------------------------------------------------------------------------
void Definition::setCategories()
{
  if (this->m_baseDefinition != NULL)
    {
    this->m_categories = this->m_baseDefinition->m_categories;
    }
  else
    {
    this->m_categories.clear();
    }
  int i, n = this->m_itemDefs.size();
  for (i = 0; i < n; i++)
    {
    this->m_itemDefs[i]->updateCategories();
    const std::set<std::string> &itemCats = this->m_itemDefs[i]->categories();
    this->m_categories.insert(itemCats.begin(), itemCats.end());
    }
}
//----------------------------------------------------------------------------
