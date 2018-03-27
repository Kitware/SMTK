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

/**\brief A definition for attribute items that store T::Ptr as values.
  *
  * Subclasses must implement
  * + the type() method inherited from ItemDefinition;
  * + the buildItem() methods inherited from ItemDefinition;
  * + the createCopy() method inherited from ItemDefinition (making use of this->copyTo());
  * + the pure virtual isValueValid() method this class declares; and
  * + a static method that constructs shared pointer to a new instance.
  */
template <typename T>
class ReferenceItemDefinition : public ItemDefinition
{
public:
  /// Construct an item definition given a name. Names should be unique and non-empty.
  smtkTypeMacro(ReferenceItemDefinition<T>);
  smtkSuperclassMacro(ItemDefinition);

  ~ReferenceItemDefinition() override;

  std::multimap<std::string, std::string> acceptableEntries() const { return m_acceptable; }

  bool setAcceptsEntries(const std::string& typeName, const std::string& queryString, bool accept);

  virtual bool isValueValid(typename T::Ptr entity) const = 0;

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

protected:
  ReferenceItemDefinition(const std::string& myName);

  /// Overwrite \a dst with a copy of this instance.
  void copyTo(Ptr dst) const;

  bool m_useCommonLabel;
  std::vector<std::string> m_valueLabels;
  bool m_isExtensible;
  std::size_t m_numberOfRequiredValues;
  std::size_t m_maxNumberOfValues;
  std::multimap<std::string, std::string> m_acceptable;
  smtk::resource::LockType m_lockType;
};

} // namespace attribute
} // namespace smtk

#endif
