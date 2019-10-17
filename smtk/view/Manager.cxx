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

namespace smtk
{
namespace view
{

Manager::Manager()
{
}

Manager::~Manager()
{
}

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
    // Create the phrase model, set its manager
    phraseModel = iter->second();
    phraseModel->m_manager = shared_from_this();
  }

  return phraseModel;
}

} // view
} // smtk
