//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DateTimeItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"

#include "smtk/resource/Manager.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/UUIDGenerator.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/algorithm/string.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <cassert>
#include <functional>
#include <iostream>

using namespace smtk::attribute;
using namespace smtk::common;

Attribute::Attribute(
  const std::string& myName,
  const smtk::attribute::DefinitionPtr& myDefinition,
  const smtk::common::UUID& myId)
  : m_name(myName)
  , m_definition(myDefinition)
  , m_appliesToBoundaryNodes(false)
  , m_appliesToInteriorNodes(false)
  , m_isColorSet(false)
  , m_aboutToBeDeleted(false)
  , m_id(myId)
{
  m_hasLocalAdvanceLevelInfo[0] = false;
  m_hasLocalAdvanceLevelInfo[1] = false;
  m_localAdvanceLevel[0] = m_localAdvanceLevel[1] = 0;
}

Attribute::Attribute(const std::string& myName, const smtk::attribute::DefinitionPtr& myDefinition)
  : m_name(myName)
  , m_definition(myDefinition)
  , m_appliesToBoundaryNodes(false)
  , m_appliesToInteriorNodes(false)
  , m_isColorSet(false)
  , m_aboutToBeDeleted(false)
  , m_id(smtk::common::UUIDGenerator::instance().random())
  , m_includeIndex(0)
{
  m_hasLocalAdvanceLevelInfo[0] = false;
  m_hasLocalAdvanceLevelInfo[1] = false;
  m_localAdvanceLevel[0] = m_localAdvanceLevel[1] = 0;
}

Attribute::~Attribute()
{
  m_aboutToBeDeleted = true;
  // Detatch the association item
  if (m_associatedObjects)
  {
    m_associatedObjects->detachOwningAttribute();
  }
  this->removeAllItems();
}

// Though technically the attribute could be built within its constructor, this would
// force a constraint that no underlying code could try to access the attribute's
// shared pointer - for example an Item could not call its attribute() method without
// dire consequences (since the attribute is not shared yet by its resource the call
// would result in the attribute being deleted)
void Attribute::build()
{
  if (m_definition != nullptr)
  {
    m_definition->buildAttribute(this);
  }
}
void Attribute::removeAllItems()
{
  // we need to detatch all items owned by this attribute
  std::size_t i, n = m_items.size();
  for (i = 0; i < n; i++)
  {
    m_items[i]->detachOwningAttribute();
  }
  m_items.clear();
}

const double* Attribute::color() const
{
  if (m_isColorSet)
  {
    return m_color;
  }
  return m_definition->defaultColor();
}

const std::string& Attribute::type() const
{
  return m_definition->type();
}

std::vector<std::string> Attribute::types() const
{
  std::vector<std::string> tvec;
  // To avoid the overhead of shared pointers
  // we can safely grab the raw pointer of the definition(s)
  // The reason being the attribute shares ownership of its
  // definition which in turn shares ownership with its base
  auto def = m_definition.get();
  while (def != nullptr)
  {
    tvec.push_back(def->type());
    def = def->baseDefinition().get();
  }
  return tvec;
}

bool Attribute::isA(const smtk::attribute::DefinitionPtr& def) const
{
  return m_definition->isA(def);
}

void Attribute::setLocalAdvanceLevel(int mode, unsigned int level)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  m_hasLocalAdvanceLevelInfo[mode] = true;
  m_localAdvanceLevel[mode] = level;
}

void Attribute::unsetLocalAdvanceLevel(int mode)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  m_hasLocalAdvanceLevelInfo[mode] = false;
}

unsigned int Attribute::advanceLevel(int mode) const
{
  // Any invalid mode returns mode = 0
  if ((mode < 0) || (mode > 1))
  {
    mode = 0;
  }
  if (m_hasLocalAdvanceLevelInfo[mode])
  {
    return m_localAdvanceLevel[mode];
  }

  if (m_definition)
  {
    return m_definition->advanceLevel(mode);
  }

  return 0;
}

/**\brief Return an item given a string specifying a path to it.
  *
  */
