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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Component.h"

#include <algorithm> // for std::sort

using namespace smtk::view;

PhraseModelPtr ResourcePhraseModel::create(const smtk::view::ViewPtr& viewSpec)
{
  (void)viewSpec;
  auto model = PhraseModel::Ptr(new ResourcePhraseModel);
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

void ResourcePhraseModel::handleResourceEvent(Resource::Ptr rsrc, smtk::resource::Event event)
{
  this->processResource(rsrc, event == smtk::resource::Event::RESOURCE_ADDED);
}

void ResourcePhraseModel::handleCreated(
  Operation::Ptr op, Operation::Result res, ComponentItemPtr data)
{
  (void)op;
  if (!res || !data)
  {
    return;
  }

  // First, handle root node:
  for (auto it = data->begin(); it != data->end(); ++it)
  {
    smtk::resource::ComponentPtr comp = *it;
    if (comp == nullptr)
    {
      continue;
    }
    smtk::resource::ResourcePtr rsrc = comp->resource();
    this->processResource(rsrc, true);
  }

  // Now we need to traverse the existing phrases to see if any entries in
  // data need to be inserted.
  //
  // There are a couple strategies we might use:
  // 1. For each phrase in existence with built-out subphrases, rebuild them and call updateChildren.
  //    This is expensive, but perhaps less error prone.
  // 2. For each entry in data, call this->root()->visitChildren(...) and decide whether it
  //    belongs in the subphrases of each visited phrase. If it does and the visited phrase has
  //    its subphrases built, modify and call updateChildren().
  // 3. For each entry in data, identify a vector<int> (or multiple vector<int>?) where it
  //    belongs and add it.
  // TODO
}

void ResourcePhraseModel::processResource(Resource::Ptr rsrc, bool adding)
{
  if (adding)
  {
    if (m_resources.find(rsrc) == m_resources.end())
    {
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

smtkImplementsPhraseModel(
  SMTKCORE_EXPORT, smtk::view::ResourcePhraseModel, resource, ResourcePhrase);
