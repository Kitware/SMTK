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
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/System.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"
#include "smtk/common/UUID.h"

using namespace smtk::attribute;

/// Construct an item definition given a name. Names should be unique and non-empty.
ModelEntityItemDefinition::ModelEntityItemDefinition(const std::string& sname):
  ItemDefinition(sname)
{
  this->m_membershipMask = smtk::model::ANY_ENTITY;
  this->m_numberOfRequiredValues = 0;
  this->m_useCommonLabel = false;
  this->m_isExtensible = false;
  this->m_maxNumberOfValues = 0;
}

/// Destructor.
ModelEntityItemDefinition::~ModelEntityItemDefinition()
{
}

/// Return the type of storage used by items defined by this class.
Item::Type ModelEntityItemDefinition::type() const
{
  return Item::MODEL_ENTITY;
}

/// Return the mask used to accept or reject entities as attribute values.
smtk::model::BitFlags ModelEntityItemDefinition::membershipMask() const
{
  return this->m_membershipMask;
}

/**\brief Set the mask used to accept or reject entities as attribute values.
  *
  * This mask should include at least one dimension bit and at least one
  * entity-type bit. See smtk::model::EntityTypeBits for valid bits.
  */
void ModelEntityItemDefinition::setMembershipMask(smtk::model::BitFlags entMask)
{
  // FIXME: Should we enforce constraints?
  this->m_membershipMask = entMask;
}

/**\brief Is an attribute's value consistent with its definition?
  *
  * This returns false when the definition has an item bitmask
  * that does not include entities of the matching type.
  * However, if the model entity's type cannot be determined
  * because the model manager is NULL or it is not in the model
  * manager's storage, we silently assume that the value is valid.
  * Note that only UUIDs are stored
  */
bool ModelEntityItemDefinition::isValueValid(const smtk::model::EntityRef& c) const
{
  if (!this->m_membershipMask)
    return false; // Nothing can possibly match.
  if (this->m_membershipMask == smtk::model::ANY_ENTITY)
    return true; // Fast-track the trivial case.
  if (!c.isValid())
    return true;

  smtk::model::BitFlags itemType = c.entityFlags();
  // The m_membershipMask must match the entity type, the dimension, and (if the
  // item is a group) group constraint flags separately;
  // In other words, we require the entity type, the dimension, and the
  // group constraints to be acceptable independently.
  if (
    ((this->m_membershipMask & smtk::model::ENTITY_MASK)               &&
     !(itemType & this->m_membershipMask & smtk::model::ENTITY_MASK)   &&
     (itemType & smtk::model::ENTITY_MASK) != smtk::model::GROUP_ENTITY)  ||
    ((this->m_membershipMask & smtk::model::ANY_DIMENSION)             &&
     !(itemType & this->m_membershipMask & smtk::model::ANY_DIMENSION))   ||
    ((itemType & smtk::model::GROUP_ENTITY) &&
     (this->m_membershipMask & smtk::model::GROUP_CONSTRAINT_MASK)    &&
     !(itemType & this->m_membershipMask & smtk::model::GROUP_CONSTRAINT_MASK))
    )
    return false;
  if (itemType & smtk::model::GROUP_ENTITY)
    {
    // The item is a group: recursively check that its members
    // all match the criteria. Also, if the HOMOGENOUS_GROUP bit is set,
    // require all entries to have the same entity type flag as the first.
    smtk::model::BitFlags typeMask = this->m_membershipMask;
    bool mustBeHomogenous = (typeMask & smtk::model::HOMOGENOUS_GROUP) ? true : false;
    if (!(typeMask & smtk::model::NO_SUBGROUPS) && !(typeMask & smtk::model::GROUP_ENTITY))
      typeMask |= smtk::model::GROUP_ENTITY; // if groups aren't banned, allow them.
    if (
      !c.as<model::Group>().meetsMembershipConstraintsInternal(
        c, typeMask, mustBeHomogenous))
      return false;
    }
  return true;
}

//// Construct an item from the definition given its owning attribute and position.
smtk::attribute::ItemPtr ModelEntityItemDefinition::buildItem(
  Attribute* owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(
    new ModelEntityItem(owningAttribute, itemPosition));
}

//// Construct an item from the definition given its owning item and position.
smtk::attribute::ItemPtr ModelEntityItemDefinition::buildItem(
  Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(
    new ModelEntityItem(owningItem, itemPosition, subGroupPosition));
}

/// Return the number of values (model entities) required by this definition.
std::size_t ModelEntityItemDefinition::numberOfRequiredValues() const
{
  return this->m_numberOfRequiredValues;
}

/// Set the number of values (model entities) required by this definition. Use 0 when there is no requirement.
void ModelEntityItemDefinition::setNumberOfRequiredValues(std::size_t esize)
{
  if (esize == this->m_numberOfRequiredValues)
    {
    return;
    }
  this->m_numberOfRequiredValues = esize;
  if (!this->m_useCommonLabel)
    {
    this->m_valueLabels.resize(esize);
    }
}

/// Set the maximum number of values accepted (or 0 for no limit).
void ModelEntityItemDefinition::setMaxNumberOfValues(std::size_t maxNum)
{
  this->m_maxNumberOfValues = maxNum;
}

/// Return whether the definition provides labels for each value.
bool ModelEntityItemDefinition::hasValueLabels() const
{
  return !this->m_valueLabels.empty();
}

/// Return the label for the \a i-th model entity.
std::string ModelEntityItemDefinition::valueLabel(std::size_t i) const
{
  if (this->m_useCommonLabel)
    {
    return this->m_valueLabels[0];
    }
  if (this->m_valueLabels.size())
    {
    return this->m_valueLabels[i];
    }
  return "";
}

/// Set the label for the \a i-th entity.
void ModelEntityItemDefinition::setValueLabel(std::size_t i, const std::string &elabel)
{
  if (this->m_numberOfRequiredValues == 0)
    {
    return;
    }
  if (this->m_valueLabels.size() != this->m_numberOfRequiredValues)
    {
    this->m_valueLabels.resize(this->m_numberOfRequiredValues);
    }
  this->m_useCommonLabel = false;
  this->m_valueLabels[i] = elabel;
}

/// Indicate that all values share the \a elabel provided.
void ModelEntityItemDefinition::setCommonValueLabel(const std::string &elabel)
{
  this->m_useCommonLabel = true;
  this->m_valueLabels.resize(1);
  this->m_valueLabels[0] = elabel;
}

/// Returns true when all values share a common label and false otherwise.
bool ModelEntityItemDefinition::usingCommonLabel() const
{
  return this->m_useCommonLabel;
}

/// Builds and returns copy of self
ItemDefinitionPtr
ModelEntityItemDefinition::
createCopy(ItemDefinition::CopyInfo& info) const
{
  (void)info;
  std::size_t i;

  smtk::attribute::ModelEntityItemDefinitionPtr newDef =
    smtk::attribute::ModelEntityItemDefinition::New(this->name());
  ItemDefinition::copyTo(newDef);

  newDef->setMembershipMask(m_membershipMask);
  newDef->setNumberOfRequiredValues(m_numberOfRequiredValues);
  newDef->setMaxNumberOfValues(m_maxNumberOfValues);
  newDef->setIsExtensible(m_isExtensible);
  if (m_useCommonLabel)
    {
    newDef->setCommonValueLabel(m_valueLabels[0]);
    }
  else if (this->hasValueLabels())
    {
    for (i=0; i<m_valueLabels.size(); ++i)
      {
      newDef->setValueLabel(i, m_valueLabels[i]);
      }
    }

  return newDef;
}