smtk::attribute::ConstItemPtr
Attribute::itemAtPath(const std::string& path, const std::string& seps, bool activeOnly) const
{
  std::vector<std::string> tree;
  std::vector<std::string>::iterator it;
  boost::split(tree, path, boost::is_any_of(seps));
  if (tree.empty())
  {
    return nullptr;
  }

  it = tree.begin();
  smtk::attribute::ConstItemPtr current = this->find(*it, IMMEDIATE);
  if (current)
  {
    SearchStyle style = activeOnly ? IMMEDIATE_ACTIVE : IMMEDIATE;
    for (++it; it != tree.end(); ++it)
    {
      current = current->find(*it, style);
      if (current == nullptr)
      {
        break; // we couldn't find the next item in the path
      }
    }
  }
  return current;
}

smtk::attribute::ItemPtr
Attribute::itemAtPath(const std::string& path, const std::string& seps, bool activeOnly)
{
  std::vector<std::string> tree;
  std::vector<std::string>::iterator it;
  boost::split(tree, path, boost::is_any_of(seps));
  if (tree.empty())
  {
    return nullptr;
  }

  it = tree.begin();
  smtk::attribute::ItemPtr current = this->find(*it, IMMEDIATE);
  if (current)
  {
    SearchStyle style = activeOnly ? IMMEDIATE_ACTIVE : IMMEDIATE;
    for (++it; it != tree.end(); ++it)
    {
      current = current->find(*it, style);
      if (current == nullptr)
      {
        break; // we couldn't find the next item in the path
      }
    }
  }
  return current;
}

const smtk::attribute::Categories& Attribute::categories() const
{
  return m_definition->categories();
}

/**\brief Validate the attribute against its definition.
  *
  * This method will only return true when every (required) item in the
  * attribute is set and considered a valid value by its definition.
  * This can be used to ensure that an attribute is in a good state
  * before using it to perform some operation.
  *
  * If useActiveCategories is true and if the resource has active
  * categories enabled, then the resource's active categories will be
  * taken into consideration
  */
bool Attribute::isValid(bool useActiveCategories) const
{
  if (useActiveCategories)
  {
    auto aResource = this->attributeResource();
    if (aResource && aResource->activeCategoriesEnabled())
    {
      return this->isValid(aResource->activeCategories());
    }
  }

  for (auto it = m_items.begin(); it != m_items.end(); ++it)
  {
    if (!(*it)->isValid(false))
    {
      return false;
    }
  }

  // The the Attribute can be evaluated, it should evaluate successfully.
  if (canEvaluate() && !doesEvalaute())
  {
    return false;
  }

  // also check associations
  return !(m_associatedObjects && !m_associatedObjects->isValid(false));
}

bool Attribute::isValid(const std::set<std::string>& cats) const
{
  // First lest check the attribute itself to see if it would
  // have been filtered out
  if (!this->categories().passes(cats))
  {
    return true;
  }

  for (auto it = m_items.begin(); it != m_items.end(); ++it)
  {
    if (!(*it)->isValid(cats))
    {
      return false;
    }
  }

  // The the Attribute can be evaluated, it should evaluate successfully.
  if (canEvaluate() && !doesEvalaute())
  {
    return false;
  }

  // also check associations
  return !(m_associatedObjects && !m_associatedObjects->isValid());
}

bool Attribute::isRelevant() const
{
  auto aResource = this->attributeResource();
  if (aResource && aResource->activeCategoriesEnabled())
  {
    return this->categories().passes(aResource->activeCategories());
  }
  return true;
}

ResourcePtr Attribute::attributeResource() const
{
  return m_definition->resource();
}

const smtk::resource::ResourcePtr Attribute::resource() const
{
  return this->attributeResource();
}

bool Attribute::canEvaluate() const
{
  smtk::attribute::ResourcePtr attRes = attributeResource();
  if (attRes)
  {
    smtk::attribute::ConstAttributePtr myself = shared_from_this();
    if (myself)
    {
      return attRes->canEvaluate(myself);
    }
  }

  return false;
}

bool Attribute::doesEvalaute() const
{
  smtk::attribute::ResourcePtr attRes = attributeResource();
  if (attRes)
  {
    smtk::attribute::ConstAttributePtr myself = shared_from_this();
    if (myself)
    {
      std::unique_ptr<Evaluator> evaluator = attRes->createEvaluator(myself);
      return evaluator ? evaluator->doesEvaluate() : false;
    }
  }
  return false;
}

std::unique_ptr<Evaluator> Attribute::createEvaluator() const
{
  smtk::attribute::ResourcePtr attRes = attributeResource();
  if (attRes)
  {
    smtk::attribute::ConstAttributePtr myself = shared_from_this();
    if (myself)
    {
      return attRes->createEvaluator(myself);
    }
  }

  return std::unique_ptr<smtk::attribute::Evaluator>();
}

