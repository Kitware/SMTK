//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME RefItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_RefItemDefinition_h
#define __smtk_attribute_RefItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/attribute/ItemDefinition.h"

namespace smtk
{
namespace attribute
{
class Attribute;
class SMTKCORE_EXPORT RefItemDefinition : public ItemDefinition
{
public:
  smtkTypeMacro(RefItemDefinition);
  static smtk::attribute::RefItemDefinitionPtr New(const std::string& myName)
  {
    return smtk::attribute::RefItemDefinitionPtr(new RefItemDefinition(myName));
  }

  ~RefItemDefinition() override;

  Item::Type type() const override;
  smtk::attribute::DefinitionPtr attributeDefinition() const { return m_definition.lock(); }

  void setAttributeDefinition(smtk::attribute::DefinitionPtr def) { m_definition = def; }

  bool isValueValid(smtk::attribute::AttributePtr att) const;

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const override;
  std::size_t numberOfRequiredValues() const { return m_numberOfRequiredValues; }
  void setNumberOfRequiredValues(std::size_t esize);

  bool hasValueLabels() const { return !m_valueLabels.empty(); }

  void setValueLabel(std::size_t element, const std::string& elabel);
  void setCommonValueLabel(const std::string& elabel);
  bool usingCommonLabel() const { return m_useCommonLabel; }

  std::string valueLabel(std::size_t element) const;
  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

protected:
  RefItemDefinition(const std::string& myName);
  smtk::attribute::WeakDefinitionPtr m_definition;
  bool m_useCommonLabel;
  std::vector<std::string> m_valueLabels;
  std::size_t m_numberOfRequiredValues;

private:
};
}
}

#endif /* __smtk_attribute_RefItemDefinition_h */
