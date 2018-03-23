//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Collection.h - the main class for storing attribute information
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_attribute_Collection_h
#define __smtk_attribute_Collection_h

#include "smtk/common/UUID.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h" // base class

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
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

class SMTKCORE_EXPORT Collection : public smtk::resource::Resource
{
public:
  smtkTypeMacro(smtk::attribute::Collection);
  smtkCreateMacro(smtk::attribute::Collection);
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

  ~Collection() override;

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

  // given a std::string describing a query, return a functor for performing the
  // query.
  std::function<bool(const smtk::resource::ComponentPtr&)> queryOperation(
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
    smtk::attribute::DefinitionPtr attDef) const;

  bool rename(AttributePtr att, const std::string& newName);
  bool defineAnalysis(const std::string& analysisName, const std::set<std::string>& categories);
  std::size_t numberOfAnalyses() const { return m_analyses.size(); }
  std::set<std::string> analysisCategories(const std::string& analysisType) const;
  const std::map<std::string, std::set<std::string> >& analyses() const { return m_analyses; }

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

  smtk::model::ManagerPtr refModelManager() const { return m_refModelMgr.lock(); }
  void setRefModelManager(smtk::model::ManagerPtr refModelMgr);

  bool hasAttributes() { return m_attributes.size() > 0; }

  // When a definition's items has changed use this method to update derived def
  // item offsets which is used by the find item method
  void updateDerivedDefinitionIndexOffsets(smtk::attribute::DefinitionPtr def);

  // Copies definition from another Collection
  smtk::attribute::DefinitionPtr copyDefinition(
    const smtk::attribute::DefinitionPtr def, unsigned int options = 0);
  // Copies attribute from another Collection
  // Note: that if the attribute is unique (meaning only 1 attribute of this type can be asociated
  // to a model entity, the copyModelAssociations flag is ignored since it would violate this constraint.
  // In terms of options - these are item assignment options - see Item.h for documentation.
  smtk::attribute::AttributePtr copyAttribute(const smtk::attribute::AttributePtr att,
    const bool& copyModelAssociations = false, const unsigned int& options = 0);

  //Get a list of all definitions in the Collection
  void definitions(std::vector<smtk::attribute::DefinitionPtr>& result) const;
  //Get a list of all attributes in the Collection
  void attributes(std::vector<smtk::attribute::AttributePtr>& result) const;

protected:
  Collection(const smtk::common::UUID& myID, smtk::resource::ManagerPtr manager);
  Collection(smtk::resource::ManagerPtr manager = nullptr);
  void internalFindAllDerivedDefinitions(smtk::attribute::DefinitionPtr def, bool onlyConcrete,
    std::vector<smtk::attribute::DefinitionPtr>& result) const;
  void internalFindAttributes(
    attribute::DefinitionPtr def, std::vector<smtk::attribute::AttributePtr>& result) const;
  bool copyDefinitionImpl(const smtk::attribute::DefinitionPtr sourceDef,
    smtk::attribute::ItemDefinition::CopyInfo& info);

  std::map<std::string, smtk::attribute::DefinitionPtr> m_definitions;
  std::map<std::string, std::set<smtk::attribute::AttributePtr> > m_attributeClusters;
  std::map<std::string, smtk::attribute::AttributePtr> m_attributes;
  std::map<smtk::common::UUID, smtk::attribute::AttributePtr> m_attributeIdMap;
  std::map<smtk::attribute::DefinitionPtr, smtk::attribute::WeakDefinitionPtrSet> m_derivedDefInfo;
  std::set<std::string> m_categories;
  std::map<std::string, std::set<std::string> > m_analyses;
  std::map<std::string, smtk::view::ViewPtr> m_views;

  smtk::model::WeakManagerPtr m_refModelMgr;
  // Advance levels, <int-level, <string-label, color[4]>
  // higher level means more advanced.
  std::map<int, std::string> m_advLevels;
  std::map<int, std::vector<double> > m_advLevelColors;

private:
};

inline smtk::view::ViewPtr Collection::findView(const std::string& name) const
{
  std::map<std::string, smtk::view::ViewPtr>::const_iterator it;
  it = m_views.find(name);
  return (it == m_views.end()) ? smtk::view::ViewPtr() : it->second;
}

inline smtk::attribute::AttributePtr Collection::findAttribute(const std::string& name) const
{
  std::map<std::string, AttributePtr>::const_iterator it;
  it = m_attributes.find(name);
  return (it == m_attributes.end()) ? smtk::attribute::AttributePtr() : it->second;
}

inline smtk::attribute::AttributePtr Collection::findAttribute(
  const smtk::common::UUID& attId) const
{
  std::map<smtk::common::UUID, AttributePtr>::const_iterator it;
  it = m_attributeIdMap.find(attId);
  return (it == m_attributeIdMap.end()) ? smtk::attribute::AttributePtr() : it->second;
}

inline smtk::attribute::DefinitionPtr Collection::findDefinition(const std::string& typeName) const
{
  std::map<std::string, smtk::attribute::DefinitionPtr>::const_iterator it;
  it = m_definitions.find(typeName);
  return (it == m_definitions.end()) ? smtk::attribute::DefinitionPtr() : it->second;
}

inline void Collection::findDefinitionAttributes(
  const std::string& typeName, std::vector<smtk::attribute::AttributePtr>& result) const
{
  result.clear();
  std::map<std::string, std::set<smtk::attribute::AttributePtr> >::const_iterator it;
  it = m_attributeClusters.find(typeName);
  if (it != m_attributeClusters.end())
  {
    result.insert(result.end(), it->second.begin(), it->second.end());
  }
}

inline void Collection::findAttributes(
  const std::string& type, std::vector<smtk::attribute::AttributePtr>& result) const
{
  result.clear();
  smtk::attribute::DefinitionPtr def = this->findDefinition(type);
  if (def)
  {
    this->internalFindAttributes(def, result);
  }
}

inline std::vector<smtk::attribute::AttributePtr> Collection::findAttributes(
  const std::string& type) const
{
  std::vector<smtk::attribute::AttributePtr> result;
  this->findAttributes(type, result);
  return result;
}

inline std::set<std::string> Collection::analysisCategories(const std::string& analysisType) const
{
  std::map<std::string, std::set<std::string> >::const_iterator it;
  it = m_analyses.find(analysisType);
  if (it != m_analyses.end())
  {
    return it->second;
  }
  return std::set<std::string>();
}

inline bool Collection::defineAnalysis(
  const std::string& analysisName, const std::set<std::string>& categoriesIn)
{
  std::map<std::string, std::set<std::string> >::const_iterator it;
  it = m_analyses.find(analysisName);
  if (it != m_analyses.end())
  {
    // it already exists
    return false;
  }
  m_analyses[analysisName] = categoriesIn;
  return true;
}
}
}

#endif /* __smtk_attribute_Collection_h */