void Attribute::detachItemsFromOwningResource()
{
  for (auto it = m_items.begin(); it != m_items.end(); ++it)
  {
    (*it)->detachOwningResource();
  }

  if (m_associatedObjects != nullptr)
  {
    m_associatedObjects->detachOwningResource();
  }
}

bool Attribute::removeAllAssociations(bool partialRemovalOk)
{
  if (m_associatedObjects == nullptr)
  {
    return true;
  }

  // Do we need to worry about prerequisites?
  if (m_definition->isUsedAsAPrerequisite())
  {
    std::vector<smtk::resource::PersistentObjectPtr> objs;
    objs = this->associatedObjects<std::vector<smtk::resource::PersistentObjectPtr>>();
    bool result = true;
    // if we are allowed to remove what we can then disassociate where we can
    if (partialRemovalOk)
    {
      for (const auto& obj : objs)
      {
        if (!this->disassociate(obj))
        {
          result = false;
        }
      }
      if (result)
      {
        // OK - we have been able to remove all associations, so we now need to
        // sever the association item's connection to its parent resource
        m_associatedObjects->detachOwningResource();
      }
      return result;
    }
    // We need to precheck all of the objects
    AttributePtr probAtt;
    for (const auto& obj : objs)
    {
      if (!this->canBeDisassociated(obj, probAtt))
      {
        result = false;
      }
    }

    if (!result)
    {
      // can not remove all of the associations
      return false;
    }
  }
  m_associatedObjects->detachOwningResource();
  return true;
}

/**\brief Update attribute when entities has been expunged
  * Note it would check associations and every modelEntityItem
  */
bool Attribute::removeExpungedEntities(const smtk::model::EntityRefs& expungedEnts)
{
  bool associationChanged = false;
  // update all modelEntityItems
  std::set<smtk::attribute::ModelEntityItemPtr> modelEntityPtrs;
  std::function<bool(smtk::attribute::ModelEntityItemPtr)> filter =
    [](smtk::attribute::ModelEntityItemPtr /*unused*/) { return true; };
  this->filterItems(modelEntityPtrs, filter, false);
  for (std::set<smtk::attribute::ModelEntityItemPtr>::iterator iterator = modelEntityPtrs.begin();
       iterator != modelEntityPtrs.end();
       iterator++)
  {
    smtk::attribute::ModelEntityItemPtr MEItem = *iterator;
    if (MEItem && MEItem->isValid())
    {
      for (smtk::model::EntityRefs::const_iterator bit = expungedEnts.begin();
           bit != expungedEnts.end();
           ++bit)
      {
        std::ptrdiff_t idx = MEItem->find(*bit);
        if (idx >= 0)
        {
          MEItem->removeValue(static_cast<std::size_t>(idx));
          associationChanged = true;
        }
      }
    }
  }
  if (this->associations())
  {
    for (smtk::model::EntityRefs::const_iterator bit = expungedEnts.begin();
         bit != expungedEnts.end();
         ++bit)
    {
      if (this->isEntityAssociated(*bit))
      {
        this->disassociateEntity(*bit);
        associationChanged = true;
      }
    }
  }
  return associationChanged;
}

bool Attribute::isObjectAssociated(const smtk::common::UUID& entity) const
{
  return m_associatedObjects ? m_associatedObjects->contains(entity) : false;
}

bool Attribute::isObjectAssociated(const smtk::resource::PersistentObjectPtr& comp) const
{
  return m_associatedObjects ? m_associatedObjects->contains(comp) : false;
}

/**\brief Is the model \a entity associated with this attribute?
  *
  */
bool Attribute::isEntityAssociated(const smtk::common::UUID& entity) const
{
  return this->isObjectAssociated(entity);
}

/**\brief Is the model entity of the \a entityref associated with this attribute?
  *
  */
bool Attribute::isEntityAssociated(const smtk::model::EntityRef& entityref) const
{
  auto comp = entityref.component();
  return (comp && m_associatedObjects) ? m_associatedObjects->contains(comp) : false;
}

/**\brief Return the associated model entities as a set of UUIDs.
  *
  */
smtk::common::UUIDs Attribute::associatedModelEntityIds() const
{
  smtk::common::UUIDs result;
  auto assoc = this->associations();
  if (assoc)
  {
    for (std::size_t i = 0; i < assoc->numberOfValues(); ++i)
    {
      if (assoc->isSet(i))
      {
        auto key = assoc->objectKey(i);
        result.insert(this->resource()->links().data().at(key.first).at(key.second).right);
      }
    }
  }
  return result;
}

