//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_attribute_ResourceItemDefinition_h
#define __smtk_attribute_ResourceItemDefinition_h

#include "smtk/attribute/ItemDefinition.h"
#include "smtk/common/UUID.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags
#include "smtk/resource/Resource.h"

#include <set>

namespace smtk
{
namespace attribute
{

/**\brief A definition for attribute items that store Resources as values.
  */
class SMTKCORE_EXPORT ResourceItemDefinition : public ItemDefinition
{
public:
  smtkTypeMacro(ResourceItemDefinition);
  static smtk::attribute::ResourceItemDefinitionPtr New(const std::string& sname)
  {
    return smtk::attribute::ResourceItemDefinitionPtr(new ResourceItemDefinition(sname));
  }

  ~ResourceItemDefinition() override;

  Item::Type type() const override;

  /// Check whether or not this definition is allowed to hold a resource. The
  /// default mode is to accept all resource types.
  bool acceptsResource(const smtk::resource::ResourcePtr& resource) const;

  const std::set<std::string>& acceptableResources() const { return m_acceptable; }

  bool setAcceptsResources(const std::string& uniqueName, bool accept);

  bool isValueValid(smtk::resource::ResourcePtr entity) const;

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const override;

  std::size_t numberOfRequiredValues() const;
  void setNumberOfRequiredValues(std::size_t esize);

  bool isExtensible() const { return this->m_isExtensible; }
  void setIsExtensible(bool extensible) { this->m_isExtensible = extensible; }

  std::size_t maxNumberOfValues() const { return this->m_maxNumberOfValues; }
  void setMaxNumberOfValues(std::size_t maxNum);

  bool hasValueLabels() const;
  std::string valueLabel(std::size_t element) const;
  void setValueLabel(std::size_t element, const std::string& elabel);
  void setCommonValueLabel(const std::string& elabel);
  bool usingCommonLabel() const;

  // Set/Get the writable (vs read-only) property of the item defintiion.
  // The default is true.
  void setIsWritable(bool val) { this->m_isWritable = val; }
  bool isWritable() const { return this->m_isWritable; }

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  ResourceItemDefinition(const std::string& myName);

  bool m_useCommonLabel;
  std::vector<std::string> m_valueLabels;
  bool m_isExtensible;
  std::size_t m_numberOfRequiredValues;
  std::size_t m_maxNumberOfValues;
  std::set<std::string> m_acceptable;
  bool m_isWritable;
};

} // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ResourceItemDefinition_h */
