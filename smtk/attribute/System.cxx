//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/System.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/RefItemDefinition.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"
#include "smtk/model/Manager.h"
#include "smtk/common/UUID.h"
#include "smtk/common/UUIDGenerator.h"
#include "smtk/common/View.h"
#include <iostream>
#include <sstream>
#include <queue>

using namespace smtk::attribute;

//----------------------------------------------------------------------------
System::System()
{
}

//----------------------------------------------------------------------------
System::~System()
{
  std::map<std::string,  smtk::attribute::DefinitionPtr>::const_iterator it;
  for (it = this->m_definitions.begin(); it != this->m_definitions.end(); it++)
    {
    // Decouple all defintions from this system
    (*it).second->clearSystem();
    }
 }

//----------------------------------------------------------------------------
smtk::common::Resource::Type
System::resourceType() const
{
  return smtk::common::Resource::ATTRIBUTE;
}

//----------------------------------------------------------------------------
smtk::attribute::DefinitionPtr
System::createDefinition(const std::string &typeName,
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
System::createDefinition(const std::string &typeName,
                          smtk::attribute::DefinitionPtr baseDef)
{
  smtk::attribute::DefinitionPtr  def = this->findDefinition(typeName);
  // Does this definition already exist or if the base def is not part
  // of this manger
  if (!(!def && (!baseDef || ((baseDef->system() == this)))))
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
smtk::attribute::AttributePtr System::createAttribute(const std::string &name,
                                             smtk::attribute::DefinitionPtr def)
{
  // Make sure the definition belongs to this system or if the definition
  // is abstract
  if ((def->system() != this) || def->isAbstract())
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
  this->m_attributeClusters[def->type()].insert(a);
  this->m_attributes[name] = a;
  this->m_attributeIdMap[a->id()] = a;
  return a;
}

//----------------------------------------------------------------------------
smtk::attribute::AttributePtr
System::createAttribute(smtk::attribute::DefinitionPtr def)
{
  smtk::attribute::AttributePtr att =
    this->createAttribute(this->createUniqueName(def->rootName()), def);
  return att;
}

//----------------------------------------------------------------------------
smtk::attribute::AttributePtr System::createAttribute(const std::string &typeName)
{
  smtk::attribute::DefinitionPtr def = this->findDefinition(typeName);
  if (!def)
    {
    return smtk::attribute::AttributePtr();
    }

  smtk::attribute::AttributePtr att =
    this->createAttribute(this->createUniqueName(def->rootName()), def);
  return att;
}

//----------------------------------------------------------------------------
smtk::attribute::AttributePtr System::createAttribute(const std::string &name,
                                             const std::string &typeName)
{
  smtk::attribute::DefinitionPtr def = this->findDefinition(typeName);
  if(!def)
    {
    return smtk::attribute::AttributePtr();
    }
  smtk::attribute::AttributePtr att = this->createAttribute(name, def);
  return att;
}

//----------------------------------------------------------------------------
void System::definitions(std::vector<smtk::attribute::DefinitionPtr> &result) const
{
  std::map<std::string, DefinitionPtr>::const_iterator it;
  result.resize(this->m_definitions.size());
  int i;
  for (it = this->m_definitions.begin(), i = 0; it != this->m_definitions.end(); it++, i++)
    {
    result[i] = it->second;
    }
}
//----------------------------------------------------------------------------
void System::attributes(std::vector<smtk::attribute::AttributePtr> &result) const
{
  std::map<std::string, AttributePtr>::const_iterator it;
  result.resize(this->m_attributes.size());
  int i;
  for (it = this->m_attributes.begin(), i = 0; it != this->m_attributes.end(); it++, i++)
    {
    result[i] = it->second;
    }
}
//----------------------------------------------------------------------------
// For Reader classes
//----------------------------------------------------------------------------
smtk::attribute::AttributePtr
System::createAttribute(const std::string &name,
                        smtk::attribute::DefinitionPtr def,
                        const smtk::common::UUID &id)
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
smtk::attribute::AttributePtr
System::createAttribute(const std::string &name,
                        const std::string &typeName,
                        const smtk::common::UUID &id)
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
bool System::removeAttribute(smtk::attribute::AttributePtr att)
{
  // Make sure that this system is managing this attribute
  if (att->system() != this)
    {
    return false;
    }
  this->m_attributes.erase(att->name());
  this->m_attributeIdMap.erase(att->id());
  this->m_attributeClusters[att->type()].erase(att);
  return true;
}

//----------------------------------------------------------------------------
/**\brief Find the attribute definitions that can be associated with \a mask.
  *
  */
void System::findDefinitions(unsigned long mask, std::vector<smtk::attribute::DefinitionPtr> &result) const
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
void System::
findAttributes(smtk::attribute::DefinitionPtr def,
               std::vector<smtk::attribute::AttributePtr> &result) const
{
  result.clear();
  if (def && (def->system() == this))
    {
    this->internalFindAttributes(def, result);
    }
}
//----------------------------------------------------------------------------
void System::
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
void System::
findAllDerivedDefinitions(smtk::attribute::DefinitionPtr def,
                          bool onlyConcrete,
                          std::vector<smtk::attribute::DefinitionPtr> &result) const
{
  result.clear();
  if (def && (def->system() == this))
    {
    this->internalFindAllDerivedDefinitions(def, onlyConcrete, result);
    }
}
//----------------------------------------------------------------------------
void System::
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
bool System::rename(smtk::attribute::AttributePtr att, const std::string &newName)
{
  // Make sure that this system is managing this attribute
  if (att->system() != this)
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
std::string System::createUniqueName(const std::string &type) const
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
void System::
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
void System::updateCategories()
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
  // of their categories to form the systems
  this->m_categories.clear();
  for (it = this->m_definitions.begin(); it != this->m_definitions.end(); it++)
    {
    this->m_categories.insert(it->second->categories().begin(),
                              it->second->categories().end());
    }
}
//----------------------------------------------------------------------------
void System::
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
smtk::attribute::ConstDefinitionPtr System::findIsUniqueBaseClass(
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
void System::setRefModelManager(smtk::model::ManagerPtr refModelMgr)
{
  this->m_refModelMgr = refModelMgr;
}
//----------------------------------------------------------------------------
void
System::updateDerivedDefinitionIndexOffsets(smtk::attribute::DefinitionPtr def)
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
void System::addAdvanceLevel(
  int level, std::string label, const double *l_color)
{
  this->m_advLevels[level] = label;
  this->setAdvanceLevelColor(level, l_color);
}
//----------------------------------------------------------------------------
const double* System::advanceLevelColor(int level) const
{
  std::map<int, std::vector<double> >::const_iterator it =
    this->m_advLevelColors.find(level);
  if( it != this->m_advLevelColors.end() &&
     it->second.size() == 4)
    {
    return &it->second[0];
    }
  return NULL;
}
//----------------------------------------------------------------------------
void System::setAdvanceLevelColor(int level, const double *l_color)
{
  if(l_color && this->m_advLevels.find(level) != this->m_advLevels.end())
    {
    std::vector<double> acolor (l_color, l_color + 4);
    this->m_advLevelColors[level] = acolor;
    }
}
//----------------------------------------------------------------------------
// Copies attribute defintion into this system
// Returns smart pointer (will be empty if operation unsuccessful)
// If definition contains RefItemDefinition instances, might have to
// copy additional definitions for their targets.
smtk::attribute::DefinitionPtr
System::copyDefinition(const smtk::attribute::DefinitionPtr sourceDef,
                        unsigned int /*options*/)
{
  // Returns defintion
  smtk::attribute::DefinitionPtr newDef = smtk::attribute::DefinitionPtr();

  // Call internal copy method
  smtk::attribute::ItemDefinition::CopyInfo info(this);
  if (this->copyDefinitionImpl(sourceDef, info))
    {
    newDef = this->findDefinition(sourceDef->type());

    // Process any unresolved ref & exp items
    while (!info.UnresolvedRefItems.empty() || !info.UnresolvedExpItems.empty())
      {
      // Process ref items first
      while (!info.UnresolvedRefItems.empty())
        {
        // Check if type has been created (copied) already
        std::pair<std::string, smtk::attribute::ItemDefinitionPtr>& frontDef =
          info.UnresolvedRefItems.front();
        std::string type = frontDef.first;
        smtk::attribute::DefinitionPtr def = this->findDefinition(type);
        if (def)
          {
          smtk::attribute::ItemDefinitionPtr nextItemDef = frontDef.second;
          smtk::attribute::RefItemDefinitionPtr refItemDef =
            smtk::dynamic_pointer_cast<smtk::attribute::RefItemDefinition>(nextItemDef);
          refItemDef->setAttributeDefinition(def);
          info.UnresolvedRefItems.pop();
          }
        else
          {
          // Need to copy definition, first find it in the input system
          std::cout << "Copying \"" << type << "\" definition" << std::endl;
          smtk::attribute::DefinitionPtr nextDef =
            sourceDef->system()->findDefinition(type);
          // Definition missing only if source system is invalid, but check anyway
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
        } // while (unresolved references)

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
          // Need to copy definition, first find it in the input system
          std::cout << "Copying \"" << type << "\" definition" << std::endl;
          smtk::attribute::DefinitionPtr nextDef =
            sourceDef->system()->findDefinition(type);
          // Definition missing only if source system is invalid, but check anyway
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

      }  // while (references or expressions)
    }  // if (this->copyDefinition())

  return newDef;
}
//----------------------------------------------------------------------------
// Copies attribute definition into this system, returning true if successful
bool System::copyDefinitionImpl(smtk::attribute::DefinitionPtr sourceDef,
                                 smtk::attribute::ItemDefinition::CopyInfo& info)
{
  // Check for type conflict
  std::string typeName = sourceDef->type();
  if (this->findDefinition(typeName))
    {
    std::cerr << "WARNING: Will not overwrite attribute definition "
              << typeName << std::endl;
    return false;
    }

  // Check if input definition has a base definition
  smtk::attribute::DefinitionPtr newDef;
  smtk::attribute::DefinitionPtr sourceBaseDef = sourceDef->baseDefinition();
  if (sourceBaseDef)
    {
    // Check if base definition of this type already exists in this system
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
    smtk::attribute::DefinitionPtr newBaseDef =
      this->findDefinition(baseTypeName);
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
  newDef->setAdvanceLevel(sourceDef->advanceLevel());
  newDef->setIsUnique(sourceDef->isUnique());
  newDef->setIsNodal(sourceDef->isNodal());
  if (sourceDef->isNotApplicableColorSet())
    {
    newDef->setNotApplicableColor(sourceDef->notApplicableColor());
    }
  if (sourceDef->isDefaultColorSet())
    {
    newDef->setDefaultColor(sourceDef->defaultColor());
    }
  if (sourceDef->associationRule())
    {
    newDef->setAssociationRule(sourceDef->associationRule());
    }
  newDef->setAssociationMask(sourceDef->associationMask());

  // Copy new item definitions only (i.e., not inherited item defs)
  for (std::size_t i = sourceDef->itemOffset();
       i < sourceDef->numberOfItemDefinitions();
       ++i)
    {
    smtk::attribute::ItemDefinitionPtr sourceItemDef =
      sourceDef->itemDefinition(static_cast<int>(i));
    smtk::attribute::ItemDefinitionPtr newItemDef =
      sourceItemDef->createCopy(info);
    if (newItemDef)
      {
      newDef->addItemDefinition(newItemDef);
      }
    }

  // Update categories
  newDef->setCategories();

  // TODO Update CopyInfo to include set of categories in new attribute(s)
  // For now, use brute force to update
  this->updateCategories();

  return true;
}
//----------------------------------------------------------------------------
// Copies attribute into this system
// Returns smart pointer (will be empty if operation unsuccessful)
// If definition contains RefItem or ExpressionType instances, might also
// copy additional attributes from the source attribute system.
smtk::attribute::AttributePtr
System::copyAttribute(const smtk::attribute::AttributePtr sourceAtt,
                      const bool &copyModelAssocs, const unsigned int &itemCopyOptions)
{
  smtk::attribute::AttributePtr newAtt;
  smtk::attribute::DefinitionPtr newDef;
  // Are we copying the attribute to the same attribute system?
  bool sameSystem = (this == sourceAtt->system());
  std::string newName;
  if (sameSystem)
    {
    newName = this->createUniqueName(sourceAtt->definition()->type());
    newDef = sourceAtt->definition();
    }
  else
    {
    // Copying into a new system
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

    std::string name =  sourceAtt->name();
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
  for (std::size_t i=0; i<sourceAtt->numberOfItems(); ++i)
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
  if (copyModelAssocs && !(sameSystem && sourceAtt->definition()->isUnique()))
    {
    smtk::common::UUIDs uuidSet = sourceAtt->associatedModelEntityIds();
    smtk::common::UUIDs::const_iterator it;
    for (it=uuidSet.begin(); it != uuidSet.end(); it++)
      {
      newAtt->associateEntity(*it);
      }
    }

  // TODO what about m_userData?
  return newAtt;
}
//----------------------------------------------------------------------------
void System::addView(smtk::common::ViewPtr v)
{
  this->m_views[v->title()] = v;
}
//----------------------------------------------------------------------------
smtk::common::ViewPtr System::findViewByType(const std::string &vtype) const
{
  std::map<std::string, smtk::common::ViewPtr>::const_iterator it;
  for (it = this->m_views.begin(); it != this->m_views.end(); ++it)
    {
    if (it->second->type() == vtype)
      {
      return it->second;
      }
    }
  return smtk::common::ViewPtr();
}
//----------------------------------------------------------------------------
smtk::common::ViewPtr System::findTopLevelView() const
{
  std::map<std::string, smtk::common::ViewPtr>::const_iterator it;
  bool isTopLevel;
  for (it = this->m_views.begin(); it != this->m_views.end(); ++it)
    {
    if (it->second->details().attributeAsBool("TopLevel", isTopLevel) && isTopLevel)
      {
      return it->second;
      }
    }
  return smtk::common::ViewPtr();
}
//----------------------------------------------------------------------------
std::vector<smtk::common::ViewPtr> System::findTopLevelViews() const
{
  std::map<std::string, smtk::common::ViewPtr>::const_iterator it;
  bool isTopLevel;
  std::vector<smtk::common::ViewPtr> topViews;
  for (it = this->m_views.begin(); it != this->m_views.end(); ++it)
    {
    if (it->second->details().attributeAsBool("TopLevel", isTopLevel) && isTopLevel)
      {
      topViews.push_back(it->second);
      }
    }
  return topViews;
}
//----------------------------------------------------------------------------
