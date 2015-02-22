//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/MeshEntityItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/MeshEntityItem.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
MeshEntityItemDefinition::
MeshEntityItemDefinition(const std::string &myName):
  ItemDefinition(myName)
{
}

//----------------------------------------------------------------------------
MeshEntityItemDefinition::~MeshEntityItemDefinition()
{
}
//----------------------------------------------------------------------------
Item::Type MeshEntityItemDefinition::type() const
{
  return Item::MESH_ENTITY;
}

//----------------------------------------------------------------------------
bool MeshEntityItemDefinition::isValueValid(const int &val) const
{
  return val >= 0;
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr MeshEntityItemDefinition::buildItem(
  Attribute *owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(new MeshEntityItem(owningAttribute,
                                              itemPosition));
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr MeshEntityItemDefinition::buildItem(
Item *owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new MeshEntityItem(owningItem,
                                              itemPosition,
                                              subGroupPosition));
}

//----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr
smtk::attribute::MeshEntityItemDefinition::
createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;

  std::size_t i;

  smtk::attribute::MeshEntityItemDefinitionPtr instance =
    smtk::attribute::MeshEntityItemDefinition::New(this->name());
  ItemDefinition::copyTo(instance);

  return instance;
}
//----------------------------------------------------------------------------
