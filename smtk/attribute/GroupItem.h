//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_GroupItem_h
#define __smtk_attribute_GroupItem_h

#include "smtk/CoreExports.h"
#include "smtk/attribute/Item.h"
#include <cassert>
#include <iostream>
#include <vector>
namespace smtk
{
namespace attribute
{
class GroupItemDefinition;

/**\brief A group is an array of structures in SMTK.
  *
  * Groups are arrays of structures.
  * The structure is defined by the children items in the group;
  * each child item in the group is repeated once for every element
  * of the array.
  *
  * The numberOfGroups() method returns the number of entries in the array,
  * while numberOfItemsPerGroup() returns the number of children items.
  * Groups may have other groups as children.
  *
  * As an example, consider a group g with 2 child items: a string item
  * named "key" and a double item named "value".
  * Calling setNumberOfGroups(5) will cause 5 key-value pairs to be allocated.
  * You may then set values by like so:
  *
  * ```
  *   const char* keyNames[];
  *   for (std::size_t ii = 0; ii < 5; ++ii)
  *   {
  *     g->findAs<StringItem>(ii, "key")->setValue(keyNames[ii]);
  *     g->findAs<DoubleItem>(ii, "value")->setValue(0.5 * ii);
  *   }
  */
class SMTKCORE_EXPORT GroupItem : public Item
{
  friend class GroupItemDefinition;

public:
  typedef std::vector<std::vector<smtk::attribute::ItemPtr> >::const_iterator const_iterator;

  smtkTypeMacro(smtk::attribute::GroupItem);
  ~GroupItem() override;
  Item::Type type() const override;
  bool isValid() const override;
  std::size_t numberOfRequiredGroups() const;
  std::size_t maxNumberOfGroups() const;
  /**
   * @brief visitChildren Invoke a function on each (or, if \a findInActiveChildren
   * is true, each active) child item. If a subclass presents children items(ValueItem,
   * Group, RefItem, ...) then this function should be overriden.
   * @param visitor a lambda function which would be applied on children items
   * @param activeChildren a flag indicating whether it should be applied to active children only or not
   */
  void visitChildren(std::function<void(smtk::attribute::ItemPtr, bool)> visitor,
    bool activeChildren = true) override;

  bool isExtensible() const;

  std::size_t numberOfGroups() const { return m_items.size(); }
  bool setNumberOfGroups(std::size_t newSize);
  std::size_t numberOfItemsPerGroup() const;
  bool appendGroup();
  bool removeGroup(std::size_t element);

  /// Return the i-th item in the first entry of the group.
  smtk::attribute::ItemPtr item(std::size_t ith) const { return this->item(0, ith); }
  /**\brief Return the \a i-th item in for the \a element-th value of the group.
    *
    * If a group has M required values and each value consists of N items,
    * then \a element must be in [0,M - 1] and \a ith in [0, N - 1].
    *
    * Note that numberOfGroups() returns M and numberOfItemsPerGroup() returns N.
    */
  smtk::attribute::ItemPtr item(std::size_t element, std::size_t ith) const
  {
    assert(m_items.size() > element);
    assert(m_items[element].size() > ith);
    return m_items[element][ith];
  }

  smtk::attribute::ItemPtr find(const std::string& inName) { return this->find(0, inName); }
  smtk::attribute::ItemPtr find(std::size_t element, const std::string& name);
  smtk::attribute::ConstItemPtr find(const std::string& inName) const
  {
    return this->find(0, inName);
  }
  smtk::attribute::ConstItemPtr find(std::size_t element, const std::string& name) const;

  template <typename T>
  typename T::Ptr findAs(std::size_t element, const std::string& name);

  template <typename T>
  typename T::ConstPtr findAs(std::size_t element, const std::string& name) const;

  template <typename T>
  typename T::Ptr findAs(const std::string& name);

  template <typename T>
  typename T::ConstPtr findAs(const std::string& name) const;

  void reset() override;

  // Iterator-style access to values:
  const_iterator begin() const;
  const_iterator end() const;

  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occured - options are use when copying sub-items.
  // See Items.h for a description of these options.
  bool assign(smtk::attribute::ConstItemPtr& sourceItem, unsigned int options = 0) override;

protected:
  GroupItem(Attribute* owningAttribute, int itemPosition);
  GroupItem(Item* owningItem, int myPosition, int mySubGroupPosition);
  bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def) override;
  // This method will detach all of the items directly owned by
  // this group
  void detachAllItems();
  std::vector<std::vector<smtk::attribute::ItemPtr> > m_items;

private:
};

template <typename T>
typename T::Ptr GroupItem::findAs(std::size_t element, const std::string& iname)
{
  return smtk::dynamic_pointer_cast<T>(this->find(element, iname));
}

template <typename T>
typename T::ConstPtr GroupItem::findAs(std::size_t element, const std::string& iname) const
{
  return smtk::dynamic_pointer_cast<const T>(this->find(element, iname));
}

template <typename T>
typename T::Ptr GroupItem::findAs(const std::string& iname)
{
  return smtk::dynamic_pointer_cast<T>(this->find(iname));
}

template <typename T>
typename T::ConstPtr GroupItem::findAs(const std::string& iname) const
{
  return smtk::dynamic_pointer_cast<const T>(this->find(iname));
}
} // namespace attribute
} // namespace smtk

#endif /* __GroupItem_h */
