//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DirectoryItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_DirectoryItemDefinition_h
#define smtk_attribute_DirectoryItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/FileSystemItemDefinition.h"

namespace smtk
{
namespace attribute
{
class SMTKCORE_EXPORT DirectoryItemDefinition : public FileSystemItemDefinition
{
public:
  smtkTypeMacro(smtk::attribute::DirectoryItemDefinition);
  static smtk::attribute::DirectoryItemDefinitionPtr New(const std::string& myName)
  {
    return smtk::attribute::DirectoryItemDefinitionPtr(new DirectoryItemDefinition(myName));
  }

  ~DirectoryItemDefinition() override;

  Item::Type type() const override;

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(Item* owningItem, int position, int subGroupPosition)
    const override;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  DirectoryItemDefinition(const std::string& myName);

private:
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_DirectoryItemDefinition_h */
