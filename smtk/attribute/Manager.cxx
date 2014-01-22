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
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/model/Storage.h"
#include "smtk/view/Root.h"
#include <iostream>
#include <sstream>
#include <queue>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
Manager::Manager(): m_nextAttributeId(0), m_rootView(new view::Root(""))
{
}

//----------------------------------------------------------------------------
Manager::~Manager()
{
  std::map<std::string,  smtk::attribute::DefinitionPtr>::const_iterator it;
  for (it = this->m_definitions.begin(); it != this->m_definitions.end(); it++)
    {
    // Decouple all defintions from this manager
    (*it).second->clearManager();
    }
 }

//----------------------------------------------------------------------------
smtk::util::Resource::Type
Manager::resourceType() const
{
  return smtk::util::Resource::ATTRIBUTE;
}

//----------------------------------------------------------------------------
smtk::attribute::DefinitionPtr
Manager::createDefinition(const std::string &typeName,
                          const std::string &baseTypeName)
{
  smtk::attribute::DefinitionPtr def = this->findDefinition(typeName);
  // Does this definition already exist
  if (def)
    {
    return smtk::attribute::DefinitionPtr();
    }

  // If baseTypeName is not empty then it better exist
  if (baseTypeName != "")
    {
    def = this->findDefinition(baseTypeName);
    if (!def)
      {
      return smtk::attribute::DefinitionPtr();
      }
    }
  smtk::attribute::DefinitionPtr newDef(new Definition(typeName, def, this));
  this->m_definitions[typeName] = newDef;
  if (def)
    {
    // Need to add this new definition to the list of derived defs
    this->m_derivedDefInfo[def].insert(newDef);
    }
  return newDef;
}

//----------------------------------------------------------------------------
smtk::attribute::DefinitionPtr
Manager::createDefinition(const std::string &typeName,
                          smtk::attribute::DefinitionPtr baseDef)
{
  smtk::attribute::DefinitionPtr  def = this->findDefinition(typeName);
  // Does this definition already exist or if the base def is not part
  // of this manger
  if (!(!def && (!baseDef || ((baseDef->manager() == this)))))
    {
    return smtk::attribute::DefinitionPtr();
    }

  smtk::attribute::DefinitionPtr newDef(new Definition(typeName, baseDef, this));
  this->m_definitions[typeName] = newDef;
  if (baseDef)
    {
    // Need to add this new definition to the list of derived defs
    this->m_derivedDefInfo[baseDef].insert(newDef);
    }
  return newDef;
}

//----------------------------------------------------------------------------
smtk::attribute::AttributePtr Manager::createAttribute(const std::string &name,
                                             smtk::attribute::DefinitionPtr def)
{
  // Make sure the definition belongs to this manager or if the definition
  // is abstract
  if ((def->manager() != this) || def->isAbstract())
    {
    return smtk::attribute::AttributePtr();
    }
  // Next we need to check to see if an attribute exists by the same name
  smtk::attribute::AttributePtr a = this->findAttribute(name);
  if (a)
    {
    return smtk::attribute::AttributePtr();
    }
  a = Attribute::New(name, def, this->m_nextAttributeId++);
  this->m_attributeClusters[def->type()].insert(a);
  this->m_attributes[name] = a;
  this->m_attributeIdMap[a->id()] = a;
  return a;
}

//----------------------------------------------------------------------------
smtk::attribute::AttributePtr Manager::createAttribute(const std::string &typeName)
{
  smtk::attribute::AttributePtr att =
    this->createAttribute(this->createUniqueName(typeName), typeName,
                          this->m_nextAttributeId);
  if (att)
    {
    this->m_nextAttributeId++;
    }
  return att;
}

//----------------------------------------------------------------------------
smtk::attribute::AttributePtr Manager::createAttribute(const std::string &name,
                                             const std::string &typeName)
{
  smtk::attribute::AttributePtr att = this->createAttribute(name, typeName,
                                                  this->m_nextAttributeId);
  if (att)
    {
    this->m_nextAttributeId++;
    }
  return att;
}