bool Attribute::canBeAssociated(const smtk::resource::PersistentObjectPtr& obj) const
{
  if (!obj)
  {
    return false;
  }

  auto associationRule =
    this->attributeResource()->associationRules().associationRuleForDefinition(m_definition);
  if (associationRule)
  {
    std::shared_ptr<const Attribute> self = this->shared_from_this();
    return (*associationRule)(self, obj);
  }
  return true;
}

/*! \fn template<typename T> T Attribute::associatedModelEntities() const
 *\brief Return a container of associated entityref-subclass instances.
 *
 * This method returns a container (usually a std::vector or std::set) of
 * entityref-subclass instances (e.g., Edge, EdgeUse, Loop) that are
 * associated with this attribute.
 *
 * Note that if you request a container of EntityRef entities, you will obtain
 * <b>all</b> of the associated model entities. However, if you request
 * a container of some subclass, only entities of that type will be returned.
 * For example, if an attribute is associated with two faces, an edge,
 * a group, and a shell, calling `associatedModelEntities<EntityRefs>()` will
 * return 5 EntityRef entries while `associatedModelEntities<CellEntities>()`
 * will return 3 entries (2 faces and 1 edge) since the other entities
 * do not construct valid CellEntity instances.
 */

bool Attribute::associate(smtk::resource::PersistentObjectPtr obj)
{
  if (this->isObjectAssociated(obj))
  {
    return true;
  }

  // Lets see if we have any conflicts
  if (m_definition->checkForConflicts(obj) != nullptr)
  {
    return false;
  }
  // What about missing prerequisites?
  if (m_definition->checkForPrerequisites(obj) != nullptr)
  {
    return false;
  }

  // Do the user-defined association rules allow for this association?
  if (!this->canBeAssociated(obj))
  {
    return false;
  }

  // Did it pass the association rules?
  if ((m_associatedObjects != nullptr) && m_associatedObjects->appendValue(obj))
  {
    return true;
  }
  // Failed the association rules
  return false;
}

/**\brief Associate a new-style model ID (a UUID) with this attribute.
  *
  * This function returns true when the association is valid and
  * successful. It will return false if the association is prohibited.
  */
bool Attribute::associateEntity(const smtk::common::UUID& objId)
{
  std::set<smtk::resource::Resource::Ptr> rsrcs = this->attributeResource()->associations();
  rsrcs.insert(this->attributeResource()); // We can always look for other attributes.

  // If we have a resource manager, we can also look for components in other resources:
  auto rsrcMgr = this->attributeResource()->manager();
  if (rsrcMgr)
  {
    rsrcMgr->visit([&rsrcs](resource::Resource& rsrc) {
      rsrcs.insert(rsrc.shared_from_this());
      return common::Processing::CONTINUE;
    });
  }

  // Look for anything with the given UUID:
  for (const auto& rsrc : rsrcs)
  {
    if (rsrc != nullptr)
    {
      if (rsrc->id() == objId)
      {
        return this->associate(rsrc);
      }
      auto comp = rsrc->find(objId);
      if (comp)
      {
        return this->associate(comp);
      }
    }
  }
  return false;
}

/**\brief Associate a new-style model ID (a EntityRef) with this attribute.
  *
  * This function returns true when the association is valid and
  * successful. It may return false if the association is prohibited.
  * (This is not currently implemented.)
  */
bool Attribute::associateEntity(const smtk::model::EntityRef& entityRef)
{
  bool res = this->isEntityAssociated(entityRef);
  if (res)
    return res;

  res = (m_associatedObjects) ? m_associatedObjects->appendValue(entityRef.component()) : false;
  return res;
}

/**\brief Disassociate a new-style model ID (a UUID) from this attribute.
  *
  * If \a reverse is true (the default), then the model resource is notified
  * of the item's disassociation immediately after its removal from this
  * attribute, allowing the model and attribute to stay in sync.
  */
