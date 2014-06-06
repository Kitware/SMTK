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


#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/Manager.h"
#include "smtk/model/Cursor.h"
#include "smtk/model/GroupEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/util/UUID.h"

using namespace smtk::attribute;

/// Construct an item definition given a name. Names should be unique and non-empty.
ModelEntityItemDefinition::ModelEntityItemDefinition(const std::string& sname):
  ItemDefinition(sname)
{
  this->m_membershipMask = smtk::model::ANY_ENTITY;
  this->m_numberOfRequiredValues = 0;
  this->m_useCommonLabel = false;
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
bool ModelEntityItemDefinition::isValueValid(const smtk::model::Cursor& c) const
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
     !(itemType & this->m_membershipMask & smtk::model::ENTITY_MASK))     ||
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
    bool mustBeHomogenous = typeMask & smtk::model::HOMOGENOUS_GROUP;
    if (
      !c.as<model::GroupEntity>().meetsMembershipConstraintsInternal(
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
