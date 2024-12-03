//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkAttribute.h - Represents a standalone piece of simulation information
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_Attribute_h
#define smtk_attribute_Attribute_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/resource/Component.h"

#include "smtk/attribute/CopyAssignmentOptions.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/SearchStyle.h"
#include "smtk/attribute/ValueItem.h"

#include "smtk/common/Deprecation.h"
#include "smtk/common/UUID.h" // for template associatedModelEntities()

#include <map>
#include <set>
#include <string>
#include <vector>

namespace smtk
{
namespace io
{
class Logger;
};

namespace attribute
{
class Evaluator;
class Item;
class Resource;

/**\brief Represent a (possibly composite) value according to a definition.
      *
      */
class SMTKCORE_EXPORT Attribute : public resource::Component
{
  friend class smtk::attribute::Definition;
  friend class smtk::attribute::Resource;

public:
  smtkTypeMacro(smtk::attribute::Attribute);
  smtkSuperclassMacro(smtk::resource::Component);

  struct SMTKCORE_EXPORT CompareByName
  {
    bool operator()(
      const smtk::attribute::AttributePtr& lhs,
      const smtk::attribute::AttributePtr& rhs) const
    {
      return lhs->name() < rhs->name();
    }

    bool operator()(
      const smtk::attribute::WeakAttributePtr& lhs,
      const smtk::attribute::WeakAttributePtr& rhs) const
    {
      auto left = lhs.lock();
      if (left == nullptr)
      {
        return true;
      }
      auto right = rhs.lock();
      if (right == nullptr)
      {
        return false;
      }
      return left->name() < right->name();
    }
  };

  static smtk::attribute::AttributePtr New(
    const std::string& myName,
    const smtk::attribute::DefinitionPtr& myDefinition,
    const smtk::common::UUID& myId)
  {
    return smtk::attribute::AttributePtr(new Attribute(myName, myDefinition, myId));
  }

  ~Attribute() override;

  AttributePtr shared_from_this()
  {
    return static_pointer_cast<Attribute>(Component::shared_from_this());
  }

  std::shared_ptr<const Attribute> shared_from_this() const
  {
    return static_pointer_cast<const Attribute>(Component::shared_from_this());
  }

  // NOTE: To rename an attribute use the resource!
  std::string name() const override { return m_name; }

  const std::string& type() const;
  std::vector<std::string> types() const;
  bool isA(const smtk::attribute::DefinitionPtr& def) const;
  const smtk::attribute::DefinitionPtr& definition() const { return m_definition; }

  const double* color() const;
  void setColor(double r, double g, double b, double alpha);
  void setColor(const double* l_color)
  {
    this->setColor(l_color[0], l_color[1], l_color[2], l_color[3]);
  }
  bool isColorSet() const { return m_isColorSet; }
  void unsetColor() { m_isColorSet = false; }

  /// \brief Get the Attribute 's advance level
  ///
  /// if mode is 1 then the write access level is returned;
  /// else the read access level is returned
  /// The information can either be specificied directly to the attribute
  /// using setLocalAdvanceLevel() or from the attributes's definition.
  /// NOTE: This information is used in GUI only
  unsigned int advanceLevel(int mode = 0) const;
  void setLocalAdvanceLevel(int mode, unsigned int level);
  unsigned int localAdvanceLevel(int mode = 0) const
  {
    return (mode == 1 ? m_localAdvanceLevel[1] : m_localAdvanceLevel[0]);
  }
  // unsetLocalAdvanceLevel causes the attribute to return its
  // definition advance level information for the specified mode when calling
  // the advanceLevel(mode) method
  void unsetLocalAdvanceLevel(int mode = 0);
  // Returns true if the attribute is returning its local
  // advance level information
  bool hasLocalAdvanceLevelInfo(int mode = 0) const
  {
    return (mode == 1 ? m_hasLocalAdvanceLevelInfo[1] : m_hasLocalAdvanceLevelInfo[0]);
  }

  const std::vector<smtk::attribute::ItemPtr>& items() const { return m_items; }
  smtk::attribute::ItemPtr item(int ith) const
  {
    return (ith < 0)
      ? smtk::attribute::ItemPtr()
      : (static_cast<unsigned int>(ith) >= m_items.size() ? smtk::attribute::ItemPtr()
                                                          : m_items[static_cast<std::size_t>(ith)]);
  }

