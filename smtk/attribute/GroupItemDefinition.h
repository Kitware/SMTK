//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

/// \file GroupItemDefinition.h
/// \brief Documents the GroupItemDefintion class.

#ifndef smtk_attribute_GroupItemDefinition_h
#define smtk_attribute_GroupItemDefinition_h

#include "smtk/attribute/ItemDefinition.h"
#include <map>
#include <string>
#include <vector>

namespace smtk
{
namespace attribute
{
class GroupItem;
/// \brief A GroupItemDefinition represents a collection of Item Definitions.
///
/// When instanced the resulting Group Item will contain zero or more
/// groups of Items instanced from this collection of Item Definitions.

class SMTKCORE_EXPORT GroupItemDefinition : public ItemDefinition
{
public:
  smtkTypeMacro(smtk::attribute::GroupItemDefinition);
  static smtk::attribute::GroupItemDefinitionPtr New(const std::string& myName)
  {
    return smtk::attribute::GroupItemDefinitionPtr(new GroupItemDefinition(myName));
  }

  ~GroupItemDefinition() override;
  Item::Type type() const override;
  std::size_t numberOfItemDefinitions() const { return m_itemDefs.size(); }
  smtk::attribute::ItemDefinitionPtr itemDefinition(int ith) const
  {
    return (ith < 0) ? smtk::attribute::ItemDefinitionPtr()
                     : (static_cast<unsigned int>(ith) >= m_itemDefs.size()
                          ? smtk::attribute::ItemDefinitionPtr()
                          : m_itemDefs[static_cast<std::size_t>(ith)]);
  }
  bool addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef);
  template<typename T>
  typename smtk::internal::shared_ptr_type<T>::SharedPointerType addItemDefinition(
    const std::string& inName)
  {
    typedef smtk::internal::shared_ptr_type<T> SharedTypes;
    typename SharedTypes::SharedPointerType item;

    // First see if there is a item by the same name
    if (this->findItemPosition(inName) < 0)
    {
      std::size_t n = m_itemDefs.size();
      item = SharedTypes::RawPointerType::New(inName);
      if (m_isConditional)
      {
        item->setIsOptional(true);
      }
      // We need to get a pointer to the base Item class to set the unitsSystem
      static_cast<ItemDefinition*>(item.get())->setUnitsSystem(m_unitsSystem);
      m_itemDefs.push_back(item);
      m_itemDefPositions[inName] = static_cast<int>(n);
    }
    return item;
  }

  /// \brief This method will only remove the specified ItemDefinition (if it exists)
  /// from the class internals.
  ///
  /// Warning:
  /// It is up to the caller to ensure integrity of the attribute::Resource
  /// instance (e.g. Attribute instances/ Items created using this ItemDefinition
  /// need to be cleansed from the resource).
  bool removeItemDefinition(ItemDefinitionPtr itemDef);

  int findItemPosition(const std::string& name) const;

  ///@{
  /// \brief Returns or sets the def's extensibility property.
  ///
  /// If true then items from this def
  /// can have a variable number of groups.  The number of sub groups is always <= to number of
  /// required groups and max number of groups (provided max number of groups > 0)
  /// Default value is false.
  bool isExtensible() const { return m_isExtensible; }
  void setIsExtensible(bool mode);
  ///@}

  std::size_t numberOfRequiredGroups() const { return m_numberOfRequiredGroups; }

  ///  \brief Returns false if gsize is greater than max number of groups (and max number > 0)
  bool setNumberOfRequiredGroups(std::size_t gsize);

  ///@{
  /// \brief Returns or sets the definition's conditional property
  ///
  /// If the Conditional Property is true, then the Group Item represents a collection
  /// of choices.  All of the Group's Item Definitions will automatically be marked optional.
  /// in additional, the resulting GroupItem will use its minNumberOfChoices and maxNumberOfChoices
  /// properties to determine its validity.  Default is false.
  void setIsConditional(bool value) { m_isConditional = value; }
  bool isConditional() const { return m_isConditional; }
  ///@}

  ///@{
  /// \brief Returns or sets the default minimum number of choices that must be set for
  /// GroupItems, whose Conditional property is true, to be considered valid.  If set to 0
  /// (default) then there is no minimum number.
  void setMinNumberOfChoices(unsigned int value) { m_minNumberOfChoices = value; }
  unsigned int minNumberOfChoices() const { return m_minNumberOfChoices; }
  ///@}

  ///@{
  /// \brief Returns or sets the default maximum number of choices that must be set for
  /// GroupItems, whose Conditional property is true, to be considered valid.  If set to 0
  /// (default) then there is no maximum number.
  void setMaxNumberOfChoices(unsigned int value) { m_maxNumberOfChoices = value; }
  unsigned int maxNumberOfChoices() const { return m_maxNumberOfChoices; }
  ///@}

  bool hasSubGroupLabels() const { return !m_labels.empty(); }

  /// \brief Returns or Sets the maximum number of groups that items from this def can have.
  ///
  /// If 0 is returned then there is no max limit.  Default value is 0
  /// Note that this is used only when the def is extensible
  std::size_t maxNumberOfGroups() const { return m_maxNumberOfGroups; }
  /// \brief  Returns false if the new max is less than the number of required groups
  /// and is not 0
  bool setMaxNumberOfGroups(std::size_t esize);

  void setSubGroupLabel(std::size_t element, const std::string& elabel);
  void setCommonSubGroupLabel(const std::string& elabel);
  bool usingCommonSubGroupLabel() const { return m_useCommonLabel; }
  std::string subGroupLabel(std::size_t element) const;

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(Item* owningItem, int position, int subGroupPosition)
    const override;
  void buildGroup(smtk::attribute::GroupItem* group, int subGroupPosition) const;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  GroupItemDefinition(const std::string& myname);
  void applyCategories(
    const smtk::common::Categories::Stack& inheritedFromParent,
    smtk::common::Categories& inheritedToParent) override;
  void applyAdvanceLevels(
    const unsigned int& readLevelFromParent,
    const unsigned int& writeLevelFromParent) override;
  void setUnitsSystem(const shared_ptr<units::System>& unitsSystem) override;
  std::vector<smtk::attribute::ItemDefinitionPtr> m_itemDefs;
  std::map<std::string, int> m_itemDefPositions;
  std::vector<std::string> m_labels;
  std::size_t m_numberOfRequiredGroups = 1;
  std::size_t m_maxNumberOfGroups = 0;
  bool m_isExtensible = false;
  bool m_useCommonLabel = false;
  bool m_isConditional = false;
  unsigned int m_maxNumberOfChoices = 0;
  unsigned int m_minNumberOfChoices = 0;

private:
};

inline int GroupItemDefinition::findItemPosition(const std::string& inName) const
{
  std::map<std::string, int>::const_iterator it;
  it = m_itemDefPositions.find(inName);
  if (it == m_itemDefPositions.end())
  {
    return -1; // named item doesn't exist
  }
  return it->second;
}
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_GroupItemDefinition_h */
