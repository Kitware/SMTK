//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ModelEntityItemDefinition.h - A definition for attribute items that store UUIDs as values.
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_ModelEntityItemDefinition_h
#define __smtk_attribute_ModelEntityItemDefinition_h

#include "smtk/attribute/ItemDefinition.h"
#include "smtk/common/UUID.h"
#include "smtk/model/EntityTypeBits.h" // for smtk::model::BitFlags

namespace smtk
{
  namespace attribute
  {

class SMTKCORE_EXPORT ModelEntityItemDefinition : public ItemDefinition
{
public:
  smtkTypeMacro(ModelEntityItemDefinition);
  static smtk::attribute::ModelEntityItemDefinitionPtr New(const std::string& sname)
    { return smtk::attribute::ModelEntityItemDefinitionPtr(new ModelEntityItemDefinition(sname));}

  virtual ~ModelEntityItemDefinition();

  virtual Item::Type type() const;

  smtk::model::BitFlags membershipMask() const;
  void setMembershipMask(smtk::model::BitFlags entMask);

  bool isValueValid(const smtk::model::EntityRef& entity) const;

  virtual smtk::attribute::ItemPtr buildItem(
    Attribute* owningAttribute, int itemPosition) const;
  virtual smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const;

  std::size_t numberOfRequiredValues() const;
  void setNumberOfRequiredValues(std::size_t esize);

  bool isExtensible() const { return this->m_isExtensible; }
  void setIsExtensible(bool extensible) { this->m_isExtensible = extensible; }

  std::size_t maxNumberOfValues() const { return this->m_maxNumberOfValues; }
  void setMaxNumberOfValues(std::size_t maxNum);

  bool hasValueLabels() const;
  std::string valueLabel(std::size_t element) const;
  void setValueLabel(std::size_t element, const std::string &elabel);
  void setCommonValueLabel(const std::string &elabel);
  bool usingCommonLabel() const;

  virtual smtk::attribute::ItemDefinitionPtr
    createCopy(smtk::attribute::ItemDefinition::CopyInfo& info) const;
protected:
  ModelEntityItemDefinition(const std::string& myName);

  smtk::model::BitFlags m_membershipMask;
  bool m_useCommonLabel;
  std::vector<std::string> m_valueLabels;
  bool m_isExtensible;
  std::size_t m_numberOfRequiredValues;
  std::size_t m_maxNumberOfValues;
};

  } // namespace attribute
} // namespace smtk

#endif /* __smtk_attribute_ModelEntityItemDefinition_h */
