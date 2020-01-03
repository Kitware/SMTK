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

bool Manager::unregisterViewWidget(const std::string& typeName)
{
  auto iter = m_viewWidgets.find(typeName);
  if (iter != m_viewWidgets.end())
  {
    m_viewWidgets.erase(iter);
    return true;
  }

  return false;
}

smtk::extension::qtBaseView* Manager::createViewWidget(
  const std::string& typeName, const smtk::extension::ViewInfo& info)
{
  smtk::extension::qtBaseView* viewWidget = nullptr;

  // Locate the constructor associated with this resource type
  auto iter = m_viewWidgets.find(typeName);
  if (iter == m_viewWidgets.end())
  {
    // If a literal type name doesn't work, try to use the alternate legacy constructor names.
    auto nameIter = m_altViewWidgetNames.find(typeName);
    if (nameIter != m_altViewWidgetNames.end())
    {
      iter = m_viewWidgets.find(nameIter->second);
    }
  }
  if (iter != m_viewWidgets.end())
  {
    // Create the viewWidget, set its Manager
    viewWidget = iter->second(info);
    // viewWidget->m_manager = shared_from_this();
  }

  return viewWidget;
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
