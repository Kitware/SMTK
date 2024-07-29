//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Definition.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Resource.h"

#include "smtk/io/Logger.h"

#include "units/Converter.h"
#include "units/System.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>

using namespace smtk::attribute;
double Definition::s_notApplicableBaseColor[4] = { 0.0, 0.0, 0.0, 0.0 };
double Definition::s_defaultBaseColor[4] = { 1.0, 1.0, 1.0, 1.0 };

Definition::Definition(
  const std::string& myType,
  smtk::attribute::DefinitionPtr myBaseDef,
  ResourcePtr myResource,
  const smtk::common::UUID& myId)
{
  m_id = myId;
  m_resource = myResource;
  m_baseDefinition = myBaseDef;
  m_type = myType;
  m_label = m_type;
  m_version = 0;
  m_isAbstract = false;
  m_isNodal = false;
  m_hasLocalAdvanceLevelInfo[0] = m_hasLocalAdvanceLevelInfo[1] = false;
  m_localAdvanceLevel[0] = m_localAdvanceLevel[1] = 0;
  m_advanceLevel[0] = m_advanceLevel[1] = 0;
  m_isUnique = false;
  m_isRequired = false;
  m_isNotApplicableColorSet = false;
  m_isDefaultColorSet = false;
  m_rootName = m_type;
  m_includeIndex = 0;
  m_prerequisiteUsageCount = 0;
  m_combinationMode = Categories::CombinationMode::And;
  if (myBaseDef)
  {
    m_baseItemOffset = myBaseDef->numberOfItemDefinitions();
  }
  else
  {
    m_baseItemOffset = 0;
  }
}

Definition::~Definition() = default;

const Tag* Definition::tag(const std::string& name) const
{
  const Tag* tag = nullptr;

  auto t = m_tags.find(Tag(name));
  if (t != m_tags.end())
  {
    tag = &(*t);
  }

  return tag;
}

bool Definition::setId(const common::UUID&)
{
  smtkErrorMacro(
    smtk::io::Logger::instance(),
    "Changing the ID for Attribute Definition " << m_type << " is not supported\n");
  return false;
}

ResourcePtr Definition::attributeResource() const
{
  return m_resource.lock();
  ;
}

const smtk::resource::ResourcePtr Definition::resource() const
{
  return this->attributeResource();
}

Tag* Definition::tag(const std::string& name)
{
  const Tag* tag = nullptr;

  auto t = m_tags.find(Tag(name));
  if (t != m_tags.end())
  {
    tag = &(*t);
  }

  // Tags are ordered according to their name. This name is set at construction,
  // and there is deliberately no API to modify the name after construction. Tag
  // values are editable, however. Rather than make Tag values mutable, we
  // perform a const_cast here to facilitate Tag value modification. This does
  // not change the ordering of the Tag in the Tags set, so we do not break our
  // contract with std::set.
  return const_cast<Tag*>(tag);
}

bool Definition::addTag(const Tag& tag)
{
  auto t = m_tags.find(tag);
  if (t == m_tags.end())
  {
    m_tags.insert(tag);
    return true;
  }
  return false;
}

bool Definition::removeTag(const std::string& name)
{
  auto t = m_tags.find(Tag(name));
  if (t != m_tags.end())
  {
    m_tags.erase(t);
    return true;
  }
  return false;
}

bool Definition::isA(smtk::attribute::ConstDefinitionPtr targetDef) const
{
  // Walk up the inheritance tree until we either hit the root or
  // encounter this definition
  const Definition* def = this;
  for (def = this; def; def = def->m_baseDefinition.get())
  {
    if (def == targetDef.get())
    {
      return true;
    }
  }
  return false;
}

