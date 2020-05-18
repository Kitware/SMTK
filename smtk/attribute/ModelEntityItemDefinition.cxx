//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/common/UUID.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Resource.h"

#include <cassert>

using namespace smtk::attribute;

/// Construct an item definition given a name. Names should be unique and non-empty.
ModelEntityItemDefinition::ModelEntityItemDefinition(const std::string& sname)
  : ComponentItemDefinition(sname)
{
  this->setAcceptsEntries(smtk::common::typeName<smtk::model::Resource>(), "", true);
}

/// Destructor.
ModelEntityItemDefinition::~ModelEntityItemDefinition() = default;

/// Return the type of storage used by items defined by this class.
Item::Type ModelEntityItemDefinition::type() const
{
  return Item::ModelEntityType;
}

/// Return the mask used to accept or reject entities as attribute values.
smtk::model::BitFlags ModelEntityItemDefinition::membershipMask() const
{
  std::string queryString = this->acceptableEntries().begin()->first;
  return queryString.empty() ? smtk::model::ANY_ENTITY
                             : smtk::model::Entity::specifierStringToFlag(queryString);
}

/**\brief Set the mask used to accept or reject entities as attribute values.
  *
  * This mask should include at least one dimension bit and at least one
  * entity-type bit. See smtk::model::EntityTypeBits for valid bits.
  */
void ModelEntityItemDefinition::setMembershipMask(smtk::model::BitFlags entMask)
{
  // FIXME: Should we enforce constraints?
  this->m_acceptable.clear();
  this->setAcceptsEntries(
    smtk::common::typeName<smtk::model::Resource>(),
    smtk::model::Entity::flagToSpecifierString(entMask),
    true);
}

//// Construct an item from the definition given its owning attribute and position.
smtk::attribute::ItemPtr ModelEntityItemDefinition::buildItem(
  Attribute* owningAttribute,
  int itemPosition) const
{
  return smtk::attribute::ItemPtr(new ModelEntityItem(owningAttribute, itemPosition));
}

//// Construct an item from the definition given its owning item and position.
smtk::attribute::ItemPtr
ModelEntityItemDefinition::buildItem(Item* owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new ModelEntityItem(owningItem, itemPosition, subGroupPosition));
}

bool ModelEntityItemDefinition::setAcceptsEntries(
  const std::string& typeName,
  const std::string& queryString,
  bool accept)
{
  return this->ComponentItemDefinition::setAcceptsEntries(typeName, queryString, accept);
}
