//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/Manager.h"
#include "smtk/view/QueryFilterSubphraseGenerator.h"
#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace view
{

Manager::Manager()
  : m_phraseModelFactory(this)
{
}

Manager::~Manager() = default;

bool Manager::unregisterSubphraseGenerator(const std::string& typeName)
{
  auto iter = m_subphraseGenerators.find(typeName);
  if (iter != m_subphraseGenerators.end())
  {
    m_subphraseGenerators.erase(iter);
    return true;
  }

  return false;
}

std::shared_ptr<SubphraseGenerator> Manager::createSubphrase(const Configuration::Component* config)
{
  std::shared_ptr<SubphraseGenerator> result;
  if (!config)
  {
    return result;
  }
  std::string sgType;
  bool haveType = config->attribute("Type", sgType);
  if (!haveType || sgType.empty() || sgType == "default")
  {
    sgType = "smtk::view::SubphraseGenerator";
  }
  result = this->createSubphrase(sgType);
  return result;
}

std::shared_ptr<SubphraseGenerator> Manager::createSubphrase(const std::string& typeName)
{
  std::shared_ptr<SubphraseGenerator> subphraseGenerator;

  // Locate the constructor associated with this resource type
  auto iter = m_subphraseGenerators.find(typeName);
  if (iter != m_subphraseGenerators.end())
  {
    // Create the subphraseGenerator, set its manager
    subphraseGenerator = iter->second();
    subphraseGenerator->m_manager = shared_from_this();
  }

  return subphraseGenerator;
}

} // view
} // smtk
