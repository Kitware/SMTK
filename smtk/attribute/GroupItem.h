//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_GroupItem_h
#define smtk_attribute_GroupItem_h

#include "smtk/CoreExports.h"
#include "smtk/attribute/Item.h"
#include <cassert>
#include <vector>
namespace smtk
{
namespace attribute
{
class GroupItemDefinition;

///\brief A group item represents an array of structures in SMTK.
///
/// Groups are arrays of structures.
/// The structure is defined by the children items in the group;
/// each child item in the group is repeated once for every element
/// of the array.
///
/// The numberOfGroups() method returns the number of entries in the array,
/// while numberOfItemsPerGroup() returns the number of children items.
/// Groups may have other groups as children.
///
/// As an example, consider a group g with 2 child items: a string item
/// named "key" and a double item named "value".
/// Calling setNumberOfGroups(5) will cause 5 key-value pairs to be allocated.
/// You may then set values by like so:
///
/// ```
///  const char* keyNames[];
///   for (std::size_t ii = 0; ii < 5; ++ii)
///   {
///     g->findAs<StringItem>(ii, "key")->setValue(keyNames[ii]);
///     g->findAs<DoubleItem>(ii, "value")->setValue(0.5 * ii);
///   }
/// ```
///
class SMTKCORE_EXPORT GroupItem : public Item
{
  friend class GroupItemDefinition;

public:
  typedef std::vector<std::vector<smtk::attribute::ItemPtr>>::const_iterator const_iterator;

  smtkTypeMacro(smtk::attribute::GroupItem);
  ~GroupItem() override;
  Item::Type type() const override;

  std::size_t numberOfRequiredGroups() const;
  std::size_t maxNumberOfGroups() const;
  ///
  /// @brief visitChildren Invoke a function on each (or, if \a findInActiveChildren
  /// is true, each active) child item. If a subclass presents children items(ValueItem,
  /// Group, ...) then this function should be overriden.
  /// @param visitor a lambda function which would be applied on children items
  /// @param activeChildren a flag indicating whether it should be applied to active children only or not
  ///
  void visitChildren(
    std::function<void(smtk::attribute::ItemPtr, bool)> visitor,
    bool activeChildren = true) override;

  bool isExtensible() const;

  std::size_t numberOfGroups() const { return m_items.size(); }
  bool setNumberOfGroups(std::size_t newSize);
  std::size_t numberOfItemsPerGroup() const;
  bool appendGroup();
  bool prependGroup();
  ///\brief Insert num groups before index pos - so append is pos = numberOfGroups
  /// and prepend would be pos = 0
  bool insertGroups(std::size_t pos, std::size_t num);
  bool removeGroup(std::size_t element);

  /// Return the i-th item in the first entry of the group.
  smtk::attribute::ItemPtr item(std::size_t ith) const { return this->item(0, ith); }
  ///\brief Return the \a i-th item in for the \a element-th value of the group.
  ///
  /// If a group has M required values and each value consists of N items,
  /// then \a element must be in [0,M - 1] and \a ith in [0, N - 1].
  ///
  /// Note that numberOfGroups() returns M and numberOfItemsPerGroup() returns N.
  ///
  smtk::attribute::ItemPtr item(std::size_t element, std::size_t ith) const
  {
    assert(m_items.size() > element);
    assert(m_items[element].size() > ith);
    return m_items[element][ith];
  }

  smtk::attribute::ItemPtr
  find(std::size_t element, const std::string& name, SearchStyle style = IMMEDIATE);
  smtk::attribute::ConstItemPtr
  find(std::size_t element, const std::string& name, SearchStyle style = IMMEDIATE) const;
  using Item::find;

  template<typename T>
  typename T::Ptr
  findAs(std::size_t element, const std::string& name, SearchStyle style = IMMEDIATE);
  template<typename T>
  typename T::ConstPtr
  findAs(std::size_t element, const std::string& name, SearchStyle style = IMMEDIATE) const;
  using Item::findAs;

  /// \brief Release the item's dependency on its parent attribute's Resource.
  void detachOwningResource() override;

