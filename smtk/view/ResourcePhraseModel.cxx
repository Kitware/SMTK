//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ResourcePhraseModel.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseListContent.h"
#include "smtk/view/ResourcePhraseContent.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/View.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/groups/WriterGroup.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Component.h"

#include <algorithm> // for std::sort

using namespace smtk::view;

PhraseModelPtr ResourcePhraseModel::create(const smtk::view::ViewPtr& viewSpec)
{
  (void)viewSpec;
  auto model = PhraseModel::Ptr(new ResourcePhraseModel);
  model->root()->findDelegate()->setModel(model);
  return model;
}

PhraseModelPtr ResourcePhraseModel::create(const smtk::view::View::Component& viewSpec)
{
  (void)viewSpec;
  auto model = PhraseModel::Ptr(new ResourcePhraseModel);
  model->root()->findDelegate()->setModel(model);
  return model;
}

ResourcePhraseModel::ResourcePhraseModel()
  : m_root(DescriptivePhrase::create())
{
  // Do not allow color of resources to be mutable by default.
  m_mutableAspects &= ~static_cast<int>(smtk::view::PhraseContent::ContentType::COLOR);
  auto generator = smtk::view::SubphraseGenerator::create();
  m_root->setDelegate(generator);
}

ResourcePhraseModel::~ResourcePhraseModel()
{
  this->resetSources();
}

DescriptivePhrasePtr ResourcePhraseModel::root() const
{
  return m_root;
}

bool ResourcePhraseModel::setResourceFilters(const std::multimap<std::string, std::string>& src)
{
  if (src == m_resourceFilters)
  {
    return false;
  }

  m_resourceFilters = src;

  // Filter out current entries that do not pass the filter
  DescriptivePhrases children(m_root->subphrases());
  children.erase(std::remove_if(children.begin(), children.end(),
                   [this](const DescriptivePhrase::Ptr& phr) -> bool {
                     bool acceptable = false;
                     for (auto& filter : m_resourceFilters)
                     {
                       if (phr->relatedResource()->isOfType(filter.first))
                       {
                         acceptable = true;
                         break;
                       }
                     }
                     return acceptable == false;
                   }),
    children.end());
  this->updateChildren(m_root, children, std::vector<int>());

  return true;
}

void ResourcePhraseModel::handleResourceEvent(Resource::Ptr rsrc, smtk::resource::EventType event)
{
  if (event == smtk::resource::EventType::MODIFIED)
  {
    this->triggerModified(rsrc);
  }
  else
  {
    this->processResource(rsrc, event == smtk::resource::EventType::ADDED);
  }
}

void ResourcePhraseModel::processResource(Resource::Ptr rsrc, bool adding)
{
  if (adding)
  {
    if (m_resourceIds.find(rsrc->id()) == m_resourceIds.end())
    {
      // Only attempt to filter resource out if there are filters defined.
      bool acceptable = m_resourceFilters.empty() ? true : false;
      for (auto filter : m_resourceFilters)
      {
        if (rsrc->isOfType(filter.first))
        {
          acceptable = true;
          break;
        }
      }

      if (!acceptable)
      {
        return;
      }

      m_resourceIds.insert(rsrc->id());
      DescriptivePhrases children(m_root->subphrases());
      children.push_back(
        smtk::view::ResourcePhraseContent::createPhrase(rsrc, m_mutableAspects, m_root));
      std::sort(children.begin(), children.end(), DescriptivePhrase::compareByTypeThenTitle);
      this->root()->findDelegate()->decoratePhrases(children);
      this->updateChildren(m_root, children, std::vector<int>());
    }
  }
  else
  {
    auto it = m_resourceIds.find(rsrc->id());
    if (it != m_resourceIds.end())
    {
      m_resourceIds.erase(it);
      DescriptivePhrases children(m_root->subphrases());
      children.erase(std::remove_if(children.begin(), children.end(),
                       [&rsrc](const DescriptivePhrase::Ptr& phr) -> bool {
                         return phr->relatedResource() == rsrc;
                       }),
        children.end());
      this->updateChildren(m_root, children, std::vector<int>());
    }
  }
}

void ResourcePhraseModel::triggerModified(const Resource::Ptr& rsrc)
{
  if (rsrc)
  {
    int childIndex = m_root->argFindChild(rsrc, true);
    if (childIndex >= 0)
    {
      std::vector<int> idx(1, childIndex);
      std::vector<int> empty;
      this->trigger(m_root, PhraseModelEvent::PHRASE_MODIFIED, idx, idx, empty);
    }
  }
}