  /// @{
  /// \brief Find an item via its path with respects to the attribute.
  ///
  /// If activeOnly is true then all items in the path must active (with respect to a parent ValueItem)
  /// else nullptr will be returned.  This parameter will have no effect on other types of items.
  /// Note that the path should not start with a separator character.
  ///
  /// Note that itemAtPath() can also be used to reference an attribute's associations (if any
  /// are allowed) as long as the association-definition's name does not collide with any immediate
  /// child-item's name.
  smtk::attribute::ConstItemPtr
  itemAtPath(const std::string& path, const std::string& sep = "/", bool activeOnly = false) const;
  smtk::attribute::ItemPtr
  itemAtPath(const std::string& path, const std::string& sep = "/", bool activeOnly = false);

  template<typename T>
  typename T::ConstPtr itemAtPathAs(
    const std::string& path,
    const std::string& sep = "/",
    bool activeOnly = false) const;
  template<typename T>
  typename T::Ptr
  itemAtPathAs(const std::string& path, const std::string& sep = "/", bool activeOnly = false);
  /// @}

  /// @{
  /// \brief Formats the full path to the item with respect to the attribute.
  SMTK_DEPRECATED_IN_24_11("Use Item::path() instead.")
  std::string itemPath(const ItemPtr& item, const std::string& sep = "/") const;
  /// @}

  smtk::attribute::ItemPtr find(const std::string& name, SearchStyle style = RECURSIVE_ACTIVE);
  smtk::attribute::ConstItemPtr find(const std::string& name, SearchStyle style = RECURSIVE_ACTIVE)
    const;
  std::size_t numberOfItems() const { return m_items.size(); }

  template<typename T>
  typename T::Ptr findAs(const std::string& name, SearchStyle style = RECURSIVE_ACTIVE);

  template<typename T>
  typename T::ConstPtr findAs(const std::string& name, SearchStyle style = RECURSIVE_ACTIVE) const;

  /**
   * @brief Given a container, filter items in the attribute by a lambda function
   * @param values a container which holds items
   * @param test a lambda function which would be applied on children items
   * Example filter double and int items
   *  [](Item::Ptr item) { return item->type() == DoubleType || item->type() == IntType; }
   * Example filter modelEntity items
   *  [](ModelEntity::Ptr item) { return true; }
   * @param activeChildren a flag indicates whether it should be applied to active children only or not
   */
  template<typename T>
  void filterItems(
    T& values,
    std::function<bool(typename T::value_type)> test,
    bool activeChildren = true);

  IntItemPtr findInt(const std::string& name);
  ConstIntItemPtr findInt(const std::string& name) const;

  DoubleItemPtr findDouble(const std::string& name);
  ConstDoubleItemPtr findDouble(const std::string& name) const;

  StringItemPtr findString(const std::string& name);
  ConstStringItemPtr findString(const std::string& name) const;

  FileItemPtr findFile(const std::string& name);
  ConstFileItemPtr findFile(const std::string& name) const;

  DirectoryItemPtr findDirectory(const std::string& name);
  ConstDirectoryItemPtr findDirectory(const std::string& name) const;

  GroupItemPtr findGroup(const std::string& name);
  ConstGroupItemPtr findGroup(const std::string& name) const;

  ModelEntityItemPtr findModelEntity(const std::string& name);
  ConstModelEntityItemPtr findModelEntity(const std::string& name) const;

  VoidItemPtr findVoid(const std::string& name);
  ConstVoidItemPtr findVoid(const std::string& name) const;

  DateTimeItemPtr findDateTime(const std::string& name);
  ConstDateTimeItemPtr findDateTime(const std::string& name) const;

  ReferenceItemPtr findReference(const std::string& name);
  ConstReferenceItemPtr findReference(const std::string& name) const;
  template<typename T>
  T entityRefsAs(const std::string& name) const;

  ResourceItemPtr findResource(const std::string& name);
  ConstResourceItemPtr findResource(const std::string& name) const;

  ComponentItemPtr findComponent(const std::string& name);
  ConstComponentItemPtr findComponent(const std::string& name) const;

  ConstReferenceItemPtr associatedObjects() const { return m_associatedObjects; }
  ReferenceItemPtr associatedObjects() { return m_associatedObjects; }

  bool isObjectAssociated(const smtk::common::UUID& uid) const;
  bool isObjectAssociated(const smtk::resource::PersistentObjectPtr& componentPtr) const;

  bool canBeAssociated(const smtk::resource::PersistentObjectPtr& obj) const;
  bool canBeDisassociated(const smtk::resource::PersistentObjectPtr& obj, AttributePtr& probAtt)
    const;
  ConstReferenceItemPtr associations() const { return m_associatedObjects; }
  ReferenceItemPtr associations() { return m_associatedObjects; }