void Definition::setIsUnique(bool val)
{
  if (m_isUnique == val)
  {
    return;
  }
  // Determine if we need to add this or remove it from the exclusion list
  m_isUnique = val;
  auto def = this->shared_from_this();
  if (val)
  {
    this->m_exclusionDefs.insert(def);
  }
  else
  {
    for (auto it = m_exclusionDefs.begin(); it != m_exclusionDefs.end(); ++it)
    {
      auto wdef = (*it).lock();
      if (wdef == def)
      {
        m_exclusionDefs.erase(it);
        return;
      }
    }
  }
}

bool Definition::conflicts(smtk::attribute::DefinitionPtr def) const
{
  // 2 Definitions conflict if they exclude each other
  if (!m_exclusionDefs.empty())
  {
    for (const auto& wdef : m_exclusionDefs)
    {
      auto edef = wdef.lock(); // Need to get the shared pointer (if there is one)
      if (edef == nullptr)
      {
        continue;
      }
      if (edef == def)
      {
        return true;
      }
    }
  }
  return false;
}

bool Definition::isRelevant(
  bool includeCategoryCheck,
  bool includeReadAccess,
  unsigned int readAccessLevel) const
{
  if (includeCategoryCheck && (!m_ignoreCategories))
  {
    auto aResource = this->attributeResource();
    if (aResource && aResource->activeCategoriesEnabled())
    {
      if (!this->categories().passes(aResource->activeCategories()))
      {
        return false;
      }
    }
  }
  if (includeReadAccess)
  {
    return this->advanceLevel() <= readAccessLevel;
  }
  return true;
}

ConstReferenceItemDefinitionPtr Definition::associationRule() const
{
  if (!m_acceptsRules)
  {
    if (m_baseDefinition)
    {
      return m_baseDefinition->associationRule();
    }
  }
  return m_acceptsRules;
}

/**\brief Create the definition's local association rule that governs attribute associations.
  *
  * A Definition's local association rule overrides the rule it inherits from its Base Definition.
  * This creates a local association rule (if one does not already exist) and returns it.
  * The default is to create an empty association rule (nothing can be associated).
  */
ReferenceItemDefinitionPtr Definition::createLocalAssociationRule(const std::string& name)
{
  if (!m_acceptsRules)
  {
    if (name.empty())
    {
      std::ostringstream assocName;
      assocName << this->type() << "Associations";
      m_acceptsRules = ReferenceItemDefinition::New(assocName.str());
    }
    else
    {
      m_acceptsRules = ReferenceItemDefinition::New(name);
    }
    m_acceptsRules->setIsExtensible(false);
    m_acceptsRules->setNumberOfRequiredValues(0);
    m_acceptsRules->setRole(smtk::attribute::Resource::AssociationRole);
  }
  return m_acceptsRules;
}

/**\brief Return the definition's local association rule that governs attribute associations.
  *
  * A Definition's local association rule overrides the rule it inherits from its Base Definition.
  * This method can return a nullptr.  Note that the ReferenceDefinition returned is not constant.
  * Modifying it will change the Definition's association behavior.
  */
ReferenceItemDefinitionPtr Definition::localAssociationRule() const
{
  return m_acceptsRules;
}

/**\brief Set the rule that decides which model entities may be associated with instances of this definition.
  *
  * This will override the association rule the definition inherits from the Definition's Base Definition.
  */
void Definition::setLocalAssociationRule(ReferenceItemDefinitionPtr rule)
{
  m_acceptsRules = rule;
  m_acceptsRules->setRole(smtk::attribute::Resource::AssociationRole);
}

/**\brief Return the mask specifying which types of model entities this attribute can be associated with.
  *
  * As with the associationRule() method, the mask can be inherited from the Base Definition if it is not
  * set locally
  */
smtk::model::BitFlags Definition::associationMask() const
{
  smtk::model::BitFlags result = 0;
  auto accepts = this->associationRule();
  if (accepts)
  {
    const auto entries = accepts->acceptableEntries();
    for (const auto& entry : entries)
    {
      // FIXME: Handle "derived" types like smtk::session::polygon::Resource
      if (entry.first == "smtk::model::Resource")
      {
        smtk::model::BitFlags tmp = smtk::model::Entity::specifierStringToFlag(entry.second);
        result |= tmp;
      }
    }
  }
  return result;
}

