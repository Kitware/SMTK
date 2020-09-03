//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/AssociationRules.h"

#include "smtk/attribute/Definition.h"

namespace smtk
{
namespace attribute
{
const Rule* AssociationRules::associationRuleForDefinition(
  const smtk::attribute::DefinitionPtr definition) const
{
  // Find the name of the rule associated with this definition
  auto it = m_associationRulesForDefinitions.find(definition->type());
  if (it != m_associationRulesForDefinitions.end())
  {
    // Find the rul associated with this name.
    auto it2 = m_associationRuleContainer.find(it->second);
    if (it2 != m_associationRuleContainer.end())
    {
      return it2->second.get();
    }
  }
  return nullptr;
}

const Rule* AssociationRules::dissociationRuleForDefinition(
  const smtk::attribute::DefinitionPtr definition) const
{
  // Find the name of the rule associated with this definition
  auto it = m_dissociationRulesForDefinitions.find(definition->type());
  if (it != m_dissociationRulesForDefinitions.end())
  {
    // Find the rul associated with this name.
    auto it2 = m_dissociationRuleContainer.find(it->second);
    if (it2 != m_dissociationRuleContainer.end())
    {
      return it2->second.get();
    }
  }
  return nullptr;
}
}
}
