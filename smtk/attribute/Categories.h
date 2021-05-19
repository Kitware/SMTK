//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_attribute_Categories_h
#define __smtk_attribute_Categories_h

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h" // quiet dll-interface warnings on windows

#include <set>
#include <string>

namespace smtk
{
namespace attribute
{
///\brief Represents the category constraints associated with an Attribute, Attribute Definition,
/// Item or Item Definition
///
/// Categories is composed of a set of Category Sets.  Each set represents a single category constraint.
/// The overall category constraint represented by Categories "or's" the result from its constraint sets.
/// Therefore, the Categories' constraint is satisfied if at least one of its Sets' constraint is satisfied.
class SMTKCORE_EXPORT Categories
{
public:
  ///\brief Categories::Set represents a single category constraint used by the Categories class.
  ///
  /// A Set consists of two sets of category names representing the categories that should exist
  /// (inclusion) and the categories that should not exist (exclusion).  Each set also has a
  /// combination mode associated with it.  For example if the inclusion mode is Any then the inclusion set's
  /// constraint is satisfied if any of its categories names exists in a input set of category names.
  /// Else the inclusion mode is All, then all of the inclusion set's names must exist in the input set.
  /// In the case of the exclusion set , the result is the complement.  So exclusion mode = Any means the
  /// exclusion test fails if any of the excluded names are in the input while All means that only if all
  /// of the names in the exclusion set are in the input will the test fail.
  ///
  /// There is also a top level combination mode that determines how the inclusion and exclusion results are
  /// combined.  If the top mode is All then both the inclusion and exclusion checks must pass while Any means
  /// only one of the checks need to pass.
  ///
  /// Special notes:
  /// If the top combination mode is All and the inclusion set is empty, then the passes check will always fail.
  /// If the top combination mode is Any and the exclusion set is empty, then the passes check will always succeed.
  class SMTKCORE_EXPORT Set
  {
  public:
    enum class CombinationMode
    {
      Any = 0, //!< Check passes if any of the set's categories are found
      All = 1  //!< Check passes if all of the set's categories are found
    };
    Set()
      : m_includeMode(CombinationMode::Any)
      , m_excludeMode(CombinationMode::Any)
      , m_combinationMode(CombinationMode::All)
    {
    }
    ///@{
    ///\brief Set/Get the how the sets of included and excluded categories are combined
    Set::CombinationMode combinationMode() const { return m_combinationMode; }
    void setCombinationMode(const Set::CombinationMode& newMode) { m_combinationMode = newMode; }
    ///@}
    ///@{
    ///\brief Set/Get the CombinationMode associated with the included categories.
    Set::CombinationMode inclusionMode() const { return m_includeMode; }
    void setInclusionMode(const Set::CombinationMode& newMode) { m_includeMode = newMode; }
    ///@}
    ///@{
    ///\brief Set/Get the CombinationMode associated with the excluded categories.
    Set::CombinationMode exclusionMode() const { return m_excludeMode; }
    void setExclusionMode(const Set::CombinationMode& newMode) { m_excludeMode = newMode; }
    ///@}
    ///\brief Return the set of category names associated with the inclusion set.
    const std::set<std::string>& includedCategoryNames() const { return m_includedCategories; }
    ///\brief Set the mode and category names of the inclusion set.
    void setInclusions(const std::set<std::string>& values, Set::CombinationMode mode)
    {
      m_includeMode = mode;
      m_includedCategories = values;
    }
    ///\brief Return the set of category names associated with the exclusion set.
    const std::set<std::string>& excludedCategoryNames() const { return m_excludedCategories; }
    ///\brief Set the mode and category names of the exclusion set.
    void setExclusions(const std::set<std::string>& values, Set::CombinationMode mode)
    {
      m_excludeMode = mode;
      m_excludedCategories = values;
    }
    ///\brief Add a category name to the inclusion set.
    void insertInclusion(const std::string& val) { m_includedCategories.insert(val); }
    ///\brief Add a category name to the exclusion set.
    void insertExclusion(const std::string& val) { m_excludedCategories.insert(val); }
    ///\brief Remove a category name from the inclusion set.
    void eraseInclusion(const std::string& val) { m_includedCategories.erase(val); }
    ///\brief Remove a category name from the exclusion set.
    void eraseExclusion(const std::string& val) { m_excludedCategories.erase(val); }
    ///\brief Remove all names from both inclusion and exclusion sets.
    void reset()
    {
      m_includedCategories.clear();
      m_excludedCategories.clear();
    }
    ///\brief Returns true if both the inclusion and exclusion sets are empty.
    bool empty() const { return m_includedCategories.empty() && m_excludedCategories.empty(); }
    ///\brief Returns the number of category names in the inclusion set.
    std::size_t inclusionSize() const { return m_includedCategories.size(); }
    ///\brief Returns the number of category names in the exclusion set.
    std::size_t exclusionSize() const { return m_excludedCategories.size(); }
    ///@{
    ///\brief  Return true if the input set of categories satifies the Set's
    /// constraints.
    ///
    /// If the input set is empty then the method will return true.  Else if
    /// the instance's mode is Any then at least one of its category names must be in the
    /// input set.  If the mode is All then all of the instance's names must be in the input set.
    bool passes(const std::set<std::string>& cats) const;
    bool passes(const std::string& cat) const;
    ///@}
    static bool passesCheck(
      const std::set<std::string>& cats,
      const std::set<std::string>& testSet,
      Set::CombinationMode comboMode);
    ///\brief Comparison operator needed to create a set of Categories::Sets
    bool operator<(const Set& rhs) const;
    static std::string combinationModeAsString(Set::CombinationMode mode);
    static bool combinationModeFromString(const std::string& val, Set::CombinationMode& mode);

