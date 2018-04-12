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
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>

using namespace smtk::attribute;
double Definition::s_notApplicableBaseColor[4] = { 0.0, 0.0, 0.0, 0.0 };
double Definition::s_defaultBaseColor[4] = { 1.0, 1.0, 1.0, 1.0 };

Definition::Definition(
  const std::string& myType, smtk::attribute::DefinitionPtr myBaseDef, CollectionPtr myCollection)
{
  m_collection = myCollection;
  m_baseDefinition = myBaseDef;
  m_type = myType;
  m_label = m_type;
  m_version = 0;
  m_isAbstract = false;
  m_isNodal = false;
  m_advanceLevel = 0;
  m_isUnique = true;
  m_isRequired = false;
  m_isNotApplicableColorSet = false;
  m_isDefaultColorSet = false;
  m_rootName = m_type;
  if (myBaseDef)
  {
    m_baseItemOffset = myBaseDef->numberOfItemDefinitions();
  }
  else
  {
    m_baseItemOffset = 0;
  }
}

Definition::~Definition()
{
}

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
  // Walk up the inheritence tree until we either hit the root or
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
  auto attresource = this->collection();
  if (attresource == nullptr)
  {
    return false; // there is no derived info
  }
  smtk::attribute::ConstDefinitionPtr baseDef = attresource->findIsUniqueBaseClass(def);
  // See if the other definition is derived from this base definition.
  // If it is not then we know there is no conflict
  return def->isA(baseDef);
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
  * A Definition's local asscoation rule overrides the rule it inherits from its Base Definition.
  * This creates a local association rule (if one does not already exist) and returns it.
  */
ReferenceItemDefinitionPtr Definition::createLocalAssociationRule()
{
  if (!m_acceptsRules)
  {
    std::ostringstream assocName;
    assocName << this->type() << "Associations";
    m_acceptsRules = ReferenceItemDefinition::New(assocName.str());
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
  * This will override the association rule the definition inherits from the Definition's Base Definiiton.
  */
void Definition::setLocalAssociationRule(ReferenceItemDefinitionPtr rule)
{
  m_acceptsRules = rule;
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
    for (auto entry : entries)
    {
      // FIXME: Handle "derived" types like smtk::bridge::polygon::Resource
      if (entry.first == "smtk::model::Manager")
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
    "smtk::model::Manager", smtk::model::Entity::flagToSpecifierString(mask), true);
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

/**\brief Return whether this attribute can be associated with the given \a entity.
  *
  * TODO:
  *   In this case we need to process BCS and DS specially
  *   We look at the model's dimension and based on that return
  *   the appropriate associatesWith method
  *   Conflicts will contain a list of attributes that prevent an attribute
  *   of this type from being associated
  */
bool Definition::canBeAssociated(
  smtk::model::EntityRef /*entity*/, std::vector<Attribute*>* /*inConflicts*/) const
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
    att->removeAllItems();
    att->m_associatedObjects = ReferenceItemPtr();
  }

  // If the definition allows associations, create an item to hold them,
  // overriding any rule from the base definition:
  auto localRule = m_acceptsRules;
  if (localRule)
  {
    att->m_associatedObjects =
      smtk::dynamic_pointer_cast<ReferenceItem>(localRule->buildItem(att, -2));
    att->m_associatedObjects->setDefinition(localRule);
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

bool Definition::isMemberOf(const std::vector<std::string>& inCategories) const
{
  std::size_t i, n = inCategories.size();
  for (i = 0; i < n; i++)
  {
    if (this->isMemberOf(inCategories[i]))
      return true;
  }
  return false;
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
  this->updateDerivedDefinitions();
  return true;
}

void Definition::updateDerivedDefinitions()
{
  DefinitionPtr def = this->shared_from_this();
  if (def)
  {
    auto attresource = this->collection();
    if (attresource != nullptr)
    {
      attresource->updateDerivedDefinitionIndexOffsets(def);
    }
  }
}

void Definition::setCategories()
{
  if (m_baseDefinition)
  {
    m_categories = m_baseDefinition->m_categories;
  }
  else
  {
    m_categories.clear();
  }
  std::size_t i, n = m_itemDefs.size();
  for (i = 0; i < n; i++)
  {
    m_itemDefs[i]->updateCategories();
    const std::set<std::string>& itemCats = m_itemDefs[i]->categories();
    m_categories.insert(itemCats.begin(), itemCats.end());
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