  void reset() override;

  ///\brief Rotate the order of subgroups between specified positions.
  ///
  /// The subgroup at fromPosition is moved to toPosition, and the subgroups
  /// in between are shifted one position.
  /// The return value is true if the rotation was applied, which is when both
  /// position arguments are valid with respect to the underlying data/range.
  ///
  bool rotate(std::size_t fromPosition, std::size_t toPosition) override;

  /// \brief Returns the item's conditional property
  ///
  /// If the Conditional Property is true, then the Group Item represents a collection
  /// of choices. the GroupItem will use its minNumberOfChoices and maxNumberOfChoices
  /// properties to determine its validity.  Default is false.
  bool isConditional() const;

  /// \brief Returns true if the GroupItem satisfies its conditional requirements.
  ///
  /// Requirements are met if the item is not conditional, or if it have the appropriate
  /// number of enabled items that are relevant.  If useActiveCategories is true, then
  /// category checking using the resource's active
  /// categories is performed, else no category checking is done.
  bool conditionalsSatisfied(bool useActiveCategories = true) const;

  ///@{
  /// \brief Returns or sets the minimum number of choices that must be set for the
  /// GroupItem, whose Conditional property is true, to be considered valid.  If set to 0
  /// then there is no minimum number.  This value is initialized by the item's definition.
  void setMinNumberOfChoices(unsigned int value) { m_minNumberOfChoices = value; }
  unsigned int minNumberOfChoices() const { return m_minNumberOfChoices; }
  ///@}

  ///@{
  /// \brief Returns or sets the maximum number of choices that must be set for
  /// GroupItem, whose Conditional property is true, to be considered valid.  If set to 0
  /// then there is no maximum number. This value is initialized by the item's definition.
  void setMaxNumberOfChoices(unsigned int value) { m_maxNumberOfChoices = value; }
  unsigned int maxNumberOfChoices() const { return m_maxNumberOfChoices; }
  ///@}

  ///@{
  /// \brief Iterator-style access to values:
  const_iterator begin() const;
  const_iterator end() const;
  ///@}

  using Item::assign;
  /// \brief Assigns this item to be equivalent to another.
  ///
  /// Options are processed by derived item classes
  /// Returns true if success and false if a problem occurred - options are use when copying sub-items.
  /// See CopyAssigmentOptions.h for a description of these options.
  Item::Status assign(
    const smtk::attribute::ConstItemPtr& sourceItem,
    const CopyAssignmentOptions& options,
    smtk::io::Logger& logger) override;

  ///\brief Returns true if the group item has relevant children.
  bool hasRelevantChildren(
    bool includeCategories = true,
    bool includeReadAccess = false,
    int readAccessLevel = 0) const;

protected:
  GroupItem(Attribute* owningAttribute, int itemPosition);
  GroupItem(Item* owningItem, int myPosition, int mySubGroupPosition);
  bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def) override;
  /// \brief Internal implementation of the find method
  smtk::attribute::ItemPtr findInternal(const std::string& name, SearchStyle style) override;
  smtk::attribute::ConstItemPtr findInternal(const std::string& name, SearchStyle style)
    const override;
  // This method will detach all of the items directly owned by
  // this group
  void detachAllItems();
  bool isValidInternal(bool useCategories, const std::set<std::string>& categories) const override;
  std::vector<std::vector<smtk::attribute::ItemPtr>> m_items;
  unsigned int m_maxNumberOfChoices;
  unsigned int m_minNumberOfChoices;

private:
};

template<typename T>
typename T::Ptr GroupItem::findAs(std::size_t element, const std::string& iname, SearchStyle style)
{
  return smtk::dynamic_pointer_cast<T>(this->find(element, iname, style));
}

template<typename T>
typename T::ConstPtr
GroupItem::findAs(std::size_t element, const std::string& iname, SearchStyle style) const
{
  return smtk::dynamic_pointer_cast<const T>(this->find(element, iname, style));
}

} // namespace attribute
} // namespace smtk

#endif /* __GroupItem_h */
