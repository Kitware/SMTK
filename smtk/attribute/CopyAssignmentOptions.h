//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_attribute_CopyAssignmentOptions_h
#define smtk_attribute_CopyAssignmentOptions_h

#include "smtk/CoreExports.h"
#include "smtk/common/UUID.h"

#include <string>
#include <unordered_map>

namespace smtk
{

namespace resource
{
class PersistentObject;
}

namespace attribute
{

///\brief Class used to control how an attribute is to be copied.
///
/// This is primarily used by smtk::attribute::Resource::copyAttribute but can be also
/// indirectly by smtk::attribute::Attribute::assign and smtk::attribute::Item::assign.

class SMTKCORE_EXPORT AttributeCopyOptions
{
public:
  /// @{
  /// \brief Methods to set and retrieve the performAssignment Option
  ///
  /// If set, this indicates that copied attributes be assigned the values of the original.
  /// **Note** : If not set then all of the items in the copied Attribute will not be assigned
  /// including the Attribute's Associations
  bool performAssignment() const { return m_performAssignment; }
  void setPerformAssignment(bool val) { m_performAssignment = val; }
  /// @}

  /// @{
  /// \brief Methods to set and retrieve the copyUUID Option
  ///
  /// If set, this indicates that copied attributes should have the same UUID as the original.
  /// **Note** : the copying process will fail if the copied attribute would reside in the same
  /// resource as the original.
  bool copyUUID() const { return m_copyUUID; }
  void setCopyUUID(bool val) { m_copyUUID = val; }
  /// @}

  /// @{
  /// \brief Methods to set and retrieve the copyDefinition Option
  ///
  /// If set, this indicates that if the source attribute's definition (by typename) does not exist in the resource
  /// making the copy, then copy the definition as well.  This can recursively cause other definitions
  /// to be copied.
  /// **Note** : the copying process will fail if this option is not set and the source attribute definition's typename
  /// does not exist in the targeted resource.
  bool copyDefinition() const { return m_copyDefinition; }
  void setCopyDefinition(bool val) { m_copyDefinition = val; }
  /// @}

  /// \brief Converts the current option state into a string that is prefixed by prefix
  std::string convertToString(const std::string& prefix = "") const;

protected:
  bool m_copyUUID = false;
  bool m_copyDefinition = false;
  bool m_performAssignment = true;
};

///\brief Class that represents functionality common to both attribute and item assignments.
///
/// Currently this is limited to providing a mapping of Persistent Object information used
/// to determine if the Persistent Object information stored in the source needs to be
/// mapped to a different Persistent Object w/r to the copy.
class SMTKCORE_EXPORT CommonAssignmentOptions
{
public:
  /// A type alias for the container holding the UUID translation table.
  using ObjectMapType = std::unordered_map<smtk::common::UUID, smtk::resource::PersistentObject*>;
  void setObjectMapping(ObjectMapType* val) { m_objectMapping = val; }
  const ObjectMapType* objectMapping() const { return m_objectMapping; }

  /// A convenience to fetch an entry from object mapping (if set), casting it to the given type.
  ///
  /// Be aware this method may return a null pointer if (a) there is no mapping, (b) there is no
  /// object that corresponds to the input \a sourceId or (c) the object
  /// cannot be cast to \a ObjectType.
  template<typename ObjectType>
  ObjectType* targetObjectFromSourceId(const smtk::common::UUID& sourceId) const
  {
    if (m_objectMapping == nullptr)
    {
      return nullptr;
    }
    auto it = m_objectMapping->find(sourceId);
    if (it == m_objectMapping->end())
    {
      return nullptr;
    }
    return dynamic_cast<ObjectType*>(it->second);
  }

  virtual std::string convertToString(const std::string& prefix = "") const;

protected:
  ObjectMapType* m_objectMapping = nullptr;
};

///\brief Class used to control how an attribute's information is assigned to another attribute.
///
/// This is primarily used by smtk::attribute::Attribute::assign but can be also
/// indirectly by smtk::attribute::Resource::copyAttribute and smtk::attribute::Item::assign.
class SMTKCORE_EXPORT AttributeAssignmentOptions : public CommonAssignmentOptions
{
public:
  /// A type alias for the container holding the UUID translation table.
  using ObjectMapType = std::unordered_map<smtk::common::UUID, smtk::resource::PersistentObject*>;

  /// @{
  /// \brief Methods to set and retrieve the ignoreMissingItems Option
  ///
  /// If set, this indicates that not all of the source attribute's items must exist in the
  /// target attribute.  This can occur if the target attribute's definition is a variation of
  /// the source attribute's.
  ///
  /// **Note** : the assignment process will fail if this option is not set and if not all of the
  /// source attribute's items are not present in the target.
  bool ignoreMissingItems() const { return m_ignoreMissingItems; }
  void setIgnoreMissingItems(bool val) { m_ignoreMissingItems = val; }
  /// @}

  /// @{
  /// \brief Methods to set and retrieve the copyAssociations Option
  ///
  /// If set, this indicates that the source attribute's associations should be copied
  /// to the target attribute which will also take into consideration allowPartialAssociations
  /// and doNotValidateAssociations options.
  bool copyAssociations() const { return m_copyAssociations; }
  void setCopyAssociations(bool val) { m_copyAssociations = val; }
  /// @}

