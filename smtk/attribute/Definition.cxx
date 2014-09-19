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
#include "smtk/attribute/Manager.h"
#include <iostream>

using namespace smtk::attribute;
double Definition::s_notApplicableBaseColor[4] = {0.0, 0.0, 0.0, 0.0};
double Definition::s_defaultBaseColor[4] = {1.0, 1.0, 1.0, 1.0};

//----------------------------------------------------------------------------
Definition::Definition(const std::string &myType,
                                         smtk::attribute::DefinitionPtr myBaseDef,
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
  if (myBaseDef)
    {
    this->m_baseItemOffset = myBaseDef->numberOfItemDefinitions();
    }
  else
    {
    this->m_baseItemOffset = 0;
    }
}
//----------------------------------------------------------------------------
Definition::~Definition()
{
}
//----------------------------------------------------------------------------
bool Definition::isA(smtk::attribute::ConstDefinitionPtr targetDef) const
{
  // Walk up the inheritence tree until we either hit the root or
  // encounter this definition
  const Definition *def = this;
  for (def = this; def; def = def->m_baseDefinition.get())
    {
    if (def == targetDef.get())
      {
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
bool Definition::conflicts(smtk::attribute::DefinitionPtr def) const
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
   smtk::attribute::ConstDefinitionPtr baseDef =
     this->m_manager->findIsUniqueBaseClass(def);
  // See if the other definition is derived from this base definition.
  // If it is not then we know there is no conflict
  return def->isA(baseDef);
}

//----------------------------------------------------------------------------
bool Definition::associatesWithVertex() const
{
  return smtk::model::isVertex(this->m_associationMask);
}
//----------------------------------------------------------------------------
bool Definition::associatesWithEdge() const
{
  return smtk::model::isEdge(this->m_associationMask);
}
//----------------------------------------------------------------------------
bool Definition::associatesWithFace() const
{
  return smtk::model::isFace(this->m_associationMask);
}
//----------------------------------------------------------------------------
bool Definition::associatesWithVolume() const
{
  return smtk::model::isVolume(this->m_associationMask);
}
//----------------------------------------------------------------------------
bool Definition::associatesWithModel() const
{
  return smtk::model::isModelEntity(this->m_associationMask);
}
//----------------------------------------------------------------------------
bool Definition::associatesWithGroup() const
{
  return smtk::model::isGroupEntity(this->m_associationMask);
}

//----------------------------------------------------------------------------
bool
Definition::canBeAssociated(smtk::model::Cursor /*entity*/,
                            std::vector<Attribute *>* /*inConflicts*/) const
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
  smtk::attribute::ItemPtr comp;
  std::size_t i, j, n = this->m_itemDefs.size();
  j = att->numberOfItems();
  for (i = 0; i < n; i++, j++)
    {
    comp = this->m_itemDefs[i]->buildItem(att, static_cast<int>(j));
    comp->setDefinition(this->m_itemDefs[i]);
    att->addItem(comp);
    }
}
//----------------------------------------------------------------------------
bool Definition::isMemberOf(const std::vector<std::string> &inCategories) const
{
  std::size_t i, n = inCategories.size();
  for (i = 0; i < n; i++)
    {
    if (this->isMemberOf(inCategories[i]))
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool Definition::addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
{
  // First see if there is a item by the same name
  if (this->findItemPosition(cdef->name()) >= 0)
    {
    return false;
    }
  std::size_t n = this->m_itemDefs.size();
  this->m_itemDefs.push_back(cdef);
  this->m_itemDefPositions[cdef->name()] = static_cast<int>(n);
  this->updateDerivedDefinitions();
  return true;
}
//----------------------------------------------------------------------------
void Definition::updateDerivedDefinitions()
{
  DefinitionPtr def = this->pointer();
  if (def)
    {
    this->m_manager->updateDerivedDefinitionIndexOffsets(def);
    }
}
//----------------------------------------------------------------------------
void Definition::setCategories()
{
  if (this->m_baseDefinition)
    {
    this->m_categories = this->m_baseDefinition->m_categories;
    }
  else
    {
    this->m_categories.clear();
    }
  std::size_t i, n = this->m_itemDefs.size();
  for (i = 0; i < n; i++)
    {
    this->m_itemDefs[i]->updateCategories();
    const std::set<std::string> &itemCats = this->m_itemDefs[i]->categories();
    this->m_categories.insert(itemCats.begin(), itemCats.end());
    }
}
//----------------------------------------------------------------------------
smtk::attribute::DefinitionPtr Definition::pointer() const
{
  Manager *m = this->manager();
  if (m)
    {
    return m->findDefinition(this->m_type);
    }
  return smtk::attribute::DefinitionPtr();
}
//----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr Definition::itemDefinition(int ith) const
{
  // Is the item in this defintion?
  if (ith >= static_cast<int>(this->m_baseItemOffset))
    {
    return this->m_itemDefs[static_cast<std::size_t>(ith-this->m_baseItemOffset)];
    }
  else if (this->m_baseDefinition)
    {
    return this->m_baseDefinition->itemDefinition(ith);
    }
  return smtk::attribute::ItemDefinitionPtr();
}
 //----------------------------------------------------------------------------
int Definition::findItemPosition(const std::string &name) const
{
  std::map<std::string, int>::const_iterator it;
  it = this->m_itemDefPositions.find(name);
  if (it == this->m_itemDefPositions.end())
    {
    // Check the base definition if there is one
    if (this->m_baseDefinition)
      {
      return this->m_baseDefinition->findItemPosition(name);
      }
    else
      {
      return -1; // named item doesn't exist
      }
    }
  return it->second + static_cast<int>(this->m_baseItemOffset);
}
//----------------------------------------------------------------------------
