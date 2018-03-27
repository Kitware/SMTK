//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Definition.h - stores the definition of an attribute.
// .SECTION Description
// Stores all of the necessary information for a definition of a
// single attribute. Attributes should be created through
// Collection::createAttribute().
// .SECTION See Also

#ifndef __smtk_attribute_Definition_h
#define __smtk_attribute_Definition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h" // For smtkTypeMacroBase.

#include "smtk/attribute/Tag.h"

#include "smtk/model/EntityRef.h"      //for EntityRef version of canBeAssociated
#include "smtk/model/EntityTypeBits.h" // for BitFlags type

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace smtk
{
namespace model
{
class Item;
}

namespace attribute
{
class Attribute;
class ItemDefinition;
class Collection;

class SMTKCORE_EXPORT Definition : public smtk::enable_shared_from_this<Definition>
{
public:
  smtkTypeMacroBase(smtk::attribute::Definition);
  virtual ~Definition();

  // Description:
  // The type is the identifier that is used to access the
  // attribute definition through the Collection. It should never change.
  const std::string& type() const { return m_type; }

  smtk::attribute::CollectionPtr collection() const { return m_collection.lock(); }

  const Tags& tags() const { return m_tags; }

  // Return a pointer to a const tag with a given name. If the tag does not
  // exist, return a null pointer.
  const Tag* tag(const std::string& name) const;
  Tag* tag(const std::string& name);

  bool addTag(const Tag& tag);
  bool removeTag(const std::string& name);

  // The label is what can be displayed in an application.  Unlike the type
  // which is constant w/r to the definition, an application can change the label
  // By default it is set to the same value as the type.
  const std::string& label() const { return m_label; }

  void setLabel(const std::string& newLabel) { m_label = newLabel; }

  smtk::attribute::DefinitionPtr baseDefinition() const { return m_baseDefinition; }

  bool isA(smtk::attribute::ConstDefinitionPtr def) const;

  int version() const { return m_version; }
  void setVersion(int myVersion) { m_version = myVersion; }

  bool isAbstract() const { return m_isAbstract; }

  void setIsAbstract(bool isAbstractValue) { m_isAbstract = isAbstractValue; }

  // The categories that the attribute applies to. Typically
  // a category will be a simulation type like heat transfer, fluid flow, etc.
  std::size_t numberOfCategories() const { return m_categories.size(); }

  bool isMemberOf(const std::string& category) const
  {
    return (m_categories.find(category) != m_categories.end());
  }

  bool isMemberOf(const std::vector<std::string>& categories) const;

  const std::set<std::string>& categories() const { return m_categories; }

  /**
   * @brief Given a container, filter item definitions in the definition by a lambda function
   * @param values a container which holds definitions
   * @param test a lambda function which would be applied on children item definitions
   * Example filter double and int item definitions
   *  [](Item::Ptr item) { return item->type() == DOUBLE || item->type() == INT; }
   * Example filter modelEntity item definitions
   *  [](ModelEntity::Ptr item) { return true; }
   */
  template <typename T>
  void filterItemDefinitions(T& values, std::function<bool(typename T::value_type)> test);

  // Description:
  // The attributes advance level. 0 is the simplest.
  int advanceLevel() const { return m_advanceLevel; }
  void setAdvanceLevel(int level) { m_advanceLevel = level; }

  // Indicates if a model entity can have multiple attributes of this
  // type associated with it
  bool isUnique() const { return m_isUnique; }
  // Be careful with setting isUnique to be false
  // in order to be consistant all definitions that this is
  // a descendant of should also have isUnique set to false!!
  // isUnique can be set to true without requiring its parent
  // class to also be true.
  void setIsUnique(bool isUniqueValue) { m_isUnique = isUniqueValue; }

  // Indicates if the attribute applies to the
  // nodes of the analysis mesh
  bool isNodal() const { return m_isNodal; }
  void setIsNodal(bool isNodalValue) { m_isNodal = isNodalValue; }

  //Color Specifications
  // Color in the case the attribute does not exist on the model entity
  // If the color has not been set and the def has no base definition it will
  // return s_notApplicableBaseColor
  const double* notApplicableColor() const;
  void setNotApplicableColor(double r, double g, double b, double alpha);
  void setNotApplicableColor(const double* color)
  {
    this->setNotApplicableColor(color[0], color[1], color[2], color[3]);
  }
  // By unsetting the color it is now inherited from the def's base definition
  void unsetNotApplicableColor() { m_isNotApplicableColorSet = false; }
  bool isNotApplicableColorSet() const { return m_isNotApplicableColorSet; }

  // Default Color for attributes created from this definition -
  // If the color has not been set and the def has no base definition it will
  // return s_defaultBaseColor
  const double* defaultColor() const;
  void setDefaultColor(double r, double g, double b, double alpha);
  void setDefaultColor(const double* color)
  {
    this->setDefaultColor(color[0], color[1], color[2], color[3]);
  }
  // By unsetting the color it is now inherited from the def's base definition
  void unsetDefaultColor() { m_isDefaultColorSet = false; }
  bool isDefaultColorSet() const { return m_isDefaultColorSet; }
  // return the asscoiationRule that the definition will use when creating
  // the attribute - Note that if the definition does not have a local
  //association rule specified, its base definition  will be returned.  If the
  // definition does not have a base definition then a local one is created
  ConstModelEntityItemDefinitionPtr associationRule() const;
  // sets an association rule that overides the base definition rule
  ModelEntityItemDefinitionPtr localAssociationRule() const;
  // Create a new local association rule (if needed) and returns it
  ModelEntityItemDefinitionPtr createLocalAssociationRule();
  // Local the local Assoicate Rule for the definition
  virtual void setLocalAssociationRule(ModelEntityItemDefinitionPtr);
  // Returns the association mask used by the definition for model association
  //Note that this may come from the base definition if there is no local
  //association rule
  smtk::model::BitFlags associationMask() const;
  //Sets the association mask - note that this will always create a local
  //association rule
  void setLocalAssociationMask(smtk::model::BitFlags mask);
  //Removes the local association rule
  void clearLocalAssociationRule();

  bool associatesWithVertex() const;
  bool associatesWithEdge() const;
  bool associatesWithFace() const;
  bool associatesWithVolume() const;
  bool associatesWithModel() const;
  bool associatesWithGroup() const;

  bool canBeAssociated(smtk::model::BitFlags maskType) const;
  bool canBeAssociated(
    smtk::model::EntityRef entity, std::vector<smtk::attribute::Attribute*>* conflicts) const;

  bool conflicts(smtk::attribute::DefinitionPtr definition) const;

  std::size_t numberOfItemDefinitions() const { return m_itemDefs.size() + m_baseItemOffset; }

  smtk::attribute::ItemDefinitionPtr itemDefinition(int ith) const;

  const std::vector<smtk::attribute::ItemDefinitionPtr>& localItemDefinitions() const
  {
    return m_itemDefs;
  }

  // Description:
  // Item definitions are the definitions of what data is stored
  // in the attribute. For example, an IntItemDefinition would store
  // an integer value.
  bool addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef);
  template <typename T>
  typename smtk::internal::shared_ptr_type<T>::SharedPointerType addItemDefinition(
    const std::string& name)
  {
    typedef smtk::internal::shared_ptr_type<T> SharedTypes;
    typename SharedTypes::SharedPointerType item;

    // First see if there is a item by the same name
    if (this->findItemPosition(name) < 0)
    {
      std::size_t n = m_itemDefs.size();
      item = SharedTypes::RawPointerType::New(name);
      m_itemDefs.push_back(item);
      m_itemDefPositions[name] = static_cast<int>(n);
      this->updateDerivedDefinitions();
    }
    return item;
  }

  // Description:
  // This method will only remove the specified ItemDefinition (if it exists)
  // from the class internals. Only ItemDefinitions local to this Definition
  // can be removed. Remove inherited ItemDefinitinos directy from the inherited
  // type.
  //
  // Warning:
  // It is up to the caller to ensure integrity of the attribute::Collection
  // instance (e.g. Attribute instances of this Definition type need to be
  // cleansed from the collection).
  bool removeItemDefinition(ItemDefinitionPtr itemDef);

  int findItemPosition(const std::string& name) const;

  const std::string& detailedDescription() const { return m_detailedDescription; }
  void setDetailedDescription(const std::string& text) { m_detailedDescription = text; }

  const std::string& briefDescription() const { return m_briefDescription; }
  void setBriefDescription(const std::string& text) { m_briefDescription = text; }

  // Description:
  // Build an attribute corresponding to this definition. If the
  // attribute already has items, clear them out.
  void buildAttribute(smtk::attribute::Attribute* attribute) const;

  // Description:
  // Sets and returns the root name to be used to construct the name for
  // an attribute. This is used by the attribute collection when creating an
  // attribute without specifying a name - by default it is set to be the
  // type name of the definition
  void setRootName(const std::string& val) { m_rootName = val; }
  std::string rootName() const { return m_rootName; }

  //This method resets the definition item offset - this is used by the
  // collection when a definition is modified
  void resetItemOffset();
  std::size_t itemOffset() const { return m_baseItemOffset; }

protected:
  friend class smtk::attribute::Collection;
  // AttributeDefinitions can only be created by an attribute collection
  Definition(const std::string& myType, smtk::attribute::DefinitionPtr myBaseDef,
    smtk::attribute::CollectionPtr myCollection);

  void clearCollection() { m_collection.reset(); }

  void setCategories();

  // This method updates derived definitions when this
  // definition's items have been changed
  void updateDerivedDefinitions();

  smtk::attribute::WeakCollectionPtr m_collection;
  int m_version;
  bool m_isAbstract;
  smtk::attribute::DefinitionPtr m_baseDefinition;
  std::string m_type;
  std::string m_label;
  bool m_isNodal;
  std::set<std::string> m_categories;
  int m_advanceLevel;
  std::vector<smtk::attribute::ItemDefinitionPtr> m_itemDefs;
  std::map<std::string, int> m_itemDefPositions;
  //Is Unique indicates if more than one attribute of this type can be assigned to a
  // model entity - NOTE This can be inherited meaning that if the definition's Super definition
  // has isUnique = true it will also prevent an attribute from this definition being assigned if the
  // targeted model entity has an attribute derived from the Super Definition
  bool m_isUnique;
  bool m_isRequired;
  bool m_isNotApplicableColorSet;
  bool m_isDefaultColorSet;
  smtk::attribute::ModelEntityItemDefinitionPtr m_associationRule;

  std::string m_detailedDescription;
  std::string m_briefDescription;
  // Used by the find method to calculate an item's position
  std::size_t m_baseItemOffset;
  std::string m_rootName;
  Tags m_tags;

private:
  // These colors are returned for base definitions w/o set colors
  static double s_notApplicableBaseColor[4];
  static double s_defaultBaseColor[4];

  double m_notApplicableColor[4];
  double m_defaultColor[4];
};

inline void Definition::resetItemOffset()
{
  if (m_baseDefinition)
  {
    m_baseItemOffset = m_baseDefinition->numberOfItemDefinitions();
  }
}

inline const double* Definition::notApplicableColor() const
{
  if (m_isNotApplicableColorSet)
  {
    return m_notApplicableColor;
  }
  else if (m_baseDefinition)
  {
    return m_baseDefinition->notApplicableColor();
  }
  return s_notApplicableBaseColor;
}

inline void Definition::setNotApplicableColor(double r, double g, double b, double a)
{
  m_isNotApplicableColorSet = true;
  m_notApplicableColor[0] = r;
  m_notApplicableColor[1] = g;
  m_notApplicableColor[2] = b;
  m_notApplicableColor[3] = a;
}

inline const double* Definition::defaultColor() const
{
  if (m_isDefaultColorSet)
  {
    return m_defaultColor;
  }
  else if (m_baseDefinition)
  {
    return m_baseDefinition->defaultColor();
  }
  return s_defaultBaseColor;
}

inline void Definition::setDefaultColor(double r, double g, double b, double a)
{
  m_isDefaultColorSet = true;
  m_defaultColor[0] = r;
  m_defaultColor[1] = g;
  m_defaultColor[2] = b;
  m_defaultColor[3] = a;
}

template <typename T>
void Definition::filterItemDefinitions(
  T& filtered, std::function<bool(typename T::value_type)> test)
{

  auto conditionalAdd = [&](smtk::attribute::ItemDefinitionPtr item) {
    typename T::value_type testItem =
      smtk::dynamic_pointer_cast<typename T::value_type::element_type>(item);

    if (testItem && test(testItem))
    {
      filtered.insert(filtered.end(), testItem);
    }
  };

  std::for_each(m_itemDefs.begin(), m_itemDefs.end(), conditionalAdd);
}
}
}
#endif /* __smtk_attribute_Definition_h */
