//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/EvaluatorFactory.h"

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"

namespace smtk
{
namespace attribute
{

bool EvaluatorFactory::addDefinitionForEvaluator(
  const std::string& alias, const std::string& definitionName)
{
  if (isDefinitionRegistered(definitionName))
  {
    return false;
  }

  const auto aliasIt = m_aliasesToFactoryInfo.find(alias);

  if (aliasIt != m_aliasesToFactoryInfo.end())
  {
    aliasIt->second.m_definitionNames.insert(definitionName);
    return true;
  }

  return false;
}

bool EvaluatorFactory::unregisterEvaluator(const std::string& alias)
{
  auto it = m_aliasesToFactoryInfo.find(alias);

  if (it != m_aliasesToFactoryInfo.end())
  {
    std::size_t typeHash = it->second.m_typeHash;

    m_aliasesToFactoryInfo.erase(it);

    return m_internal.unregisterType(typeHash);
  }

  return false;
}

std::unique_ptr<smtk::attribute::Evaluator> EvaluatorFactory::createEvaluator(
  smtk::attribute::ConstAttributePtr att) const
{
  if (!att)
  {
    return std::unique_ptr<smtk::attribute::Evaluator>();
  }

  smtk::attribute::ResourcePtr attRes = att->attributeResource();
  if (!attRes)
  {
    return std::unique_ptr<smtk::attribute::Evaluator>();
  }

  smtk::attribute::DefinitionPtr currentDef = att->definition();
  std::string currentDefName;
  if (currentDef)
  {
    currentDefName = currentDef->type();
  }

  while (!currentDefName.empty())
  {
    for (const auto& p : m_aliasesToFactoryInfo)
    {
      if (p.second.m_definitionNames.count(currentDefName))
      {
        return m_internal.createFromIndex(p.second.m_typeHash, att);
      }
    }

    currentDef = currentDef->baseDefinition();
    if (currentDef)
    {
      currentDefName = currentDef->type();
    }
    else
    {
      currentDefName = std::string();
    }
  }

  return std::unique_ptr<smtk::attribute::Evaluator>();
}

bool EvaluatorFactory::isDefinitionRegistered(const std::string& definitionName) const
{
  for (const auto& p : m_aliasesToFactoryInfo)
  {
    if (p.second.m_definitionNames.count(definitionName))
    {
      return true;
    }
  }

  return false;
}

std::map<std::string, std::vector<std::string> > EvaluatorFactory::aliasesToDefinitions() const
{
  std::map<std::string, std::vector<std::string> > result;
  for (const auto& p : m_aliasesToFactoryInfo)
  {
    std::vector<std::string> aliasDefinitions;
    for (const std::string& defName : p.second.m_definitionNames)
    {
      if (!defName.empty())
      {
        aliasDefinitions.emplace_back(defName);
      }
    }

    std::sort(aliasDefinitions.begin(), aliasDefinitions.end());
    result.insert(std::make_pair(p.first, aliasDefinitions));
  }

  return result;
}

} // namespace attribute
} // namespace smtk
