//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_attribute_ResourceItemDefinition_h
#define __smtk_attribute_ResourceItemDefinition_h

#include "smtk/attribute/ReferenceItemDefinition.txx"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace attribute
{

/**\brief A definition for attribute items that store Resources as values.
  */
class SMTKCORE_EXPORT ResourceItemDefinition
  : public ReferenceItemDefinition<smtk::resource::Resource>
{
public:
  smtkTypeMacro(ResourceItemDefinition);
  smtkSuperclassMacro(ReferenceItemDefinition<smtk::resource::Resource>);

  static smtk::attribute::ResourceItemDefinitionPtr New(const std::string& sname)
  {
    return smtk::attribute::ResourceItemDefinitionPtr(new ResourceItemDefinition(sname));
  }

  ~ResourceItemDefinition() override;

  Item::Type type() const override;

  bool isValueValid(smtk::resource::ResourcePtr value) const override;

  /// Construct an item from the definition given its owning attribute and position.
  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  /// Construct an item from the definition given its owning item and position.
  smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const override;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  ResourceItemDefinition(const std::string& myName);
};

} // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ResourceItemDefinition_h */
