//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_attribute_ReferenceItemDefinition_h
#define smtk_attribute_ReferenceItemDefinition_h

#include "smtk/attribute/ItemDefinition.h"

#include "smtk/common/UUID.h"

#include "smtk/resource/Resource.h"

#include <cstdint>
#include <map>
#include <string>
#include <typeindex>
#include <unordered_set>

namespace smtk
{
namespace attribute
{

/**\brief A definition for attribute items that store smtk::resource::PersistentObjectPtr as values.
  *
  * Subclasses should implement
  * + the type() method inherited from ItemDefinition;
  * + the buildItem() methods inherited from ItemDefinition;
  * + the createCopy() method inherited from ItemDefinition (making use of this->copyTo());
  * + the virtual isValueValid() method this class declares; and
  * + a static method that constructs shared pointer to a new instance.
  */
class SMTKCORE_EXPORT ReferenceItemDefinition : public ItemDefinition
{

  friend class ValueItemDefinition; // So that ValueItemDefinitions can copy expressions

public:
  using PersistentObjectPtr = smtk::resource::PersistentObjectPtr;

  /// Construct an item definition given a name. Names should be unique and non-empty.
  smtkTypeMacro(ReferenceItemDefinition);
  smtkSuperclassMacro(ItemDefinition);
  static ReferenceItemDefinitionPtr New(const std::string& name)
  {
    return ReferenceItemDefinitionPtr(new ReferenceItemDefinition(name));
  }

  ~ReferenceItemDefinition() override;

  Item::Type type() const override { return Item::ReferenceType; }

  const std::multimap<std::string, std::string>& acceptableEntries() const { return m_acceptable; }

  virtual bool
  setAcceptsEntries(const std::string& typeName, const std::string& queryString, bool accept);

  void clearAcceptableEntries() { m_acceptable.clear(); }

  const std::multimap<std::string, std::string>& rejectedEntries() const { return m_rejected; }

  virtual bool
  setRejectsEntries(const std::string& typeName, const std::string& queryString, bool accept);

  void clearRejectedEntries() { m_rejected.clear(); }

  void setEnforcesCategories(bool mode) { m_enforcesCategories = mode; }
  bool enforcesCategories() const { return m_enforcesCategories; }

  virtual bool isValueValid(resource::ConstPersistentObjectPtr entity) const;
  /// Debug method that returns a description concerning the validity of the entity
  std::string validityCheck(resource::ConstPersistentObjectPtr entity) const;

  /// Return the number of values required by this definition.
  std::size_t numberOfRequiredValues() const;
  /// Set the number of values required by this definition. Use 0 when there is no requirement.
  void setNumberOfRequiredValues(std::size_t esize);

  bool isExtensible() const { return m_isExtensible; }
  void setIsExtensible(bool extensible) { m_isExtensible = extensible; }

  std::size_t maxNumberOfValues() const { return m_maxNumberOfValues; }
  /// Set the maximum number of values accepted (or 0 for no limit).
  void setMaxNumberOfValues(std::size_t maxNum);

  /// Return whether the definition provides labels for each value.
  bool hasValueLabels() const;
  /// Return the label for the \a i-th value.
  std::string valueLabel(std::size_t element) const;
  /// Set the label for the \a i-th value in the item.
  void setValueLabel(std::size_t element, const std::string& elabel);
  /// Indicate that all values share the \a elabel provided.
  void setCommonValueLabel(const std::string& elabel);
  /// Returns true when all values share a common label and false otherwise.
  bool usingCommonLabel() const;

  /// Set/Get the reference resource's lock type (Read/Write/DoNotLock) for read
  /// lock, write lock or bypass lock. The default is DoNotLock.
  void setLockType(smtk::resource::LockType val) { m_lockType = val; }
  smtk::resource::LockType lockType() const { return m_lockType; }