  bool isEntityAssociated(const smtk::common::UUID& entity) const;
  bool isEntityAssociated(const smtk::model::EntityRef& entityref) const;

  smtk::common::UUIDs associatedModelEntityIds() const;
  template<typename T>
  T associatedModelEntities() const;

  template<typename T>
  T associatedObjects() const;

  bool associate(smtk::resource::PersistentObjectPtr obj);
  bool associateEntity(const smtk::common::UUID& entity);
  bool associateEntity(const smtk::model::EntityRef& entity);

  void disassociateEntity(const smtk::common::UUID& entity, bool reverse = true);
  void disassociateEntity(const smtk::model::EntityRef& entity, bool reverse = true);
  /// Disassociate attribute from an object.  Returns true if successful, else
  /// it will set probAtt to attribute using this as a prerequisite
  bool
  disassociate(smtk::resource::PersistentObjectPtr obj, AttributePtr& probAtt, bool reverse = true);
  /// Disassociate attribute from an object.  Returns true if successful.
  bool disassociate(smtk::resource::PersistentObjectPtr obj, bool reverse = true);

  bool assign(
    const AttributePtr& sourceAtt,
    const CopyAssignmentOptions& options = CopyAssignmentOptions());

  bool assign(
    const AttributePtr& sourceAtt,
    const CopyAssignmentOptions& options,
    smtk::io::Logger& logger);

  /**
  * @brief Sever dependencies between items and the owning resource.
  *
  * Some items implicitly interact with the parent resource to store and
  * retrieve data (e.g. ReferenceItem uses links to retrieve references). For
  * items with this type of relationship with the owning resource, this method
  * disables those routines, forcing the item (and, therefore, the containing
  * attribute) to satisfy its API without using the owning resource. This is
  * useful when an attribute has been removed from its parent resource and is
  * still held by ancillary routines (i.e. when the attribute is expunged and
  * Operation observers are inspecting expunged resources).
  */
  void detachItemsFromOwningResource();

  /**
  * @brief Remove all associations of this attribute with model entities.
  *
  * When dealing with prerequisite constraints it may not be possible to
  * remove all associations.  If partialRemovalOk is true, then all
  * associations that can be removed all.  If false, then associations are
  * only removed iff all can be removed.
  * Note that this may reset the associations.
  * If there are any default associations, they will be present
  * but typically there are none.
  */
  bool removeAllAssociations(bool partialRemovalOk = false);

  /**
   * @brief Remove expunged Entities from attribute
   * @param expungedEnts a set of expunged entities
   * @return if association or modelEntityItem has been updated, return true.
   * (then operator widget should update its UI)
   */
  bool removeExpungedEntities(const smtk::model::EntityRefs& expungedEnts);

  // These methods only applies to Attributes whose
  // definition returns true for isNodal()
  bool appliesToBoundaryNodes() const { return m_appliesToBoundaryNodes; }
  void setAppliesToBoundaryNodes(bool appliesValue) { m_appliesToBoundaryNodes = appliesValue; }
  bool appliesToInteriorNodes() const { return m_appliesToInteriorNodes; }
  void setAppliesToInteriorNodes(bool appliesValue) { m_appliesToInteriorNodes = appliesValue; }

  ///\brief The categories that the attribute applies to. Typically
  /// a category will be a simulation type like heat transfer, fluid flow, etc.
  const smtk::attribute::Categories& categories() const;

  ///@{
  ///\brief Returns true if the attribute is valid.
  ///
  /// An irrelevant attribute is considered valid since for all intents and purposes it is treated
  /// as though it does not exist.  A relevant attribute if valid if all of its relevant children items
  /// are valid
  bool isValid(bool useActiveCategories = true) const;
  bool isValid(const std::set<std::string>& categories) const;

  ///\brief Returns true if the attribute is relevant.
  ///
  /// If requestCatagories is true, the Attribute's Definition does not ignore categories,
  /// and the attribute does not pass it's category checks with respects
  /// to the resource's active category settings then return false,
  /// If includeReadAccess is true, and if all of the items in the attribute have their
  ///  advanceLevel > readAccessLevel then return false.
  /// Else return true.
  bool isRelevant(
    bool requestCatagories = true,
    bool includeReadAccess = false,
    unsigned int readAccessLevel = 0) const;

  smtk::attribute::ResourcePtr attributeResource() const;
  const smtk::resource::ResourcePtr resource() const override;

  void setUserData(const std::string& key, smtk::simulation::UserDataPtr value)
  {
    m_userData[key] = value;
  }
  smtk::simulation::UserDataPtr userData(const std::string& key) const;
  void clearUserData(const std::string& key) { m_userData.erase(key); }
  void clearAllUserData() { m_userData.clear(); }