/**\brief Set the mask specifying which types of model entities this attribute can be associated with.
  *
  * Note that this will create a local association rule if the Definition did not already have one
  * specified
  */
void Definition::setLocalAssociationMask(smtk::model::BitFlags mask)
{
  // FIXME: Restrict mask to be narrowing of base definition's rule?
  auto localRule = this->createLocalAssociationRule();
  localRule->setAcceptsEntries(
    "smtk::model::Resource", smtk::model::Entity::flagToSpecifierString(mask), true);
}

/**\brief Reoved the local assocaition rule on the definition.
  *
  * The Definition will now inherit its association rule from its Base Definition.
  */
void Definition::clearLocalAssociationRule()
{
  m_acceptsRules = nullptr;
}

/// Returns whether this attribute can be associated with vertices.
bool Definition::associatesWithVertex() const
{
  return smtk::model::isVertex(this->associationMask());
}

/// Returns whether this attribute can be associated with edges.
bool Definition::associatesWithEdge() const
{
  return smtk::model::isEdge(this->associationMask());
}

/// Returns whether this attribute can be associated with faces.
bool Definition::associatesWithFace() const
{
  return smtk::model::isFace(this->associationMask());
}

/// Returns whether this attribute can be associated with volumes.
bool Definition::associatesWithVolume() const
{
  return smtk::model::isVolume(this->associationMask());
}

/// Returns whether this attribute can be associated with models.
bool Definition::associatesWithModel() const
{
  return smtk::model::isModel(this->associationMask());
}

/// Returns whether this attribute can be associated with groups.
bool Definition::associatesWithGroup() const
{
  return smtk::model::isGroup(this->associationMask());
}

/// Return whether this attribute can be associated with entities that have the given \a flag value.
bool Definition::canBeAssociated(smtk::model::BitFlags flag) const
{
  // This is a simpler version of Group::meetsMembershipConstraints().
  smtk::model::BitFlags mask = this->associationMask();
  smtk::model::BitFlags memberTest = flag & mask;
  if (!(memberTest & smtk::model::ANY_DIMENSION) && (mask & smtk::model::ANY_DIMENSION))
    return false;
  if (!(memberTest & smtk::model::ENTITY_MASK) && (mask & smtk::model::ENTITY_MASK))
    return false;
  return true;
}

/**\brief Return whether this attribute can be associated with the given \a object.
  *
  * This method first tests to see if the attribute can be associated with the object
  * based on the attribute definition's association rules.  If the object fails the check
  * AssociationResultType::Illegal is returned. Else the attribute definition's
  * exclusion rules are checked.  If it fails this test, Definition::AssociationResultType::Conflict
  * is returned along with the conflicting attributes.
  * Finally, the attribute definition's prerequisite rules are tested.
  * If it fails this test, Definition::AssociationResultType::Prerequisite is returned
  * along with the missing prerequiste attribute definition.  If all checks pass,
  * Definition::AssociationResultType::Valid is returned.
  *
  * Note that testing is stop when the first issue is found so there may be
  * additional issues beyond the one reported.  For example, there may be multiple
  * conflicts and/or missing prerequisites
  */
