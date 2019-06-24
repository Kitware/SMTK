//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/ComponentPhraseModel.h"

#include "smtk/view/ComponentPhraseContent.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/EmptySubphraseGenerator.h"
#include "smtk/view/PhraseListContent.h"
#include "smtk/view/View.h"

#include "smtk/operation/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Component.h"

#include <algorithm> // for std::sort

using namespace smtk::view;

PhraseModelPtr ComponentPhraseModel::create(const smtk::view::View::Component& itemViewSpec)
{
  // Currently the itemViewSpec is not used but in the future it wll contain information to
  // customize how the model should behave
  (void)itemViewSpec;
  auto model = PhraseModel::Ptr(new ComponentPhraseModel);
  model->root()->findDelegate()->setModel(model);
  return model;
}

ComponentPhraseModel::ComponentPhraseModel()
  : m_root(DescriptivePhrase::create())
  , m_onlyShowActiveResourceComponents(false)
{
  auto generator = smtk::view::EmptySubphraseGenerator::create();
  m_root->setDelegate(generator);
}

ComponentPhraseModel::~ComponentPhraseModel()
{
  this->resetSources();
}

DescriptivePhrasePtr ComponentPhraseModel::root() const
{
  return m_root;
}

bool ComponentPhraseModel::setActiveResource(smtk::resource::ResourcePtr rsrc)
{
  if (rsrc == m_activeResource)
  {
    return false;
  }

  m_activeResource = rsrc;
  this->populateRoot();
  return true;
}

bool ComponentPhraseModel::setOnlyShowActiveResourceComponents(bool limitToActiveResource)
{
  if (limitToActiveResource == m_onlyShowActiveResourceComponents)
  {
    return false;
  }

  m_onlyShowActiveResourceComponents = limitToActiveResource;
  this->populateRoot();
  return true;
}

bool ComponentPhraseModel::setComponentFilters(const std::multimap<std::string, std::string>& src)
{
  if (src == m_componentFilters)
  {
    return false;
  }

  m_componentFilters = src;
  this->populateRoot();
  return true;
}

void ComponentPhraseModel::handleResourceEvent(Resource::Ptr rsrc, smtk::resource::EventType event)
{
  if (event != smtk::resource::EventType::MODIFIED)
  {
    this->processResource(rsrc, event == smtk::resource::EventType::ADDED);
  }
}

void ComponentPhraseModel::handleCreated(
  Operation::Ptr op, Operation::Result res, ComponentItemPtr data)
{
  (void)op;
  if (!res || !data)
  {
    return;
  }

  // TODO: Instead of looking for new resources (which perhaps we should leave to the
  //       handleResourceEvent()), we should optimize by adding new components to the
  //       root phrase if they pass m_componentFilters.
  for (auto it = data->begin(); it != data->end(); ++it)
  {
    auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(*it);
    if (comp == nullptr)
    {
      continue;
    }
    smtk::resource::ResourcePtr rsrc = comp->resource();
    this->processResource(rsrc, true);
  }

  // TODO: Instead of querying all resources in m_resources for a list of matching
  //       components, we should simply add anything in res that passes our test
  //       for matching components.
  this->populateRoot();

  // Finally, call our subclass method to deal with children of the root element
  this->PhraseModel::handleCreated(op, res, data);
}

void ComponentPhraseModel::processResource(Resource::Ptr rsrc, bool adding)
{
  if (adding)
  {
    // Insert the resource.
    if (m_resources.find(rsrc) == m_resources.end())
    {
      m_resources.insert(rsrc);
      this->populateRoot();
    }
  }
  else
  {
    // Delete the resource.
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
    if (rsrc == m_activeResource)
    {
      // Reset the active resource
      m_activeResource = smtk::resource::ResourcePtr();
      // If the resource was the active resource, then we have an issue since
      // now we have no context for what should be displayed. The thing most
      // developers would expect is to show all matching components from all
      // resources. This may not be what users expect, so perhaps we should
      // give some observer a way to respond to the situation here?
      this->populateRoot();
    }
  }
}

void ComponentPhraseModel::populateRoot()
{
  // FIXME: What should happen if m_componentFilters is empty?
  //        It seems like _all_ components (possibly just from the active resource)
  //        should be displayed. If so, we need to handle it here. On the other hand
  //        if nothing should be displayed, then no action is needed.
  smtk::resource::ComponentSet comps;
  if (m_onlyShowActiveResourceComponents)
  {
    if (m_activeResource)
    {
      for (auto filter : m_componentFilters)
      {
        // Skip filters that do not apply to this resource.
        if (!m_activeResource->isOfType(filter.first))
        {
          continue;
        }

        auto entries = m_activeResource->find(filter.second);
        comps.insert(entries.begin(), entries.end());
      }
    }
  }
  else
  {
    for (auto rsrc : m_resources)
    {
      for (auto filter : m_componentFilters)
      {
        if (!rsrc->isOfType(filter.first))
        {
          continue;
        }

        auto entries = rsrc->find(filter.second);
        comps.insert(entries.begin(), entries.end());
      }
    }
  }

  // Turn each entry of comps into a decorated phrase, sort, and update.
  DescriptivePhrases children;
  for (auto comp : comps)
  {
    children.push_back(
      smtk::view::ComponentPhraseContent::createPhrase(comp, m_mutableAspects, m_root));
  }
  std::sort(children.begin(), children.end(), DescriptivePhrase::compareByTypeThenTitle);
  this->root()->findDelegate()->decoratePhrases(children);
  this->updateChildren(m_root, children, std::vector<int>());
}
