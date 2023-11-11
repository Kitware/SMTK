//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_attribute_Definition_h
#define smtk_attribute_Definition_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h" // For smtkTypeMacroBase.

#include "smtk/attribute/Categories.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
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
namespace attribute
{
class Attribute;
class ItemDefinition;
class Resource;

/**\brief Stores the definition of an attribute.
  *
  * Stores all of the necessary information for a definition of a
  * single attribute. Instances of a definition should be created
  * through Resource::createAttribute().
  */
class SMTKCORE_EXPORT Definition : public smtk::enable_shared_from_this<Definition>
{
public:
  /// Return types for canBeAssociated method
  enum class AssociationResultType
  {
    Valid,       //!< Association is allowed.
    Illegal,     //!< Association with the given component type is disallowed.
    Conflict,    //!< An association with a mutually exclusive attribute already exists.
    Prerequisite //!< A prerequisite association does not yet exist.
  };

  smtkTypeMacroBase(smtk::attribute::Definition);
  struct SMTKCORE_EXPORT WeakDefinitionPtrCompare
  {
    bool operator()(
      const smtk::attribute::WeakDefinitionPtr& lhs,
      const smtk::attribute::WeakDefinitionPtr& rhs) const
    {
      auto left = lhs.lock();
      if (left == nullptr)
        return true;
      auto right = rhs.lock();
      if (right == nullptr)
        return false;
      return left->type() < right->type();
    }
  };

  typedef std::set<WeakDefinitionPtr, WeakDefinitionPtrCompare> WeakDefinitionSet;
  virtual ~Definition();

  // Description:
  // The type is the identifier that is used to access the
  // attribute definition through the Resource. It should never change.
  const std::string& type() const { return m_type; }

  smtk::attribute::ResourcePtr resource() const { return m_resource.lock(); }

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

  // Returns the label if set else it will return the type
  const std::string& displayedTypeName() const { return m_label.empty() ? m_type : m_label; }

  // The label is what can be displayed in an application.  Unlike the type
  // which is constant w/r to the definition, an application can change the label
  // By default it is set to the same value as the type.
  const std::string& label() const { return m_label; }

  void setLabel(const std::string& newLabel) { m_label = newLabel; }

  const smtk::attribute::DefinitionPtr& baseDefinition() const { return m_baseDefinition; }

  bool isA(smtk::attribute::ConstDefinitionPtr def) const;

  ///\brief Returns true if the definition is relevant.
  ///
  /// If includeCatagories is true and the definition does not pass it's category checks with respects
  /// to the resource's active category settings then return false,
  /// If includeReadAccess is true, and if all of the item definitions in the attribute have their
  ///  advanceLevel > readAccessLevel then return false.
  /// Else return true.
  bool isRelevant(
    bool includeCategories = true,
    bool includeReadAccess = false,
    unsigned int readAccessLevel = 0) const;

  int version() const { return m_version; }
  void setVersion(int myVersion) { m_version = myVersion; }

  bool isAbstract() const { return m_isAbstract; }

  void setIsAbstract(bool isAbstractValue) { m_isAbstract = isAbstractValue; }

  ///\brief Returns the categories (both explicitly assigned and inherited) associated to the Definition
  ///
  /// The categories that the attribute applies to. Typically
  /// a category will be a simulation type like heat transfer, fluid flow, etc.
  const smtk::attribute::Categories& categories() const { return m_categories; }

  ///\brief Determines how the Definition should combine its local category Set with the
  /// category constraints being inherited from it's Base Definition (if one exists)
  ///@{
  Categories::CombinationMode categoryInheritanceMode() const { return m_combinationMode; }
  void setCategoryInheritanceMode(Categories::CombinationMode mode) { m_combinationMode = mode; }
  ///@}

  ///\brief Returns the categories explicitly assigned to the Definition
  smtk::attribute::Categories::Set& localCategories() { return m_localCategories; }
  const smtk::attribute::Categories::Set& localCategories() const { return m_localCategories; }

  ///\brief Sets the local categories.
  ///
  /// This method is intended for use by Python applications, because Python code cannot
  /// manipulate the reference returned by the localCategories() method.
  void setLocalCategories(const smtk::attribute::Categories::Set& catSet)
  {
    m_localCategories = catSet;
  }

  /**
   * @brief Given a container, filter item definitions in the definition by a lambda function
   * @param values a container which holds definitions
   * @param test a lambda function which would be applied on children item definitions
   * Example filter double and int item definitions
   *  [](Item::Ptr item) { return item->type() == DOUBLE || item->type() == INT; }
   * Example filter modelEntity item definitions
   *  [](ModelEntity::Ptr item) { return true; }
   */
  template<typename T>
  void filterItemDefinitions(T& values, std::function<bool(typename T::value_type)> test);