Definition::AssociationResultType Definition::canBeAssociated(
  smtk::resource::ConstPersistentObjectPtr object,
  AttributePtr& conflictAtt,
  DefinitionPtr& prerequisiteDef) const
{
  if (!this->checkAssociationRules(object))
  {
    return Definition::AssociationResultType::Illegal;
  }

  // Let's check for conflicts
  conflictAtt = this->checkForConflicts(object);
  if (conflictAtt != nullptr)
  {
    return Definition::AssociationResultType::Conflict;
  }

  // Finally let's check for missing prerequisite attributes
  prerequisiteDef = this->checkForPrerequisites(object);
  if (prerequisiteDef != nullptr)
  {
    return Definition::AssociationResultType::Prerequisite;
  }
  return Definition::AssociationResultType::Valid;
}
bool Definition::checkAssociationRules(smtk::resource::ConstPersistentObjectPtr object) const
{
  if (m_acceptsRules != nullptr)
  {
    // Let's verify that the object passes the contraints imposed by the association rules
    return m_acceptsRules->isValueValid(object);
  }
  if (m_baseDefinition)
  {
    return m_baseDefinition->checkAssociationRules(object);
  }
  return false;
}

AttributePtr Definition::checkForConflicts(smtk::resource::ConstPersistentObjectPtr object) const
{
  if (!m_exclusionDefs.empty())
  {
    for (const auto& wdef : m_exclusionDefs)
    {
      auto def = wdef.lock(); // Need to get the shared pointer (if there is one)
      if (def == nullptr)
      {
        continue;
      }
      auto atts = def->attributes(object);
      if (!atts.empty())
      {
        return *(atts.begin());
      }
    }
  }
  // If we have a base definition we need to test it as well
  if (m_baseDefinition)
  {
    return m_baseDefinition->checkForConflicts(object);
  }
  return nullptr;
}

DefinitionPtr Definition::checkForPrerequisites(
  smtk::resource::ConstPersistentObjectPtr object) const
{
  // Next let's see if there are any attributes that would exclude this one
  if (!m_prerequisiteDefs.empty())
  {
    for (const auto& wdef : m_prerequisiteDefs)
    {
      auto def = wdef.lock(); // Need to get the shared pointer (if there is one)
      if (def == nullptr)
      {
        continue;
      }
      auto atts = def->attributes(object);
      if (atts.empty())
      {
        return def;
      }
    }
  }
  // if we have a base definition we need to test it as well
  if (m_baseDefinition)
  {
    return m_baseDefinition->checkForPrerequisites(object);
  }
  return nullptr;
}

void Definition::removeExclusion(smtk::attribute::DefinitionPtr def)
{
  for (auto it = m_exclusionDefs.begin(); it != m_exclusionDefs.end(); ++it)
  {
    auto exDef = (*it).lock();
    if (def == exDef)
    {
      m_exclusionDefs.erase(it);
      break;
    }
  }

  for (auto it = def->m_exclusionDefs.begin(); it != def->m_exclusionDefs.end(); ++it)
  {
    auto exDef = (*it).lock();
    if (exDef.get() == this)
    {
      def->m_exclusionDefs.erase(it);
      return;
    }
  }
}

void Definition::removePrerequisite(smtk::attribute::DefinitionPtr def)
{
  for (auto it = m_prerequisiteDefs.begin(); it != m_prerequisiteDefs.end(); ++it)
  {
    auto reqDef = (*it).lock();
    if (def == reqDef)
    {
      m_prerequisiteDefs.erase(it);
      --def->m_prerequisiteUsageCount;
      return;
    }
  }
}

bool Definition::isUsedAsAPrerequisite() const
{
  if (m_prerequisiteUsageCount)
  {
    return true;
  }
  auto def = m_baseDefinition;
  while (def != nullptr)
  {
    if (def->m_prerequisiteUsageCount)
    {
      return true;
    }
    def = def->m_baseDefinition;
  }
  return false;
}

std::vector<std::string> Definition::excludedTypeNames() const
{
  std::vector<std::string> types;
  for (auto it = m_exclusionDefs.begin(); it != m_exclusionDefs.end(); ++it)
  {
    auto exDef = (*it).lock();
    if (exDef)
    {
      types.push_back(exDef->type());
    }
  }
  std::sort(types.begin(), types.end());
  return types;
}

