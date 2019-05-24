//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Item.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_Item_h
#define __smtk_attribute_Item_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/attribute/SearchStyle.h"
#include <map>
#include <queue>
#include <string>
#include <vector>

namespace smtk
{
namespace attribute
{
class ItemDefinition;
class GroupItem;
class GroupItemDefinition;
class ValueItemDefinition;
class Attribute;

class SMTKCORE_EXPORT Item : public smtk::enable_shared_from_this<Item>
{
  friend class Definition;
  friend class GroupItemDefinition;
  friend class ValueItemDefinition;

public:
  smtkTypeMacroBase(smtk::attribute::Item);
  enum Type
  {
    AttributeRefType,
    DoubleType,
    GroupType,
    IntType,
    StringType,
    VoidType,
    FileType,
    DirectoryType,
    ColorType,
    ModelEntityType,
    MeshSelectionType,
    MeshEntityType,
    DateTimeType,
    ReferenceType,
    ResourceType,
    ComponentType,
    NUMBER_OF_TYPES
  };

  enum AssignmentOptions
  {
    IGNORE_EXPRESSIONS = 0x001,         //!< Don't assign source value item's expressions
    IGNORE_MODEL_ENTITIES = 0x002,      //!< Don't assign source model entity items
    IGNORE_ATTRIBUTE_REF_ITEMS = 0x004, //!< Don't assign source attribute reference items
    IGNORE_RESOURCE_COMPONENTS = 0x008, //!< Don't assign source component items
    COPY_MODEL_ASSOCIATIONS = 0x010     //!< If creating attributes, copy their model associations
  };

  virtual ~Item();
  std::string name() const;
  std::string label() const;
  virtual Item::Type type() const = 0;
  virtual bool isValid() const = 0;

  /// @{
  /// \brief return a child item that matches name and satisfies the SearchStyle
  smtk::attribute::ItemPtr find(const std::string& name, SearchStyle style = RECURSIVE_ACTIVE);
  smtk::attribute::ConstItemPtr find(
    const std::string& name, SearchStyle style = RECURSIVE_ACTIVE) const;

  template <typename T>
  typename T::Ptr findAs(const std::string& name, SearchStyle style = RECURSIVE_ACTIVE);

  template <typename T>
  typename T::ConstPtr findAs(const std::string& name, SearchStyle style = RECURSIVE_ACTIVE) const;
  /// @}

  /**
   * @brief visitChildren Invoke a function on each (or, if \a findInActiveChildren
   * is true, each active) child item. If a subclass presents childern items(ValueItem,
   * Group, RefItem, ...) then this function should be overriden.
   * @param visitor a lambda function which would be applied on children items
   * @param activeChildren a flag indicating whether it should be applied to active children only or not
   */
  virtual void visitChildren(
    std::function<void(smtk::attribute::ItemPtr, bool)> /*visitor*/, bool /*activeChildren = true*/)
  {
  }
  smtk::attribute::ConstItemDefinitionPtr definition() const { return m_definition; }

  template <typename DefType>
  std::shared_ptr<const DefType> definitionAs() const
  {
    return std::dynamic_pointer_cast<const DefType>(this->definition());
  }

  // Return the attribute that owns this item
  smtk::attribute::AttributePtr attribute() const;
  smtk::attribute::ItemPtr owningItem() const
  {
    return (m_owningItem ? m_owningItem->shared_from_this() : smtk::attribute::ItemPtr());
  }
  //Position is the item's location w/r to the owning item if not null
  // or the owning attribute. Currently the only items that can own other items are
  // GroupItem and ValueItem (for expressions)
  int position() const { return m_position; }

  int subGroupPosition() const { return m_subGroupPosition; }

  // Returns true if the item is optional
  bool isOptional() const;

  // An item is enabled under the following coditions:
  // 1. If it is not owned by another item (such as a group), and either
  // it is not optional or it has been explicitly enabled
  // 2. If it's owning item is enabled and  either
  // it is not optional or it has been explicitly enabled
  bool isEnabled() const;
  void setIsEnabled(bool isEnabledValue) { m_isEnabled = isEnabledValue; }

  bool isMemberOf(const std::string& category) const;
  bool isMemberOf(const std::vector<std::string>& categories) const;

  //Get the item 's advance level:
  //if mode is 1 then the write access level is returned;
  //else the read access level is returned
  //NOTE: if the advance level was not explicitly set then the item's
  //definition's advance level is returned
  int advanceLevel(int mode = 0) const;
  void setAdvanceLevel(int mode, int level);
  // unsetAdvanceLevel causes the item to return its
  // definition advance level information for the specified mode when calling
  // the advanceLevel(mode) method
  void unsetAdvanceLevel(int mode = 0);
  // Returns true if the item is returning its Definition's
  // advance level information
  bool usingDefinitionAdvanceLevel(int mode = 0) const
  {
    return (mode == 1 ? m_usingDefAdvanceLevelInfo[1] : m_usingDefAdvanceLevelInfo[0]);
  }

  void setUserData(const std::string& key, smtk::simulation::UserDataPtr value)
  {
    m_userData[key] = value;
  }
  smtk::simulation::UserDataPtr userData(const std::string& key) const;
  void clearUserData(const std::string& key) { m_userData.erase(key); }
  void clearAllUserData() { m_userData.clear(); }

  virtual void reset();

  //This should be used only by attributes
  void detachOwningAttribute() { m_attribute = NULL; }
  //This should only be called by the item that owns
  // this one
  void detachOwningItem() { m_owningItem = NULL; }

  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occured
  virtual bool assign(smtk::attribute::ConstItemPtr& sourceItem, unsigned int options = 0);

  static std::string type2String(Item::Type t);
  static Item::Type string2Type(const std::string& s);

protected:
  Item(Attribute* owningAttribute, int itemPosition);
  Item(Item* owningItem, int myPosition, int mySubGroupPOsition);
  virtual bool setDefinition(smtk::attribute::ConstItemDefinitionPtr def);
  /// \brief Internal implementation of the find method
  virtual smtk::attribute::ItemPtr findInternal(const std::string& name, SearchStyle style);
  virtual smtk::attribute::ConstItemPtr findInternal(
    const std::string& name, SearchStyle style) const;

  Attribute* m_attribute;
  Item* m_owningItem;
  int m_position;
  int m_subGroupPosition;
  bool m_isEnabled;
  mutable std::string m_tempString;
  smtk::attribute::ConstItemDefinitionPtr m_definition;
  std::map<std::string, smtk::simulation::UserDataPtr> m_userData;

private:
  bool m_usingDefAdvanceLevelInfo[2];
  int m_advanceLevel[2];
};

inline smtk::simulation::UserDataPtr Item::userData(const std::string& key) const
{
  std::map<std::string, smtk::simulation::UserDataPtr>::const_iterator it = m_userData.find(key);
  return ((it == m_userData.end()) ? smtk::simulation::UserDataPtr() : it->second);
}

template <typename T>
typename T::Ptr Item::findAs(const std::string& iname, SearchStyle style)
{
  return smtk::dynamic_pointer_cast<T>(this->find(iname, style));
}

template <typename T>
typename T::ConstPtr Item::findAs(const std::string& iname, SearchStyle style) const
{
  return smtk::dynamic_pointer_cast<const T>(this->find(iname, style));
}
}
}

#endif /* __smtk_attribute_Item_h */