  /// @{
  /// \brief Methods to set and retrieve the allowPartialAssociations Option
  ///
  /// Assuming that copyAssociations option is set, if the allowPartialAssociations
  /// ** is not set ** then all of the source's associations must be associated
  /// to the target attribute, else the assignment process will return failure.
  bool allowPartialAssociations() const { return m_allowPartialAssociations; }
  void setAllowPartialAssociations(bool val) { m_allowPartialAssociations = val; }
  /// @}

  /// @{
  /// \brief Methods to set and retrieve the doNotValidateAssociations *Hint* Option
  ///
  /// Assuming that copyAssociations option is set, the doNotValidateAssociations
  /// hint indicates that if it possible to assign the association information
  /// without accessing the corresponding persistent object, then do so without
  /// validation.
  bool doNotValidateAssociations() const { return m_doNotValidateAssociations; }
  void setDoNotValidateAssociations(bool val) { m_doNotValidateAssociations = val; }
  /// @}

  std::string convertToString(const std::string& prefix = "") const override;

protected:
  bool m_ignoreMissingItems = false;
  bool m_copyAssociations = false;
  bool m_allowPartialAssociations = false;
  bool m_doNotValidateAssociations = false;
};

///\brief Class used to control how an item's information is assigned to another item.
///
/// This is primarily used by smtk::attribute::Item::assign but can be also
/// indirectly by smtk::attribute::Attribute::assign and smtk::attribute::Resource::copyAttribute.
class SMTKCORE_EXPORT ItemAssignmentOptions : public CommonAssignmentOptions
{
public:
  /// @{
  /// \brief Methods to set and retrieve the ignoreMissingChildren Option
  ///
  /// If set, this indicates that not all of the source item's children items must exist in the
  /// target item.  This can occur if the target item's definition is a variation of
  /// the source item's.
  ///
  /// **Note** : the assignment process will fail if this option is not set and if not all of the
  /// source item's children items are not present in the target.
  bool ignoreMissingChildren() const { return m_ignoreMissingChildren; }
  void setIgnoreMissingChildren(bool val) { m_ignoreMissingChildren = val; }
  /// @}

  /// @{
  /// \brief Methods to set and retrieve the allowPartialValues Option
  ///
  /// If set,  this indicates that not all of the source item's values must be
  /// copied to the target item. If this option ** is not set ** then all of the
  /// source item's values must be copied, else the assignment process will return failure.
  bool allowPartialValues() const { return m_allowPartialValues; }
  void setAllowPartialValues(bool val) { m_allowPartialValues = val; }
  /// @}

  /// @{
  /// \brief Methods to set and retrieve the ignoreExpressions Option
  ///
  /// If set, this indicates that if a source Value item that have been assigned
  /// an expression attribute, it's corresponding target item should be left unset.
  bool ignoreExpressions() const { return m_ignoreExpressions; }
  void setIgnoreExpressions(bool val) { m_ignoreExpressions = val; }
  ///@}

  /// @{
  /// \brief Methods to set and retrieve the ignoreReferenceValues Option
  ///
  /// If set, this indicates that a target Reference item should not be assigned
  /// the values of the corresponding source item.
  bool ignoreReferenceValues() const { return m_ignoreReferenceValues; }
  void setIgnoreReferenceValues(bool val) { m_ignoreReferenceValues = val; }
  ///@}

  /// @{
  /// \brief Methods to set and retrieve the doNotValidateReferenceInfo *Hint* Option
  ///
  /// The doNotValidateReferenceInfo hint indicates that if it possible to assign a source Reference item's
  /// values to a target item without accessing the corresponding persistent object, then do so without
  /// validation.
  bool doNotValidateReferenceInfo() const { return m_doNotValidateReferenceInfo; }
  void setDoNotValidateReferenceInfo(bool val) { m_doNotValidateReferenceInfo = val; }
  /// @}

  /// @{
  /// \brief Methods to set and retrieve the disableCopyAttributes Option
  ///
  /// If set, this indicates that no attributes should be created when doing item assignments.
  /// An item assignment can cause an attribute to be created in two situations.
  ///
  /// First - A source Value item is set to an expression attribute that resides in the same
  /// resource and the target item resides in a different one.  In this case the default
  /// behavior is to also copy the expression attribute to the target item's resource and
  /// assign the copied attribute to the target item.
  ///
  /// Second - A source Reference item refers to an attribute that resides in the same
  /// resource and the target item resides in a different one.  In this case the default
  /// behavior is to also copy the referenced attribute to the target item's resource and
  /// assign the copied attribute to the target item.
  bool disableCopyAttributes() const { return m_disableCopyAttributes; }
  void setDisableCopyAttributes(bool val) { m_disableCopyAttributes = val; }
  /// @}

  /// \brief Converts the current option state into a string that is prefixed by prefix
  std::string convertToString(const std::string& prefix = "") const override;

protected:
  bool m_ignoreMissingChildren = false;
  bool m_allowPartialValues = false;
  bool m_ignoreExpressions = false;
  bool m_ignoreReferenceValues = false;
  bool m_doNotValidateReferenceInfo = false;
  bool m_disableCopyAttributes = false;
};

///\brief Class used to specify copy and assignment options
class SMTKCORE_EXPORT CopyAssignmentOptions
{
public:
  AttributeCopyOptions copyOptions;
  AttributeAssignmentOptions attributeOptions;
  ItemAssignmentOptions itemOptions;

  /// \brief Converts the current option state into a string that is prefixed by prefix
  std::string convertToString(const std::string& prefix = "") const;
};

};     // namespace attribute
};     // namespace smtk
#endif // smtk_attribute_CopyAssignmentOptions_h
