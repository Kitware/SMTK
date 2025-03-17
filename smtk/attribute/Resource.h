//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_attribute_Resource_h
#define smtk_attribute_Resource_h

#include "smtk/common/Factory.h"
#include "smtk/common/UUID.h"

#include "smtk/geometry/Resource.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Links.h"

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Analyses.h"
#include "smtk/attribute/AssociationRules.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/CopyAssignmentOptions.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryInfo.h"
#include "smtk/attribute/Evaluator.h"
#include "smtk/attribute/EvaluatorFactory.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/SymbolDependencyStorage.h"

#include "smtk/attribute/filter/GrammarInfo.h"

#include "smtk/common/Categories.h"

#include "smtk/string/Token.h"

#include "smtk/view/Configuration.h"

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace smtk
{

namespace io
{
class Logger;
};

namespace attribute
{
class Attribute;
class Definition;

typedef smtk::common::Factory<ItemDefinition, std::string> CustomItemDefinitionFactory;

/**\brief Store information about attribute definitions and instances.
  *
  * This subclass of smtk::geometry::Resource holds attribute data.
  * The file contains at least a schema (definitions and item-definitions)
  * but may also contain attribute instances that conform to the schema
  * as well as information about how to present the attribute system
  * through a series of views.
  *
  * This class inherits smtk::geometry::Resource so that attributes may
  * (if desired) provide geometric data via a plugin.
  * By default, no geometry will be available since attributes
  * model information that is not spatial in nature and are instead
  * associated with geometric components from other resources.
  */
class SMTKCORE_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::geometry::Resource>
{
public:
  smtkTypeMacro(smtk::attribute::Resource);
  smtkCreateMacro(smtk::attribute::Resource);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  // typedef referring to the parent resource.
  typedef smtk::geometry::Resource ParentResource;

  /// Link from an attribute to persistent objects
  /// which manifest, exhibit, or possess the attribute.
  static constexpr smtk::resource::Links::RoleType AssociationRole = -1;
  static constexpr smtk::resource::Links::RoleType associationRole()
  {
    return Resource::AssociationRole;
  }

  /// Link from an attribute item to persistent objects
  /// which the item contains or references.
  static constexpr smtk::resource::Links::RoleType ReferenceRole = -2;
  static constexpr smtk::resource::Links::RoleType referenceRole()
  {
    return Resource::ReferenceRole;
  }

  ~Resource() override;

  bool setUnitSystem(const shared_ptr<units::System>& unitSystem) override;
  SMTK_DEPRECATED_IN_NEXT("Use setUnitSystem instead.")
  bool setUnitsSystem(const shared_ptr<units::System>& unitsSystem) override
  {
    return this->setUnitSystem(unitsSystem);
  }

  smtk::attribute::DefinitionPtr createDefinition(
    const std::string& typeName,
    const std::string& baseTypeName = "",
    const smtk::common::UUID& id = smtk::common::UUID::null());

  smtk::attribute::DefinitionPtr createDefinition(
    const std::string& name,
    attribute::DefinitionPtr baseDefiniiton,
    const smtk::common::UUID& id = smtk::common::UUID::null());

  // Description:
  // For simplicity, only Definitions without any children can be currently
  // removed (external nodes).
  bool removeDefinition(smtk::attribute::DefinitionPtr def);

  // Description:
  // Provide a way to mark a resource enabled/disabled
  // so that we can hide it in certain contexts
  void setIsPrivate(bool isPrivateValue) { m_isPrivate = isPrivateValue; }
  bool isPrivate() const { return m_isPrivate; };

  /**\brief Get the separator used for new Attributes whose names are not unique
   */
  const std::string& defaultNameSeparator() const;
  /**\brief Reset the separator used for new Attributes whose names are not unique to to the default which is '-'.
   */
  void resetDefaultNameSeparator();
  /**\brief Set the separator used for new Attributes whose names are not unique
   */
  bool setDefaultNameSeparator(const std::string& separator);

  smtk::attribute::AttributePtr createAttribute(attribute::DefinitionPtr def);
  smtk::attribute::AttributePtr createAttribute(const std::string& type);
  smtk::attribute::AttributePtr createAttribute(
    const std::string& name,
    const std::string& type,
    const smtk::common::UUID& id = smtk::common::UUID::null());
  smtk::attribute::AttributePtr createAttribute(
    const std::string& name,
    attribute::DefinitionPtr def,
    const smtk::common::UUID& id = smtk::common::UUID::null());

  bool removeAttribute(smtk::attribute::AttributePtr att);
  smtk::attribute::AttributePtr findAttribute(const std::string& name) const;
  smtk::attribute::AttributePtr findAttribute(const smtk::common::UUID& id) const;
  smtk::attribute::AttributePtr findAttribute(
    const smtk::resource::ComponentPtr& comp,
    const smtk::resource::Links::RoleType& role) const;

  void addUniqueRoles(const std::set<smtk::resource::Links::RoleType>& roles);
  void addUniqueRole(const smtk::resource::Links::RoleType& role);
  const std::set<smtk::resource::Links::RoleType>& uniqueRoles() const;
  bool isRoleUnique(const smtk::resource::Links::RoleType& role) const;

  // given a resource component's UUID, return the resource component.
  smtk::resource::ComponentPtr find(const smtk::common::UUID& id) const override;

  /**\brief Given a std::string describing a query, return a functor for performing the query.
    *
    * Currently, the query string must be either empty, `*`, `any`, or of the form
    * `attribute[type='xxx']`, where `xxx` specifies the name of a definition that
    * the resulting attributes instantiate.
    * Note that if an attribute type `xxx` is provided, any attribute whose
    * definition inherits from `xxx` as a base will be included, not just
    * those whose immediate, concrete type is `xxx`.
    *
    * If the query string filters attributes by their definition types, note that
    * the definition must exist at the time that queryOperation() is called.
    * This requirement allows faster repeated evaluation of the query.
    */
  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string&) const override;
  static filter::GrammarInfo extractGrammarInfo(const std::string& s);
  ///\brief Given an attribute definition, construct a valid query to retrive attributes of that type
  static std::string createAttributeQuery(const smtk::attribute::DefinitionPtr& def);
  static std::string createAttributeQuery(const std::string& defType);
  // visit all components in the resource.
  void visit(smtk::resource::Component::Visitor&) const override;

  void findAttributes(const std::string& type, std::vector<smtk::attribute::AttributePtr>& result)
    const;
  std::vector<smtk::attribute::AttributePtr> findAttributes(const std::string& type) const;
  void findAttributes(
    smtk::attribute::DefinitionPtr def,
    std::vector<smtk::attribute::AttributePtr>& result) const;

  smtk::attribute::DefinitionPtr findDefinition(const std::string& type) const;
  smtk::attribute::DefinitionPtr findDefinition(const smtk::common::UUID& id) const;

  /// Return true if the Resource has a Definition with the requested type
  bool hasDefinition(const std::string& type) const;

  // Return a list of definitions that are not derived from another definition
  void findBaseDefinitions(std::vector<smtk::attribute::DefinitionPtr>& result) const;

  void derivedDefinitions(
    smtk::attribute::DefinitionPtr def,
    std::vector<smtk::attribute::DefinitionPtr>& result) const;

  void findAllDerivedDefinitions(
    smtk::attribute::DefinitionPtr def,
    bool concreteOnly,
    std::vector<smtk::attribute::DefinitionPtr>& result) const;

  void findDefinitionAttributes(
    const std::string& type,
    std::vector<smtk::attribute::AttributePtr>& result) const;
  void findDefinitions(unsigned long mask, std::vector<smtk::attribute::DefinitionPtr>& result)
    const;

  smtk::attribute::ConstDefinitionPtr findIsUniqueBaseClass(
    smtk::attribute::ConstDefinitionPtr attDef) const;

  bool rename(AttributePtr att, const std::string& newName);

  /// Changes the ID of an Attribute.
  ///
  /// If /a newId is currently in use, the Attribute's ID will not
  /// be changed and the method will return false.
  ///
  /// NOTE: Care must be taken when resetting the ID since this currently
  /// does not update the links that refer to the Attribute's original
  /// ID.
  bool resetId(AttributePtr att, const smtk::common::UUID& newId);

  // Access Analysis Information
  smtk::attribute::Analyses& analyses() { return m_analyses; }

  std::size_t numberOfAdvanceLevels() const { return m_advLevels.size(); }
  void addAdvanceLevel(int level, std::string label, const double* l_color = nullptr);
  const std::map<int, std::string>& advanceLevels() const { return m_advLevels; }
  // the color is expected in the format of double[4] - rgba
  const double* advanceLevelColor(int level) const;
  void setAdvanceLevelColor(int level, const double* l_color);

  std::string createUniqueName(const std::string& type) const;

  void finalizeDefinitions();

  ///@{
  ///\brief API for accessing Category information.
  ///
  /// These categories are specified when defining Definitions and are gathered
  /// as a result of calling finalizeDefinitions.
  std::size_t numberOfCategories() const { return m_categories.size(); }
  const std::set<std::string>& categories() const { return m_categories; }
  ///@}

  ///@{
  ///\brief API for setting and accessing Active Category information.
  ///
  /// Active Categories are used to determine attribute/item validity as well as item values.
  /// If m_ActiveCategoriesEnabled is true then active categories will be taken into consideration.
  void setActiveCategoriesEnabled(bool mode);
  bool activeCategoriesEnabled() const { return m_activeCategoriesEnabled; }
  void setActiveCategories(const std::set<std::string>& cats);
  const std::set<std::string>& activeCategories() const { return m_activeCategories; }
  ///@}

  bool passActiveCategoryCheck(const smtk::common::Categories::Expression& cats) const;
  bool passActiveCategoryCheck(const smtk::common::Categories& cats) const;

  void addView(smtk::view::ConfigurationPtr);
  smtk::view::ConfigurationPtr findView(const std::string& name) const;
  smtk::view::ConfigurationPtr findViewByType(const std::string& vtype) const;
  smtk::view::ConfigurationPtr findTopLevelView() const;
  std::vector<smtk::view::ConfigurationPtr> findTopLevelViews() const;
  const std::map<std::string, smtk::view::ConfigurationPtr>& views() const { return m_views; }

  ///@{
  ///\brief API for setting and accessing style information.
  ///
  /// A style is represented as a smtk::view::Configuration::Component and represents
  /// customizations for displaying the attribute in a GUI.
  void addStyle(const std::string& definitionType, smtk::view::Configuration::Component style);
  const smtk::view::Configuration::Component& findStyle(
    const smtk::attribute::DefinitionPtr& def,
    const std::string& styleName = "") const;
  const std::map<std::string, smtk::view::Configuration::Component>& findStyles(
    const smtk::attribute::DefinitionPtr& def) const;
  const std::map<std::string, std::map<std::string, smtk::view::Configuration::Component>>& styles()
    const
  {
    return m_styles;
  }
  ///@}

  /// \brief Return a set of resources associated to this attribute resource.
  smtk::resource::ResourceSet associations() const;

  /// \brief Add a resource to the set of associated resources, and return true if the
  /// association is successful.
  bool associate(const smtk::resource::ResourcePtr& resource);

  /// \brief Remove a resource from the set of associated resources, and return true if
  /// the disassociation is successful.
  bool disassociate(const smtk::resource::ResourcePtr& resource);

  /// \brief Returns true if the attribute resource has other resources associated with it
  bool hasAssociations() const;

  // Return the attributes that are associated on a PersistentObject
  std::set<AttributePtr> attributes(const smtk::resource::ConstPersistentObjectPtr& object) const;

  // true if the PersistentObject has attributes associated with it
  bool hasAttributes(const smtk::resource::ConstPersistentObjectPtr& object) const;

  bool hasAttributes() const { return !m_attributes.empty(); }

  void disassociateAllAttributes(const smtk::resource::PersistentObjectPtr& object);

  // When a definition's items has changed use this method to update derived def
  // item offsets which is used by the find item method
  void updateDerivedDefinitionIndexOffsets(smtk::attribute::DefinitionPtr def);

  // Copies definition from another Resource
  smtk::attribute::DefinitionPtr copyDefinition(
    smtk::attribute::DefinitionPtr def,
    unsigned int options = 0);

  /// @{
  ///\brief  Copies an attribute.
  ///
  /// Will create a copy of an attribute.  Note that the source attribute can belong to
  /// a different attribute resource.  The logger will contain any information including warnings
  /// or errors encountered in the copying/assignment process.  In the version that does not take in
  /// a logger, smtk::io::Logger::instance() will be used.  Note that the source attribute does not
  /// need to be a component of the attribute resource holding the copy.
  /// If errors occur that prevent the copy process from successfully completing, no attribute will
  /// be created.  - see CopyAssignmentOptions.h for attribute and item assignment/copy options.
  smtk::attribute::AttributePtr copyAttribute(
    const smtk::attribute::AttributePtr& att,
    const CopyAssignmentOptions& options,
    smtk::io::Logger& logger);

  smtk::attribute::AttributePtr copyAttribute(
    const smtk::attribute::AttributePtr& att,
    const CopyAssignmentOptions& options = CopyAssignmentOptions());
  /// @}

  //Get a list of all definitions in the Resource
  void definitions(std::vector<smtk::attribute::DefinitionPtr>& result, bool sortList = false)
    const;
  //Get a list of all attributes in the Resource
  void attributes(std::vector<smtk::attribute::AttributePtr>& result) const;

  smtk::attribute::EvaluatorFactory& evaluatorFactory() { return m_evaluatorFactory; }

  const smtk::attribute::EvaluatorFactory& evaluatorFactory() const { return m_evaluatorFactory; }

  bool canEvaluate(smtk::attribute::ConstAttributePtr att)
  {
    return !!m_evaluatorFactory.createEvaluator(att);
  }

  std::unique_ptr<smtk::attribute::Evaluator> createEvaluator(
    smtk::attribute::ConstAttributePtr att)
  {
    return m_evaluatorFactory.createEvaluator(att);
  }

  smtk::attribute::SymbolDependencyStorage& symbolDependencyStorage()
  {
    return queries().cache<smtk::attribute::SymbolDependencyStorage>();
  }

  // Set/Get the directory structure of the resource on disk
  void setDirectoryInfo(const DirectoryInfo& dinfo) { m_directoryInfo = dinfo; }
  const DirectoryInfo& directoryInfo() const { return m_directoryInfo; }

  // Access the factory for generating custom item defnitions
  CustomItemDefinitionFactory& customItemDefinitionFactory()
  {
    return m_customItemDefinitionFactory;
  }
  const CustomItemDefinitionFactory& customItemDefinitionFactory() const
  {
    return m_customItemDefinitionFactory;
  }

  AssociationRules& associationRules() { return m_associationRules; }
  const AssociationRules& associationRules() const { return m_associationRules; }

  class GuardedLinks
  {
  public:
    GuardedLinks(std::mutex& mutex, const smtk::resource::Resource::Links& links)
      : m_guard(mutex)
      , m_links(links)
    {
    }

    const smtk::resource::Resource::Links* operator->() const { return &m_links; }

    smtk::resource::Resource::Links* operator->()
    {
      return const_cast<smtk::resource::Resource::Links*>(&m_links);
    }

  private:
    std::unique_lock<std::mutex> m_guard;
    const smtk::resource::Resource::Links& m_links;
  };

  // Attributes are uniquely used outside of an operation context, where they
  // are not guarded from concurrency issues. Specifically, ReferenceItems use
  // ResourceLinks to store references to other resources, and the
  // Resource::Links and Component::Links API is not thread-safe. This API
  // ensures thread safety when manipulating smtk::attribute::(Resource,Attribute
  // Links.
  const GuardedLinks guardedLinks() const;
  GuardedLinks guardedLinks();

  std::mutex& mutex() const { return m_mutex; }

  /// Set/get the "type" of a resource's template.
  ///
  /// A resource template-type is not required, but if present it can be used to
  /// register updaters for migrating from an old template to a newer version.
  bool setTemplateType(const smtk::string::Token& templateType) override;
  smtk::string::Token templateType() const override { return m_templateType; }

  /// Set/get the version of the template this instance of the resource is based upon.
  ///
  /// If non-zero, this number indicates the version number of the
  /// template (i.e., SBT file) the definitions in the current resource
  /// are drawn from. It is used during the update process to determine
  /// which updaters are applicable.
  bool setTemplateVersion(std::size_t templateVersion) override;
  std::size_t templateVersion() const override { return m_templateVersion; }

  ///\brief Create an empty, un-managed clone of this resource instance's meta information.
  ///
  /// If \a options has copyTemplateData() set to true, then this resource's
  /// Definition instances will be copied to the output resources.
  /// In addition, unitSystem and Analysis information is copied.
  std::shared_ptr<smtk::resource::Resource> clone(
    smtk::resource::CopyOptions& options) const override;

  ///\brief Copy data from the \a other resource into this resource, as specified by \a options.
  ///
  /// In the case of attribute resources - only the structure defined by the source's attributes
  /// are copied (note their items' values), as well as properties, views, geometry (if any),
  /// and active category information.
  /// Note that the following attribute copy options will always be assumed to be set and cannot
  /// be overridden by \a options:
  ///   CopyUUID(false)
  ///   PerformAssignment(false)
  ///   CopyDefinition(false)

  bool copyInitialize(
    const std::shared_ptr<const smtk::resource::Resource>& other,
    smtk::resource::CopyOptions& options) override;

  ///\brief Copy attribute resource contents.
  ///
  /// Besides items' values this includes:
  ///    Relations (associations, references) from the \a source resource
  ///    into this resource, as specified by \a options.
  ///    Associated resource information.
  ///    Link information
  ///
  /// Note that since the structure of the resource is assumed to be copied
  /// in copyFinalize, the attribute copy option DisableCopyAttributes always set to true.
  bool copyFinalize(
    const std::shared_ptr<const smtk::resource::Resource>& source,
    smtk::resource::CopyOptions& options) override;

  /// Copy View Information from the \a source resource
  /// into this resource, as specified by \a options.
  void copyViews(
    const std::shared_ptr<const smtk::attribute::Resource>& source,
    smtk::resource::CopyOptions& options);
  /// Update ID information of a View Component using the object mapping (if provided)
  ///
  /// If /a comp contains an ID attribute and it maps to a different persistent object,
  /// the mapped object's id will be substituted in /a comp.
  void updateViewComponentIdAttributes(
    smtk::view::Configuration::Component& comp,
    smtk::resource::CopyOptions& options);

protected:
  Resource(const smtk::common::UUID& myID, smtk::resource::ManagerPtr manager);
  Resource(smtk::resource::ManagerPtr manager = nullptr);
  void internalFindAllDerivedDefinitions(
    smtk::attribute::DefinitionPtr def,
    bool onlyConcrete,
    std::vector<smtk::attribute::DefinitionPtr>& result) const;
  void internalFindAttributes(
    attribute::DefinitionPtr def,
    std::vector<smtk::attribute::AttributePtr>& result) const;
  bool copyDefinitionImpl(
    smtk::attribute::DefinitionPtr sourceDef,
    smtk::attribute::ItemDefinition::CopyInfo& info);

  std::map<std::string, smtk::attribute::DefinitionPtr> m_definitions;
  std::map<std::string, std::set<smtk::attribute::AttributePtr, Attribute::CompareByName>>
    m_attributeClusters;
  std::map<std::string, smtk::attribute::AttributePtr> m_attributes;
  std::map<smtk::common::UUID, smtk::attribute::AttributePtr> m_attributeIdMap;
  std::map<smtk::common::UUID, smtk::attribute::DefinitionPtr> m_definitionIdMap;

  std::map<
    smtk::attribute::DefinitionPtr,
    std::set<smtk::attribute::WeakDefinitionPtr, Definition::WeakDefinitionPtrCompare>>
    m_derivedDefInfo;
  std::set<std::string> m_categories;
  std::set<std::string> m_activeCategories;
  bool m_activeCategoriesEnabled = false;
  smtk::attribute::Analyses m_analyses;
  std::map<std::string, smtk::view::ConfigurationPtr> m_views;
  std::map<std::string, std::map<std::string, smtk::view::Configuration::Component>> m_styles;

  // Advance levels, <int-level, <string-label, color[4]>
  // higher level means more advanced.
  std::map<int, std::string> m_advLevels;
  std::map<int, std::vector<double>> m_advLevelColors;
  DirectoryInfo m_directoryInfo;
  std::set<smtk::resource::Links::RoleType> m_roles;

  CustomItemDefinitionFactory m_customItemDefinitionFactory;

  AssociationRules m_associationRules;

  bool m_isPrivate = false;

  EvaluatorFactory m_evaluatorFactory;

  std::string m_defaultAttNameSeparator = "-";

  smtk::string::Token m_templateType;
  std::size_t m_templateVersion = 0;

private:
  mutable std::mutex m_mutex;
};

inline smtk::view::ConfigurationPtr Resource::findView(const std::string& name) const
{
  std::map<std::string, smtk::view::ConfigurationPtr>::const_iterator it;
  it = m_views.find(name);
  return (it == m_views.end()) ? smtk::view::ConfigurationPtr() : it->second;
}

inline smtk::attribute::AttributePtr Resource::findAttribute(const std::string& name) const
{
  std::map<std::string, AttributePtr>::const_iterator it;
  it = m_attributes.find(name);
  return (it == m_attributes.end()) ? smtk::attribute::AttributePtr() : it->second;
}

inline smtk::attribute::AttributePtr Resource::findAttribute(const smtk::common::UUID& attId) const
{
  std::map<smtk::common::UUID, AttributePtr>::const_iterator it;
  it = m_attributeIdMap.find(attId);
  return (it == m_attributeIdMap.end()) ? smtk::attribute::AttributePtr() : it->second;
}

inline smtk::attribute::DefinitionPtr Resource::findDefinition(const std::string& typeName) const
{
  std::map<std::string, smtk::attribute::DefinitionPtr>::const_iterator it;
  it = m_definitions.find(typeName);
  return (it == m_definitions.end()) ? smtk::attribute::DefinitionPtr() : it->second;
}

inline smtk::attribute::DefinitionPtr Resource::findDefinition(
  const smtk::common::UUID& defId) const
{
  std::map<smtk::common::UUID, DefinitionPtr>::const_iterator it;
  it = m_definitionIdMap.find(defId);
  return (it == m_definitionIdMap.end()) ? smtk::attribute::DefinitionPtr() : it->second;
}

inline bool Resource::hasDefinition(const std::string& typeName) const
{
  auto it = m_definitions.find(typeName);
  return (it != m_definitions.end());
}

inline void Resource::findDefinitionAttributes(
  const std::string& typeName,
  std::vector<smtk::attribute::AttributePtr>& result) const
{
  result.clear();
  auto it = m_attributeClusters.find(typeName);
  if (it != m_attributeClusters.end())
  {
    result.insert(result.end(), it->second.begin(), it->second.end());
  }
}

inline void Resource::findAttributes(
  const std::string& type,
  std::vector<smtk::attribute::AttributePtr>& result) const
{
  result.clear();
  smtk::attribute::DefinitionPtr def = this->findDefinition(type);
  if (def)
  {
    this->internalFindAttributes(def, result);
  }
}

inline std::vector<smtk::attribute::AttributePtr> Resource::findAttributes(
  const std::string& type) const
{
  std::vector<smtk::attribute::AttributePtr> result;
  this->findAttributes(type, result);
  return result;
}

} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_Resource_h */