  /// \brief Get the Definition 's advance level
  ///
  /// if mode is 1 then the write access level is returned;
  /// else the read access level is returned
  /// The information can either be specificied directly to the definition
  /// using setLocalAdvanceLevel() or from the definition's base definition.
  /// If the definition does not have either a local advance level or a
  /// base definition, then 0 is returned.
  /// NOTE: This information is used in GUI only
  unsigned int advanceLevel(int mode = 0) const;
  void setLocalAdvanceLevel(int mode, unsigned int level);
  void setLocalAdvanceLevel(unsigned int level);
  unsigned int localAdvanceLevel(int mode = 0) const
  {
    return (mode == 1 ? m_localAdvanceLevel[1] : m_localAdvanceLevel[0]);
  }
  /// unsetLocalAdvanceLevel causes the definition to return its
  /// base definition advance level information for the specified mode when calling
  /// the advanceLevel(mode) method or 0 if there is no base definition
  void unsetLocalAdvanceLevel(int mode = 0);
  /// Returns true if the definition is returning its local
  /// advance level information
  bool hasLocalAdvanceLevelInfo(int mode = 0) const
  {
    return (mode == 1 ? m_hasLocalAdvanceLevelInfo[1] : m_hasLocalAdvanceLevelInfo[0]);
  }

  /// Indicates if a persistent object can have multiple attributes of this
  /// type associated with it (true means it can not)
  bool isUnique() const { return m_isUnique; }
  /// Setting isUnique to be true indicates that only one attribute of this
  /// defintion (or any definition derived from this) can be associated to a
  /// persistent object.
  void setIsUnique(bool isUniqueValue);

  /// Indicates if the attribute applies to the
  /// nodes of the analysis mesh
  bool isNodal() const { return m_isNodal; }
  void setIsNodal(bool isNodalValue) { m_isNodal = isNodalValue; }

  ///Color Specifications
  /// Color in the case the attribute does not exist on the model entity
  /// If the color has not been set and the def has no base definition it will
  /// return s_notApplicableBaseColor
  const double* notApplicableColor() const;
  void setNotApplicableColor(double r, double g, double b, double alpha);
  void setNotApplicableColor(const double* color)
  {
    this->setNotApplicableColor(color[0], color[1], color[2], color[3]);
  }
  /// By unsetting the color it is now inherited from the def's base definition
  void unsetNotApplicableColor() { m_isNotApplicableColorSet = false; }
  bool isNotApplicableColorSet() const { return m_isNotApplicableColorSet; }

  /// Default Color for attributes created from this definition -
  /// If the color has not been set and the def has no base definition it will
  /// return s_defaultBaseColor
  const double* defaultColor() const;
  void setDefaultColor(double r, double g, double b, double alpha);
  void setDefaultColor(const double* color)
  {
    this->setDefaultColor(color[0], color[1], color[2], color[3]);
  }
  /// By unsetting the color it is now inherited from the def's base definition
  void unsetDefaultColor() { m_isDefaultColorSet = false; }
  bool isDefaultColorSet() const { return m_isDefaultColorSet; }

  /**\brief Return the definition's rule that governs attribute associations.
    *
    * A ReferenceItemDefinition is used to store information about
    * the allowable associations that may be made between attributes
    * specified by this definition and model entities.
    *
    * The definition's list of acceptable resources/components
    * serves as a mask for allowable associations while the definition's
    * minimum and maximum number of associations can be used to indicate whether
    * an association is required, optional, and/or extensible.
    *
    * A Definition can inherit the association rule from its Base Definition
    * when it does not have a local association rule specified.
    */
  ConstReferenceItemDefinitionPtr associationRule() const;
  /// Return the local association rule if one is set
  ReferenceItemDefinitionPtr localAssociationRule() const;
  ///\brief Create a new local association rule (if needed) and returns it.
  ///
  /// If a non-empty \a name is provided **and** there is no pre-existing
  /// association rule, the newly-created instance will be given the \a name.
  ReferenceItemDefinitionPtr createLocalAssociationRule(const std::string& name = std::string());
  /// Set the local Association Rule for the definition that overrides the base definition rule
  virtual void setLocalAssociationRule(ReferenceItemDefinitionPtr);
  /// Returns the association mask used by the definition for model association
  /// Note that this may come from the base definition if there is no local
  /// association rule
  smtk::model::BitFlags associationMask() const;
  /// Sets the association mask - note that this will always create a local
  /// association rule
  void setLocalAssociationMask(smtk::model::BitFlags mask);
  /// Removes the local association rule
  void clearLocalAssociationRule();

