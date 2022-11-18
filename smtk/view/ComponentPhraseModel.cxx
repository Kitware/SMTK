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
#include "smtk/view/Configuration.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/EmptySubphraseGenerator.h"
#include "smtk/view/Manager.h"
#include "smtk/view/PhraseListContent.h"

#include "smtk/operation/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Component.h"

#include <algorithm> // for std::sort

using namespace smtk::view;

ComponentPhraseModel::ComponentPhraseModel()
  : m_root(DescriptivePhrase::create())
{
  auto generator = smtk::view::EmptySubphraseGenerator::create();
  m_root->setDelegate(generator);
}

ComponentPhraseModel::ComponentPhraseModel(const Configuration* config, Manager* manager)
  : Superclass(config, manager)
  , m_root(DescriptivePhrase::create())
{
  auto generator = PhraseModel::configureSubphraseGenerator(config, manager);
  m_root->setDelegate(generator);

  std::multimap<std::string, std::string> filters =
    PhraseModel::configureFilterStrings(config, manager);
  this->setComponentFilters(filters);
}

ComponentPhraseModel::~ComponentPhraseModel()
{
  this->resetSources();
}

DescriptivePhrasePtr ComponentPhraseModel::root() const
{
  return m_root;
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

void ComponentPhraseModel::handleResourceEvent(
  const Resource& resource,
  smtk::resource::EventType event)
{
  if (event != smtk::resource::EventType::MODIFIED)
  {
    // The PhraseModle system has been designed to handle shared pointers to
    // non-const resources and components. We const-cast here to accommodate
    // this pattern.
    smtk::resource::ResourcePtr rsrc =
      const_cast<smtk::resource::Resource&>(resource).shared_from_this();
    this->processResource(rsrc, event == smtk::resource::EventType::ADDED);
  }
}

DescriptivePhrasePtr ComponentPhraseModel::createTopPhrase(const smtk::resource::ComponentPtr& comp)
{
  if (comp == nullptr)
  {
    return nullptr;
  }
  auto resource = comp->resource();
  if (resource == nullptr)
  {
    return nullptr;
  }
  for (const auto& filter : m_componentFilters)
  {
    if (resource->isOfType(filter.first))
    {
      auto query = resource->queryOperation(filter.second);
      if (query(*comp))
      {
        return smtk::view::ComponentPhraseContent::createPhrase(comp);
      }
    }
  }
  return nullptr;
}

void ComponentPhraseModel::handleCreated(const smtk::resource::PersistentObjectSet& createdObjects)
{
  // TODO: Instead of looking for new resources (which perhaps we should leave to the
  //       handleResourceEvent()), we should optimize by adding new components to the
  //       root phrase if they pass m_componentFilters.
  for (auto it = createdObjects.begin(); it != createdObjects.end(); ++it)
  {
    auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(*it);
    if (comp == nullptr)
    {
      continue;
    }
    smtk::resource::ResourcePtr rsrc = comp->resource();
    this->processResource(rsrc, true);
  }
  // We need to determine which components will be top-level and insert them
  // into the root's subphrases.

  // Create a vector of new top level phrases
  DescriptivePhrases newTopLevelPhrases;
  for (const auto& obj : createdObjects)
  {
    auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj);
    if (comp)
    {
      auto topPhrase = this->createTopPhrase(comp);
      if (topPhrase)
      {
        this->setPhraseParent(topPhrase, m_root);
        newTopLevelPhrases.push_back(topPhrase);
      }
    }
  }
  // Did we find top-level phrases?
  if (!newTopLevelPhrases.empty())
  {
    // let's sort the phrases to be in the proper order
    std::sort(newTopLevelPhrases.begin(), newTopLevelPhrases.end(), m_comparator);

    // Now let's insert this ordered set into the root's ordered subphrases
    auto& rootPhrases = m_root->subphrases();
    auto rit = rootPhrases.begin();
    auto nit = newTopLevelPhrases.begin();
    std::vector<int> rootPath; // Which should be empty
    std::vector<int> range(2);

    while (nit != newTopLevelPhrases.end())
    {
      // Have we reached the end of the root's subphrases?
      if (rit == rootPhrases.end())
      {
        // The remaining phrases need to be appended to the end
        range[0] = static_cast<int>(rootPhrases.size());
        range[1] = range[0] + (static_cast<int>(std::distance(nit, newTopLevelPhrases.end())) - 1);
        this->trigger(m_root, PhraseModelEvent::ABOUT_TO_INSERT, rootPath, rootPath, range);
        for (; nit != newTopLevelPhrases.end(); nit++)
        {
          rootPhrases.push_back(*nit);
        }
        this->trigger(m_root, PhraseModelEvent::INSERT_FINISHED, rootPath, rootPath, range);
        break;
      }
      // Does the next new toplevel phrase go before the current subphrase in the root?
      if (m_comparator(*nit, *rit))
      {
        range[0] = static_cast<int>(std::distance(rootPhrases.begin(), rit));
        range[1] = range[0];
        this->trigger(m_root, PhraseModelEvent::ABOUT_TO_INSERT, rootPath, rootPath, range);
        rit = rootPhrases.insert(rit, *nit);
        this->trigger(m_root, PhraseModelEvent::INSERT_FINISHED, rootPath, rootPath, range);
        nit++;
      }
      // Move to the next subphrase in the root
      else
      {
        rit++;
      }
    }
  }

  // Finally, call our subclass method to deal with children of the root element
  this->PhraseModel::handleCreated(createdObjects);
}

void ComponentPhraseModel::processResource(const smtk::resource::ResourcePtr& rsrc, bool adding)
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
      children.erase(
        std::remove_if(
          children.begin(),
          children.end(),
          [&rsrc](const DescriptivePhrase::Ptr& phr) -> bool {
            return phr->relatedResource() == rsrc;
          }),
        children.end());
      this->updateChildren(m_root, children, std::vector<int>());
    }
  }
}

void ComponentPhraseModel::populateRoot()
{
  // FIXME: What should happen if m_componentFilters is empty?
  //        It seems like _all_ components should be displayed.
  //        If so, we need to handle it here. On the other hand
  //        if nothing should be displayed, then no action is needed.
  smtk::resource::ComponentSet comps;
  for (const auto& rsrc : m_resources)
  {
    auto resource = rsrc.lock();
    if (resource == nullptr)
    {
      continue;
    }

    for (const auto& filter : m_componentFilters)
    {
      if (!resource->isOfType(filter.first))
      {
        continue;
      }

      auto entries = resource->filter(filter.second);
      comps.insert(entries.begin(), entries.end());
    }
  }

  // Turn each entry of comps into a decorated phrase, sort, and update.
  DescriptivePhrases children;
  for (const auto& comp : comps)
  {
    children.push_back(
      smtk::view::ComponentPhraseContent::createPhrase(comp, m_mutableAspects, m_root));
  }
  std::sort(children.begin(), children.end(), m_comparator);
  this->updateChildren(m_root, children, std::vector<int>());
}

void ComponentPhraseModel::setSortFunction(const SortingCompFunc& comparator)
{
  m_comparator = comparator;
  this->populateRoot();
}
