//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME ItemDefinition.h - the definition of a value of an attribute definition.
// .SECTION Description
// ItemDefinition is meant to store definitions of values that can be
// stored inside of an attribute. Derived classes give specific
// types of items.
// .SECTION See Also

#ifndef smtk_attribute_ItemDefinition_h
#define smtk_attribute_ItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h" // For smtkTypeMacro.
#include "smtk/attribute/Categories.h"
#include "smtk/attribute/Item.h" // For Item Types.
#include "smtk/attribute/Tag.h"

#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace units
{
struct System;
}

namespace smtk
{
namespace attribute
{
class SMTKCORE_EXPORT ItemDefinition
{
  friend class Definition;
  friend class GroupItemDefinition;
  friend class ReferenceItemDefinition;
  friend class ValueItemDefinition;

public:
  smtkTypeMacroBase(smtk::attribute::ItemDefinition);
  // Temp structure used for copying definitions
  struct CopyInfo
  {
    // Reference to resource that is getting modified ("to")
    const smtk::attribute::Resource& ToResource;
    // List of ValueItemDefinitions that reference expressions not currently in this resource
    std::queue<std::pair<std::string, smtk::attribute::ItemDefinitionPtr>> UnresolvedExpItems;
    CopyInfo(const smtk::attribute::ResourcePtr resource)
      : ToResource(*resource)
    {
    }
  };

  virtual ~ItemDefinition();
  // The name used to access the item - this name is unique w/r to the attribute
  // or parent item
  const std::string& name() const { return m_name; }

  virtual Item::Type type() const = 0;
  // The label is what can be displayed in an application.  Unlike the type
  // which is constant w/r to the definition, an application can change the label
  const std::string& label() const { return (!m_label.empty() ? m_label : m_name); }

  void setLabel(const std::string& newLabel) { m_label = newLabel; }

  int version() const { return m_version; }
  void setVersion(int myVersion) { m_version = myVersion; }

  bool isOptional() const { return m_isOptional; }

  void setIsOptional(bool isOptionalValue) { m_isOptional = isOptionalValue; }

  // This only comes into play if the item is optional
  bool isEnabledByDefault() const { return m_isEnabledByDefault; }

  void setIsEnabledByDefault(bool isEnabledByDefaultValue)
  {
    m_isEnabledByDefault = isEnabledByDefaultValue;
  }

  ///\brief Returns the categories (both explicitly assigned and inherited) associated to the Item Definition
  ///
  /// The categories that the attribute applies to. Typically
  /// a category will be a simulation type like heat transfer, fluid flow, etc.
  const smtk::attribute::Categories& categories() const { return m_categories; }

  ///\brief Returns the categories::Expression explicitly assigned to the Items Definition
  smtk::attribute::Categories::Expression& localCategories() { return m_localCategories; }
  const smtk::attribute::Categories::Expression& localCategories() const
  {
    return m_localCategories;
  }

  ///\brief Sets the local categories.
  ///
  /// This method is intended for use by Python applications, because Python code cannot
  /// manipulate the reference returned by the localCategories() method.
  void setLocalCategories(const smtk::attribute::Categories::Expression& catExp)
  {
    m_localCategories = catExp;
  }

  ///\brief Determines how the Definition should combine its local category Set with the
  /// category constraints being inherited from it's owning Attribute or Item Definition
  ///@{
  Categories::CombinationMode categoryInheritanceMode() const { return m_combinationMode; }
  void setCategoryInheritanceMode(Categories::CombinationMode mode) { m_combinationMode = mode; }
  ///@}

  //Get the item definition's advance level:
  //if mode is 1 then the write access level is returned;
  //else the read access level is returned
  unsigned int advanceLevel(int mode = 0) const
  {
    return (mode == 1 ? m_advanceLevel[1] : m_advanceLevel[0]);
  }
  unsigned int localAdvanceLevel(int mode = 0) const
  {
    return (mode == 1 ? m_localAdvanceLevel[1] : m_localAdvanceLevel[0]);
  }
  void setLocalAdvanceLevel(int mode, unsigned int level);
  // Convenience Method that sets both read and write to the same value
  void setLocalAdvanceLevel(unsigned int level);

  // unsetAdvanceLevel causes the item to return its
  // definition advance level information for the specified mode when calling
  // the advanceLevel(mode) method
  void unsetLocalAdvanceLevel(int mode = 0);
  // Returns true if the item is returning its local
  // advance level information
  bool hasLocalAdvanceLevelInfo(int mode = 0) const
  {
    return (mode == 1 ? m_hasLocalAdvanceLevelInfo[1] : m_hasLocalAdvanceLevelInfo[0]);
  }
  const std::string& detailedDescription() const { return m_detailedDescription; }
  void setDetailedDescription(const std::string& text) { m_detailedDescription = text; }

  const std::string& briefDescription() const { return m_briefDescription; }
  void setBriefDescription(const std::string& text) { m_briefDescription = text; }

  ///\brief return the smtk::attribute::Tags associated with the Definition
  const Tags& tags() const { return m_tags; }

  ///@{
  ///\brief Return a pointer to a smtk::attribute::Tag with a given name. If the Tag does not
  /// exist, return a null pointer.
  const Tag* tag(const std::string& name) const;
  Tag* tag(const std::string& name);
  ///@}

  ///@{
  ///\brief Add/Remove a smtk::attribute::Tag from a Definition
  bool addTag(const Tag& tag);
  bool removeTag(const std::string& name);
  ///@}

  ///\brief Return the unitsSystem of the Definition
  const shared_ptr<units::System>& unitsSystem() const { return m_unitsSystem; }

  virtual smtk::attribute::ItemPtr buildItem(Attribute* owningAttribute, int itemPosition)
    const = 0;
  virtual smtk::attribute::ItemPtr buildItem(Item* owningItem, int position, int subGroupPosition)
    const = 0;
  virtual smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const = 0;

protected:
  // The constructor must have the value for m_name passed
  // in because that should never change.
  ItemDefinition(const std::string& myname);
  void copyTo(ItemDefinitionPtr def) const;
  virtual void applyCategories(
    const smtk::attribute::Categories::Stack& inheritedFromParent,
    smtk::attribute::Categories& inheritedToParent);
  virtual void applyAdvanceLevels(
    const unsigned int& readLevelFromParent,
    const unsigned int& writeLevelFromParent);

  ///\brief Set the unitsSystem of the Definition
  ///
  /// Note that this should be done before units are specified in the Definition
  virtual void setUnitsSystem(const shared_ptr<units::System>& unitsSystem);

  int m_version;
  bool m_isOptional;
  bool m_isEnabledByDefault;
  std::string m_label;
  attribute::Categories::Expression m_localCategories;
  attribute::Categories m_categories;
  std::string m_detailedDescription;
  std::string m_briefDescription;
  bool m_hasLocalAdvanceLevelInfo[2];
  unsigned int m_localAdvanceLevel[2];
  unsigned int m_advanceLevel[2];
  attribute::Tags m_tags;
  Categories::CombinationMode m_combinationMode;
  std::shared_ptr<units::System> m_unitsSystem;

private:
  // constant value that should never be changed
  const std::string m_name;
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_ItemDefinition_h */