  /// Get the reference's role when generating links between the containing
  /// attribute and the reference item. By default, this value is set to
  /// smtk::attribute::Resource::ReferenceRole.
  smtk::resource::Links::RoleType role() const { return m_role; }

  /// Set the reference's role when generating links between the containing
  /// attribute and the reference item. By default, this value is set to
  /// smtk::attribute::Resource::ReferenceRole.  Note that attribute::Definition needs to be able
  /// call this method when this object is used for its association rule
  void setRole(const smtk::resource::Links::RoleType& role) { m_role = role; }

  /// Set/Get a flag to determine whether the ReferenceItem should keep an
  /// assigned reference in memory (i.e. shared_ptr vs weak_ptr to the
  /// reference).
  void setHoldReference(bool choice) { m_holdReference = choice; }
  bool holdReference() const { return m_holdReference; }

  /// Set/Get a flag to determine whether the ReferenceItem should only hold
  /// references. Currently, there is no convention for assigning a filter that
  /// only accepts references. This feature is required when associations (which
  /// cannot be sub-classed to ResourceItems) must be restricted as such.
  virtual void setOnlyResources(bool choice) { m_onlyResources = choice; }
  bool onlyResources() const { return m_onlyResources; }

  // The following are methods for dealing with conditional items
  // For conditional children items based on the item's current  value

  /// Return the number of children item definitions
  std::size_t numberOfChildrenItemDefinitions() const { return m_itemDefs.size(); }

  /// Return the map of children item definitions.  Note that the key is the name
  /// of the item definition
  const std::map<std::string, smtk::attribute::ItemDefinitionPtr>& childrenItemDefinitions() const
  {
    return m_itemDefs;
  }

  /// returns true if this item has a child item definition of itemName
  bool hasChildItemDefinition(const std::string& itemName) const
  {
    return (m_itemDefs.find(itemName) != m_itemDefs.end());
  }

  /// Adds a children item definition to this item definition.
  /// Returns false if there is already a child item definition with
  /// the same name.
  bool addChildItemDefinition(smtk::attribute::ItemDefinitionPtr cdef);

  /// This method is identical to addChildItemDefinition and exists
  /// for JSON serialization.
  bool addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef);

  // Create an item definition based on a given idName. If an item
  // with that name already exists then return a shared_pointer
  // that points to nullptr.
  template<typename T>
  typename smtk::internal::shared_ptr_type<T>::SharedPointerType addItemDefinition(
    const std::string& idName)
  {
    typedef smtk::internal::shared_ptr_type<T> SharedTypes;
    typename SharedTypes::SharedPointerType item;

    // First see if there is a item by the same name
    if (this->hasChildItemDefinition(idName))
    {
      // Already has an item of this name - do nothing
      return item;
    }
    item = SharedTypes::RawPointerType::New(idName);
    m_itemDefs[item->name()] = item;
    return item;
  }

  /// Constant used to indicate an invalid conditional index
  static constexpr std::size_t s_invalidIndex = SIZE_MAX;

  ///\brief Add a new conditional to the definition.
  ///
  /// A conditional is represented by 3 things:
  /// 1. A Resource Query (which maybe "")
  /// 2. A Component Query (which maybe "")
  /// 3. A list of item names that corresponds the ordered list
  ///    of active children if this conditional is met (maybe empty)
  /// If the conditional is valid, then the method will return index
  /// corresponding to its place in the vector of conditionals.
  /// If either both query strings are empty or if the itemNames
  /// contain a name that doesn't exist, then the conditional is
  /// not added and s_invalidIndex is returned
  ///
  /// Note when assigning resources, only the resource query will be used,
  /// hence the component query can be empty
  /// In the case of components, the resource query can be empty iff the
  /// reference item can only be assigned to components of the same "type"
  /// of resources.
  std::size_t addConditional(
    const std::string& resourceQuery,
    const std::string& componentQuery,
    const std::vector<std::string>& itemNames);

  /// returns true if ith conditional has a child item definition of itemName
  bool hasChildItemDefinition(std::size_t ith, const std::string& itemName);