  bool isAboutToBeDeleted() const { return m_aboutToBeDeleted; }

  const common::UUID& id() const override { return m_id; }

  /// Assign an ID to this attribute.
  ///
  /// This not supported for attributes.  The Id is set in the constructor and should
  /// never be changed.
  bool setId(const common::UUID& uid) override;

  // These methods are use primarily by I/O operations.  The include ID corresponds to
  // the include directory information store in the attribute resource and is used
  // when writing out the resource to use include files
  void setIncludeIndex(std::size_t index) { m_includeIndex = index; }

  std::size_t includeIndex() const { return m_includeIndex; }

  // Returns true if an Evaluator can be created for this Attribute. Does not
  // indicate that evaluation would be successful if attempted. Use
  // doesEvaluate() for that information.
  bool canEvaluate() const;

  // If an Evaluator can be created for this Attribute, returns the result of
  // Evaluator::doesEvaluate(). Returns false if no Evaluator can be created.
  bool doesEvalaute() const;

  // Returns an Evaluator for this Attribute if this Attribute's Definition is
  // registered with an Evaluator, else returns nullptr.
  std::unique_ptr<smtk::attribute::Evaluator> createEvaluator() const;

  ///\brief Return the units associated to the attribute
  ///
  /// Returns the units that have been locally set on the attribute, else it will return those
  /// returned from its definition's units method
  ///
  /// This means that the information that the attributes represents
  /// conceptually has units but unlike attribute Items, does not have a numerical value
  /// associated with it.  For example an attribute may be used to indicate that a numerical
  /// field being output by a simulation represents a temperature whose units should be in Kelvin.
  ///
  /// If the attribute has no local units set and its definition's units method returns "*", then
  /// this will return an empty string indicating that the attribute is currently unit-less
  const std::string& units() const;

  ///\brief Return the units locally set on the attribute
  const std::string& localUnits() const { return m_localUnits; }

  ///\brief Locally set the units on the attribute
  ///
  /// This will fail and return false under the following circumstances:
  /// 1. There are no units associated with its definition and the newUnits string is not empty
  /// 2. There is no units system associated with its definition and newUnits is not the same as those on the definition
  /// 3. newUnits are not supported by the associated units system
  /// 4. Its definition's units are not "*" or there is no way to convert between the units associated with its definition
  ///    and newUnits
  bool setLocalUnits(const std::string& newUnits);

  ///\brief Returns true if the attribute supports units.
  ///
  /// An attribute supports units if its definition's units() does not return an empty string.
  bool supportsUnits() const;

  class GuardedLinks
  {
  public:
    GuardedLinks(std::mutex& mutex, const smtk::resource::Component::Links& links)
      : m_guard(mutex)
      , m_links(links)
    {
    }

    const smtk::resource::Component::Links* operator->() const { return &m_links; }

    smtk::resource::Component::Links* operator->()
    {
      return const_cast<smtk::resource::Component::Links*>(&m_links);
    }

  private:
    std::unique_lock<std::mutex> m_guard;
    const smtk::resource::Component::Links& m_links;
  };

  // Attributes are uniquely used outside of an operation context, where they
  // are not guarded from concurrency issues. Specifically, ReferenceItems use
  // ResourceLinks to store references to other resources, and the
  // Resource::Links and Component::Links API is not thread-safe. This API
  // ensures thread safety when manipulating smtk::attribute::(Resource,Attribute
  // Links.
  const GuardedLinks guardedLinks() const;
  GuardedLinks guardedLinks();

protected:
  Attribute(
    const std::string& myName,
    const smtk::attribute::DefinitionPtr& myDefinition,
    const smtk::common::UUID& myId);

  /// Constructs the attribute from its definition
  void build();

  void removeAllItems();
  /// Used to disassociate an attribute from an object without checking constraints.
  /// Typical use is either when all attributes are being disassociated from the same
  /// object or if the attribute is being deleted.
  void forceDisassociate(smtk::resource::PersistentObjectPtr);
  void addItem(smtk::attribute::ItemPtr& iPtr) { m_items.push_back(iPtr); }
  void setName(const std::string& newname) { m_name = newname; }

  /// This allows the resource to change an Attribute ID
  void resetId(const smtk::common::UUID& newId) { m_id = newId; }