void Attribute::disassociateEntity(const smtk::common::UUID& entity, bool reverse)
{
  if (!m_associatedObjects)
  {
    return;
  }

  std::set<smtk::resource::Resource::Ptr> rsrcs = this->attributeResource()->associations();
  rsrcs.insert(this->attributeResource()); // We can always look for other attributes.

  // If we have a resource manager, we can also look for components in other resources:
  auto rsrcMgr = this->attributeResource()->manager();
  if (rsrcMgr)
  {
    rsrcMgr->visit([&rsrcs](resource::Resource& rsrc) {
      rsrcs.insert(rsrc.shared_from_this());
      return common::Processing::CONTINUE;
    });
  }

  // Look for anything with the given UUID:
  for (const auto& rsrc : rsrcs)
  {
    if (rsrc != nullptr)
    {
      auto comp = rsrc->find(entity);
      if (comp)
      {
        this->disassociate(comp, reverse);
      }
    }
  }
}

/**\brief Disassociate a new-style model entity (a EntityRef) from this attribute.
  *
  */
void Attribute::disassociateEntity(const smtk::model::EntityRef& entity, bool reverse)
{
  this->disassociate(entity.component(), reverse);
}

bool Attribute::canBeDisassociated(
  const smtk::resource::PersistentObjectPtr& obj,
  AttributePtr& probAtt) const
{
  probAtt = nullptr;
  if (!m_associatedObjects)
  {
    return true; // obj is not associated with the attribute
  }

  auto attRes = this->attributeResource();
  if (attRes == nullptr)
  {
    // This attribute is not part of a resource so there is nothing to remove
    return true;
  }

  // Is there a user-defined dissociation rule that prevents this action?
  {
    auto dissociationRule =
      this->attributeResource()->associationRules().dissociationRuleForDefinition(m_definition);
    if (dissociationRule)
    {
      std::shared_ptr<const Attribute> self = this->shared_from_this();
      if ((*dissociationRule)(self, obj))
      {
        return false;
      }
    }
  }

  // Is this attribute's definition is not used as a prerequisite
  // we can safely remove it
  if (!m_definition->isUsedAsAPrerequisite())
  {
    return true;
  }

  // Ok we found the object - now will removing the association
  // invalidate a prerequisite condition? To determine this do the following:
  // 1. Get all of the attributes associated with the object
  // 2. For each attribute do the following:
  // 2a. See if the attribute has this attribute's type as a prerequisite
  // Note that hasPrerequisite returns the required prerequisite def
  // 2b. If it does then see how many other attributes associated with the object
  // match the type - if there is only one then return false
  auto atts = attRes->attributes(obj);
  for (const auto& att : atts)
  {
    // If the attribite is the same type as this, or it has no
    // prerequisites, we can skip it
    if ((att->m_definition == m_definition) || (!att->m_definition->hasPrerequisites()))
    {
      // Don't need to check any atts from the original
      // definition
      continue;
    }
    auto preDef = att->m_definition->hasPrerequisite(m_definition);
    if (preDef == nullptr)
    {
      // Not a prerequisite for this attribute so skip it
      continue;
    }
    // Count number of atts that match the preDef
    int count = 0;
    for (const auto& att1 : atts)
    {
      if (att1->m_definition->isA(preDef))
      {
        count++;
      }
    }
    if (count == 1)
    {
      // Can't disassociate the attribute
      probAtt = att;
      return false;
    }
  }
  return true;
}

bool Attribute::disassociate(smtk::resource::PersistentObjectPtr obj, bool reverse)
{
  AttributePtr foo;
  return this->disassociate(obj, foo, reverse);
}

bool Attribute::disassociate(
  smtk::resource::PersistentObjectPtr obj,
  AttributePtr& probAtt,
  bool /*unused*/)
{
  if (!this->canBeDisassociated(obj, probAtt))
  {
    return false;
  }
  std::ptrdiff_t idx = m_associatedObjects->find(obj);
  if (idx < 0)
  {
    // the obj is not associated with the attribute
    return true;
  }
  m_associatedObjects->removeValue(idx);
  return true;
}

void Attribute::forceDisassociate(smtk::resource::PersistentObjectPtr obj)
{
  std::ptrdiff_t idx = m_associatedObjects->find(obj);
  if (idx >= 0)
  {
    m_associatedObjects->removeValue(idx);
  }
}
/**\brief Return the item with the given \a inName, searching in the given \a style.
  *
  * The search style dictates whether children of conditional items are included
  * and, if so, whether all of their children are searched or just the active children.
  * The default is to search active children.
  */
