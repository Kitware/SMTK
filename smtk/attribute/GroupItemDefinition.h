//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME GroupItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_GroupItemDefinition_h
#define __smtk_attribute_GroupItemDefinition_h

#include "smtk/attribute/ItemDefinition.h"
#include <map>
#include <string>
#include <vector>

namespace smtk
{
namespace attribute
{
class GroupItem;
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
  template <typename T>
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
      m_itemDefs.push_back(item);
      m_itemDefPositions[inName] = static_cast<int>(n);
    }
    return item;
  }

  // Description:
  // This method will only remove the specified ItemDefinition (if it exists)
  // from the class internals.
  //
  // Warning:
  // It is up to the caller to ensure integrity of the attribute::Resource
  // instance (e.g. Attribute instances/ Items created using this ItemDefinition
  // need to be cleansed from the resource).
  bool removeItemDefinition(ItemDefinitionPtr itemDef);

  int findItemPosition(const std::string& name) const;

  // Returns or Sets the def's extensiblity property.  If true then items from this def
  // can have a variable number of groups.  The number of sub groups is always <= to number of
  // required groups and max number of groups (provided max number of groups > 0)
  // Default value is false.
  bool isExtensible() const { return m_isExtensible; }
  void setIsExtensible(bool mode);

  std::size_t numberOfRequiredGroups() const { return m_numberOfRequiredGroups; }

  // Returns false if gsize is greater than max number of groups (and max number > 0)
  bool setNumberOfRequiredGroups(std::size_t gsize);

  bool hasSubGroupLabels() const { return !m_labels.empty(); }

  // Returns or Sets the maximum number of groups that items from this def can have.
  // if 0 is returned then there is no max limit.  Default value is 0
  // Note that this is used only when the def is extensible
  std::size_t maxNumberOfGroups() const { return m_maxNumberOfGroups; }
  // Returns false if the new max is less than the number of required groups
  // and is not 0
  bool setMaxNumberOfGroups(std::size_t esize);

  void setSubGroupLabel(std::size_t element, const std::string& elabel);
  void setCommonSubGroupLabel(const std::string& elabel);
  bool usingCommonSubGroupLabel() const { return m_useCommonLabel; }
  std::string subGroupLabel(std::size_t element) const;

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const override;
  void buildGroup(smtk::attribute::GroupItem* group, int subGroupPosition) const;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  GroupItemDefinition(const std::string& myname);
  virtual void applyCategories(const smtk::attribute::Categories& inheritedFromParent,
    smtk::attribute::Categories& inheritedToParent) override;
  void applyAdvanceLevels(
    const unsigned int& readLevelFromParent, const unsigned int& writeLevelFromParent) override;
  std::vector<smtk::attribute::ItemDefinitionPtr> m_itemDefs;
  std::map<std::string, int> m_itemDefPositions;
  std::vector<std::string> m_labels;
  std::size_t m_numberOfRequiredGroups;
  std::size_t m_maxNumberOfGroups;
  bool m_isExtensible;
  bool m_useCommonLabel;

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
}
}

#endif /* __smtk_attribute_GroupItemDefinition_h */
