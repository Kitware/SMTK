//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Resource.h"

#include "smtk/view/Configuration.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/GroupItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/attribute/VoidItemDefinition.h"

#include "smtk/model/Resource.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"

#include "smtk/common/UUID.h"

#include <iostream>
#include <numeric>
#include <queue>
#include <sstream>

using namespace smtk::attribute;

constexpr smtk::resource::Links::RoleType Resource::AssociationRole;
constexpr smtk::resource::Links::RoleType Resource::ReferenceRole;

Resource::Resource(const smtk::common::UUID& myID, smtk::resource::ManagerPtr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::resource::Resource>(myID, manager)
{
}

Resource::Resource(smtk::resource::ManagerPtr manager)
  : smtk::resource::DerivedFrom<Resource, smtk::resource::Resource>(manager)
{
}

Resource::~Resource()
{
  std::map<std::string, smtk::attribute::DefinitionPtr>::const_iterator it;
  for (it = m_definitions.begin(); it != m_definitions.end(); it++)
  {
    // Decouple all defintions from this Resource
    (*it).second->clearResource();
  }
}

smtk::attribute::DefinitionPtr Resource::createDefinition(
  const std::string& typeName, const std::string& baseTypeName)
{
  smtk::attribute::DefinitionPtr def = this->findDefinition(typeName);
  // Does this definition already exist
  if (def)
  {
    return smtk::attribute::DefinitionPtr();
  }

  // If baseTypeName is not empty then it better exist
  if (!baseTypeName.empty())
  {
    def = this->findDefinition(baseTypeName);
    if (!def)
    {
      return smtk::attribute::DefinitionPtr();
    }
  }
  smtk::attribute::DefinitionPtr newDef(new Definition(typeName, def, shared_from_this()));
  m_definitions[typeName] = newDef;
  if (def)
  {
    // Need to add this new definition to the list of derived defs
    m_derivedDefInfo[def].insert(newDef);
  }
  this->setClean(false);
  return newDef;
}

smtk::attribute::DefinitionPtr Resource::createDefinition(
  const std::string& typeName, smtk::attribute::DefinitionPtr baseDef)
{
  smtk::attribute::DefinitionPtr def = this->findDefinition(typeName);
  // Does this definition already exist or if the base def is not part
  // of this manger
  if (!(!def && (!baseDef || ((baseDef->resource() == shared_from_this())))))
  {
    return smtk::attribute::DefinitionPtr();
  }

  smtk::attribute::DefinitionPtr newDef(new Definition(typeName, baseDef, shared_from_this()));
  m_definitions[typeName] = newDef;
  if (baseDef)
  {
    // Need to add this new definition to the list of derived defs
    m_derivedDefInfo[baseDef].insert(newDef);
  }
  this->setClean(false);
  return newDef;
}

bool Resource::removeDefinition(DefinitionPtr def)
{
  if (!def || def->resource() != shared_from_this())
  {
    std::cerr << "ERROR: " << def->type() << " does not belong to the specified"
                                             " attribute Resource!";
    return false;
  }

  const auto childrenIt = m_derivedDefInfo.find(def);
  const bool hasChildren = childrenIt != m_derivedDefInfo.cend() && !(*childrenIt).second.empty();
  if (hasChildren)
  {
    std::cerr << "ERROR: Removing base Definition instances is not"
                 " supported!\n";
    return false;
  }

  const auto baseDef = def->baseDefinition();
  auto& siblings = m_derivedDefInfo[baseDef];
  const auto defIt = siblings.find(def);
  if (defIt != siblings.cend())
  {
    siblings.erase(defIt);
  }

  if (childrenIt != m_derivedDefInfo.cend())
  {
    m_derivedDefInfo.erase(childrenIt);
  }

  m_definitions.erase(def->type());
  this->setClean(false);
  return true;
}

smtk::attribute::AttributePtr Resource::createAttribute(
  const std::string& name, smtk::attribute::DefinitionPtr def)
{
  // Make sure the definition belongs to this Resource or if the definition
  // is abstract
  if ((def->resource() != shared_from_this()) || def->isAbstract())
  {
    return smtk::attribute::AttributePtr();
  }
  // Next we need to check to see if an attribute exists by the same name
  smtk::attribute::AttributePtr a = this->findAttribute(name);
  if (a)
  {
    return smtk::attribute::AttributePtr();
  }
  a = Attribute::New(name, def);
  a->build();
  m_attributeClusters[def->type()].insert(a);
  m_attributes[name] = a;
  m_attributeIdMap[a->id()] = a;
  this->setClean(false);
  return a;
}

smtk::attribute::AttributePtr Resource::createAttribute(smtk::attribute::DefinitionPtr def)
{
  smtk::attribute::AttributePtr att =
    this->createAttribute(this->createUniqueName(def->rootName()), def);
  return att;
}

smtk::attribute::AttributePtr Resource::createAttribute(const std::string& typeName)
{
  smtk::attribute::DefinitionPtr def = this->findDefinition(typeName);
  if (def == nullptr)
  {
    return smtk::attribute::AttributePtr();
  }

  smtk::attribute::AttributePtr att =
    this->createAttribute(this->createUniqueName(def->rootName()), def);
  return att;
}

smtk::attribute::AttributePtr Resource::createAttribute(
  const std::string& name, const std::string& typeName)
{
  smtk::attribute::DefinitionPtr def = this->findDefinition(typeName);
  if ((def == nullptr) || def->isAbstract())
  {
    return smtk::attribute::AttributePtr();
  }
  smtk::attribute::AttributePtr att = this->createAttribute(name, def);
  return att;
}

void Resource::definitions(std::vector<smtk::attribute::DefinitionPtr>& result, bool sortList) const
{
  result.resize(m_definitions.size());
  int i;
  if (!sortList)
  {
    std::map<std::string, DefinitionPtr>::const_iterator it;
    for (it = m_definitions.begin(), i = 0; it != m_definitions.end(); it++, i++)
    {
      result[i] = it->second;
    }
    return;
  }
  std::vector<std::string> keys;
  for (auto info : m_definitions)
  {
    keys.push_back(info.first);
  }
  std::sort(keys.begin(), keys.end());
  std::vector<std::string>::const_iterator kit;
  for (kit = keys.begin(), i = 0; kit != keys.end(); kit++, i++)
  {
    result[i] = m_definitions.at(*(kit));
  }
}
void Resource::attributes(std::vector<smtk::attribute::AttributePtr>& result) const
{
  std::map<std::string, AttributePtr>::const_iterator it;
  result.resize(m_attributes.size());
  int i;
  for (it = m_attributes.begin(), i = 0; it != m_attributes.end(); it++, i++)
  {
    result[i] = it->second;
  }
}

// For Reader classes - Note that since these methods are restoring an attribute
// into the resource it does not call setClean(false)
smtk::attribute::AttributePtr Resource::createAttribute(
  const std::string& name, smtk::attribute::DefinitionPtr def, const smtk::common::UUID& id)
{
  // Lets make sure the definition is valid
  if ((def == nullptr) || (def->resource() != shared_from_this()) || def->isAbstract())
  {
    return smtk::attribute::AttributePtr();
  }

  // We need to check to see if an attribute exists by the same name
  smtk::attribute::AttributePtr a = this->findAttribute(name);
  if (a)
  {
    return smtk::attribute::AttributePtr();
  }

  a = Attribute::New(name, def, id);
  a->build();
  m_attributeClusters[def->type()].insert(a);
  m_attributes[name] = a;
  m_attributeIdMap[id] = a;
  return a;
}

smtk::attribute::AttributePtr Resource::createAttribute(
  const std::string& name, const std::string& typeName, const smtk::common::UUID& id)
{
  smtk::attribute::DefinitionPtr def = this->findDefinition(typeName);
  if (def == nullptr)
  {
    return smtk::attribute::AttributePtr();
  }
  return this->createAttribute(name, def, id);
}

bool Resource::removeAttribute(smtk::attribute::AttributePtr att)
{
  // Make sure that this Resource is managing this attribute
  if (!att || att->attributeResource() != shared_from_this())
  {
    return false;
  }
  if (!att->removeAllAssociations(false))
  {
    return false;
  }

  m_attributes.erase(att->name());
  m_attributeIdMap.erase(att->id());
  m_attributeClusters[att->type()].erase(att);
  this->setClean(false);
  return true;
}

/**\brief Find the attribute definitions that can be associated with \a mask.
  *
  */
void Resource::findDefinitions(
  unsigned long mask, std::vector<smtk::attribute::DefinitionPtr>& result) const
{
  smtk::attribute::DefinitionPtr def;
  result.clear();
  std::map<std::string, smtk::attribute::DefinitionPtr>::const_iterator it;
  for (it = m_definitions.begin(); it != m_definitions.end(); it++)
  {
    def = (*it).second;
    // the mask could be 'ef', so this will return both 'e' and 'f'
    if (!def->isAbstract() && def->associationMask() != 0 && ((def->associationMask() & mask) != 0))
    {
      result.push_back(def);
    }
  }
}

smtk::attribute::AttributePtr Resource::findAttribute(
  const smtk::resource::ComponentPtr& comp, const smtk::resource::Links::RoleType& role) const
{
  for (const auto& attInfo : m_attributes)
  {
    if (attInfo.second->links().isLinkedTo(comp, role))
    {
      return attInfo.second;
    }
  }
  return smtk::attribute::AttributePtr();
}
void Resource::findAttributes(
  smtk::attribute::DefinitionPtr def, std::vector<smtk::attribute::AttributePtr>& result) const
{
  result.clear();
  if (def && (def->resource() == shared_from_this()))
  {
    this->internalFindAttributes(def, result);
  }
}

void Resource::internalFindAttributes(
  smtk::attribute::DefinitionPtr def, std::vector<smtk::attribute::AttributePtr>& result) const
{
  if (!def->isAbstract())
  {
    auto it = m_attributeClusters.find(def->type());
    if (it != m_attributeClusters.end())
    {
      result.insert(result.end(), it->second.begin(), it->second.end());
    }
  }
  auto dit = m_derivedDefInfo.find(def);
  if (dit == m_derivedDefInfo.end())
  {
    return;
  }
  for (auto defIt = dit->second.begin(); defIt != dit->second.end(); defIt++)
  {
    this->internalFindAttributes(defIt->lock(), result);
  }
}

void Resource::findAllDerivedDefinitions(smtk::attribute::DefinitionPtr def, bool onlyConcrete,
  std::vector<smtk::attribute::DefinitionPtr>& result) const
{
  result.clear();
  if (def && (def->resource() == shared_from_this()))
  {
    this->internalFindAllDerivedDefinitions(def, onlyConcrete, result);
  }
}

void Resource::internalFindAllDerivedDefinitions(smtk::attribute::DefinitionPtr def,
  bool onlyConcrete, std::vector<smtk::attribute::DefinitionPtr>& result) const
{
  if (!(def->isAbstract() && onlyConcrete))
  {
    result.push_back(def);
  }
  auto dit = m_derivedDefInfo.find(def);
  if (dit == m_derivedDefInfo.end())
  {
    return;
  }
  for (auto defIt = dit->second.begin(); defIt != dit->second.end(); defIt++)
  {
    this->internalFindAllDerivedDefinitions(defIt->lock(), onlyConcrete, result);
  }
}

bool Resource::rename(smtk::attribute::AttributePtr att, const std::string& newName)
{
  // Make sure that this Resource is managing this attribute
  if (att->resource() != shared_from_this())
  {
    return false;
  }
  // Make sure that the new name doesn't exists
  smtk::attribute::AttributePtr a = this->findAttribute(newName);
  if (a)
  {
    return false;
  }
  m_attributes.erase(att->name());
  att->setName(newName);
  m_attributes[newName] = att;
  this->setClean(false);
  return true;
}

std::string Resource::createUniqueName(const std::string& type) const
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

void Resource::findBaseDefinitions(std::vector<smtk::attribute::DefinitionPtr>& result) const
{
  result.clear();
  // Insert all top most definitions into the queue
  std::map<std::string, smtk::attribute::DefinitionPtr>::const_iterator it;
  for (it = m_definitions.begin(); it != m_definitions.end(); it++)
  {
    if (!it->second->baseDefinition())
    {
      result.push_back(it->second);
    }
  }
}

/// This method updates information passed down to Attribute and Item Definitions.
/// It should be called whenever shared information such as local categories and
/// local advance information is changed.
/// Note that this method can be called multiple times without issue.
void Resource::finalizeDefinitions()
{
  // We need to process the definitions that don't have
  // a base definition
  std::vector<DefinitionPtr> baseDefs;
  std::set<std::string> initialCats;
  this->findBaseDefinitions(baseDefs);
  // Lets apply their categories and their item definitions' categories
  for (auto& def : baseDefs)
  {
    def->applyCategories(initialCats);
    def->applyAdvanceLevels(0, 0);
  }
  // Now all of the definitions have been processed we need to combine all
  // of their categories to form the Resource's categories
  m_categories.clear();
  for (auto it = m_definitions.begin(); it != m_definitions.end(); it++)
  {
    m_categories.insert(it->second->categories().begin(), it->second->categories().end());
  }
}

void Resource::derivedDefinitions(
  smtk::attribute::DefinitionPtr def, std::vector<smtk::attribute::DefinitionPtr>& result) const
{
  auto it = m_derivedDefInfo.find(def);
  result.clear();
  if (it == m_derivedDefInfo.end())
  {
    return;
  }
  std::size_t i(0), n = it->second.size();
  result.resize(n);
  for (auto dit = it->second.begin(); i < n; dit++, i++)
  {
    result[i] = dit->lock();
  }
}

smtk::attribute::ConstDefinitionPtr Resource::findIsUniqueBaseClass(
  smtk::attribute::ConstDefinitionPtr attDef) const
{
  // If there is no definition or the definition is not
  // unique then return an empty shared pointer
  if (!attDef.get() || !attDef->isUnique())
  {
    return smtk::attribute::ConstDefinitionPtr();
  }
  // If there is no base definition then we know this
  // definiiton must be the most common unique base class
  else if (!attDef->baseDefinition().get())
  {
    return attDef;
  }
  // Keep traveling up the definition's ancestors until
  // we come to the end or we find one that isn't unique
  smtk::attribute::ConstDefinitionPtr uDef = attDef, def;
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

smtk::resource::ResourceSet Resource::associations() const
{
  auto associatedObjects = this->links().linkedTo(AssociationRole);
  smtk::resource::ResourceSet resources;
  for (auto& object : associatedObjects)
  {
    auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(object);
    if (resource != nullptr)
    {
      resources.insert(resource);
    }
  }
  return resources;
}

bool Resource::associate(const smtk::resource::ResourcePtr& resource)
{
  // Resource links allow for multiple links between the same objects. Since
  // associations are unique, we must first check if an association between this
  // resource and the resource parameter exists.
  if (!this->links().isLinkedTo(resource, AssociationRole))
  {
    if (this->links().addLinkTo(resource, AssociationRole).first != smtk::common::UUID::null())
    {
      this->setClean(false);
      return true;
    }
    else
    {
      return false;
    }
  }
  return true;
}

bool Resource::disassociate(const smtk::resource::ResourcePtr& resource)
{
  // Resource links allow for multiple links between the same objects. Since
  // associations are unique, we can erase all links from this resource to the
  // input resource that have an association role.
  if (this->links().removeLinksTo(resource, AssociationRole))
  {
    this->setClean(false);
    return true;
  }
  return false;
}

void Resource::updateDerivedDefinitionIndexOffsets(smtk::attribute::DefinitionPtr def)
{
  auto ddefs = m_derivedDefInfo[def];
  smtk::attribute::DefinitionPtr d;
  for (auto iter = ddefs.begin(); iter != ddefs.end(); ++iter)
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

void Resource::addAdvanceLevel(int level, std::string label, const double* l_color)
{
  m_advLevels[level] = label;
  this->setAdvanceLevelColor(level, l_color);
  this->setClean(false);
}

const double* Resource::advanceLevelColor(int level) const
{
  std::map<int, std::vector<double> >::const_iterator it = m_advLevelColors.find(level);
  if (it != m_advLevelColors.end() && it->second.size() == 4)
  {
    return &it->second[0];
  }
  return nullptr;
}

void Resource::setAdvanceLevelColor(int level, const double* l_color)
{
  if (l_color && m_advLevels.find(level) != m_advLevels.end())
  {
    std::vector<double> acolor(l_color, l_color + 4);
    if (m_advLevelColors[level] != acolor)
    {
      m_advLevelColors[level] = acolor;
      this->setClean(false);
    }
  }
}

// Copies attribute defintion into this Resource
// Returns smart pointer (will be empty if operation unsuccessful)
// If definition contains ValueItem instances, might have to
// copy additional definitions for expressions.
smtk::attribute::DefinitionPtr Resource::copyDefinition(
  const smtk::attribute::DefinitionPtr sourceDef, unsigned int /*options*/)
{
  // Returns defintion
  smtk::attribute::DefinitionPtr newDef = smtk::attribute::DefinitionPtr();

  // Call internal copy method
  smtk::attribute::ItemDefinition::CopyInfo info(shared_from_this());
  if (this->copyDefinitionImpl(sourceDef, info))
  {
    newDef = this->findDefinition(sourceDef->type());

    // Process exp items second
    while (!info.UnresolvedExpItems.empty())
    {
      // Check if type has been created (copied) already
      std::pair<std::string, smtk::attribute::ItemDefinitionPtr>& frontDef =
        info.UnresolvedExpItems.front();
      std::string type = frontDef.first;
      smtk::attribute::DefinitionPtr def = this->findDefinition(type);
      if (def)
      {
        smtk::attribute::ItemDefinitionPtr nextItemDef = frontDef.second;
        smtk::attribute::ValueItemDefinitionPtr valItemDef =
          smtk::dynamic_pointer_cast<smtk::attribute::ValueItemDefinition>(nextItemDef);
        valItemDef->setExpressionDefinition(def);
        info.UnresolvedExpItems.pop();
      }
      else
      {
        // Need to copy definition, first find it in the input Resource
        std::cout << "Copying \"" << type << "\" definition" << std::endl;
        smtk::attribute::DefinitionPtr nextDef = sourceDef->resource()->findDefinition(type);
        // Definition missing only if source Resource is invalid, but check anyway
        if (!nextDef)
        {
          std::cerr << "ERROR: Unable to find source definition " << type
                    << " -- copy operation incomplete" << std::endl;
          return newDef;
        }

        // Copy definition
        if (!this->copyDefinitionImpl(nextDef, info))
        {
          std::cerr << "ERROR: Unable to copy definition " << type
                    << " -- copy operation incomplete" << std::endl;
          return newDef;
        }
      }
    } // while (expressions)
  }   // if (this->copyDefinition())

  return newDef;
}

// Copies attribute definition into this Resource, returning true if successful
bool Resource::copyDefinitionImpl(
  smtk::attribute::DefinitionPtr sourceDef, smtk::attribute::ItemDefinition::CopyInfo& info)
{
  // Check for type conflict
  std::string typeName = sourceDef->type();
  if (this->findDefinition(typeName))
  {
    std::cerr << "WARNING: Will not overwrite attribute definition " << typeName << std::endl;
    return false;
  }

  // Check if input definition has a base definition
  smtk::attribute::DefinitionPtr newDef;
  smtk::attribute::DefinitionPtr sourceBaseDef = sourceDef->baseDefinition();
  if (sourceBaseDef)
  {
    // Check if base definition of this type already exists in this Resource
    std::string baseTypeName = sourceBaseDef->type();
    if (!this->findDefinition(baseTypeName))
    {
      //  If not found, copy it
      if (!this->copyDefinitionImpl(sourceBaseDef, info))
      {
        return false;
      }
    }

    // Retrieve base definition and create new def
    smtk::attribute::DefinitionPtr newBaseDef = this->findDefinition(baseTypeName);
    newDef = this->createDefinition(typeName, newBaseDef);
  }
  else
  {
    // No base definition
    newDef = this->createDefinition(typeName);
  }

  // Set contents of new definition (defer categories)
  newDef->setLabel(sourceDef->label());
  newDef->setVersion(sourceDef->version());
  newDef->setIsAbstract(sourceDef->isAbstract());
  if (sourceDef->hasLocalAdvanceLevelInfo(0))
  {
    newDef->setLocalAdvanceLevel(0, sourceDef->localAdvanceLevel(0));
  }
  if (sourceDef->hasLocalAdvanceLevelInfo(1))
  {
    newDef->setLocalAdvanceLevel(1, sourceDef->localAdvanceLevel(1));
  }
  newDef->setIsUnique(sourceDef->isUnique());
  newDef->setIsNodal(sourceDef->isNodal());
  newDef->setDetailedDescription(sourceDef->detailedDescription());
  newDef->setBriefDescription(sourceDef->briefDescription());
  if (sourceDef->isNotApplicableColorSet())
  {
    newDef->setNotApplicableColor(sourceDef->notApplicableColor());
  }
  if (sourceDef->isDefaultColorSet())
  {
    newDef->setDefaultColor(sourceDef->defaultColor());
  }
  if (sourceDef->localAssociationRule())
  {
    newDef->setLocalAssociationRule(sourceDef->localAssociationRule());
  }

  // Copy new item definitions only (i.e., not inherited item defs)
  for (std::size_t i = sourceDef->itemOffset(); i < sourceDef->numberOfItemDefinitions(); ++i)
  {
    smtk::attribute::ItemDefinitionPtr sourceItemDef =
      sourceDef->itemDefinition(static_cast<int>(i));
    smtk::attribute::ItemDefinitionPtr newItemDef = sourceItemDef->createCopy(info);
    if (newItemDef)
    {
      newDef->addItemDefinition(newItemDef);
    }
  }

  newDef->m_localCategories = sourceDef->m_localCategories;
  newDef->m_categories = sourceDef->m_categories;
  // TODO Update CopyInfo to include set of categories in new attribute(s)
  // For now, use brute force to update
  this->finalizeDefinitions();

  return true;
}

// Copies attribute into this Resource
// Returns smart pointer (will be empty if operation unsuccessful)
// If definition contains ExpressionType instances, might also
// copy additional attributes from the source attribute Resource.
smtk::attribute::AttributePtr Resource::copyAttribute(const smtk::attribute::AttributePtr sourceAtt,
  const bool& copyModelAssocs, const unsigned int& itemCopyOptions)
{
  smtk::attribute::AttributePtr newAtt;
  smtk::attribute::DefinitionPtr newDef;
  // Are we copying the attribute to the same attribute Resource?
  bool sameResource = (shared_from_this() == sourceAtt->attributeResource());
  std::string newName;
  if (sameResource)
  {
    newName = this->createUniqueName(sourceAtt->definition()->type());
    newDef = sourceAtt->definition();
  }
  else
  {
    // Copying into a new Resource
    // First do we need tp copy its definition?
    newDef = this->findDefinition(sourceAtt->definition()->type());
    if (!newDef)
    {
      newDef = this->copyDefinition(sourceAtt->definition());
      if (!newDef)
      {
        std::cerr << "ERROR: Could not copy Definition: " << sourceAtt->definition()->type()
                  << " needed to create Attribute: " << sourceAtt->name() << "\n";
        return newAtt;
      }
    }
    else
    {
      //TODO Should make sure the definition we found matches the structure of the definition
      // used by the Source Attribute if they don't then error out
    }

    std::string name = sourceAtt->name();
    if (this->findAttribute(name))
    {
      newName = this->createUniqueName(sourceAtt->definition()->type());
    }
    else
    {
      newName = name;
    }
  }
  newAtt = this->createAttribute(newName, newDef);
  // Copy properties
  if (sourceAtt->isColorSet())
  {
    newAtt->setColor(sourceAtt->color());
  }
  newAtt->setAppliesToBoundaryNodes(sourceAtt->appliesToBoundaryNodes());
  newAtt->setAppliesToInteriorNodes(sourceAtt->appliesToInteriorNodes());
  // Copy/update items
  for (std::size_t i = 0; i < sourceAtt->numberOfItems(); ++i)
  {
    smtk::attribute::ConstItemPtr sourceItem =
      smtk::const_pointer_cast<const Item>(sourceAtt->item(static_cast<int>(i)));
    smtk::attribute::ItemPtr newItem = newAtt->item(static_cast<int>(i));
    if (!newItem->assign(sourceItem, itemCopyOptions))
    {
      std::cerr << "ERROR:Could not copy Attribute: " << sourceAtt->name() << "\n";
      this->removeAttribute(newAtt);
      newAtt.reset();
      return newAtt;
    }
  }
  // Copy model associations if requested and the attribute is not Unique
  // with respects to the model entitiy
  if (copyModelAssocs && !(sameResource && sourceAtt->definition()->isUnique()))
  {
    smtk::common::UUIDs uuidSet = sourceAtt->associatedModelEntityIds();
    smtk::common::UUIDs::const_iterator it;
    for (it = uuidSet.begin(); it != uuidSet.end(); it++)
    {
      newAtt->associateEntity(*it);
    }
  }

  // TODO what about m_userData?
  return newAtt;
}

void Resource::addView(smtk::view::ConfigurationPtr v)
{
  m_views[v->name()] = v;
  this->setClean(false);
}

smtk::view::ConfigurationPtr Resource::findViewByType(const std::string& vtype) const
{
  std::map<std::string, smtk::view::ConfigurationPtr>::const_iterator it;
  for (it = m_views.begin(); it != m_views.end(); ++it)
  {
    if (it->second->type() == vtype)
    {
      return it->second;
    }
  }
  return smtk::view::ConfigurationPtr();
}

smtk::view::ConfigurationPtr Resource::findTopLevelView() const
{
  std::map<std::string, smtk::view::ConfigurationPtr>::const_iterator it;
  bool isTopLevel;
  for (it = m_views.begin(); it != m_views.end(); ++it)
  {
    if (it->second->details().attributeAsBool("TopLevel", isTopLevel) && isTopLevel)
    {
      return it->second;
    }
  }
  return smtk::view::ConfigurationPtr();
}

std::vector<smtk::view::ConfigurationPtr> Resource::findTopLevelViews() const
{
  std::map<std::string, smtk::view::ConfigurationPtr>::const_iterator it;
  bool isTopLevel;
  std::vector<smtk::view::ConfigurationPtr> topViews;
  for (it = m_views.begin(); it != m_views.end(); ++it)
  {
    if (it->second->details().attributeAsBool("TopLevel", isTopLevel) && isTopLevel)
    {
      topViews.push_back(it->second);
    }
  }
  return topViews;
}

smtk::resource::ComponentPtr Resource::find(const smtk::common::UUID& attId) const
{
  return this->findAttribute(attId);
}

std::string Resource::createAttributeQuery(const smtk::attribute::DefinitionPtr& def)
{
  std::string s("attribute[type='");
  s.append(def->type()).append("']");
  return s;
}

std::string Resource::createAttributeQuery(const std::string& defType)
{
  std::string s("attribute[type='");
  s.append(defType).append("']");
  return s;
}

std::function<bool(const smtk::resource::ConstComponentPtr&)> Resource::queryOperation(
  const std::string& filter) const
{
  if (filter.empty() || filter == "any" || filter == "*")
  {
    return [](const smtk::resource::ConstComponentPtr&) { return true; };
  }
  const std::string attributeFilter("attribute");
  if (!filter.compare(0, attributeFilter.size(), attributeFilter))
  {
    std::string spec = filter.substr(attributeFilter.size());
    const std::string definitionFilter("[type='");
    auto sszm2 = spec.size() - 2;
    auto dfsz = definitionFilter.size();
    if (!spec.compare(0, dfsz, definitionFilter) && spec.substr(sszm2) == "']")
    {
      const std::string sdef = spec.substr(dfsz, sszm2 - dfsz);
      smtk::attribute::DefinitionPtr defn = this->findDefinition(sdef);
      if (defn)
      {
        return [defn](const smtk::resource::ConstComponentPtr& comp) {
          auto attr = std::dynamic_pointer_cast<const Attribute>(comp);
          return (attr && attr->isA(defn));
        };
      }
    }
  }
  return [](const smtk::resource::ConstComponentPtr&) { return false; };
}

// visit all components in the resource.
void Resource::visit(smtk::resource::Component::Visitor& visitor) const
{
  auto convertedVisitor = [&](const std::pair<std::string, const AttributePtr>& attributePair) {
    const smtk::resource::ComponentPtr resource =
      std::static_pointer_cast<smtk::resource::Component>(attributePair.second);
    visitor(resource);
  };
  std::for_each(m_attributes.begin(), m_attributes.end(), convertedVisitor);
}

std::set<AttributePtr> Resource::attributes(
  const smtk::resource::ConstPersistentObjectPtr& object) const
{
  std::set<AttributePtr> result;
  // Get the attributes associated with this object - Note that the
  // linkedFrom method takes in a const shared pointer to a non-const resource even though the method
  // does not change the resource's state. The reason for this is due to issues of auto casting shared pointers to
  // non-const objects to shared pointers to const objects.
  auto objs = object->links().linkedFrom(
    const_cast<Resource*>(this)->shared_from_this(), Resource::AssociationRole);
  for (auto obj : objs)
  {
    auto entry = std::dynamic_pointer_cast<Attribute>(obj);
    if (entry)
    { // Never insert a failed dynamic cast (null pointer)
      result.insert(entry);
    }
  }
  return result;
}

bool Resource::hasAttributes(const smtk::resource::ConstPersistentObjectPtr& object) const
{
  // See if the object has any attributes - Note that the
  // linkedFrom method takes in a const shared pointer to a non-const resource even though the method
  // does not change the resource's state. The reason for this is due to issues of auto casting shared pointers to
  // non-const objects to shared pointers to const objects.
  auto objs = object->links().linkedFrom(
    const_cast<Resource*>(this)->shared_from_this(), Resource::AssociationRole);
  for (auto obj : objs)
  {
    auto entry = std::dynamic_pointer_cast<Attribute>(obj);
    if (entry)
    { //If we find even one attribute report yes
      return true;
    }
  }
  return false;
}

void Resource::disassociateAllAttributes(const smtk::resource::PersistentObjectPtr& object)
{
  // Get the attributes associated with this object - Note that the
  // linkedFrom method takes in a const shared pointer to a non-const resource even though the method
  // does not change the resource's state. The reason for this is due to issues of auto casting shared pointers to
  // non-const objects to shared pointers to const objects.
  auto objs = object->links().linkedFrom(
    const_cast<Resource*>(this)->shared_from_this(), Resource::AssociationRole);
  for (auto obj : objs)
  {
    auto entry = std::dynamic_pointer_cast<Attribute>(obj);
    if (entry)
    { // Never insert a failed dynamic cast (null pointer)
      entry->forceDisassociate(object);
    }
  }
}

bool Resource::hasAssociations() const
{
  // Get the data from the resource's links and see if it contains a link with the
  // association role
  return !this->links().linkedTo(smtk::attribute::Resource::AssociationRole).empty();
}

bool Resource::isRoleUnique(const smtk::resource::Links::RoleType& role) const
{
  return (m_roles.find(role) != m_roles.end());
}

void Resource::addUniqueRoles(const std::set<smtk::resource::Links::RoleType>& roles)
{
  m_roles.insert(roles.begin(), roles.end());
}

void Resource::addUniqueRole(const smtk::resource::Links::RoleType& role)
{
  m_roles.insert(role);
}

const std::set<smtk::resource::Links::RoleType>& Resource::uniqueRoles() const
{
  return m_roles;
}
