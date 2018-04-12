//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_attribute_ComponentItemDefinition_h
#define __smtk_attribute_ComponentItemDefinition_h

#include "smtk/attribute/ReferenceItemDefinition.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

namespace smtk
{
namespace attribute
{

/**\brief A definition for attribute items that store component UUIDs as values.
  */
class SMTKCORE_EXPORT ComponentItemDefinition : public ReferenceItemDefinition
{
public:
  smtkTypeMacro(smtk::attribute::ComponentItemDefinition);
  smtkSuperclassMacro(ReferenceItemDefinition);

  static smtk::attribute::ComponentItemDefinitionPtr New(const std::string& sname)
  {
    return smtk::attribute::ComponentItemDefinitionPtr(new ComponentItemDefinition(sname));
  }

  ~ComponentItemDefinition() override;

  Item::Type type() const override;

  bool isValueValid(smtk::resource::PersistentObjectPtr value) const override;

  /// Construct an item from the definition given its owning attribute and position.
  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  /// Construct an item from the definition given its owning item and position.
  smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const override;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  ComponentItemDefinition(const std::string& myName);
};

} // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ComponentItemDefinition_h */
