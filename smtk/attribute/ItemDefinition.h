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

#ifndef __smtk_attribute_ItemDefinition_h
#define __smtk_attribute_ItemDefinition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h" // For smtkTypeMacro.
#include "smtk/attribute/Item.h" // For Item Types.

#include <queue>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace smtk
{
namespace attribute
{
class Attribute;
class Item;
class GroupItemDefinition;
class SMTKCORE_EXPORT ItemDefinition
{
  friend class smtk::attribute::Definition;
  friend class smtk::attribute::GroupItemDefinition;
  friend class smtk::attribute::ValueItemDefinition;

public:
  smtkTypeMacroBase(ItemDefinition);
  // Temp structure used for copying definitions
  struct CopyInfo
  {
    // Reference to collection that is getting modified ("to")
    const smtk::attribute::Collection& ToCollection;
    // List of ValueItemDefinitions that reference expressions not currently in this collection
    std::queue<std::pair<std::string, smtk::attribute::ItemDefinitionPtr> > UnresolvedExpItems;
    // List of RefItemDefinitions that reference types not currently in this collection
    std::queue<std::pair<std::string, smtk::attribute::ItemDefinitionPtr> > UnresolvedRefItems;
    CopyInfo(const smtk::attribute::CollectionPtr mgr)
      : ToCollection(*mgr)
    {
    }
  };

  virtual ~ItemDefinition();
  const std::string& name() const { return this->m_name; }

  virtual Item::Type type() const = 0;
  // The label is what can be displayed in an application.  Unlike the type
  // which is constant w/r to the definition, an application can change the label
  const std::string& label() const { return (this->m_label != "" ? this->m_label : this->m_name); }

  void setLabel(const std::string& newLabel) { this->m_label = newLabel; }

  int version() const { return this->m_version; }
  void setVersion(int myVersion) { this->m_version = myVersion; }

  bool isOptional() const { return this->m_isOptional; }

  void setIsOptional(bool isOptionalValue) { this->m_isOptional = isOptionalValue; }

  // This only comes into play if the item is optional
  bool isEnabledByDefault() const { return this->m_isEnabledByDefault; }

  void setIsEnabledByDefault(bool isEnabledByDefaultValue)
  {
    this->m_isEnabledByDefault = isEnabledByDefaultValue;
  }

  std::size_t numberOfCategories() const { return this->m_categories.size(); }

  const std::set<std::string>& categories() const { return this->m_categories; }

  bool isMemberOf(const std::string& category) const
  {
    return (this->m_categories.find(category) != this->m_categories.end());
  }

  bool isMemberOf(const std::vector<std::string>& categories) const;

  virtual void addCategory(const std::string& category);

  virtual void removeCategory(const std::string& category);

  //Get the item definition's advance level:
  //if mode is 1 then the write access level is returned;
  //else the read access level is returned
  int advanceLevel(int mode = 0) const
  {
    return (mode == 1 ? this->m_advanceLevel[1] : this->m_advanceLevel[0]);
  }
  void setAdvanceLevel(int mode, int level);
  // Convinence Method that sets both read and write to the same value
  void setAdvanceLevel(int level);

  const std::string& detailedDescription() const { return this->m_detailedDescription; }
  void setDetailedDescription(const std::string& text) { this->m_detailedDescription = text; }

  const std::string& briefDescription() const { return this->m_briefDescription; }
  void setBriefDescription(const std::string& text) { this->m_briefDescription = text; }

  virtual smtk::attribute::ItemPtr buildItem(
    Attribute* owningAttribute, int itemPosition) const = 0;
  virtual smtk::attribute::ItemPtr buildItem(
    Item* owningItem, int position, int subGroupPosition) const = 0;
  virtual smtk::attribute::ItemDefinitionPtr createCopy(
    smtk::attribute::ItemDefinition::CopyInfo& info) const = 0;

protected:
  // The constructor must have the value for m_name passed
  // in because that should never change.
  ItemDefinition(const std::string& myname);
  void copyTo(ItemDefinitionPtr def) const;
  virtual void updateCategories();
  int m_version;
  bool m_isOptional;
  bool m_isEnabledByDefault;
  std::string m_label;
  std::set<std::string> m_categories;
  std::string m_detailedDescription;
  std::string m_briefDescription;

private:
  // constant value that should never be changed
  const std::string m_name;
  int m_advanceLevel[2];
};
}
}

#endif /* __smtk_attribute_ItemDefinition_h */