std::vector<std::string> Definition::prerequisiteTypeNames() const
{
  std::vector<std::string> types;
  for (auto it = m_prerequisiteDefs.begin(); it != m_prerequisiteDefs.end(); ++it)
  {
    auto preDef = (*it).lock();
    if (preDef)
    {
      types.push_back(preDef->type());
    }
  }
  std::sort(types.begin(), types.end());
  return types;
}

smtk::attribute::ConstDefinitionPtr Definition::hasPrerequisite(
  smtk::attribute::ConstDefinitionPtr def) const
{
  if (!m_prerequisiteDefs.empty())
  {
    for (auto it = m_prerequisiteDefs.begin(); it != m_prerequisiteDefs.end(); ++it)
    {
      auto preDef = (*it).lock();
      if (preDef && def->isA(preDef))
      {
        return preDef;
      }
    }
  }
  if (m_baseDefinition)
  {
    return m_baseDefinition->hasPrerequisite(def);
  }
  smtk::attribute::ConstDefinitionPtr nope;
  return nope;
}

bool Definition::hasPrerequisites() const
{
  if (!m_prerequisiteDefs.empty())
  {
    return true;
  }
  auto def = m_baseDefinition;
  while (def != nullptr)
  {
    if (!def->m_prerequisiteDefs.empty())
    {
      return true;
    }
    def = def->m_baseDefinition;
  }
  return false;
}

void Definition::addPrerequisite(smtk::attribute::DefinitionPtr def)
{
  if (def == nullptr)
  {
    return;
  }
  auto num = m_prerequisiteDefs.size();
  m_prerequisiteDefs.insert(def);
  if (num < m_prerequisiteDefs.size())
  {
    // OK we did add a prerequisite
    ++def->m_prerequisiteUsageCount;
  }
}

void Definition::buildAttribute(Attribute* att) const
{
  // If there is a super definition have it prep the attribute and add its items
  const Definition* bdef = m_baseDefinition.get();
  if (bdef)
  {
    bdef->buildAttribute(att);
  }
  else
  {
    // This is the "base definition" so first we should make sure the attribute
    // is "empty" of items and associations
    if (att->m_associatedObjects)
    {
      att->m_associatedObjects->detachOwningAttribute();
    }
    att->removeAllItems();
    att->m_associatedObjects = ReferenceItemPtr();
  }

  // If the definition allows associations, create an item to hold them,
  // overriding any rule from the base definition:
  if (m_acceptsRules)
  {
    if (att->m_associatedObjects)
    {
      att->m_associatedObjects->detachOwningAttribute();
    }

    att->m_associatedObjects =
      smtk::dynamic_pointer_cast<ReferenceItem>(m_acceptsRules->buildItem(att, -2));
    att->m_associatedObjects->setDefinition(m_acceptsRules);
  }

  // Next - for each item definition we have build and add the appropriate
  // item to the attribute
  smtk::attribute::ItemPtr comp;
  std::size_t i, j, n = m_itemDefs.size();
  j = att->numberOfItems();
  for (i = 0; i < n; i++, j++)
  {
    comp = m_itemDefs[i]->buildItem(att, static_cast<int>(j));
    comp->setDefinition(m_itemDefs[i]);
    att->addItem(comp);
  }
}

bool Definition::addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef)
{
  // First see if there is a item by the same name
  if (this->findItemPosition(cdef->name()) >= 0)
  {
    return false;
  }
  std::size_t n = m_itemDefs.size();
  m_itemDefs.push_back(cdef);
  m_itemDefPositions[cdef->name()] = static_cast<int>(n);
  this->setItemDefinitionUnitsSystem(cdef);
  this->updateDerivedDefinitions();
  return true;
}

void Definition::setItemDefinitionUnitsSystem(
  const smtk::attribute::ItemDefinitionPtr& itemDef) const
{
  const auto& defUnitsSystem = this->unitsSystem();
  auto attRes = this->attributeResource();
  if (defUnitsSystem)
  {
    itemDef->setUnitsSystem(defUnitsSystem);
  }
}

