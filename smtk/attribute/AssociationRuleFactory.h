//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_AssociationRuleFactory_h
#define __smtk_attribute_AssociationRuleFactory_h

#include "smtk/CoreExports.h"

#include "smtk/common/Factory.h"

#include "smtk/attribute/AssociationRule.h"

namespace smtk
{
namespace attribute
{

/// A factory for generating association rules. AssociationRuleFactory is an
/// implementation of a common factory for Rule types, with additional logic for
/// handling human-readable aliases.
class SMTKCORE_EXPORT AssociationRuleFactory : public smtk::common::Factory<Rule, void>
{
public:
  template<typename Type>
  void addAlias(const std::string& alias)
  {
    addAlias(smtk::common::typeName<Type>(), alias);
  }

  /// Add an alternative constructor name for an association rule.
  void addAlias(const std::string& typeName, const std::string& alias)
  {
    m_aliases[alias] = typeName;
    m_reverseLookup.insert(std::make_pair(typeName, alias));
  }

  /// Determine whether or not a Type is available using its alias.
  bool containsAlias(const std::string& alias) const
  {
    return m_aliases.find(alias) != m_aliases.end();
  }

  /// Create an instance of a Type using its type name.
  template<typename... Args>
  std::unique_ptr<Rule> createFromAlias(const std::string& alias, Args&&... args) const
  {
    auto found = m_aliases.find(alias);
    if (found != m_aliases.end())
    {
      return createFromName(found->second, std::forward<Args>(args)...);
    }
    return std::unique_ptr<Rule>();
  }

  const std::map<std::string, std::string>& aliases() const { return m_aliases; }
  const std::map<std::string, std::string>& reverseLookup() const { return m_reverseLookup; }

private:
  std::map<std::string, std::string> m_aliases;
  std::map<std::string, std::string> m_reverseLookup;
};

typedef AssociationRuleFactory DissociationRuleFactory;
} // namespace attribute
} // namespace smtk

#endif
