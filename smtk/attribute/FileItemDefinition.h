//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME FileItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_FileItemDefinition_h
#define __smtk_attribute_FileItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/FileSystemItemDefinition.h"

namespace smtk
{
namespace attribute
{
class Attribute;
class SMTKCORE_EXPORT FileItemDefinition : public FileSystemItemDefinition
{
public:
  smtkTypeMacro(FileItemDefinition);
  static smtk::attribute::FileItemDefinitionPtr New(const std::string& myName)
  {
    return smtk::attribute::FileItemDefinitionPtr(new FileItemDefinition(myName));
  }

  virtual ~FileItemDefinition();

  virtual Item::Type type() const;
  virtual bool isValueValid(const std::string& val) const;
  // return the index of the filter that accepts val, or -1 if the value is
  // invalid
  int filterId(const std::string& val) const;

  virtual smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const;
  virtual smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const;

  const std::string& getFileFilters() const { return this->m_fileFilters; }
  void setFileFilters(const std::string& filters) { this->m_fileFilters = filters; }

  virtual smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const;

protected:
  FileItemDefinition(const std::string& myName);

  // A string describing file filters in the Qt format. For example:
  // "Ext1 (*.ex1);;Ext2 or 3 (*.ex2 *.ex3);;All (*.*)"
  std::string m_fileFilters;

private:
};
}
}

#endif /* __smtk_attribute_FileItemDefinition_h */
