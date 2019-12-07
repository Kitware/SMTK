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
#include "smtk/view/PhraseModel.h"
#include "smtk/view/QueryFilterSubphraseGenerator.h"
#include "smtk/view/SubphraseGenerator.h"

namespace smtk
{
namespace view
{

Manager::Manager() = default;

Manager::~Manager() = default;

bool Manager::unregisterPhraseModel(const std::string& typeName)
{
  auto iter = m_phraseModels.find(typeName);
  if (iter != m_phraseModels.end())
  {
    m_phraseModels.erase(iter);
    return true;
  }

  return false;
}

std::shared_ptr<PhraseModel> Manager::create(const std::string& typeName)
{
  std::shared_ptr<PhraseModel> phraseModel;

  // Locate the constructor associated with this resource type
  auto iter = m_phraseModels.find(typeName);
  if (iter != m_phraseModels.end())
  {
    // Create the phraseModel, set its manager
    phraseModel = iter->second();
    phraseModel->m_manager = shared_from_this();
  }

  return phraseModel;
}

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
