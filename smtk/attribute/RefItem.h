//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME RefItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_RefItem_h
#define __smtk_attribute_RefItem_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h"
#include <cassert>
#include <vector>

namespace smtk
{
namespace attribute
{
class Attribute;
class RefItemDefinition;
class ValueItemDefinition;
class SMTKCORE_EXPORT RefItem : public Item
{
  friend class RefItemDefinition;
  friend class ValueItemDefinition;

public:
  typedef std::vector<attribute::WeakAttributePtr>::const_iterator const_iterator;

  smtkTypeMacro(RefItem);
  ~RefItem() override;
  Item::Type type() const override;
  // A RefItem is valid if it is either no enabled or if all of
  // its values are set and the attributes it references exist
  // It does NOT check to see if the attribute(s) it refers to are
  // valid - the reason for this is to avoid infinite loops if 2
  // attributes contain items that reference each other.
  bool isValid() const override;

  std::size_t numberOfValues() const { return m_values.size(); }
  bool setNumberOfValues(std::size_t newSize);
  std::size_t numberOfRequiredValues() const;
  smtk::attribute::AttributePtr value(std::size_t element = 0) const
  {
    assert(m_values.size() > element);
    return m_values[element].lock();
  }
  //  /**
  //   * @brief visitChildren Invoke a function on each (or, if \a findInActiveChildren
  //   * is true, each active) child item. If a subclass presents childern items(ValueItem,
  //   * Group, RefItem, ...) then this function should be overriden.
  //   * @param visitor a lambda function which would be applied on children items
  //   * @param activeChildren a flag indicating whether it should be applied to active children only or not
  //   */
  void visitChildren(std::function<void(smtk::attribute::ItemPtr, bool)> visitor,
    bool activeChildren = true) override;
  bool setValue(smtk::attribute::AttributePtr val) { return this->setValue(0, val); }
  bool setValue(std::size_t element, smtk::attribute::AttributePtr val);
  bool appendValue(smtk::attribute::AttributePtr val);
  bool removeValue(std::size_t element);
  void reset() override;
  virtual std::string valueAsString(const std::string& format = "") const
  {
    return this->valueAsString(0, format);
  }
  virtual std::string valueAsString(std::size_t element, const std::string& format = "") const;
  virtual bool isSet(std::size_t element = 0) const
  {
    assert(m_values.size() > element);
    return m_values[element].lock().get() != NULL;
  }
  virtual void unset(std::size_t element = 0);

  // Iterator-style access to values:
  const_iterator begin() const;
  const_iterator end() const;
  template <typename I>
  bool setValues(I vbegin, I vend, std::size_t offset = 0);
  template <typename I>
  bool appendValues(I vbegin, I vend);

  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occured.  By default, an attribute being referenced by this
  // item will also be copied if needed.  Use IGNORE_ATTRIBUTE_REF_ITEMS option to prevent this.
  // When the reference attribute is copied, its model associations are not copied by default.
  // Use COPY_MODEL_ASSOCIATIONS if you want them copied as well These options are defined in Item.h .
  bool assign(smtk::attribute::ConstItemPtr& sourceItem, unsigned int options = 0) override;

protected:
  RefItem(Attribute* owningAttribute, int itemPosition);
  RefItem(Item* owningItem, int myPosition, int mySubGroupPosition);
  bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def) override;
  void clearAllReferences();
  std::vector<attribute::WeakAttributePtr> m_values;

private:
};

template <typename I>
bool RefItem::setValues(I vbegin, I vend, std::size_t offset)
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
    this->setIsEnabled(num > 0 ? true : false);
  return ok;
}

template <typename I>
bool RefItem::appendValues(I vbegin, I vend)
{
  return this->setValues(vbegin, vend, this->numberOfValues());
}
}
}

#endif /* __smtk_attribute_RefItem_h */