  std::string m_name;
  std::vector<smtk::attribute::ItemPtr> m_items;
  ReferenceItemPtr m_associatedObjects;
  smtk::attribute::DefinitionPtr m_definition;
  bool m_appliesToBoundaryNodes;
  bool m_appliesToInteriorNodes;
  bool m_isColorSet;
  std::map<std::string, smtk::simulation::UserDataPtr> m_userData;
  // We need something to indicate that the attribute is in process of
  // being deleted - this is used skip certain clean up steps that
  // would need to be done otherwise
  bool m_aboutToBeDeleted;
  double m_color[4];
  smtk::common::UUID m_id;
  std::size_t m_includeIndex;
  bool m_hasLocalAdvanceLevelInfo[2];
  unsigned int m_localAdvanceLevel[2];
  std::string m_localUnits;
};

inline smtk::simulation::UserDataPtr Attribute::userData(const std::string& key) const
{
  std::map<std::string, smtk::simulation::UserDataPtr>::const_iterator it = m_userData.find(key);
  return ((it == m_userData.end()) ? smtk::simulation::UserDataPtr() : it->second);
}

inline void Attribute::setColor(double r, double g, double b, double a)
{
  m_isColorSet = true;
  m_color[0] = r;
  m_color[1] = g;
  m_color[2] = b;
  m_color[3] = a;
}

template<typename T>
T Attribute::entityRefsAs(const std::string& iname) const
{
  T result;
  ConstReferenceItemPtr itm = this->findReference(iname);
  if (!itm)
  {
    return result;
  }

  for (auto it = itm->begin(); it != itm->end(); ++it)
  {
    if (!it.isSet())
    {
      continue;
    }

    typename T::value_type entry = std::dynamic_pointer_cast<smtk::model::Entity>(*it);
    if (entry.isValid())
    {
      result.insert(result.end(), entry);
    }
  }
  return result;
}

template<typename T>
T Attribute::associatedObjects() const
{
  T result;
  if (!m_associatedObjects)
  {
    return result;
  }

  for (auto it = m_associatedObjects->begin(); it != m_associatedObjects->end(); ++it)
  {
    if (!it.isSet())
    {
      continue;
    }

    auto entry = std::dynamic_pointer_cast<typename T::value_type::element_type>(*it);
    if (entry)
    {
      result.insert(result.end(), entry);
    }
  }
  return result;
}

template<typename T>
T Attribute::associatedModelEntities() const
{
  T result;
  if (!m_associatedObjects)
  {
    return result;
  }

  for (auto it = m_associatedObjects->begin(); it != m_associatedObjects->end(); ++it)
  {
    if (!it.isSet())
    {
      continue;
    }

    typename T::value_type entry = std::dynamic_pointer_cast<smtk::model::Entity>(*it);
    if (entry.isValid())
    {
      result.insert(result.end(), entry);
    }
  }
  return result;
}

/**\brief Return an item given its path, converted to a given pointer type.
      */
template<typename T>
typename T::Ptr
Attribute::itemAtPathAs(const std::string& path, const std::string& sep, bool activeOnly)
{
  return smtk::dynamic_pointer_cast<T>(this->itemAtPath(path, sep, activeOnly));
}

template<typename T>
typename T::ConstPtr
Attribute::itemAtPathAs(const std::string& path, const std::string& sep, bool activeOnly) const
{
  return smtk::dynamic_pointer_cast<const T>(this->itemAtPath(path, sep, activeOnly));
}

template<typename T>
typename T::Ptr Attribute::findAs(const std::string& iname, SearchStyle style)
{
  return smtk::dynamic_pointer_cast<T>(this->find(iname, style));
}

template<typename T>
typename T::ConstPtr Attribute::findAs(const std::string& iname, SearchStyle style) const
{
  return smtk::dynamic_pointer_cast<const T>(this->find(iname, style));
}

template<typename T>
void Attribute::filterItems(
  T& filtered,
  std::function<bool(typename T::value_type)> test,
  bool activeChildren)
{
  // Given an item, this lambda function which would recursively visit all children and apply test function
  std::function<void(ItemPtr, bool)> visitor =
    [&filtered, test, &visitor](smtk::attribute::ItemPtr item, bool activeChildrenLocal) {
      typename T::value_type testItem =
        smtk::dynamic_pointer_cast<typename T::value_type::element_type>(item);
      // base condition
      if (testItem && test(testItem))
      {
        filtered.insert(filtered.end(), testItem);
      }
      // Only items which have children would have a non-empty visitChildren method
      item->visitChildren(visitor, activeChildrenLocal);
    };

  for (std::size_t index = 0; index < this->numberOfItems(); ++index)
  {
    visitor(this->item(static_cast<int>(index)), activeChildren);
  }
}

} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_Attribute_h */
