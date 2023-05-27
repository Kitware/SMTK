//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME FileSystemItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_FileSystemItemDefinition_h
#define smtk_attribute_FileSystemItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ItemDefinition.h"

namespace smtk
{
namespace attribute
{
class Attribute;
class SMTKCORE_EXPORT FileSystemItemDefinition : public ItemDefinition
{
public:
  smtkTypeMacro(smtk::attribute::FileSystemItemDefinition);

  ~FileSystemItemDefinition() override;

  Item::Type type() const override;
  virtual bool isValueValid(const std::string& val) const;

  // Returns or Sets the def's extensiblity property.  If true then items from this def
  // can have a variable number of items.  The number of items is always <= to number of
  // required items and max number of items (provided max number of items > 0)
  // Default value is false.
  bool isExtensible() const { return m_isExtensible; }
  void setIsExtensible(bool mode);

  std::size_t numberOfRequiredValues() const { return m_numberOfRequiredValues; }
  bool setNumberOfRequiredValues(std::size_t esize);

  // Returns or Sets the maximum number of items that items from this def can have.
  // if 0 is returned then there is no max limit.  Default value is 0
  // Note that this is used only when the def is extensible
  std::size_t maxNumberOfValues() const { return m_maxNumberOfValues; }
  // Returns false if the new max is less than the number of required groups
  // and is not 0
  bool setMaxNumberOfValues(std::size_t esize);

  bool hasValueLabels() const { return !m_valueLabels.empty(); }

  void setValueLabel(std::size_t element, const std::string& elabel);
  void setCommonValueLabel(const std::string& elabel);
  bool usingCommonLabel() const { return m_useCommonLabel; }
  std::string valueLabel(std::size_t element) const;
  bool shouldExist() const { return m_shouldExist; }
  void setShouldExist(bool val) { m_shouldExist = val; }
  bool shouldBeRelative() const { return m_shouldBeRelative; }
  void setShouldBeRelative(bool val) { m_shouldBeRelative = val; }
  std::string defaultValue() const { return m_defaultValue; }
  void setDefaultValue(const std::string& val);
  void unsetDefaultValue() { m_hasDefault = false; }
  bool hasDefault() const { return m_hasDefault; }
  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override =
    0;
  smtk::attribute::ItemPtr buildItem(Item* owningItem, int position, int subGroupPosition)
    const override = 0;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override = 0;

protected:
  FileSystemItemDefinition(const std::string& myName);
  bool m_shouldExist = false;
  bool m_shouldBeRelative = false;
  bool m_useCommonLabel = false;
  bool m_isExtensible = false;
  bool m_hasDefault = false;
  std::string m_defaultValue;
  std::vector<std::string> m_valueLabels;
  std::size_t m_numberOfRequiredValues = 1;
  std::size_t m_maxNumberOfValues = 0;

private:
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_FileSystemItemDefinition_h */
