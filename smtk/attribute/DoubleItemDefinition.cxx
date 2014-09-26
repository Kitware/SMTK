//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "smtk/attribute/DoubleItemDefinition.h"
#include "smtk/attribute/DoubleItem.h"
using namespace smtk::attribute;

//----------------------------------------------------------------------------
DoubleItemDefinition::DoubleItemDefinition(const std::string &myName):
  ValueItemDefinitionTemplate<double>(myName)
{
}

//----------------------------------------------------------------------------
DoubleItemDefinition::~DoubleItemDefinition()
{
}
//----------------------------------------------------------------------------
Item::Type DoubleItemDefinition::type() const
{
  return Item::DOUBLE;
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
DoubleItemDefinition::buildItem(Attribute *owningAttribute,
                                int itemPosition) const
{
  return smtk::attribute::ItemPtr(new DoubleItem(owningAttribute,
                                                itemPosition));
}
//----------------------------------------------------------------------------
smtk::attribute::ItemPtr
DoubleItemDefinition::buildItem(Item *owningItem,
                                int itemPosition,
                                int subGroupPosition) const
{
  return smtk::attribute::ItemPtr(new DoubleItem(owningItem,
                                                itemPosition,
                                                subGroupPosition));
}
//----------------------------------------------------------------------------
smtk::attribute::ItemDefinitionPtr
smtk::attribute::DoubleItemDefinition::
createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const
{
  smtk::attribute::DoubleItemDefinitionPtr newDef =
    smtk::attribute::DoubleItemDefinition::New(this->name());

  ValueItemDefinitionTemplate<double>::copyTo(newDef, info);
  return newDef;
}
//----------------------------------------------------------------------------