void Definition::updateDerivedDefinitions()
{
  DefinitionPtr def = this->shared_from_this();
  if (def)
  {
    auto attresource = this->attributeResource();
    if (attresource != nullptr)
    {
      attresource->updateDerivedDefinitionIndexOffsets(def);
    }
  }
}

smtk::attribute::ItemDefinitionPtr Definition::itemDefinition(int ith) const
{
  // Is the item in this defintion?
  if (ith >= static_cast<int>(m_baseItemOffset))
  {
    assert(m_itemDefs.size() > static_cast<std::size_t>(ith - m_baseItemOffset));
    return m_itemDefs[static_cast<std::size_t>(ith - m_baseItemOffset)];
  }
  else if (m_baseDefinition)
  {
    return m_baseDefinition->itemDefinition(ith);
  }
  return smtk::attribute::ItemDefinitionPtr();
}

int Definition::findItemPosition(const std::string& name) const
{
  std::map<std::string, int>::const_iterator it;
  it = m_itemDefPositions.find(name);
  if (it == m_itemDefPositions.end())
  {
    // Check the base definition if there is one
    if (m_baseDefinition)
    {
      return m_baseDefinition->findItemPosition(name);
    }
    else
    {
      return -1; // named item doesn't exist
    }
  }
  return it->second + static_cast<int>(m_baseItemOffset);
}

bool Definition::removeItemDefinition(ItemDefinitionPtr itemDef)
{
  if (!itemDef || this->findItemPosition(itemDef->name()) < 0)
  {
    // Not found
    return false;
  }

  auto itItemDef = std::find(m_itemDefs.begin(), m_itemDefs.end(), itemDef);
  if (itItemDef != m_itemDefs.end())
  {
    m_itemDefs.erase(itItemDef);
  }
  m_itemDefPositions.erase(itemDef->name());
  this->updateDerivedDefinitions();
  return true;
}

std::set<AttributePtr> Definition::attributes(
  const smtk::resource::ConstPersistentObjectPtr& object) const
{
  // Get all attributes that are associated with the object and then remove all attributes whose definitions
  // are not derived from this one.
  auto atts = this->attributeResource()->attributes(object);
  auto sharedDef = this->shared_from_this();
  for (auto it = atts.begin(); it != atts.end();)
  {
    if ((*it)->definition()->isA(sharedDef))
    {
      // Keep this in the list
      ++it;
    }
    else
    {
      // its not derived from this so remove it from the list
      it = atts.erase(it);
    }
  }
  return atts;
}

void Definition::applyCategories(smtk::attribute::Categories::Stack inherited)
{
  smtk::attribute::Categories inheritedFromItems;
  m_categories.reset();

  // First append the definition's local category info to
  // what we are inheriting. Note that we want to not modify the original list which is why
  // its passed by value
  inherited.append(m_combinationMode, m_localCategories);

  // Next append the def's categories to those we have inherited (if we are not only considering local Only)
  if (m_baseDefinition && (m_combinationMode != Categories::CombinationMode::LocalOnly))
  {
    m_categories.insert(m_baseDefinition->m_categories);
  }

  // Lets go to each item and process its categories
  for (auto& item : m_itemDefs)
  {
    item->applyCategories(inherited, inheritedFromItems);
  }

  // Now we need to assign the categories to the def itself.
  // We start with all of the categories associated with the def's
  // base definition - note that we assume that the inherited set passed
  // in is contained within the base's categories
  if (m_baseDefinition && (m_combinationMode != Categories::CombinationMode::LocalOnly))
  {
  }
  m_categories.insert(inherited);

  // We need to add all of the categories that were locally defined
  // on the items contained within the definition
  m_categories.insert(inheritedFromItems);

  // Finally we need to process all definitions that are derived from this one
  auto attResource = m_resource.lock();
  if (attResource == nullptr)
  {
    return; // Can't get the derived definitions
  }

  std::vector<DefinitionPtr> derivedDefs;
  attResource->derivedDefinitions(this->shared_from_this(), derivedDefs);
  for (auto& def : derivedDefs)
  {
    def->applyCategories(inherited);
  }
}

