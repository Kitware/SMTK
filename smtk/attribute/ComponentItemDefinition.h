//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_attribute_ComponentItemDefinition_h
#define __smtk_attribute_ComponentItemDefinition_h

#include "smtk/attribute/ItemDefinition.h"
#include "smtk/common/UUID.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

#include <typeindex>
#include <unordered_set>

namespace smtk
{
namespace attribute
{

/**\brief A definition for attribute items that store component UUIDs as values.
  *
  */
class SMTKCORE_EXPORT ComponentItemDefinition : public ItemDefinition
{
public:
  smtkTypeMacro(ComponentItemDefinition);
  static smtk::attribute::ComponentItemDefinitionPtr New(const std::string& sname)
  {
    return smtk::attribute::ComponentItemDefinitionPtr(new ComponentItemDefinition(sname));
  }

  ~ComponentItemDefinition() override;

  Item::Type type() const override;

  bool acceptsResourceComponents(const std::string& uniqueName) const;
  bool acceptsResourceComponents(std::type_index resourceIndex) const;

  bool setAcceptsResourceComponents(const std::string& uniqueName, bool accept);
  bool setAcceptsResourceComponents(std::type_index resourceIndex, bool accept);

  /// Return the mask used to accept or reject model entities as attribute values.
  smtk::model::BitFlags membershipMask() const;
  /**\brief Set the mask used to accept or reject entities as attribute values.
    *
    * This mask should include at least one dimension bit and at least one
    * entity-type bit. See smtk::model::EntityTypeBits for valid bits.
    */
  void setMembershipMask(smtk::model::BitFlags entMask);

  bool isValueValid(smtk::resource::ComponentPtr entity) const;

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

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  ComponentItemDefinition(const std::string& myName);

  smtk::model::BitFlags m_membershipMask;
  bool m_useCommonLabel;
  std::vector<std::string> m_valueLabels;
  bool m_isExtensible;
  std::size_t m_numberOfRequiredValues;
  std::size_t m_maxNumberOfValues;
  std::unordered_set<std::type_index> m_acceptable;
};

} // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ComponentItemDefinition_h */
