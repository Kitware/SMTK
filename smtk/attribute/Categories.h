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
///\brief Represents the category constriants associated with an Attribute, Attrbute Definition,
/// Item or Item Definition
///
/// Categories is composed of a set of Category Sets.  Each set represents a single category constraint.
/// The overall category constraint represented by Categories "or's" the result from its constraint sets.
/// Therefore, the Categories' constraint is satified if atleast one of its Sets' constraint is satisfied.
class SMTKCORE_EXPORT Categories
{
public:
  ///\brief Categories::Set represents a single category constraint used by the Categories class.
  ///
  /// A Set cnsists of a set of category names and a combination mode.  If the mode is Any then the Set's
  /// constraint is satisfied if any of its categories names exsits in a set of category names.  If the mode
  /// is All, then all of the Set's names must exsit.
  ///
  /// Note that an empty Set is considered to always fail.
  class SMTKCORE_EXPORT Set
  {
  public:
    enum class CombinationMode
    {
      Any = 0, //!< Check passes if any of the set's categories are found
      All = 1  //!< Check passes if all of the set's categories are found
    };
    Set()
      : m_mode(CombinationMode::Any)
    {
    }
    ///@{
    ///\brief Set/Get the CombinationMode associated with the instance.
    Set::CombinationMode mode() const { return m_mode; }
    void setMode(const Set::CombinationMode& newMode) { m_mode = newMode; }
    ///@}
    ///\brief Return the set of category names associated with the instance.
    const std::set<std::string>& categoryNames() const { return m_categoryNames; }
    ///\brief Set the mode and category names of the instance.
    void set(const std::set<std::string>& values, Set::CombinationMode mode)
    {
      m_mode = mode;
      m_categoryNames = values;
    }
    ///\brief Add a category name to the instance.
    void insert(const std::string& val) { m_categoryNames.insert(val); }
    ///\brief Remove a category name form the instance.
    void erase(const std::string& val) { m_categoryNames.erase(val); }
    ///\brief Remove all names from the instance.
    void reset() { m_categoryNames.clear(); }
    ///\brief Returns true if the instance's category names is empty.
    bool empty() const { return m_categoryNames.empty(); }
    ///\brief Returns the number of category names in the instance.
    std::size_t size() const { return m_categoryNames.size(); }
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
    ///\brief Comparison operator needed to create a set of Categories::Sets
    bool operator<(const Set& rhs) const;

  private:
    Set::CombinationMode m_mode;
    std::set<std::string> m_categoryNames;
  };
  Categories() {}
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
}
}
#endif