//----------------------------------------------------------------------------
void Manager::recomputeNextAttributeID()
{
  std::map<std::string, AttributePtr>::const_iterator it;
  for (it = this->m_attributes.begin(); it != this->m_attributes.end(); it++)
    {
    if (it->second->id() > this->m_nextAttributeId)
      {
      this->m_nextAttributeId = it->second->id() + 1;
      }
    }
}
//----------------------------------------------------------------------------
// For Reader classes
//----------------------------------------------------------------------------
smtk::attribute::AttributePtr Manager::createAttribute(const std::string &name,
                                             smtk::attribute::DefinitionPtr def,
                                             unsigned long id)
{
  // First we need to check to see if an attribute exists by the same name
  smtk::attribute::AttributePtr a = this->findAttribute(name);
  if (a)
    {
    return smtk::attribute::AttributePtr();
    }

  a = Attribute::New(name, def, id);
  this->m_attributeClusters[def->type()].insert(a);
  this->m_attributes[name] = a;
  this->m_attributeIdMap[id] = a;
  return a;
}
//----------------------------------------------------------------------------
smtk::attribute::AttributePtr Manager::createAttribute(const std::string &name,
                                             const std::string &typeName,
                                             unsigned long id)
{
  // First we need to check to see if an attribute exists by the same name
  smtk::attribute::AttributePtr a = this->findAttribute(name);
  if (a)
    {
    return smtk::attribute::AttributePtr();
    }

  // Second we need to find the definition that corresponds to the type and make sure it
  // is not abstract
  smtk::attribute::DefinitionPtr def = this->findDefinition(typeName);
  if (!def || def->isAbstract())
    {
    return smtk::attribute::AttributePtr();
    }
  a = Attribute::New(name, def, id);
  this->m_attributeClusters[typeName].insert(a);
  this->m_attributes[name] = a;
  this->m_attributeIdMap[id] = a;
  return a;
}
//----------------------------------------------------------------------------
bool Manager::removeAttribute(smtk::attribute::AttributePtr att)
{
  // Make sure that this manager is managing this attribute
  if (att->manager() != this)
    {
    return false;
    }
  this->m_attributes.erase(att->name());
  this->m_attributeIdMap.erase(att->id());
  this->m_attributeClusters[att->type()].erase(att);
  return true;
}
//----------------------------------------------------------------------------
void Manager::findDefinitions(long mask, std::vector<smtk::attribute::DefinitionPtr> &result) const
{
  smtk::attribute::DefinitionPtr def;
  result.clear();
  std::map<std::string,  smtk::attribute::DefinitionPtr>::const_iterator it;
  for (it = this->m_definitions.begin(); it != this->m_definitions.end(); it++)
    {
    def =  (*it).second;
    // the mask could be 'ef', so this will return both 'e' and 'f'
    if (!def->isAbstract() && def->associationMask() != 0 &&
      ((def->associationMask() & mask) != 0))
      {
      result.push_back(def);
      }
    }
}

//----------------------------------------------------------------------------
void Manager::
findAttributes(smtk::attribute::DefinitionPtr def,
               std::vector<smtk::attribute::AttributePtr> &result) const
{
  result.clear();
  if (def && (def->manager() == this))
    {
    this->internalFindAttributes(def, result);
    }
}
//----------------------------------------------------------------------------
void Manager::
internalFindAttributes(smtk::attribute::DefinitionPtr def,
                       std::vector<smtk::attribute::AttributePtr> &result) const
{
  if (!def->isAbstract())
    {
    std::map<std::string, std::set<smtk::attribute::AttributePtr> >::const_iterator it;
    it = this->m_attributeClusters.find(def->type());
    if (it != this->m_attributeClusters.end())
      {
      result.insert(result.end(), it->second.begin(), it->second.end());
      }
    }
  std::map<smtk::attribute::DefinitionPtr,
           smtk::attribute::WeakDefinitionPtrSet >::const_iterator dit;
  dit = this->m_derivedDefInfo.find(def);
  if (dit == this->m_derivedDefInfo.end())
    {
    return;
    }
  smtk::attribute::WeakDefinitionPtrSet::const_iterator defIt;
  for (defIt = dit->second.begin(); defIt != dit->second.end(); defIt++)
    {
    this->internalFindAttributes(defIt->lock(), result);
    }
}

//----------------------------------------------------------------------------
void Manager::
findAllDerivedDefinitions(smtk::attribute::DefinitionPtr def,
                          bool onlyConcrete,
                          std::vector<smtk::attribute::DefinitionPtr> &result) const
{
  result.clear();
  if (def && (def->manager() == this))
    {
    this->internalFindAllDerivedDefinitions(def, onlyConcrete, result);
    }
}
//----------------------------------------------------------------------------
void Manager::
internalFindAllDerivedDefinitions(smtk::attribute::DefinitionPtr def,
                                  bool onlyConcrete,
                                  std::vector<smtk::attribute::DefinitionPtr> &result) const
{
  if (!(def->isAbstract() && onlyConcrete))
    {
    result.push_back(def);
    }
  std::map<smtk::attribute::DefinitionPtr,
    smtk::attribute::WeakDefinitionPtrSet >::const_iterator dit;
  dit = this->m_derivedDefInfo.find(def);
  if (dit == this->m_derivedDefInfo.end())
    {
    return;
    }
  smtk::attribute::WeakDefinitionPtrSet::const_iterator defIt;
  for (defIt = dit->second.begin(); defIt != dit->second.end(); defIt++)
    {
    this->internalFindAllDerivedDefinitions(defIt->lock(), onlyConcrete, result);
    }
}