void Definition::setLocalAdvanceLevel(int mode, unsigned int level)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  m_hasLocalAdvanceLevelInfo[mode] = true;
  m_advanceLevel[mode] = m_localAdvanceLevel[mode] = level;
}

void Definition::setLocalAdvanceLevel(unsigned int level)
{
  m_hasLocalAdvanceLevelInfo[0] = m_hasLocalAdvanceLevelInfo[1] = true;
  m_advanceLevel[0] = m_advanceLevel[1] = m_localAdvanceLevel[0] = m_localAdvanceLevel[1] = level;
}

void Definition::unsetLocalAdvanceLevel(int mode)
{
  if ((mode < 0) || (mode > 1))
  {
    return;
  }
  m_hasLocalAdvanceLevelInfo[mode] = false;
}

unsigned int Definition::advanceLevel(int mode) const
{
  // Any invalid mode returns mode = 0
  if ((mode < 0) || (mode > 1))
  {
    mode = 0;
  }
  return m_advanceLevel[mode];
}

void Definition::applyAdvanceLevels(
  const unsigned int& readLevelFromParent,
  const unsigned int& writeLevelFromParent)
{
  if (!m_hasLocalAdvanceLevelInfo[0])
  {
    m_advanceLevel[0] = readLevelFromParent;
  }
  if (!m_hasLocalAdvanceLevelInfo[1])
  {
    m_advanceLevel[1] = writeLevelFromParent;
  }

  // Lets go to each item and process its advance levels
  for (auto& item : m_itemDefs)
  {
    item->applyAdvanceLevels(m_advanceLevel[0], m_advanceLevel[1]);
  }

  auto attResource = m_resource.lock();
  if (attResource == nullptr)
  {
    return; // Can't get the derived definitions
  }

  std::vector<DefinitionPtr> derivedDefs;
  attResource->derivedDefinitions(this->shared_from_this(), derivedDefs);
  for (auto& def : derivedDefs)
  {
    def->applyAdvanceLevels(m_advanceLevel[0], m_advanceLevel[1]);
  }
}

const std::shared_ptr<units::System>& Definition::unitsSystem() const
{
  static std::shared_ptr<units::System> nullUnitsSystem;
  auto attRes = this->attributeResource();
  if (attRes)
  {
    return attRes->unitsSystem();
  }
  return nullUnitsSystem;
}

bool Definition::setLocalUnits(const std::string& newUnits, bool force)
{
  const auto& currentUnits = this->units();
  if (currentUnits.empty() || force)
  {
    m_localUnits = newUnits;
    return true;
  }
  const auto& unitSys = this->unitsSystem();
  if (!unitSys)
  {
    return false; // There is no unit system
  }

  // Are the requested units supported by the units system
  bool unitsSupported;
  auto nUnits = unitSys->unit(newUnits, &unitsSupported);
  if (!unitsSupported)
  {
    // the new units are not supported by the units system
    return false;
  }

  if (currentUnits == "*")
  {
    // This definition can accept any support units
    m_localUnits = newUnits;
    return true;
  }

  // Can we convert from the current units to the proposed new units?
  auto cUnits = unitSys->unit(currentUnits, &unitsSupported);
  if (!unitsSupported)
  {
    // the current units are not supported by the units system so
    // there is no conversion
    return false;
  }

  if (unitSys->convert(cUnits, nUnits))
  {
    m_localUnits = newUnits;
    return true;
  }
  // No known conversion
  return false;
}

const std::string& Definition::units() const
{
  // If it has units set, return them.  Else if there is
  // a base definition return those
  if (m_localUnits.empty() && m_baseDefinition)
  {
    return m_baseDefinition->units();
  }

  return m_localUnits;
}
