//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_attribute_Analyses_h
#define smtk_attribute_Analyses_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include <set>
#include <string>
#include <vector>

namespace smtk
{
namespace attribute
{
///\brief Represents a set of Analyses defined within an Attribute Resource

class SMTKCORE_EXPORT Analyses
{
public:
  /// \brief Represents a single Analysis defined within an Attribute Resource
  ///
  /// An Analysis represents a subset of information stored within an Attribute Resource that is
  /// required by the Analysis.  This is accomplished by using the category information associated
  /// with the items and attributes.  The subset of information is represented as a set of categories.
  /// An Analysis maintains a set of local categories.  In addition, an Analysis can a parent Analysis
  /// associated with it (using the setParent method).  The total set of categories is the union of
  /// the Analysis' local categories and the categories associated with it's parent.
  ///
  /// Note that only the Analyses class can create or delete an Analysis
  class SMTKCORE_EXPORT Analysis
  {
    friend class Analyses;

  public:
    /// \brief Return the name associated with the Analysis
    const std::string& name() const { return m_name; }

    /// \brief Return the string to represent the Analysis in UI's
    const std::string& displayedName() const { return (m_label.empty() ? m_name : m_label); }
    void setLabel(const std::string& analysisLabel) { m_label = analysisLabel; }
    const std::string& label() const { return m_label; }
    bool hasLabel() const { return !m_label.empty(); }

    /// @{
    /// \brief Methods to set and retrieve the categories locally associated with the Analysis
    void setLocalCategories(const std::set<std::string>& cats) { m_categories = cats; }
    const std::set<std::string>& localCategories() const { return m_categories; }
    /// @}

    /// \brief Returns all of the categories associated with the Analysis, including it's parent's
    std::set<std::string> categories() const;

    /// \brief Method to retrieve an Analysis' parent Analysis.
    Analysis* parent() const { return m_parent; }
    bool setParent(Analysis* p);

    /// @{
    /// \brief Methods to set and retrieve an Analysis' children exclusivity property.
    ///
    /// When defining an Analysis, it is important to model whether its children represent
    /// a set of analyses that can be combined arbitrary manner (Exclusion is false)
    /// or if only one of its children can be "selected" (Exclusion is true).
    void setExclusive(bool mode) { m_exclusive = mode; }
    bool isExclusive() const { return m_exclusive; }
    /// @}

    /// @{
    /// \brief Methods to set and retrieve an Analysis' required property.
    ///
    /// If an Analysis is required with respects to its parent then it will not
    /// appear with an optional checkbox.
    /// Note that if an Analysis' parent has isExclusive=true then this property is
    /// ignored
    void setRequired(bool mode) { m_required = mode; }
    bool isRequired() const { return m_required; }
    /// @}

    /// \brief Returns the children Analyses of this Analysis
    const std::vector<Analysis*>& children() const { return m_children; }

    /// @{
    /// \brief Methods for generating Attribute Item Definitions for an Analysis
    void buildAnalysisItem(smtk::attribute::DefinitionPtr& sitem) const;
    void buildAnalysisItem(smtk::attribute::GroupItemDefinitionPtr& gitem) const;
    void buildAnalysisItem(smtk::attribute::StringItemDefinitionPtr& sitem) const;
    /// @}

  protected:
    Analysis(const std::string& name)
      : m_name(name)
    {
    }
    ~Analysis() = default;

    std::string m_name;                 ///< Name of the Analysis
    Analysis* m_parent{ nullptr };      ///< Analysis' Parent
    bool m_exclusive{ false };          ///< Indicates if the Analysis' children are exclusive
    bool m_required{ false };           ///< Indicates if the Analysis is required
    std::string m_label;                ///< Optional label to be used for UIs
    std::set<std::string> m_categories; ///< Categories locally assigned to the analysis
    std::vector<Analysis*> m_children;  ///< Children of the Analysis
  };

  /// \brief Basic constructor - Note that by default top level Analyses are not Exclusive
  Analyses() = default;
  /// \brief Create a new Analysis and return it.
  /// Note that the name must be unique with respects to the other Analysis Instances defined within
  /// this Instance.  If the name is not unique no Analysis is created and nullptr is returned.
  Analysis* create(const std::string& name);

  /// \brief Retrieve an Analysis based on its name.  If none exists, nullptr is returned.
  Analysis* find(const std::string& name) const;

  /// \brief Return all Analysis Instances that do not have a parent Analysis.
  std::vector<Analysis*> topLevel() const;

  /// \brief Return all Analysis Instances
  const std::vector<Analysis*>& analyses() const { return m_analyses; }

  /// \brief Return the number of Analysis Instances
  std::size_t size() const { return m_analyses.size(); }

  /// @{
  /// \brief Methods to set and retrieve the exclusivity property pertaining the top level Analysis Instances.
  ///
  /// This property behaves similarly to an Analysis's Exclusive property.  If true, then only one of the top level
  /// Analysis Instances can be choosen.  Else any combination of top level analysis instances are allowed.
  void setTopLevelExclusive(bool mode) { m_topLevelExclusive = mode; }
  bool areTopLevelExclusive() const { return m_topLevelExclusive; }
  /// @}

  /// \brief Convience method that set's an Analysis' Parent by using their names.
  ///
  /// If neither name corresponds to an existing Analysis then the method returns false.
  bool setAnalysisParent(const std::string& analysis, const std::string& parent);

  /// \brief Create an Attribute Definition to represent the Analysis Instances maintained by the Analyses.
  DefinitionPtr buildAnalysesDefinition(
    smtk::attribute::ResourcePtr resource,
    const std::string& type,
    const std::string& label = "Analysis") const;

  /// \brief Calculate the set of categories associated with an Analysis Attribute's settings.
  void getAnalysisAttributeCategories(
    smtk::attribute::ConstAttributePtr attribute,
    std::set<std::string>& cats);
  std::set<std::string> getAnalysisAttributeCategories(
    smtk::attribute::ConstAttributePtr attribute);

  /// \brief Destroys the Instance and deletes all Analysis Instances contained within.
  ~Analyses();

protected:
  /// \brief Calculate the set of categories associated with an Analysis Attribute's Item.
  ///
  /// itemNotAnalysis indicates if the item does not represent an analysis itself.
  /// This occurs only when processing the top level item and m_topLevelExclusive
  /// is true.
  void
  getAnalysisItemCategories(ConstItemPtr item, std::set<std::string>& cats, bool itemNotAnalysis);

  bool m_topLevelExclusive{
    false
  }; ///< Indicates if the top level Analysis Instances are exclusive
  std::vector<Analysis*> m_analyses; ///< Analysis Instances managed by the Analyses Instance
};
} // namespace attribute
} // namespace smtk

#endif /* smtk_attribute_Analyses_h */
