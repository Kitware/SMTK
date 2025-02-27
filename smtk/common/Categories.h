//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_Categories_h
#define smtk_common_Categories_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h" // quiet dll-interface warnings on windows

#include "smtk/common/categories/Evaluators.h"
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace smtk
{
namespace common
{
///\brief Represents the category constraints associated with smtk objects such as
/// Attributes, Attribute Definitions, Attribute Items, Attribute Item Definitions, Tasks and Task Worklets
///
/// Categories is composed of a set of Category Sets.  Each set represents a single category constraint.
/// The overall category constraint represented by Categories "or's" the result from its constraint sets.
/// Therefore, the Categories' constraint is satisfied if at least one of its Sets' constraint is satisfied.
class SMTKCORE_EXPORT Categories
{
public:
  enum class CombinationMode
  {
    Or = 0,        //!< Represents logical Or
    And = 1,       //!< Represents logical And
    LocalOnly = 2, //!< Indicates no further processing of Category Set Pairs is required
  };

  ///\brief Categories::Set represents a single category constraint used by the Categories class.
  ///
  /// A Set consists of two sets of category names representing the categories that should exist
  /// (inclusion) and the categories that should not exist (exclusion).  Each set also has a
  /// combination mode associated with it.  For example if the inclusion mode is Or then the inclusion set's
  /// constraint is satisfied if any of its categories names exists in a input set of category names.
  /// Else the inclusion mode is And, then all of the inclusion set's names must exist in the input set.
  /// In the case of the exclusion set , the result is the complement.  So exclusion mode = Or means the
  /// exclusion test fails if any of the excluded names are in the input while And means that only if all
  /// of the names in the exclusion set are in the input will the test fail.
  ///
  /// There is also a top level combination mode that determines how the inclusion and exclusion results are
  /// combined.  If the top mode is And then both the inclusion and exclusion checks must pass while Or means
  /// only one of the checks need to pass.
  ///
  /// Special notes:
  /// If the top combination mode is And and the inclusion set is empty, then the passes check will always fail.
  /// If the top combination mode is Or and the exclusion set is empty, then the passes check will always succeed.
  class SMTKCORE_EXPORT Set
  {
  public:
    using CombinationMode = Categories::CombinationMode;
    Set() = default;
    ///@{
    ///\brief Set/Get the how the sets of included and excluded categories are combined
    Set::CombinationMode combinationMode() const { return m_combinationMode; }
    bool setCombinationMode(const Set::CombinationMode& newMode);
    ///@}
    ///@{
    ///\brief Set/Get the CombinationMode associated with the included categories.
    Set::CombinationMode inclusionMode() const { return m_includeMode; }
    void setInclusionMode(const Set::CombinationMode& newMode);
    ///@}
    ///@{
    ///\brief Set/Get the CombinationMode associated with the excluded categories.
    Set::CombinationMode exclusionMode() const { return m_excludeMode; }
    void setExclusionMode(const Set::CombinationMode& newMode);
    ///@}
    ///\brief Return the set of category names associated with the inclusion set.
    const std::set<std::string>& includedCategoryNames() const { return m_includedCategories; }
    ///\brief Set the mode and category names of the inclusion set.
    void setInclusions(const std::set<std::string>& values, Set::CombinationMode mode)
    {
      m_includeMode = mode;
      m_includedCategories = values;
      this->updatedSetInfo();
    }
    ///\brief Return the set of category names associated with the exclusion set.
    const std::set<std::string>& excludedCategoryNames() const { return m_excludedCategories; }
    ///\brief Set the mode and category names of the exclusion set.
    void setExclusions(const std::set<std::string>& values, Set::CombinationMode mode)
    {
      m_excludeMode = mode;
      m_excludedCategories = values;
      this->updatedSetInfo();
    }
    ///\brief Add a category name to the inclusion set.
    void insertInclusion(const std::string& val)
    {
      m_includedCategories.insert(val);
      this->updatedSetInfo();
    }
    ///\brief Add a category name to the exclusion set.
    void insertExclusion(const std::string& val)
    {
      m_excludedCategories.insert(val);
      this->updatedSetInfo();
    }
    ///\brief Remove a category name from the inclusion set.
    void eraseInclusion(const std::string& val)
    {
      m_includedCategories.erase(val);
      this->updatedSetInfo();
    }
    ///\brief Remove a category name from the exclusion set.
    void eraseExclusion(const std::string& val)
    {
      m_excludedCategories.erase(val);
      this->updatedSetInfo();
    }
    ///\brief Remove all names from both inclusion and exclusion sets.
    virtual void reset()
    {
      m_includedCategories.clear();
      m_excludedCategories.clear();
    }
    ///\brief Returns true if both the inclusion and exclusion sets are empty and the combination
    /// mode is set to And since this would represent a set that matches nothing
    bool empty() const
    {
      return m_includedCategories.empty() && m_excludedCategories.empty() &&
        (m_combinationMode == CombinationMode::And);
    }
    ///\brief Returns the number of category names in the inclusion set.
    std::size_t inclusionSize() const { return m_includedCategories.size(); }
    ///\brief Returns the number of category names in the exclusion set.
    std::size_t exclusionSize() const { return m_excludedCategories.size(); }
    ///@{
    ///\brief  Return true if the input set of categories satisfies the Set's
    /// constraints.
    ///
    /// If the input set is empty then the method will return true.  Else if
    /// the instance's mode is Or then at least one of its category names must be in the
    /// input set.  If the mode is And then all of the instance's names must be in the input set.
    virtual bool passes(const std::set<std::string>& cats) const;
    virtual bool passes(const std::string& cat) const;
    ///@}
    static bool passesCheck(
      const std::set<std::string>& cats,
      const std::set<std::string>& testSet,
      Set::CombinationMode comboMode);

    ///\brief Compares with other set - returns -1 if this < rhs, 0 if they are equal, and 1 if this > rhs
    virtual int compare(const Set& rhs) const;
    virtual std::string convertToString(const std::string& prefix = "") const;

  protected:
    virtual void updatedSetInfo(){};
    Set::CombinationMode m_includeMode{ Set::CombinationMode::Or },
      m_excludeMode{ Set::CombinationMode::Or }, m_combinationMode{ Set::CombinationMode::And };
    std::set<std::string> m_includedCategories, m_excludedCategories;
  };

  class SMTKCORE_EXPORT Expression : public Set
  {
  public:
    Expression();
    ///@{
    ///\brief Set/Get the expression string.
    ///
    /// Setting the expression will return true if the string represented a valid expression and will construct an
    /// appropriate evaluator.  If not, no changes are made and false is returned.
    bool setExpression(const std::string& expString);
    const std::string& expression() const;
    ///@}

    ///\brief Set the expression to pass all sets of categories
    void setAllPass();
    ///\brief Set the expression to reject any set of categories
    void setAllReject();
    ///\brief Indicates that the expression is set to pass all sets of categories
    bool allPass() const { return (m_expression.empty() ? m_allPass : false); }
    ///\brief Indicates that the expression is set to reject all sets of categories
    bool allReject() const { return (m_expression.empty() ? !m_allPass : false); }

    ///\brief Resets the expression to its unset condition and sets the default
    /// evaluator to reject all.
    void reset() override
    {
      this->setAllReject();
      m_isSet = false;
    }

    ///\brief Indicates that the expression has been set
    bool isSet() const { return m_isSet; }
    ///@{
    ///\brief  Return true if the input set of categories satisfies the Set's
    /// constraints.
    ///
    /// If the input set is empty then the method will return true.  Else if
    /// the instance's mode is Or then at least one of its category names must be in the
    /// input set.  If the mode is And then all of the instance's names must be in the input set.
    bool passes(const std::set<std::string>& cats) const override;
    bool passes(const std::string& cat) const override;
    ///@}
    ///\brief Compares with other set - returns -1 if this < rhs, 0 if they are equal, and 1 if this > rhs
    int compare(const Set& rhs) const override;

    std::string convertToString(const std::string& prefix = "") const override;

    const std::set<std::string>& categoryNames() const { return m_categoryNames; }

  protected:
    void updatedSetInfo() override;
    bool buildEvaluator();
    std::string m_expression;
    categories::Evaluators::Eval m_evaluator;
    bool m_allPass = false;
    bool m_isSet = false;
    std::set<std::string> m_categoryNames;
  };

  class SMTKCORE_EXPORT Stack
  {
  public:
    bool append(CombinationMode mode, const Expression& exp);
    void clear() { m_stack.clear(); }
    bool passes(const std::set<std::string>& cats) const;
    bool passes(const std::string& cat) const;
    std::string convertToString(const std::string& prefix = "") const;
    std::set<std::string> categoryNames() const;
    bool empty() const { return m_stack.empty(); }
    ///\brief Comparison operator needed to create a set of Categories::Stacks
    bool operator<(const Stack& rhs) const;

  protected:
    std::vector<std::pair<CombinationMode, Expression>> m_stack;
  };

  Categories() = default;
  ///@{
  /// \brief Returns true if atleast one of its sets passes its check
  bool passes(const std::set<std::string>& cats) const;
  bool passes(const std::string& cat) const;
  ///@}
  ///@{
  ///\brief Insert either a Categories::Set or the sets of another Categories instance into
  /// this instance.
  bool insert(const Stack& stack);
  void insert(const Categories& cats);
  ///@}
  ///\brief Remove all stacks from this instance.
  void reset() { m_stacks.clear(); }
  ///\brief Return the number of stacks in this instance.
  std::size_t size() const { return m_stacks.size(); }
  ///\brief Return the stacks contained in this instance.
  const std::set<Stack>& stacks() const { return m_stacks; }
  ///\brief Return a set of all category names referenced in all of the instance's stacks.
  std::set<std::string> categoryNames() const;
  ///\brief produce a formatted string representing the Categories' information.
  std::string convertToString() const;
  ///\brief Print to cerr the current contents of the instance.
  void print() const;
  static std::string combinationModeAsString(Set::CombinationMode mode);
  static std::string combinationModeAsSymbol(Set::CombinationMode mode);
  static bool combinationModeFromString(const std::string& val, Set::CombinationMode& mode);
  static bool combinationModeStringToSymbol(const std::string& val, std::string& symbol);

private:
  std::set<Stack> m_stacks;
};
} // namespace common
} // namespace smtk
#endif
