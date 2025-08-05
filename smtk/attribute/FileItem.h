//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME FileItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_FileItem_h
#define smtk_attribute_FileItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/FileSystemItem.h"
#include <string>
#include <vector>

namespace smtk
{
namespace attribute
{
class FileItemDefinition;
class SMTKCORE_EXPORT FileItem : public FileSystemItem
{
  friend class FileItemDefinition;

public:
  smtkTypeMacro(smtk::attribute::FileItem);
  smtkSuperclassMacro(smtk::attribute::FileSystemItem);
  ~FileItem() override;

  Item::Type type() const override;

  const std::vector<std::string>& recentValues() const { return m_recentValues; }
  void addRecentValue(const std::string& val);

protected:
  FileItem(Attribute* owningAttribute, int itemPosition);
  FileItem(Item* owningItem, int position, int subGroupPosition);
  bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef) override;

  std::vector<std::string> m_recentValues;

private:
};

} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_FileItem_h */