//----------------------------------------------------------------------------
bool Manager::rename(smtk::attribute::AttributePtr att, const std::string &newName)
{
  // Make sure that this manager is managing this attribute
  if (att->manager() != this)
    {
    return false;
    }
  // Make sure that the new name doesn't exists
  smtk::attribute::AttributePtr a = this->findAttribute(newName);
  if (a)
    {
    return false;
    }
  this->m_attributes.erase(att->name());
  att->setName(newName);
  this->m_attributes[newName] = att;
  return true;
}
//----------------------------------------------------------------------------
std::string Manager::createUniqueName(const std::string &type) const
{
  int i = 0;
  std::string base = type, newName;
  base.append("-");
  while (1)
    {
    std::ostringstream n;
    n << i++;
    newName = base + n.str();
    // Make sure that the new name doesn't exists
    smtk::attribute::AttributePtr a = this->findAttribute(newName);
    if (!a)
      {
      return newName;
      }
    }
  return "";
}
//----------------------------------------------------------------------------
void Manager::
findBaseDefinitions(std::vector<smtk::attribute::DefinitionPtr> &result) const
{
  result.clear();
  // Insert all top most definitions into the queue
  std::map<std::string,  smtk::attribute::DefinitionPtr>::const_iterator it;
  for (it = this->m_definitions.begin(); it != this->m_definitions.end(); it++)
    {
    if (!it->second->baseDefinition())
      {
      result.push_back(it->second);
      }
    }
}
//----------------------------------------------------------------------------
void Manager::updateCategories()
{
  std::queue<attribute::DefinitionPtr> toBeProcessed;
  // Insert all top most definitions into the queue
  std::map<std::string,  smtk::attribute::DefinitionPtr>::const_iterator it;
  for (it = this->m_definitions.begin(); it != this->m_definitions.end(); it++)
    {
    if (!it->second->baseDefinition())
      {
      toBeProcessed.push(it->second);
      }
    }
  // Now for each definition in the queue do the following:
  // update its categories (which will be used by def derived from it
  // Add all of its derived definitions into the queue
  smtk::attribute::DefinitionPtr def;
  while (!toBeProcessed.empty())
    {
    def = toBeProcessed.front();
    def->setCategories();
    // Does this definition have derived defs from it?
    std::map<smtk::attribute::DefinitionPtr,
             smtk::attribute::WeakDefinitionPtrSet>::iterator dit =
      this-> m_derivedDefInfo.find(def);
    if (dit != this-> m_derivedDefInfo.end())
      {
      smtk::attribute::WeakDefinitionPtrSet::iterator ddit;
      for (ddit = dit->second.begin(); ddit != dit->second.end(); ddit++)
        {
        toBeProcessed.push(ddit->lock());
        }
      }
    toBeProcessed.pop();
    }
  // Now all of the definitions have been processed we need to combine all
  // of their categories to form the managers
  this->m_categories.clear();
  for (it = this->m_definitions.begin(); it != this->m_definitions.end(); it++)
    {
    this->m_categories.insert(it->second->categories().begin(),
                              it->second->categories().end());
    }
}
//----------------------------------------------------------------------------
void Manager::
derivedDefinitions(smtk::attribute::DefinitionPtr def,
                   std::vector<smtk::attribute::DefinitionPtr> &result) const
{
  std::map<smtk::attribute::DefinitionPtr,
           smtk::attribute::WeakDefinitionPtrSet >::const_iterator it;
  it = this->m_derivedDefInfo.find(def);
  result.clear();
  if (it == this->m_derivedDefInfo.end())
    {
    return;
    }
  std::size_t i, n = it->second.size();
  result.resize(n);
  smtk::attribute::WeakDefinitionPtrSet::const_iterator dit;
  for (i = 0, dit = it->second.begin(); i < n; dit++, i++)
    {
    result[i] = dit->lock();
    }
}
//----------------------------------------------------------------------------
smtk::attribute::ConstDefinitionPtr Manager::findIsUniqueBaseClass(
  smtk::attribute::DefinitionPtr attDef) const
{
  if(!attDef.get() || !attDef->isUnique() || !attDef->baseDefinition().get())
    {
    return attDef;
    }
  smtk::attribute::DefinitionPtr uDef = attDef, def;
  while (1 && uDef.get())
    {
    def = uDef->baseDefinition();
    if (!def.get() || (!def->isUnique()))
      {
      return uDef;
      }
    uDef = def;
    }
  return smtk::attribute::ConstDefinitionPtr();
}
//----------------------------------------------------------------------------
void Manager::setRefStorage(smtk::model::StoragePtr refstorage)
{
  smtk::model::StoragePtr curStorage = this->m_refStorage.lock();
  if (curStorage && curStorage != refstorage)
    {
    curStorage->setAttributeManager(NULL, false);
    }
  this->m_refStorage = refstorage;
  if (refstorage && this->m_refStorage.lock()->attributeManager() != this)
    {
    refstorage->setAttributeManager(this, false);
    }
}
//----------------------------------------------------------------------------
void
Manager::updateDerivedDefinitionIndexOffsets(smtk::attribute::DefinitionPtr def)
{
  WeakDefinitionPtrSet ddefs = m_derivedDefInfo[def];
  WeakDefinitionPtrSet::iterator iter;
  smtk::attribute::DefinitionPtr d;
  for (iter = ddefs.begin(); iter != ddefs.end(); ++iter)
    {
    d = iter->lock();
    if (!d)
      {
      continue;
      }
    d->resetItemOffset();
    this->updateDerivedDefinitionIndexOffsets(d);
    }
}
//----------------------------------------------------------------------------