  bool associatesWithVertex() const;
  bool associatesWithEdge() const;
  bool associatesWithFace() const;
  bool associatesWithVolume() const;
  bool associatesWithModel() const;
  bool associatesWithGroup() const;

  bool canBeAssociated(smtk::model::BitFlags maskType) const;
  /// Tests to see if attributes based on this definition can be
  /// associated with a persistent object - see the documentation
  /// for AssociationResultType for details on return values.
  /// If a conflict is found, conflictAtt is set to the conflicting attribute
  /// If a prerequisite is missing, prerequisiteDef is set to the
  /// missing requirement
  /// NOTE - testing is completed once a problem has been detected.  There maybe be
  /// other issues preventing association so this method may need be called multiple
  /// times
  AssociationResultType canBeAssociated(
    smtk::resource::ConstPersistentObjectPtr object,
    AttributePtr& conflictAtt,
    DefinitionPtr& prerequisiteDef) const;
  /// Check the association rules of the definition (and the definiion it derived from)
  /// to see if the object can be associated
  bool checkAssociationRules(smtk::resource::ConstPersistentObjectPtr object) const;
  /// Test to see if there is a conflict between this definition and attributes
  /// already associated to the object.  Returns the conflicting attribute if there is a conflict
  AttributePtr checkForConflicts(smtk::resource::ConstPersistentObjectPtr object) const;
  /// Test to see if there is a missing prerequisite attribute that would prevent attributes of
  /// this type from being associated to the object.  Returns the missing prerequisite definition
  DefinitionPtr checkForPrerequisites(smtk::resource::ConstPersistentObjectPtr object) const;

  /// Return all of the attributes associated with object that are derived from this definition
  std::set<AttributePtr> attributes(const smtk::resource::ConstPersistentObjectPtr& object) const;

  bool conflicts(smtk::attribute::DefinitionPtr definition) const;

  std::size_t numberOfItemDefinitions() const { return m_itemDefs.size() + m_baseItemOffset; }

  smtk::attribute::ItemDefinitionPtr itemDefinition(int ith) const;

  const std::vector<smtk::attribute::ItemDefinitionPtr>& localItemDefinitions() const
  {
    return m_itemDefs;
  }

  /// Item definitions are the definitions of what data is stored
  /// in the attribute. For example, an IntItemDefinition would store
  /// an integer value.
  bool addItemDefinition(smtk::attribute::ItemDefinitionPtr cdef);
  template<typename T>
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
      this->setItemDefinitionUnitsSystem(item);
      m_itemDefs.push_back(item);
      m_itemDefPositions[name] = static_cast<int>(n);
      this->updateDerivedDefinitions();
    }
    return item;
  }

  /// This method will only remove the specified ItemDefinition (if it exists)
  /// from the class internals. Only ItemDefinitions local to this Definition
  /// can be removed. Remove inherited ItemDefinitinos directy from the inherited
  /// type.
  ///
  /// Warning:
  /// It is up to the caller to ensure integrity of the attribute::Resource
  /// instance (e.g. Attribute instances of this Definition type need to be
  /// cleansed from the Resource).
  bool removeItemDefinition(ItemDefinitionPtr itemDef);

  int findItemPosition(const std::string& name) const;

  const std::string& detailedDescription() const { return m_detailedDescription; }
  void setDetailedDescription(const std::string& text) { m_detailedDescription = text; }

  const std::string& briefDescription() const { return m_briefDescription; }
  void setBriefDescription(const std::string& text) { m_briefDescription = text; }

  /// Build an attribute corresponding to this definition. If the
  /// attribute already has items, clear them out.
  void buildAttribute(smtk::attribute::Attribute* attribute) const;

  /// Sets and returns the root name to be used to construct the name for
  /// an attribute. This is used by the attribute resource when creating an
  /// attribute without specifying a name - by default it is set to be the
  /// type name of the definition
  void setRootName(const std::string& val) { m_rootName = val; }
  std::string rootName() const { return m_rootName; }

  ///This method resets the definition item offset - this is used by the
  /// resource when a definition is modified
  void resetItemOffset();
  std::size_t itemOffset() const { return m_baseItemOffset; }

  /// These methods are use primarily by I/O operations.  The include ID corresponds to
  /// the include directory information store in the attribute reosurce and is used
  /// when writing out the resource to use include files
  void setIncludeIndex(std::size_t index) { m_includeIndex = index; }

  std::size_t includeIndex() const { return m_includeIndex; }

  /// Since Exclusion Constraints are symmetric this method will
  /// also insert this "definiton" into def
  void addExclusion(smtk::attribute::DefinitionPtr def)
  {
    m_exclusionDefs.insert(def);
    def->m_exclusionDefs.insert(this->shared_from_this());
  }

  /// Since Exclusion Constriants are symmetric this method will also remove
  /// this "definition" from def.
  void removeExclusion(smtk::attribute::DefinitionPtr def);
  const WeakDefinitionSet exclusions() const { return m_exclusionDefs; }
  /// Return a list of sorted type names that exlude this type of attribute.
  std::vector<std::string> excludedTypeNames() const;

  void addPrerequisite(smtk::attribute::DefinitionPtr def);

  /// Returns true if the definition is used as a prerequisite
  bool isUsedAsAPrerequisite() const;

  void removePrerequisite(smtk::attribute::DefinitionPtr def);
  const WeakDefinitionSet prerequisites() const { return m_prerequisiteDefs; }
  /// Return a sort of list of type names that are prerequisite to the type
  /// of attribute
  std::vector<std::string> prerequisiteTypeNames() const;

  /// Return nullptr if def is not a prerequisite of this Definition else
  /// return the prerequisite definition that def is derived from
  smtk::attribute::ConstDefinitionPtr hasPrerequisite(
    smtk::attribute::ConstDefinitionPtr def) const;
  /// Returns true if the definition has prerequisites (which can be inherited)
  bool hasPrerequisites() const;

  ///\brief Indicates that the Definition's validity (and relevancy) does not
  /// depends on the Resource's set of active categories.
  ///
  ///  This is very useful for modeling information such as Analysis Definitions
  ///  since they set active categories but don't use categories themselves.
  ///@{
  bool ignoreCategories() const { return m_ignoreCategories; }
  void setIgnoreCategories(bool val) { m_ignoreCategories = val; }

