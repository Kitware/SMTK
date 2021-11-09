//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_attribute_ResourceItemDefinition_h
#define smtk_attribute_ResourceItemDefinition_h

#include "smtk/attribute/ReferenceItemDefinition.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace attribute
{

/**\brief A definition for attribute items that store Resources as values.
  */
class SMTKCORE_EXPORT ResourceItemDefinition : public ReferenceItemDefinition
{
public:
  smtkTypeMacro(smtk::attribute::ResourceItemDefinition);
  smtkSuperclassMacro(ReferenceItemDefinition);

  using ReferenceItemDefinition::setAcceptsEntries;

  static smtk::attribute::ResourceItemDefinitionPtr New(const std::string& sname)
  {
    return smtk::attribute::ResourceItemDefinitionPtr(new ResourceItemDefinition(sname));
  }

  ~ResourceItemDefinition() override;

  Item::Type type() const override;

  bool isValueValid(smtk::resource::ConstPersistentObjectPtr value) const override;

  /// Construct an item from the definition given its owning attribute and position.
  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  /// Construct an item from the definition given its owning item and position.
  smtk::attribute::ItemPtr buildItem(Item* owningItem, int position, int subGroupPosition)
    const override;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

  bool setAcceptsEntries(const std::string& typeName, bool accept)
  {
    return ReferenceItemDefinition::setAcceptsEntries(typeName, "", accept);
  }

  using ReferenceItemDefinition::setRejectsEntries;
  bool setRejectsEntries(const std::string& typeName, bool add)
  {
    return ReferenceItemDefinition::setRejectsEntries(typeName, "", add);
  }

protected:
  ResourceItemDefinition(const std::string& myName);

private:
  void setOnlyResources(bool choice) final { ReferenceItemDefinition::setOnlyResources(choice); }
};

} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_ResourceItemDefinition_h */
