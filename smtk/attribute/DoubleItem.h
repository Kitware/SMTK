//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME DoubleItem.h -
// .SECTION Description
// .SECTION See Also

#ifndef smtk_attribute_DoubleItem_h
#define smtk_attribute_DoubleItem_h

#include "smtk/CoreExports.h"
#include "smtk/attribute/ValueItemTemplate.h"

namespace smtk
{
namespace attribute
{
class Attribute;
class DoubleItemDefinition;
class SMTKCORE_EXPORT DoubleItem : public ValueItemTemplate<double>
{
  friend class DoubleItemDefinition;

public:
  smtkTypeMacro(smtk::attribute::DoubleItem);
  ~DoubleItem() override;
  Item::Type type() const override;

  using Item::assign;
  // Assigns this item to be equivalent to another.  Options are processed by derived item classes
  // Returns true if success and false if a problem occurred.  By default, an attribute being used by this
  // to represent an expression will be copied if needed.  Use itemOptions.setIgnoreExpressions option to prevent this
  // When an expression attribute is copied, its  associations are by default not.
  // Use attributeOptions.setCopyAssociations option if you want them copied as well.These options are defined in CopyAssigmentOptions.h .
  Item::Status assign(
    const smtk::attribute::ConstItemPtr& sourceItem,
    const CopyAssignmentOptions& options,
    smtk::io::Logger& logger) override;

  using ValueItemTemplate<double>::value;
  double value(std::size_t element, smtk::io::Logger& log) const override;

  using ValueItemTemplate<double>::setValue;
  ///@{
  /// \brief Sets a value of the Item in the units defined in its definition.
  /// If the definition has no units set then the value is assumed to be unit-less
  bool setValue(std::size_t element, const double& val) override;
  ///@}
  /// \brief Sets a value of the Item in the units specified.
  /// If the units differ from its definition and can not be converted
  /// (or if the converted value is not valid) then value will not be changed and the method will fail
  bool setValue(std::size_t element, const double& val, const std::string& units);
  using ValueItem::setValueFromString;
  /// \brief Sets a value from a string.
  /// Note that the string can consist of a value followed by an option units string.
  bool setValueFromString(std::size_t element, const std::string& val) override;
  bool appendValue(const double& val) override;
  bool removeValue(std::size_t element) override;
  bool setNumberOfValues(std::size_t newSize) override;
  bool rotate(std::size_t fromPosition, std::size_t toPosition) override;
  bool setToDefault(std::size_t element = 0) override;

  using ValueItem::valueAsString;
  /// \brief Return the value as a string.
  /// Note that the string will be based on the value set via setValue or setValueFromString
  /// and not the converted value.
  std::string valueAsString(std::size_t element) const override;

  ///\brief Explicitly set the units of the item.
  ///
  /// This can only be done if the item's definition does not already specify units.
  /// Changing the units will replace the units associated with the item's value strings
  /// and can result in the item's values changing.  This can happen if the string values
  /// originally needed to be converted.
  bool setUnits(const std::string& newUnits);

  ///\brief Return the units the item's values are specified in.
  ///
  /// They can come from either the item's definition or from being explicitly assigned
  const std::string& units() const override;

  ///\brief Returns true if the item's units were explicitly assigned to the item.
  bool hasExplicitUnits() const { return !m_units.empty(); }

protected:
  DoubleItem(Attribute* owningAttribute, int itemPosition);
  DoubleItem(Item* owningItem, int myPosition, int mySubGroupPosition);
  bool initializeValues() override;
  void updateDiscreteValue(std::size_t element) override;

  std::vector<std::string> m_valuesAsString;
  std::string m_units;

private:
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_DoubleItem_h */
