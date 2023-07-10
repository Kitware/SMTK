//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DoubleItemDefinition.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_DoubleItemDefinition_h
#define smtk_attribute_DoubleItemDefinition_h

#include "smtk/attribute/ValueItemDefinitionTemplate.h"

namespace smtk
{
namespace attribute
{
class SMTKCORE_EXPORT DoubleItemDefinition : public ValueItemDefinitionTemplate<double>
{
public:
  smtkTypeMacro(smtk::attribute::DoubleItemDefinition);
  static smtk::attribute::DoubleItemDefinitionPtr New(const std::string& myName)
  {
    return smtk::attribute::DoubleItemDefinitionPtr(new DoubleItemDefinition(myName));
  }

  ~DoubleItemDefinition() override;
  Item::Type type() const override;

  using ValueItemDefinitionTemplate<double>::setDefaultValue;
  bool setDefaultValue(const std::vector<double>& vals) override;
  bool setDefaultValue(const double& val, const std::string& units);
  bool setDefaultValue(const std::vector<double>& vals, const std::string& units);

  bool setDefaultValueAsString(const std::string& val);
  bool setDefaultValueAsString(const std::vector<std::string>& vals);
  const std::string defaultValueAsString(std::size_t element = 0) const;
  const std::vector<std::string> defaultValuesAsStrings() const;

  smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition) const override;
  smtk::attribute::ItemPtr buildItem(Item* owningItem, int position, int subGroupPosition)
    const override;

  smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const override;

  bool setUnits(const std::string& newUnits) override;

  /** \brief Splits input string into 2 parts with first part representing double value.
   *
   * Returns true if double was found.
   */
  static bool splitStringStartingDouble(const std::string&, std::string&, std::string&);

protected:
  DoubleItemDefinition(const std::string& myName);
  bool reevaluateDefaults();

  std::vector<std::string> m_defaultValuesAsStrings;

private:
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_DoubleItemDefinition_h */
