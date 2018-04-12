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

#ifndef __smtk_attribute_Attribute_h
#define __smtk_attribute_Attribute_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/resource/Component.h"

#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/SearchStyle.h"
#include "smtk/attribute/ValueItem.h"

#include "smtk/common/UUID.h" // for template associatedModelEntities()

#include <map>
#include <set>
#include <string>
#include <vector>

namespace smtk
{
class Model;

namespace attribute
{
class RefItem;
class Item;
class Collection;

/**\brief Represent a (possibly composite) value according to a definition.
      *
      */
class SMTKCORE_EXPORT Attribute : public resource::Component
{
  friend class smtk::attribute::Definition;
  friend class smtk::attribute::Collection;
  friend class smtk::attribute::RefItem;

public:
  static smtk::attribute::AttributePtr New(
    const std::string& myName, smtk::attribute::DefinitionPtr myDefinition)
  {
    return smtk::attribute::AttributePtr(new Attribute(myName, myDefinition));
  }

  static smtk::attribute::AttributePtr New(const std::string& myName,
    smtk::attribute::DefinitionPtr myDefinition, const smtk::common::UUID& myId)
  {
    return smtk::attribute::AttributePtr(new Attribute(myName, myDefinition, myId));
  }

  virtual ~Attribute();

  AttributePtr shared_from_this()
  {
    return static_pointer_cast<Attribute>(Component::shared_from_this());
  }

  // NOTE: To rename an attribute use the collection!
  const std::string& name() const { return m_name; }

  const std::string& type() const;
  std::vector<std::string> types() const;
  bool isA(smtk::attribute::DefinitionPtr def) const;
  smtk::attribute::DefinitionPtr definition() const { return m_definition; }

  const double* color() const;
  void setColor(double r, double g, double b, double alpha);
  void setColor(const double* l_color)
  {
    this->setColor(l_color[0], l_color[1], l_color[2], l_color[3]);
  }
  bool isColorSet() const { return m_isColorSet; }
  void unsetColor() { m_isColorSet = false; }

  bool isMemberOf(const std::string& category) const;
  bool isMemberOf(const std::vector<std::string>& categories) const;

  smtk::attribute::ItemPtr item(int ith) const
  {
    return (ith < 0)
      ? smtk::attribute::ItemPtr()
      : (static_cast<unsigned int>(ith) >= m_items.size() ? smtk::attribute::ItemPtr()
                                                          : m_items[static_cast<std::size_t>(ith)]);
  }

  smtk::attribute::ConstItemPtr itemAtPath(
    const std::string& path, const std::string& seps = "/") const;
  smtk::attribute::ItemPtr itemAtPath(const std::string& path, const std::string& seps = "/");

  template <typename T>
  typename T::ConstPtr itemAtPathAs(const std::string& path, const std::string& seps = "/") const;
  template <typename T>
  typename T::Ptr itemAtPathAs(const std::string& path, const std::string& seps = "/");

  smtk::attribute::ItemPtr find(const std::string& name, SearchStyle style = ACTIVE_CHILDREN);
  smtk::attribute::ConstItemPtr find(
    const std::string& name, SearchStyle style = ACTIVE_CHILDREN) const;
  std::size_t numberOfItems() const { return m_items.size(); }

  template <typename T>
  typename T::Ptr findAs(const std::string& name, SearchStyle style = ACTIVE_CHILDREN);

  template <typename T>
  typename T::ConstPtr findAs(const std::string& name, SearchStyle style = ACTIVE_CHILDREN) const;

  /**
   * @brief Given a container, file items in the attribute by a lambda function
   * @param values a container which holds items
   * @param test a lambda function which would be applied on children items
   * Example filter double and int items
   *  [](Item::Ptr item) { return item->type() == DoubleType || item->type() == IntType; }
   * Example filter modelEntity items
   *  [](ModelEntity::Ptr item) { return true; }
   * @param activeChildren a flag indicates whether it should be applied to active children only or not
   */
  template <typename T>
  void filterItems(
    T& values, std::function<bool(typename T::value_type)> test, bool activeChildren = true);

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

  RefItemPtr findRef(const std::string& name);
  ConstRefItemPtr findRef(const std::string& name) const;

  ModelEntityItemPtr findModelEntity(const std::string& name);
  ConstModelEntityItemPtr findModelEntity(const std::string& name) const;

  VoidItemPtr findVoid(const std::string& name);
  ConstVoidItemPtr findVoid(const std::string& name) const;

  DateTimeItemPtr findDateTime(const std::string& name);
  ConstDateTimeItemPtr findDateTime(const std::string& name) const;

  ReferenceItemPtr findReference(const std::string& name);
  ConstReferenceItemPtr findReference(const std::string& name) const;
  template <typename T>
  T entityRefsAs(const std::string& name) const;

  ResourceItemPtr findResource(const std::string& name);
  ConstResourceItemPtr findResource(const std::string& name) const;

  ComponentItemPtr findComponent(const std::string& name);
  ConstComponentItemPtr findComponent(const std::string& name) const;

  void references(std::vector<smtk::attribute::ItemPtr>& list) const;

  ConstReferenceItemPtr associatedObjects() const { return m_associatedObjects; }
  ReferenceItemPtr associatedObjects() { return m_associatedObjects; }

  bool isObjectAssociated(const smtk::common::UUID& uid) const;
  bool isObjectAssociated(const smtk::resource::PersistentObjectPtr& componentPtr) const;

