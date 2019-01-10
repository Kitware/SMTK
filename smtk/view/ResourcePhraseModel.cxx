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

int ResourcePhraseModel::handleOperationEvent(smtk::operation::Operation::Ptr op,
  smtk::operation::EventType event, smtk::operation::Operation::Result res)
{
  // First, let the superclass handle most things.
  this->Superclass::handleOperationEvent(op, event, res);

  // Now, if any components were modified or the operation was a
  // Write operation (which clears the dirty bit on its resource),
  // tell the view that the owning resource's entry is modified
  // (so that the clean/dirty state will be updated properly).
  if (!op)
  {
    // Every event should have an operator
    return 1;
  }
  // Ignore operator creation and about-to-operate events.
  if (!res || event != operation::EventType::DID_OPERATE)
  {
    return 0;
  }

  // Find out which resource components were created, modified, or expunged.
  auto modified = res->findComponent("modified");
  for (size_t ii = 0; ii < modified->numberOfValues(); ++ii)
  {
    auto comp = modified->value(ii);
    if (comp)
    {
      auto rsrc = comp->resource();
      if (rsrc)
      {
        int childIndex = m_root->argFindChild(rsrc, true);
        std::vector<int> idx(1, childIndex);
        std::vector<int> empty;
        this->trigger(m_root, PhraseModelEvent::PHRASE_MODIFIED, idx, idx, empty);
      }
    }
  }

  // Also handle write operations that mark resources clean.
  // TODO: This is a bit hacky. In the long term, it might be better
  //       to have a resource-manager event triggered when resources
  //       change their clean/dirty bit.
  if (op->manager() && smtk::operation::WriterGroup(op->manager()).has(op->index()))
  {
    // Any associated resources should be redrawn
    auto assocs = op->parameters()->associations();
    for (size_t ii = 0; assocs && ii < assocs->numberOfValues(); ++ii)
    {
      auto rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(assocs->objectValue(ii));
      if (rsrc)
      {
        int childIndex = m_root->argFindChild(rsrc, true);
        std::vector<int> idx(1, childIndex);
        std::vector<int> empty;
        this->trigger(m_root, PhraseModelEvent::PHRASE_MODIFIED, idx, idx, empty);
      }
    }
  }
  return 0;
}

void ResourcePhraseModel::handleResourceEvent(Resource::Ptr rsrc, smtk::resource::EventType event)
{
  this->processResource(rsrc, event == smtk::resource::EventType::ADDED);
}

void ResourcePhraseModel::processResource(Resource::Ptr rsrc, bool adding)
{
  if (adding)
  {
    if (m_resources.find(rsrc) == m_resources.end())
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

      m_resources.insert(rsrc);
      DescriptivePhrases children(m_root->subphrases());
      children.push_back(smtk::view::ResourcePhraseContent::createPhrase(rsrc, 0, m_root));
      std::sort(children.begin(), children.end(), DescriptivePhrase::compareByTypeThenTitle);
      this->root()->findDelegate()->decoratePhrases(children);
      this->updateChildren(m_root, children, std::vector<int>());
    }
  }
  else
  {
    auto it = m_resources.find(rsrc);
    if (it != m_resources.end())
    {
      m_resources.erase(it);
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