smtk::attribute::ItemPtr Attribute::find(const std::string& inName, SearchStyle style)
{
  // Lets see if we can find it in the attribute's items
  for (auto& item : m_items)
  {
    if (item->name() == inName)
    {
      return item;
    }
  }

  if (style == IMMEDIATE)
  {
    return nullptr; // its not amoung the attribute's items
  }

  // Lets check the children
  for (auto& item : m_items)
  {
    ItemPtr result = item->find(inName, style);
    if (result)
    {
      return result;
    }
  }
  return nullptr;
}

smtk::attribute::ConstItemPtr Attribute::find(const std::string& inName, SearchStyle style) const
{
  // Lets see if we can find it in the attribute's items
  for (auto& item : m_items)
  {
    if (item->name() == inName)
    {
      return item;
    }
  }

  if (style == IMMEDIATE)
  {
    return nullptr; // its not amoung the attribute's items
  }

  // Lets check the children
  for (auto& item : m_items)
  {
    ConstItemPtr result = item->find(inName, style);
    if (result)
    {
      return result;
    }
  }
  return nullptr;
}

smtk::attribute::IntItemPtr Attribute::findInt(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<IntItem>(this->find(nameStr));
}
smtk::attribute::ConstIntItemPtr Attribute::findInt(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const IntItem>(this->find(nameStr));
}

smtk::attribute::DoubleItemPtr Attribute::findDouble(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<DoubleItem>(this->find(nameStr));
}
smtk::attribute::ConstDoubleItemPtr Attribute::findDouble(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const DoubleItem>(this->find(nameStr));
}

smtk::attribute::StringItemPtr Attribute::findString(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<StringItem>(this->find(nameStr));
}
smtk::attribute::ConstStringItemPtr Attribute::findString(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const StringItem>(this->find(nameStr));
}

smtk::attribute::FileItemPtr Attribute::findFile(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<FileItem>(this->find(nameStr));
}
smtk::attribute::ConstFileItemPtr Attribute::findFile(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const FileItem>(this->find(nameStr));
}

smtk::attribute::DirectoryItemPtr Attribute::findDirectory(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<DirectoryItem>(this->find(nameStr));
}
smtk::attribute::ConstDirectoryItemPtr Attribute::findDirectory(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const DirectoryItem>(this->find(nameStr));
}

smtk::attribute::GroupItemPtr Attribute::findGroup(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<GroupItem>(this->find(nameStr));
}
smtk::attribute::ConstGroupItemPtr Attribute::findGroup(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const GroupItem>(this->find(nameStr));
}

smtk::attribute::ModelEntityItemPtr Attribute::findModelEntity(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<ModelEntityItem>(this->find(nameStr));
}
smtk::attribute::ConstModelEntityItemPtr Attribute::findModelEntity(
  const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const ModelEntityItem>(this->find(nameStr));
}

smtk::attribute::VoidItemPtr Attribute::findVoid(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<VoidItem>(this->find(nameStr));
}
smtk::attribute::ConstVoidItemPtr Attribute::findVoid(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const VoidItem>(this->find(nameStr));
}

smtk::attribute::DateTimeItemPtr Attribute::findDateTime(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<DateTimeItem>(this->find(nameStr));
}
smtk::attribute::ConstDateTimeItemPtr Attribute::findDateTime(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const DateTimeItem>(this->find(nameStr));
}

smtk::attribute::ReferenceItemPtr Attribute::findReference(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<ReferenceItem>(this->find(nameStr));
}
smtk::attribute::ConstReferenceItemPtr Attribute::findReference(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const ReferenceItem>(this->find(nameStr));
}

smtk::attribute::ResourceItemPtr Attribute::findResource(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<ResourceItem>(this->find(nameStr));
}
smtk::attribute::ConstResourceItemPtr Attribute::findResource(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const ResourceItem>(this->find(nameStr));
}

smtk::attribute::ComponentItemPtr Attribute::findComponent(const std::string& nameStr)
{
  return smtk::dynamic_pointer_cast<ComponentItem>(this->find(nameStr));
}
smtk::attribute::ConstComponentItemPtr Attribute::findComponent(const std::string& nameStr) const
{
  return smtk::dynamic_pointer_cast<const ComponentItem>(this->find(nameStr));
}

const smtk::attribute::Attribute::GuardedLinks Attribute::guardedLinks() const
{
  return GuardedLinks(this->attributeResource()->mutex(), this->links());
}

smtk::attribute::Attribute::GuardedLinks Attribute::guardedLinks()
{
  return GuardedLinks(this->attributeResource()->mutex(), this->links());
}
