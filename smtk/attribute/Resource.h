//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Resource.h - the main class for storing attribute information
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_Resource_h
#define __smtk_attribute_Resource_h

#include "smtk/common/UUID.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Links.h"

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Analyses.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryInfo.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace smtk
{
namespace attribute
{
class Attribute;
class Definition;

/**\brief Store information about attribute definitions and instances.
  *
  * This subclass of smtk::resource::Resource holds attribute data.
  * The file contains at least a schema (definitions and item-definitions)
  * but may also contain attribute instances that conform to the schema
  * as well as information about how to present the attribute system
  * through a series of views.
  */
class SMTKCORE_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::resource::Resource>
{
public:
  smtkTypeMacro(smtk::attribute::Resource);
  smtkCreateMacro(smtk::attribute::Resource);
  smtkSharedFromThisMacro(smtk::resource::Resource);

  // typedef referring to the parent resource.
  typedef smtk::resource::Resource ParentResource;

  enum CopyOptions
  {
    COPY_ASSOCIATIONS =
      0x00000001, //!< Should associations and model-entity items be copied if models match?
    FORCE_COPY_ASSOCIATIONS =
      0x00000003 //!< Should associations and model-entity items *always* be copied?
  };

  // Associations and references to other resources and components are managed
  // internally using smtk::resource::Links. The concepts of association and
  // reference are internally very similar, but to the outward-facing API they
  // are treated separately and serve different functions. The storage for these
  // values are therefore logically separated by different role values.
  static constexpr smtk::resource::Links::RoleType AssociationRole = -1;
  static constexpr smtk::resource::Links::RoleType ReferenceRole = -2;

  ~Resource() override;

  smtk::attribute::DefinitionPtr createDefinition(
    const std::string& typeName, const std::string& baseTypeName = "");
  smtk::attribute::DefinitionPtr createDefinition(
    const std::string& name, attribute::DefinitionPtr baseDefiniiton);
  // Description:
  // For simplicity, only Definitions without any children can be currently
  // removed (external nodes).
  bool removeDefinition(smtk::attribute::DefinitionPtr def);

  smtk::attribute::AttributePtr createAttribute(const std::string& name, const std::string& type);
  smtk::attribute::AttributePtr createAttribute(attribute::DefinitionPtr def);
  smtk::attribute::AttributePtr createAttribute(const std::string& type);
  smtk::attribute::AttributePtr createAttribute(
    const std::string& name, attribute::DefinitionPtr def);
  bool removeAttribute(smtk::attribute::AttributePtr att);
  smtk::attribute::AttributePtr findAttribute(const std::string& name) const;
  smtk::attribute::AttributePtr findAttribute(const smtk::common::UUID& id) const;

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
  std::function<bool(const smtk::resource::ConstComponentPtr&)> queryOperation(
    const std::string&) const override;

  // visit all components in the resource.
  void visit(smtk::resource::Component::Visitor&) const override;

  void findAttributes(
    const std::string& type, std::vector<smtk::attribute::AttributePtr>& result) const;
  std::vector<smtk::attribute::AttributePtr> findAttributes(const std::string& type) const;
  void findAttributes(
    smtk::attribute::DefinitionPtr def, std::vector<smtk::attribute::AttributePtr>& result) const;
  smtk::attribute::DefinitionPtr findDefinition(const std::string& type) const;

  // Return a list of definitions that are not derived from another definition
  void findBaseDefinitions(std::vector<smtk::attribute::DefinitionPtr>& result) const;

  void derivedDefinitions(
    smtk::attribute::DefinitionPtr def, std::vector<smtk::attribute::DefinitionPtr>& result) const;

  void findAllDerivedDefinitions(smtk::attribute::DefinitionPtr def, bool concreteOnly,
    std::vector<smtk::attribute::DefinitionPtr>& result) const;

  void findDefinitionAttributes(
    const std::string& type, std::vector<smtk::attribute::AttributePtr>& result) const;
  void findDefinitions(
    unsigned long mask, std::vector<smtk::attribute::DefinitionPtr>& result) const;

  smtk::attribute::ConstDefinitionPtr findIsUniqueBaseClass(
    smtk::attribute::ConstDefinitionPtr attDef) const;

  bool rename(AttributePtr att, const std::string& newName);

  // Access Analysis Information
  smtk::attribute::Analyses& analyses() { return m_analyses; }

  std::size_t numberOfAdvanceLevels() const { return m_advLevels.size(); }
  void addAdvanceLevel(int level, std::string label, const double* l_color = 0);
  const std::map<int, std::string>& advanceLevels() const { return m_advLevels; }
  // the color is expected in the format of double[4] - rgba
  const double* advanceLevelColor(int level) const;
  void setAdvanceLevelColor(int level, const double* l_color);

  // For Reader classes
  smtk::attribute::AttributePtr createAttribute(
    const std::string& name, const std::string& type, const smtk::common::UUID& id);
  smtk::attribute::AttributePtr createAttribute(
    const std::string& name, attribute::DefinitionPtr def, const smtk::common::UUID& id);
  std::string createUniqueName(const std::string& type) const;

  void updateCategories();
  std::size_t numberOfCategories() const { return m_categories.size(); }
  const std::set<std::string>& categories() const { return m_categories; }

  void addView(smtk::view::ViewPtr);
  smtk::view::ViewPtr findView(const std::string& name) const;
  smtk::view::ViewPtr findViewByType(const std::string& vtype) const;
  smtk::view::ViewPtr findTopLevelView() const;
  std::vector<smtk::view::ViewPtr> findTopLevelViews() const;
  const std::map<std::string, smtk::view::ViewPtr>& views() const { return m_views; }

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

  bool hasAttributes() const { return m_attributes.size() > 0; }

  void disassociateAllAttributes(const smtk::resource::PersistentObjectPtr& object);

  // When a definition's items has changed use this method to update derived def
  // item offsets which is used by the find item method
  void updateDerivedDefinitionIndexOffsets(smtk::attribute::DefinitionPtr def);

  // Copies definition from another Resource
  smtk::attribute::DefinitionPtr copyDefinition(
    const smtk::attribute::DefinitionPtr def, unsigned int options = 0);
  // Copies attribute from another Resource
  // Note: that if the attribute is unique (meaning only 1 attribute of this type can be asociated
  // to a model entity, the copyModelAssociations flag is ignored since it would violate this constraint.
  // In terms of options - these are item assignment options - see Item.h for documentation.
  smtk::attribute::AttributePtr copyAttribute(const smtk::attribute::AttributePtr att,
    const bool& copyModelAssociations = false, const unsigned int& options = 0);

  //Get a list of all definitions in the Resource
  void definitions(
    std::vector<smtk::attribute::DefinitionPtr>& result, bool sortList = false) const;
  //Get a list of all attributes in the Resource
  void attributes(std::vector<smtk::attribute::AttributePtr>& result) const;

  // Set/Get the directory structure of the resource on disk
  void setDirectoryInfo(const DirectoryInfo& dinfo) { m_directoryInfo = dinfo; }
  const DirectoryInfo& directoryInfo() const { return m_directoryInfo; }

protected:
  Resource(const smtk::common::UUID& myID, smtk::resource::ManagerPtr manager);
  Resource(smtk::resource::ManagerPtr manager = nullptr);
  void internalFindAllDerivedDefinitions(smtk::attribute::DefinitionPtr def, bool onlyConcrete,
    std::vector<smtk::attribute::DefinitionPtr>& result) const;
  void internalFindAttributes(
    attribute::DefinitionPtr def, std::vector<smtk::attribute::AttributePtr>& result) const;
  bool copyDefinitionImpl(const smtk::attribute::DefinitionPtr sourceDef,
    smtk::attribute::ItemDefinition::CopyInfo& info);

  std::map<std::string, smtk::attribute::DefinitionPtr> m_definitions;
  std::map<std::string, std::set<smtk::attribute::AttributePtr, Attribute::CompareByName> >
    m_attributeClusters;
  std::map<std::string, smtk::attribute::AttributePtr> m_attributes;
  std::map<smtk::common::UUID, smtk::attribute::AttributePtr> m_attributeIdMap;

  std::map<smtk::attribute::DefinitionPtr,
    std::set<smtk::attribute::WeakDefinitionPtr, Definition::WeakDefinitionPtrCompare> >
    m_derivedDefInfo;
  std::set<std::string> m_categories;
  smtk::attribute::Analyses m_analyses;
  std::map<std::string, smtk::view::ViewPtr> m_views;

  // Advance levels, <int-level, <string-label, color[4]>
  // higher level means more advanced.
  std::map<int, std::string> m_advLevels;
  std::map<int, std::vector<double> > m_advLevelColors;
  DirectoryInfo m_directoryInfo;

private:
};

inline smtk::view::ViewPtr Resource::findView(const std::string& name) const
{
  std::map<std::string, smtk::view::ViewPtr>::const_iterator it;
  it = m_views.find(name);
  return (it == m_views.end()) ? smtk::view::ViewPtr() : it->second;
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

inline void Resource::findDefinitionAttributes(
  const std::string& typeName, std::vector<smtk::attribute::AttributePtr>& result) const
{
  result.clear();
  auto it = m_attributeClusters.find(typeName);
  if (it != m_attributeClusters.end())
  {
    result.insert(result.end(), it->second.begin(), it->second.end());
  }
}

inline void Resource::findAttributes(
  const std::string& type, std::vector<smtk::attribute::AttributePtr>& result) const
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

} // end attribute namepsace
} // end smtk namespace

#endif /* __smtk_attribute_Resource_h */
