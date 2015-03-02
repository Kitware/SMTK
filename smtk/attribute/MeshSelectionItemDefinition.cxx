//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/MeshSelectionItemDefinition.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/MeshSelectionItem.h"

using namespace smtk::attribute;

//----------------------------------------------------------------------------
MeshSelectionItemDefinition::
MeshSelectionItemDefinition(const std::string &myName):
  ItemDefinition(myName)
{
}

//----------------------------------------------------------------------------
MeshSelectionItemDefinition::~MeshSelectionItemDefinition()
{
}
//----------------------------------------------------------------------------
Item::Type MeshSelectionItemDefinition::type() const
{
  return Item::MESH_SELECTION;
}

//----------------------------------------------------------------------------
bool MeshSelectionItemDefinition::isValueValid(const int &val) const
{
  return val >= 0;
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr MeshSelectionItemDefinition::buildItem(
  Attribute *owningAttribute, int itemPosition) const
{
  return smtk::attribute::ItemPtr(new MeshSelectionItem(owningAttribute,
                                              itemPosition));
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr MeshSelectionItemDefinition::buildItem(
Item *owningItem, int itemPosition, int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new MeshSelectionItem(owningItem,
                                              itemPosition,
                                              subGroupPosition));
}

//----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr
smtk::attribute::MeshSelectionItemDefinition::
createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  (void)info;

  std::size_t i;

  smtk::attribute::MeshSelectionItemDefinitionPtr instance =
    smtk::attribute::MeshSelectionItemDefinition::New(this->name());
  ItemDefinition::copyTo(instance);
  instance->setRefModelEntityName(this->refModelEntityName());
  return instance;
}
//----------------------------------------------------------------------------
