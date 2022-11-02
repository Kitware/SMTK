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

#ifndef smtk_attribute_FileItemDefinition_h
#define smtk_attribute_FileItemDefinition_h

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
  smtkTypeMacro(smtk::attribute::FileItemDefinition);
  static smtk::attribute::FileItemDefinitionPtr New(const std::string& myName)
  {
    return smtk::attribute::FileItemDefinitionPtr(new FileItemDefinition(myName));
  }

  ~FileItemDefinition() override;

  Item::Type type() const override;
  bool isValueValid(const std::string& val) const override;
  /// return the index of the filter that accepts val, or -1 if the value is
  /// invalid
  int filterId(const std::string& val) const;

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(Item* owningItem, int position, int subGroupPosition)
    const override;

  //@{
  /// A string describing file filters in the Qt format. For example:
  /// "Ext1 (*.ex1);;Ext2 or 3 (*.ex2 *.ex3);;All (*.*)"
  const std::string& getFileFilters() const { return m_fileFilters; }
  void setFileFilters(const std::string& filters) { m_fileFilters = filters; }
  //@}

  //@{
  /// The same as getFileFilters() but with an "All supported types" as the first entry.
  ///
  /// This is useful on many platforms as otherwise users must select a file type
  /// before being shown available files of that type.
  ///
  /// Example: If the file filters are set to `Ext1 (*.ex1);;Ext2 (*.ex2)`, then this
  /// method returns `All supported types (*.ex1 *.ex2);;Ext1 (*.ex1);;Ext2( *.ex2)`.
  ///
  /// If getFileFilters() returns an empty string, then so does this method.
  std::string getSummarizedFileFilters() const;
  //@}

  //@{
  /// Combine individual file filters into a single filter entry. For example:
  /// "Ext1 (*.ex1);;Ext2 or 3 (*.ex2 *.ex3)" -> "(*.ex1 *.ex2 *.ex3)"
  /// "Ext1 (*.ex1);;Ext2 or 3 (*.ex2 *.ex3);;All (*.*)" -> "(*.*)"
  ///
  /// The variant which accepts a reference to an integer returns the number
  /// of file extensions found (or 0 if `*.*` is present).
  static std::string aggregateFileFilters(const std::string&);
  static std::string aggregateFileFilters(const std::string&, int&);
  //@}

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  FileItemDefinition(const std::string& myName);

  std::string m_fileFilters;

private:
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_FileItemDefinition_h */
