//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_attribute_EvaluatorFactory_h
#define smtk_attribute_EvaluatorFactory_h

#include "smtk/CoreExports.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Evaluator.h"

#include "smtk/common/Factory.h"

#include <map>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace smtk
{
namespace attribute
{

// A factory for creating a Evaluators, registered with a std::string alias.
// Any number of Definitions can be paired with an Evaluator/alias pair, but a
// Definition can only be paired with a single Evaluator/alias pair.
class SMTKCORE_EXPORT EvaluatorFactory
{
public:
  // Registers |EvaluatorType| to the factory using the alias, |alias|. Replaces
  // any existing alias with the name |alias| and its created Evaluator and
  // Definitions. Does nothing if |alias| is already present with
  // |EvaluatorType|.
  template<typename EvaluatorType>
  void registerEvaluator(const std::string& alias);

  // Registers Definition with name |definitionName| to the Evaluator with
  // |alias|. Thus, when createEvaluator() is called for an Attribute of type
  // |definitionName|, an Evaluator registered with |alias| is returned.
  //
  // Returns false if |definitionName| is already registered to any alias or
  // |alias| is not present in the factory.
  bool addDefinitionForEvaluator(const std::string& alias, const std::string& definitionName);

  // Removes |alias| and all of its associated Definitions from the factory.
  bool unregisterEvaluator(const std::string& alias);

  // Removes |EvaluatorType| and all of its assocaited Definitions from the
  // factory.
  template<typename EvaluatorType>
  bool unregisterEvaluator();

  // Creates an Evaluator for |att|. Returns nullptr if no Definition name
  // matching |att|'s Definition is found.
  // Attempts to traverse all base Definitions of |att|.
  std::unique_ptr<smtk::attribute::Evaluator> createEvaluator(
    smtk::attribute::ConstAttributePtr att) const;

  bool isDefinitionRegistered(const std::string& definitionName) const;

  // Returns all aliases in the factory, mapped to their Definition names,
  // in sorted order to aid in deterministic serialization.
  std::map<std::string, std::vector<std::string>> aliasesToDefinitions() const;

private:
  struct FactoryInfo
  {
  public:
    FactoryInfo() = default;
    FactoryInfo(std::size_t typeHash, const std::unordered_set<std::string>& definitionNames)
      : m_typeHash(typeHash)
      , m_definitionNames(definitionNames)
    {
    }
    FactoryInfo(const FactoryInfo&) = default;

    std::size_t m_typeHash;
    std::unordered_set<std::string> m_definitionNames;
  };

  std::unordered_map<std::string, FactoryInfo> m_aliasesToFactoryInfo;

  smtk::common::Factory<smtk::attribute::Evaluator, smtk::attribute::ConstAttributePtr> m_internal;
};

template<typename EvaluatorType>
void EvaluatorFactory::registerEvaluator(const std::string& alias)
{
  std::size_t typeHash = std::type_index(typeid(EvaluatorType)).hash_code();

  const auto it = m_aliasesToFactoryInfo.find(alias);
  if (it != m_aliasesToFactoryInfo.end() && it->second.m_typeHash == typeHash)
  {
    return;
  }

  m_aliasesToFactoryInfo[alias] = FactoryInfo(typeHash, std::unordered_set<std::string>{});

  if (m_internal.contains<EvaluatorType>())
  {
    m_internal.unregisterType<EvaluatorType>();
  }

  m_internal.registerType<EvaluatorType>();
}

template<typename EvaluatorType>
bool EvaluatorFactory::unregisterEvaluator()
{
  for (const auto& p : m_aliasesToFactoryInfo)
  {
    std::size_t typeHash = std::type_index(typeid(EvaluatorType)).hash_code();
    if (p.second.m_typeHash == typeHash)
    {
      m_aliasesToFactoryInfo.erase(p.first);

      return m_internal.unregisterType(typeHash);
    }
  }

  return false;
}

} // namespace attribute
} // namespace smtk

#endif // smtk_attribute_EvaluatorFactory_h
