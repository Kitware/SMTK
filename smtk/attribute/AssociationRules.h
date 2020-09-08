//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_attribute_AssociationRules_h
#define __smtk_attribute_AssociationRules_h

#include "smtk/CoreExports.h"

#include "smtk/attribute/AssociationRule.h"
#include "smtk/attribute/AssociationRuleFactory.h"

namespace smtk
{
namespace attribute
{

/// A class for holding the state and encapsulating the logic behind custom
/// association rules for an attribute resource.
class SMTKCORE_EXPORT AssociationRules
{
public:
  typedef std::unordered_map<std::string, std::unique_ptr<Rule> > AssociationRuleContainer;
  typedef std::unordered_map<std::string, std::unique_ptr<Rule> > DissociationRuleContainer;

  /// Access the association/dissociation rule for a given definition, if one
  /// exists.
  const Rule* associationRuleForDefinition(const smtk::attribute::DefinitionPtr definition) const;
  const Rule* dissociationRuleForDefinition(const smtk::attribute::DefinitionPtr definition) const;

  /// Access the mapping between definition names and association/dissociation
  /// rules.
  std::unordered_map<std::string, std::string>& associationRulesForDefinitions()
  {
    return m_associationRulesForDefinitions;
  }
  const std::unordered_map<std::string, std::string>& associationRulesForDefinitions() const
  {
    return m_associationRulesForDefinitions;
  }
  std::unordered_map<std::string, std::string>& dissociationRulesForDefinitions()
  {
    return m_dissociationRulesForDefinitions;
  }
  const std::unordered_map<std::string, std::string>& dissociationRulesForDefinitions() const
  {
    return m_dissociationRulesForDefinitions;
  }

  /// Access the container for instances of custom association/dissociation
  /// rules.
  AssociationRuleContainer& associationRuleContainer() { return m_associationRuleContainer; }
  const AssociationRuleContainer& associationRuleContainer() const
  {
    return m_associationRuleContainer;
  }
  DissociationRuleContainer& dissociationRuleContainer() { return m_dissociationRuleContainer; }
  const DissociationRuleContainer& dissociationRuleContainer() const
  {
    return m_dissociationRuleContainer;
  }

  /// Access the factory for creating custom association/dissociation rule
  /// instances.
  AssociationRuleFactory& associationRuleFactory() { return m_associationRuleFactory; }
  const AssociationRuleFactory& associationRuleFactory() const { return m_associationRuleFactory; }
  DissociationRuleFactory& dissociationRuleFactory() { return m_dissociationRuleFactory; }
  const DissociationRuleFactory& dissociationRuleFactory() const
  {
    return m_dissociationRuleFactory;
  }

private:
  std::unordered_map<std::string, std::string> m_associationRulesForDefinitions;
  std::unordered_map<std::string, std::string> m_dissociationRulesForDefinitions;

  AssociationRuleContainer m_associationRuleContainer;
  DissociationRuleContainer m_dissociationRuleContainer;

  AssociationRuleFactory m_associationRuleFactory;
  DissociationRuleFactory m_dissociationRuleFactory;
};
}
}

#endif
