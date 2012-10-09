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
#include "attribute/Attribute.h"
#include "attribute/Definition.h"
#include "attribute/RootSection.h"
#include <iostream>
#include <sstream>
#include <queue>

using namespace slctk::attribute; 

//----------------------------------------------------------------------------
Manager::Manager(): m_nextAttributeId(0), m_rootSection(new RootSection(""))
{
}

//----------------------------------------------------------------------------
Manager::~Manager()
{
  std::map<std::string,  slctk::AttributeDefinitionPtr>::const_iterator it;
  for (it = this->m_definitions.begin(); it != this->m_definitions.end(); it++)
    {
    // Decouple all defintions from this manager
    (*it).second->clearManager();
    }
 }

//----------------------------------------------------------------------------
slctk::AttributeDefinitionPtr
Manager::createDefinition(const std::string &typeName, 
                          const std::string &baseTypeName)
{
  slctk::AttributeDefinitionPtr def = this->findDefinition(typeName);
  // Does this definition already exist
  if (def != NULL)
    {
    return slctk::AttributeDefinitionPtr();
    }

  // If baseTypeName is not empty then it better exist
  if (baseTypeName != "")
    {
    def = this->findDefinition(baseTypeName);
    if (def == NULL)
      {
      return slctk::AttributeDefinitionPtr();
      }
    }
  slctk::AttributeDefinitionPtr newDef(new Definition(typeName, def, this));
  this->m_definitions[typeName] = newDef;
  if (def != NULL)
    {
    // Need to add this new definition to the list of derived defs
    this->m_derivedDefInfo[def].insert(newDef);
    }
  return newDef;
}

//----------------------------------------------------------------------------
slctk::AttributeDefinitionPtr
Manager::createDefinition(const std::string &typeName, 
                          slctk::AttributeDefinitionPtr baseDef)
{
  slctk::AttributeDefinitionPtr  def = this->findDefinition(typeName);
  // Does this definition already exist or if the base def is not part
  // of this manger
  if (!((def == NULL) && ((baseDef == NULL) || ((baseDef->manager() == this)))))
    {
    return slctk::AttributeDefinitionPtr();
    }

  slctk::AttributeDefinitionPtr newDef(new Definition(typeName, baseDef, this));
  this->m_definitions[typeName] = newDef;
  if (baseDef != NULL)
    {
    // Need to add this new definition to the list of derived defs
    this->m_derivedDefInfo[baseDef].insert(newDef);
    }
  return newDef;
}

//----------------------------------------------------------------------------
slctk::AttributePtr Manager::createAttribute(const std::string &name,
                                             slctk::AttributeDefinitionPtr def)
{
  // Make sure the definition belongs to this manager or if the definition
  // is abstract
  if ((def->manager() != this) || def->isAbstract())
    {
    return slctk::AttributePtr();
    }
  // Next we need to check to see if an attribute exists by the same name
  slctk::AttributePtr a = this->findAttribute(name);
  if (a != NULL)
    {
    return slctk::AttributePtr();
    }
  a = slctk::AttributePtr(new Attribute(name, def, this->m_nextAttributeId++));
  this->m_attributeClusters[def->type()].insert(a);
  this->m_attributes[name] = a;
  this->m_attributeIdMap[a->id()] = a;
  return a;
}

//----------------------------------------------------------------------------
slctk::AttributePtr Manager::createAttribute(const std::string &typeName)
{
  slctk::AttributePtr att =
    this->createAttribute(this->createUniqueName(typeName), typeName,
                          this->m_nextAttributeId);
  if (att != NULL)
    {
    this->m_nextAttributeId++;
    }
  return att;
}

