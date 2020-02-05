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

bool Manager::unregisterViewWidget(std::size_t typeIndex)
{
  auto iter = m_viewWidgets.find(typeIndex);
  if (iter != m_viewWidgets.end())
  {
    m_viewWidgets.erase(iter);
    return true;
  }

  return false;
}

smtk::view::BaseView* Manager::createViewWidget(
  std::size_t typeIndex, const smtk::view::Information& info)
{
  smtk::view::BaseView* viewWidget = nullptr;
  Manager::ViewWidgetConstructor constructor = this->getViewWidgetConstructor(typeIndex);
  if (constructor)
  {
    viewWidget = constructor(info);
  }
  return viewWidget;
}

smtk::view::BaseView* Manager::createViewWidget(
  const std::string& alias, const smtk::view::Information& info)
{
  smtk::view::BaseView* viewWidget = nullptr;
  Manager::ViewWidgetConstructor constructor = this->getViewWidgetConstructor(alias);
  if (constructor)
  {
    viewWidget = constructor(info);
  }
  return viewWidget;
}

Manager::ViewWidgetConstructor Manager::getViewWidgetConstructor(const std::string& alias) const
{
  auto nameIter = m_altViewWidgetNames.find(alias);
  if (nameIter != m_altViewWidgetNames.end())
  {
    return this->getViewWidgetConstructor(nameIter->second);
  }

  return nullptr;
}

Manager::ViewWidgetConstructor Manager::getViewWidgetConstructor(std::size_t typeIndex) const
{
  // Locate the constructor associated with this resource index
  auto iter = m_viewWidgets.find(typeIndex);
  if (iter != m_viewWidgets.end())
  {
    return iter->second;
  }

  return nullptr;
}

bool Manager::hasViewWidget(const std::string& alias) const
{
  return this->getViewWidgetConstructor(alias) != nullptr;
}

void Manager::addWidgetAlias(std::size_t typeIndex, const std::string& alias)
{
  m_altViewWidgetNames[alias] = typeIndex;
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
