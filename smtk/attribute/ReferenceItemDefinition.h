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
public:
  using PersistentObjectPtr = smtk::resource::PersistentObjectPtr;

  // When this string is used as the filter string for acceptable entries, only
  // resources will be accepted.
  static constexpr const char* const only_resources = "__only_resources";

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

  virtual bool setAcceptsEntries(
    const std::string& typeName, const std::string& queryString, bool accept);

  virtual bool isValueValid(resource::ConstPersistentObjectPtr entity) const;

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

  /// Set/Get a flag to determine whether the ReferenceItem should keep an
  /// assigned reference in memory (i.e. shared_ptr vs weak_ptr to the
  /// reference).
  void setHoldReference(bool choice) { m_holdReference = choice; }
  bool holdReference() const { return m_holdReference; }

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(Item* owner, int itemPos, int subGroupPosition) const override;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

  // The attribute::Definition needs to be able call the setRole method when this object is used
  // for the Definition's association rule
  friend class Definition;

protected:
  ReferenceItemDefinition(const std::string& myName);

  /// Overwrite \a dst with a copy of this instance.
  void copyTo(Ptr dst) const;

  /// Return whether a resource is accepted by this definition. Used internally by isValueValid().
  bool checkResource(smtk::resource::ConstResourcePtr rsrc) const;
  /// Return whether a component is accepted by this definition. Used internally by isValueValid().
  bool checkComponent(smtk::resource::ConstComponentPtr comp) const;

  /// Set the reference's role when generating links between the containing
  /// attribute and the reference item. By default, this value is set to
  /// smtk::attribute::Resource::ReferenceRole.  Note that attribute::Definition needs to be able
  /// call this method when this object is used for its association rule
  void setRole(const smtk::resource::Links::RoleType& role) { m_role = role; }

  bool m_useCommonLabel;
  std::vector<std::string> m_valueLabels;
  bool m_isExtensible;
  std::size_t m_numberOfRequiredValues;
  std::size_t m_maxNumberOfValues;
  std::multimap<std::string, std::string> m_acceptable;
  smtk::resource::LockType m_lockType;
  smtk::resource::Links::RoleType m_role;
  bool m_holdReference;
};

} // namespace attribute
} // namespace smtk

#endif