//----------------------------------------------------------------------------
slctk::AttributePtr Manager::createAttribute(const std::string &name,
                                             const std::string &typeName)
{
  slctk::AttributePtr att = this->createAttribute(name, typeName,
                                                  this->m_nextAttributeId);
  if (att != NULL)
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
slctk::AttributePtr Manager::createAttribute(const std::string &name,
                                             slctk::AttributeDefinitionPtr def,
                                             unsigned long id)
{
  // First we need to check to see if an attribute exists by the same name
  slctk::AttributePtr a = this->findAttribute(name);
  if (a != NULL)
    {
    return slctk::AttributePtr();
    }

  a = slctk::AttributePtr(new Attribute(name, def, id));
  this->m_attributeClusters[def->type()].insert(a);
  this->m_attributes[name] = a;
  this->m_attributeIdMap[id] = a;
  return a;
}
//----------------------------------------------------------------------------
slctk::AttributePtr Manager::createAttribute(const std::string &name,
                                             const std::string &typeName,
                                             unsigned long id)
{
  // First we need to check to see if an attribute exists by the same name
  slctk::AttributePtr a = this->findAttribute(name);
  if (a != NULL)
    {
    return slctk::AttributePtr();
    }

  // Second we need to find the definition that corresponds to the type and make sure it
  // is not abstract
  slctk::AttributeDefinitionPtr def = this->findDefinition(typeName);
  if ((def == NULL) || def->isAbstract())
    {
    return slctk::AttributePtr();
    }
  a = slctk::AttributePtr(new Attribute(name, def, id));
  this->m_attributeClusters[typeName].insert(a);
  this->m_attributes[name] = a;
  this->m_attributeIdMap[id] = a;
  return a;
}
//----------------------------------------------------------------------------
bool Manager::removeAttribute(slctk::AttributePtr att)
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
void Manager::findDefinitions(long mask, std::vector<slctk::AttributeDefinitionPtr> &result) const
{
  slctk::AttributeDefinitionPtr def;
  result.clear();
  std::map<std::string,  slctk::AttributeDefinitionPtr>::const_iterator it;
  for (it = this->m_definitions.begin(); it != this->m_definitions.end(); it++)
    {
    def =  (*it).second;
    if ((!def->isAbstract()) && ((def->associationMask() & mask) == mask))
      {
      result.push_back(def);
      }
    }
}

//----------------------------------------------------------------------------
void Manager::
findAttributes(slctk::AttributeDefinitionPtr def, 
               std::vector<slctk::AttributePtr> &result) const
{
  result.clear();
  if ((def != NULL) && (def->manager() == this))
    {
    this->internalFindAttributes(def, result);
    }
}
//----------------------------------------------------------------------------
void Manager::
internalFindAttributes(slctk::AttributeDefinitionPtr def, 
                       std::vector<slctk::AttributePtr> &result) const
{
  if (!def->isAbstract())
    {
    std::map<std::string, std::set<slctk::AttributePtr> >::const_iterator it;
    it = this->m_attributeClusters.find(def->type());
    if (it != this->m_attributeClusters.end())
      {
      result.insert(result.end(), it->second.begin(), it->second.end());
      }
    }
  std::map<slctk::AttributeDefinitionPtr,
    std::set<slctk::WeakAttributeDefinitionPtr> >::const_iterator dit;
  dit = this->m_derivedDefInfo.find(def);
  if (dit == this->m_derivedDefInfo.end())
    {
    return;
    }
  std::set<slctk::WeakAttributeDefinitionPtr>::const_iterator defIt;
  for (defIt = dit->second.begin(); defIt != dit->second.end(); defIt++)
    {
    this->internalFindAttributes(defIt->lock(), result);
    }
}

//----------------------------------------------------------------------------
bool Manager::rename(slctk::AttributePtr att, const std::string &newName)
{
  // Make sure that this manager is managing this attribute
  if (att->manager() != this)
    {
    return false;
    }
  // Make sure that the new name doesn't exists
  slctk::AttributePtr a = this->findAttribute(newName);
  if (a != NULL)
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
    slctk::AttributePtr a = this->findAttribute(newName);
    if (a == NULL)
      {
      return newName;
      }
    }
  return "";
}
//----------------------------------------------------------------------------
void Manager::
findBaseDefinitions(std::vector<slctk::AttributeDefinitionPtr> &result) const
{
  result.clear();
  // Insert all top most definitions into the queue
  std::map<std::string,  slctk::AttributeDefinitionPtr>::const_iterator it;
  for (it = this->m_definitions.begin(); it != this->m_definitions.end(); it++)
    {
    if (it->second->baseDefinition() == NULL)
      {
      result.push_back(it->second);
      }
    }
}
//----------------------------------------------------------------------------
void Manager::updateCategories()
{
  std::queue<AttributeDefinitionPtr> toBeProcessed;
  // Insert all top most definitions into the queue
  std::map<std::string,  slctk::AttributeDefinitionPtr>::const_iterator it;
  for (it = this->m_definitions.begin(); it != this->m_definitions.end(); it++)
    {
    if (it->second->baseDefinition() == NULL)
      {
      toBeProcessed.push(it->second);
      }
    }
  // Now for each definition in the queue do the following:
  // update its categories (which will be used by def derived from it
  // Add all of its derived definitions into the queue
  slctk::AttributeDefinitionPtr def;
  while (!toBeProcessed.empty())
    {
    def = toBeProcessed.front();
    def->setCategories();
    // Does this definition have derived defs from it?
    std::map<slctk::AttributeDefinitionPtr,
      std::set<slctk::WeakAttributeDefinitionPtr> >::iterator dit = 
      this-> m_derivedDefInfo.find(def);
    if (dit != this-> m_derivedDefInfo.end())
      {
      std::set<slctk::WeakAttributeDefinitionPtr>::iterator ddit;
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
derivedDefinitions(slctk::AttributeDefinitionPtr def,
                   std::vector<slctk::AttributeDefinitionPtr> &result) const
{
  std::map<slctk::AttributeDefinitionPtr, 
    std::set<slctk::WeakAttributeDefinitionPtr> >::const_iterator it;
  it = this->m_derivedDefInfo.find(def);
  result.clear();
  if (it == this->m_derivedDefInfo.end())
    {
    return;
    }
  int i, n = it->second.size();
  result.resize(n);
  std::set<slctk::WeakAttributeDefinitionPtr>::const_iterator dit;
  for (i = 0, dit = it->second.begin(); i < n; dit++, i++)
    {
    result[i] = dit->lock();
    }
}
    
//----------------------------------------------------------------------------
