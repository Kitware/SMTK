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

#ifndef __smtk_attribute_FileItem_h
#define __smtk_attribute_FileItem_h

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
  smtkTypeMacro(FileItem);
  virtual ~FileItem();

  Item::Type type() const;

  const std::vector<std::string>& recentValues() const { return this->m_recentValues; }
  void addRecentValue(const std::string& val);

protected:
  FileItem(Attribute* owningAttribute, int itemPosition);
  FileItem(Item* owningItem, int position, int subGroupPosition);
  virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef);

  std::vector<std::string> m_recentValues;

private:
};

} // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_FileItem_h */