protected:
  friend class smtk::attribute::Resource;
  /// AttributeDefinitions can only be created by an attribute resource
  Definition(
    const std::string& myType,
    smtk::attribute::DefinitionPtr myBaseDef,
    smtk::attribute::ResourcePtr myResource);

  void clearResource() { m_resource.reset(); }

  ///\brief apply the local categories of the definition and its items.
  /// inherited is an initial set passed down from the definition's base.
  void applyCategories(smtk::attribute::Categories::Stack inherited);

  /// This method updates derived definitions when this
  /// definition's items have been changed
  void updateDerivedDefinitions();

  ///\brief update the advance level information of the definition and its items.
  /// readLevelFromParent and writeLevelFromParent are the advance level information coming
  /// from the Definition's Base Definition.  The Definition's advance level member are set
  /// to these values if the Definition does not have local versions set.
  virtual void applyAdvanceLevels(
    const unsigned int& readLevelFromParent,
    const unsigned int& writeLevelFromParent);

  void setItemDefinitionUnitsSystem(const smtk::attribute::ItemDefinitionPtr& itemDef) const;

  smtk::attribute::WeakResourcePtr m_resource;
  int m_version;
  bool m_isAbstract;
  smtk::attribute::DefinitionPtr m_baseDefinition;
  std::string m_type;
  std::string m_label;
  bool m_isNodal;
  attribute::Categories::Set m_localCategories;
  attribute::Categories m_categories;
  bool m_hasLocalAdvanceLevelInfo[2];
  unsigned int m_localAdvanceLevel[2];
  unsigned int m_advanceLevel[2];
  WeakDefinitionSet m_exclusionDefs;
  WeakDefinitionSet m_prerequisiteDefs;
  /// Used to keep track of how many definitions are using this one as a prerequisite
  size_t m_prerequisiteUsageCount;
  std::vector<smtk::attribute::ItemDefinitionPtr> m_itemDefs;
  std::map<std::string, int> m_itemDefPositions;
  ///Is Unique indicates if more than one attribute of this type can be assigned to a
  /// model entity - this constraint is implimented by using adding the definition itself
  /// into its exclusion list
  bool m_isUnique;
  bool m_isRequired;
  bool m_isNotApplicableColorSet;
  bool m_isDefaultColorSet;
  bool m_ignoreCategories = false;
  smtk::attribute::ReferenceItemDefinitionPtr m_acceptsRules;

  std::string m_detailedDescription;
  std::string m_briefDescription;
  /// Used by the find method to calculate an item's position
  std::size_t m_baseItemOffset;
  std::string m_rootName;
  Tags m_tags;
  std::size_t m_includeIndex;
  Categories::CombinationMode m_combinationMode;

private:
  /// These colors are returned for base definitions w/o set colors
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

template<typename T>
void Definition::filterItemDefinitions(
  T& filtered,
  std::function<bool(typename T::value_type)> test)
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
} // namespace attribute
} // namespace smtk
#endif /* smtk_attribute_Definition_h */
