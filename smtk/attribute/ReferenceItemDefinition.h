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
  smtkTypeMacro(ReferenceItemDefinition<T>);
  smtkSuperclassMacro(ItemDefinition);

  ~ReferenceItemDefinition() override;

  std::multimap<std::string, std::string> acceptableEntries() const { return m_acceptable; }

  bool setAcceptsEntries(
    const std::string& uniqueName, const std::string& queryString, bool accept);

  virtual bool isValueValid(typename T::Ptr entity) const = 0;

  std::size_t numberOfRequiredValues() const;
  void setNumberOfRequiredValues(std::size_t esize);

  bool isExtensible() const { return m_isExtensible; }
  void setIsExtensible(bool extensible) { m_isExtensible = extensible; }

  std::size_t maxNumberOfValues() const { return m_maxNumberOfValues; }
  void setMaxNumberOfValues(std::size_t maxNum);

  bool hasValueLabels() const;
  std::string valueLabel(std::size_t element) const;
  void setValueLabel(std::size_t element, const std::string& elabel);
  void setCommonValueLabel(const std::string& elabel);
  bool usingCommonLabel() const;

  // Set/Get the writable (vs read-only) property of the item defintiion.
  // The default is true.
  void setIsWritable(bool val) { m_isWritable = val; }
  bool isWritable() const { return m_isWritable; }

protected:
  ReferenceItemDefinition(const std::string& myName);

  void copyTo(Ptr dst) const;

  bool m_useCommonLabel;
  std::vector<std::string> m_valueLabels;
  bool m_isExtensible;
  std::size_t m_numberOfRequiredValues;
  std::size_t m_maxNumberOfValues;
  std::multimap<std::string, std::string> m_acceptable;
  bool m_isWritable;
};

} // namespace attribute
} // namespace smtk

#endif