  /// Return the number of conditionals
  std::size_t numberOfConditionals() const { return m_conditionalItemNames.size(); }

  /// Return the conditional item name information
  const std::vector<std::vector<std::string>>& conditionalInformation() const
  {
    return m_conditionalItemNames;
  }

  /// Return the vector of item names that correspond to the i th conditional
  const std::vector<std::string>& conditionalItems(std::size_t ith) const;

  /// Return the vector of resource queries of all conditionals
  const std::vector<std::string>& resourceQueries() const { return m_resourceQueries; }

  /// Return the vector of component queries of all conditionals
  const std::vector<std::string>& componentQueries() const { return m_componentQueries; }

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(Item* owner, int itemPos, int subGroupPosition) const override;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

  // The attribute::Definition needs to be able call the setRole method when this object is used
  // for the Definition's association rule
  friend class Definition;

  // Should only be called internally by the ReferenceItem
  void buildChildrenItems(ReferenceItem* ritem) const;

  // Returns the conditional that matches object
  std::size_t testConditionals(PersistentObjectPtr& objet) const;

  // Returns the criteria information as a string
  std::string criteriaAsString() const;

protected:
  ReferenceItemDefinition(const std::string& myName);

  /// Overwrite \a dst with a copy of this instance.
  void copyTo(Ptr dst, smtk::attribute::ItemDefinition::CopyInfo& info) const;

  /// Return whether a resource is accepted by this definition. Used internally by isValueValid().
  bool checkResource(const smtk::resource::Resource& rsrc) const;
  /// Return whether a component is accepted by this definition. Used internally by isValueValid().
  /// The pointer is being based so dynamic casting can be used
  bool checkComponent(const smtk::resource::Component* comp) const;
  /// Return whether a component passes the category requirements.  This is used for comps
  /// that are Attributes
  /// The pointer is being based so dynamic casting can be used
  bool checkCategories(const smtk::resource::Component* comp) const;

  void applyCategories(
    const smtk::common::Categories::Stack& inheritedFromParent,
    smtk::common::Categories& inheritedToParent) override;

  void setUnitsSystem(const shared_ptr<units::System>& unitsSystem) override;

  /// Debug method that returns a description concerning the validity of the component
  std::string componentValidityCheck(const smtk::resource::Component* comp) const;
  bool m_useCommonLabel;
  std::vector<std::string> m_valueLabels;
  bool m_isExtensible;
  std::size_t m_numberOfRequiredValues;
  std::size_t m_maxNumberOfValues;
  std::multimap<std::string, std::string> m_acceptable;
  std::multimap<std::string, std::string> m_rejected;
  smtk::resource::LockType m_lockType;
  smtk::resource::Links::RoleType m_role;
  bool m_holdReference;
  bool m_enforcesCategories = false;
  // data members for dealing with conditional children
  // definitions of all conditional children items
  std::map<std::string, smtk::attribute::ItemDefinitionPtr> m_itemDefs;
  // resource query part of the conditional
  std::vector<std::string> m_resourceQueries;
  // component query part of the conditional
  std::vector<std::string> m_componentQueries;
  // represents the conditional items that are associated with the conditional
  std::vector<std::vector<std::string>> m_conditionalItemNames;

private:
  bool m_onlyResources;
};

} // namespace attribute
} // namespace smtk

// returns true if valueName has a child item definition of itemName
inline bool smtk::attribute::ReferenceItemDefinition::hasChildItemDefinition(
  std::size_t ith,
  const std::string& itemName)
{
  // First we need to check to see if we have this child item or if ith is
  // out of range
  if (!(this->hasChildItemDefinition(itemName) && (ith < m_conditionalItemNames.size())))
  {
    return false;
  }
  return (
    std::find(m_conditionalItemNames[ith].begin(), m_conditionalItemNames[ith].end(), itemName) !=
    m_conditionalItemNames[ith].end());
}

#endif