  ConstReferenceItemPtr associations() const { return m_associatedObjects; }
  ReferenceItemPtr associations() { return m_associatedObjects; }

  bool isEntityAssociated(const smtk::common::UUID& entity) const;
  bool isEntityAssociated(const smtk::model::EntityRef& entityref) const;

  smtk::common::UUIDs associatedModelEntityIds() const;
  template <typename T>
  T associatedModelEntities() const;

  bool associate(smtk::resource::PersistentObjectPtr obj);
  bool associateEntity(const smtk::common::UUID& entity);
  bool associateEntity(const smtk::model::EntityRef& entity);

  void disassociateEntity(const smtk::common::UUID& entity, bool reverse = true);
  void disassociateEntity(const smtk::model::EntityRef& entity, bool reverse = true);
  void removeAllAssociations();

  /**
   * @brief Remove expunged Entities from attribute
   * @param expungedEnts a set of expunged entities
   * @return if assocation or modelEntityItem has been updated, return true.
   * (then operator widget should update its UI)
   */
  bool removeExpungedEntities(const smtk::model::EntityRefs& expungedEnts);

  MeshSelectionItemPtr findMeshSelection(const std::string& name);
  ConstMeshSelectionItemPtr findMeshSelection(const std::string& name) const;
  MeshItemPtr findMesh(const std::string& name);
  ConstMeshItemPtr findMesh(const std::string& name) const;

  // These methods only applies to Attributes whose
  // definition returns true for isNodal()
  bool appliesToBoundaryNodes() const { return m_appliesToBoundaryNodes; }
  void setAppliesToBoundaryNodes(bool appliesValue) { m_appliesToBoundaryNodes = appliesValue; }
  bool appliesToInteriorNodes() const { return m_appliesToInteriorNodes; }
  void setAppliesToInteriorNodes(bool appliesValue) { m_appliesToInteriorNodes = appliesValue; }

  bool isValid() const;

  smtk::attribute::CollectionPtr collection() const;
  const smtk::resource::ResourcePtr resource() const override;
  smtk::model::ManagerPtr modelManager() const;

  void setUserData(const std::string& key, smtk::simulation::UserDataPtr value)
  {
    m_userData[key] = value;
  }
  smtk::simulation::UserDataPtr userData(const std::string& key) const;
  void clearUserData(const std::string& key) { m_userData.erase(key); }
  void clearAllUserData() { m_userData.clear(); }

  bool isAboutToBeDeleted() const { return m_aboutToBeDeleted; }

  const common::UUID& id() const override { return m_id; }
  bool setId(const common::UUID& uid) override
  {
    m_id = uid;
    return true;
  }

protected:
  Attribute(const std::string& myName, smtk::attribute::DefinitionPtr myDefinition,
    const smtk::common::UUID& myId);
  Attribute(const std::string& myName, smtk::attribute::DefinitionPtr myDefinition);

  void removeAllItems();
  void addItem(smtk::attribute::ItemPtr iPtr) { m_items.push_back(iPtr); }
  void setName(const std::string& newname) { m_name = newname; }

  void addReference(smtk::attribute::RefItem* attRefItem, std::size_t pos)
  {
    m_references[attRefItem].insert(pos);
  }
  // This removes a specific ref item
  void removeReference(smtk::attribute::RefItem* attRefItem, std::size_t pos)
  {
    m_references[attRefItem].erase(pos);
  }
  // This removes all references to a specific Ref Item
  void removeReference(smtk::attribute::RefItem* attRefItem) { m_references.erase(attRefItem); }

  std::string m_name;
  std::vector<smtk::attribute::ItemPtr> m_items;
  ReferenceItemPtr m_associatedObjects;
  smtk::attribute::DefinitionPtr m_definition;
  std::map<smtk::attribute::RefItem*, std::set<std::size_t> > m_references;
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

template <typename T>
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
    typename T::value_type entry = std::dynamic_pointer_cast<smtk::model::Entity>(*it);
    if (entry.isValid())
    {
      result.insert(result.end(), entry);
    }
  }
  return result;
}

template <typename T>
T Attribute::associatedModelEntities() const
{
  T result;
  if (!m_associatedObjects)
  {
    return result;
  }

  for (auto it = m_associatedObjects->begin(); it != m_associatedObjects->end(); ++it)
  {
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
template <typename T>
typename T::ConstPtr Attribute::itemAtPathAs(const std::string& path, const std::string& seps) const
{
  typename T::ConstPtr result;
  smtk::attribute::ConstItemPtr itm = this->itemAtPath(path, seps);
  if (!!itm)
  {
    result = smtk::dynamic_pointer_cast<const T>(itm);
  }
  return result;
}

template <typename T>
typename T::Ptr Attribute::findAs(const std::string& iname, SearchStyle style)
{
  return smtk::dynamic_pointer_cast<T>(this->find(iname, style));
}

template <typename T>
typename T::ConstPtr Attribute::findAs(const std::string& iname, SearchStyle style) const
{
  return smtk::dynamic_pointer_cast<const T>(this->find(iname, style));
}

template <typename T>
void Attribute::filterItems(
  T& filtered, std::function<bool(typename T::value_type)> test, bool activeChildren)
{
  // Given an item, this lambda function which would recursively visit all children and apply test function
  std::function<void(ItemPtr, bool)> visitor = [&filtered, test, &visitor](
    smtk::attribute::ItemPtr item, bool activeChildrenLocal) {
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

} // attribute namespace
} // smtk namespace

#endif /* __smtk_attribute_Attribute_h */
