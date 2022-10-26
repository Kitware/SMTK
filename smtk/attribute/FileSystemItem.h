//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME FileSystemItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_FileSystemItem_h
#define smtk_attribute_FileSystemItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <cassert>
#include <string>
#include <vector>

namespace smtk
{
namespace attribute
{
class FileSystemItemDefinition;
class SMTKCORE_EXPORT FileSystemItem : public Item
{
  friend class FileItemDefinition;

public:
  typedef std::vector<std::string>::const_iterator const_iterator;

  smtkTypeMacro(smtk::attribute::FileSystemItem);
  ~FileSystemItem() override;
  Item::Type type() const override = 0;

  bool shouldBeRelative() const;
  bool shouldExist() const;
  std::size_t numberOfValues() const { return m_values.size(); }
  bool setNumberOfValues(std::size_t newSize);
  std::size_t numberOfRequiredValues() const;
  bool isExtensible() const;
  std::size_t maxNumberOfValues() const;
  std::string value(std::size_t element = 0) const { return m_values[element]; }
  bool setValue(const std::string& val) { return this->setValue(0, val); }
  bool setValue(std::size_t element, const std::string& val);
  bool appendValue(const std::string& val);
  bool removeValue(std::size_t element);
  void reset() override;
  virtual bool setToDefault(std::size_t elementIndex = 0);
  // Returns true if there is a default defined and the item is curently set to it
  virtual bool isUsingDefault(std::size_t elementIndex) const;
  // This method tests all of the values of the items w/r the default value
  virtual bool isUsingDefault() const;
  // Does this item have a default value?
  bool hasDefault() const;
  std::string defaultValue() const;
  virtual std::string valueAsString(const std::string& format = "") const
  {
    return this->valueAsString(0, format);
  }
  virtual std::string valueAsString(std::size_t element, const std::string& format = "") const;
  virtual bool isSet(std::size_t element = 0) const
  {
    return m_isSet.size() > element ? m_isSet[element] : false;
  }

  virtual void unset(std::size_t element = 0)
  {
    assert(m_isSet.size() > element);
    m_isSet[element] = false;
  }

  // Iterator-style access to values:
  const_iterator begin() const;
  const_iterator end() const;
  template<typename I>
  bool setValues(I vbegin, I vend, std::size_t offset = 0);
  template<typename I>
  bool appendValues(I vbegin, I vend);

  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occurred.  Does not currently support any options directly.
  using Item::assign;
  bool assign(
    const smtk::attribute::ConstItemPtr& sourceItem,
    const CopyAssignmentOptions& options,
    smtk::io::Logger& logger) override;

protected:
  FileSystemItem(Attribute* owningAttribute, int itemPosition);
  FileSystemItem(Item* owningItem, int position, int subGroupPosition);
  bool setDefinition(smtk::attribute::ConstItemDefinitionPtr vdef) override;
  bool isValidInternal(bool useCategories, const std::set<std::string>& categories) const override;
  std::vector<std::string> m_values;
  std::vector<bool> m_isSet;

private:
};

template<typename I>
bool FileSystemItem::setValues(I vbegin, I vend, std::size_t offset)
{
  bool ok = false;
  std::size_t num = vend - vbegin + offset;
  if (this->setNumberOfValues(num))
  {
    ok = true;
    std::size_t i = 0;
    for (I it = vbegin; it != vend; ++it, ++i)
    {
      if (!this->setValue(offset + i, *it))
      {
        ok = false;
        break;
      }
    }
  }
  // Enable or disable the item if it is optional.
  if (ok)
    this->setIsEnabled(num > 0);
  return ok;
}

template<typename I>
bool FileSystemItem::appendValues(I vbegin, I vend)
{
  return this->setValues(vbegin, vend, this->numberOfValues());
}

} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_FileSystemItem_h */