    // Deprecated Methods
    [[deprecated(
      "Categories::Set::mode has been replaced with Categories::Set::inclusionMode")]] Set::
      CombinationMode
      mode() const
    {
      return this->inclusionMode();
    }
    [[deprecated(
      "Categories::Set::setMode has been replaced with Categories::Set::setInclusionMode")]] void
    setMode(const Set::CombinationMode& newMode)
    {
      this->setInclusionMode(newMode);
    }
    [[deprecated("Categories::Set::categoryNames has been replaced with "
                 "Categories::Set::includedCategoryNames")]] const std::set<std::string>&
    categoryNames() const
    {
      return this->includedCategoryNames();
    }
    [[deprecated(
      "Categories::Set::set has been replaced with Categories::Set::setInclusions")]] void
    set(const std::set<std::string>& values, Set::CombinationMode mode)
    {
      this->setInclusions(values, mode);
    }
    [[deprecated(
      "Categories::Set::insert has been replaced with Categories::Set::insertInclusion")]] void
    insert(const std::string& val)
    {
      this->insertInclusion(val);
    }
    [[deprecated(
      "Categories::Set::erase has been replaced with Categories::Set::eraseInclusion")]] void
    erase(const std::string& val)
    {
      this->eraseInclusion(val);
    }
    [[deprecated(
      "Categories::Set::size has been replaced with Categories::Set::inclusionSize")]] std::size_t
    size() const
    {
      return this->inclusionSize();
    }

  private:
    Set::CombinationMode m_includeMode, m_excludeMode, m_combinationMode;
    std::set<std::string> m_includedCategories, m_excludedCategories;
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
  void insert(const Set& set);
  void insert(const Categories& cats);
  ///@}
  ///\brief Remove all sets from this instance.
  void reset() { m_sets.clear(); }
  ///\brief Return the number of sets in this instance.
  std::size_t size() const { return m_sets.size(); }
  ///\brief Return the sets contained in this instance.
  const std::set<Set>& sets() const { return m_sets; }
  ///\brief Return a set of all category names refrenced in all of te instance's sets.
  std::set<std::string> categoryNames() const;
  ///\brief Print to cerr the current contents of the instance.
  void print() const;

private:
  std::set<Set> m_sets;
};
} // namespace attribute
} // namespace smtk
#endif
