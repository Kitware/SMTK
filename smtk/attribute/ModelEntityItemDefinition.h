//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ModelEntityItemDefinition.h - A definition for attribute items that store UUIDs as values.
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_ModelEntityItemDefinition_h
#define __smtk_attribute_ModelEntityItemDefinition_h

#include "smtk/attribute/ComponentItemDefinition.h"
#include "smtk/common/UUID.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

namespace smtk
{
namespace attribute
{

class SMTKCORE_EXPORT ModelEntityItemDefinition : public ComponentItemDefinition
{
public:
  smtkTypeMacro(smtk::attribute::ModelEntityItemDefinition);
  smtkSuperclassMacro(ComponentItemDefinition);

  static smtk::attribute::ModelEntityItemDefinitionPtr New(const std::string& sname)
  {
    return smtk::attribute::ModelEntityItemDefinitionPtr(new ModelEntityItemDefinition(sname));
  }

  ~ModelEntityItemDefinition() override;

  Item::Type type() const override;

  smtk::model::BitFlags membershipMask() const;
  void setMembershipMask(smtk::model::BitFlags entMask);

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const override;

protected:
  ModelEntityItemDefinition(const std::string& myName);

private:
  bool setAcceptsEntries(
    const std::string& typeName, const std::string& queryString, bool accept) override;
};

} // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ModelEntityItemDefinition_h */
