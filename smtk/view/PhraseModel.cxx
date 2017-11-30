//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/view/PhraseModel.h"

#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/PhraseList.h"
#include "smtk/view/View.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Component.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace view
{

std::map<std::string, PhraseModel::ModelConstructor> PhraseModel::s_modelTypes;

bool PhraseModel::registerModelType(const std::string& typeName, ModelConstructor ctor)
{
  if (typeName.empty() || !ctor || s_modelTypes.find(typeName) != s_modelTypes.end())
  {
    return false;
  }
  s_modelTypes[typeName] = ctor;
  return true;
}

bool PhraseModel::unregisterModelType(const std::string& typeName)
{
  return s_modelTypes.erase(typeName) > 0;
}

PhraseModelPtr PhraseModel::create(const smtk::view::ViewPtr& viewSpec)
{
  if (!viewSpec || viewSpec->type().empty())
  {
    return nullptr;
  }
  auto entry = s_modelTypes.find(viewSpec->type());
  if (entry == s_modelTypes.end())
  {
    return nullptr;
  }
  return entry->second(viewSpec);
}

PhraseModel::PhraseModel()
{
}

PhraseModel::~PhraseModel()
{
  this->resetSources();
}

bool PhraseModel::addSource(smtk::resource::ManagerPtr rsrcMgr, smtk::operation::ManagerPtr operMgr)
{
  for (auto source : m_sources)
  {
    if (source.m_rsrcMgr == rsrcMgr && source.m_operMgr == operMgr)
    {
      return false; // Do not add what we already have
    }
  }
  int rsrcHandle = rsrcMgr
    ? rsrcMgr->observe([this](smtk::resource::Event event, Resource::Ptr rsrc) {
        this->handleResourceEvent(rsrc, event);
        return 0;
      })
    : -1;
  int operHandle = operMgr
    ? operMgr->observe([this](Operator::Ptr op, Operator::EventType event, Operator::Result res) {
        this->handleOperatorEvent(op, event, res);
        return 0;
      })
    : -1;
  m_sources.push_back(Source(rsrcMgr, operMgr, rsrcHandle, operHandle));
  return true;
}

bool PhraseModel::removeSource(
  smtk::resource::ManagerPtr rsrcMgr, smtk::operation::ManagerPtr operMgr)
{
  for (auto it = m_sources.begin(); it != m_sources.end(); ++it)
  {
    if (it->m_rsrcMgr == rsrcMgr && it->m_operMgr == operMgr)
    {
      if (it->m_rsrcHandle >= 0)
      {
        it->m_rsrcMgr->unobserve(it->m_rsrcHandle);
      }
      if (it->m_operHandle >= 0)
      {
        it->m_operMgr->unobserve(it->m_operHandle);
      }
      m_sources.erase(it);
      return true;
    }
  }
  return false;
}

bool PhraseModel::resetSources()
{
  bool removedAny = false;
  while (!m_sources.empty())
  {
    auto src = m_sources.begin();
    if (this->removeSource(src->m_rsrcMgr, src->m_operMgr))
    {
      removedAny = true;
    }
    else
    { // Failed to remove... you should not be here but perhaps we can prevent an infinite loop.
      m_sources.erase(m_sources.begin());
    }
  }
  return removedAny;
}

int PhraseModel::observe(Observer obs, bool immediatelyNotify)
{
  if (!obs)
  {
    return -1;
  }
  int handle = m_observers.empty() ? 0 : m_observers.rbegin()->first + 1;
  m_observers[handle] = obs;
  std::vector<int> parents;
  if (immediatelyNotify)
  {
    obs(this->root(), PhraseModelEvent::PHRASE_MODIFIED, parents, parents, std::vector<int>());
  }
  return handle;
}

bool PhraseModel::unobserve(int handle)
{
  return m_observers.erase(handle) > 0;
}

DescriptivePhrasePtr PhraseModel::root() const
{
  return DescriptivePhrasePtr();
}

void PhraseModel::handleSelectionEvent(const std::string& src, Selection::Ptr seln)
{
  (void)src;
  (void)seln;
}

void PhraseModel::handleResourceEvent(Resource::Ptr rsrc, smtk::resource::Event event)
{
  std::cout << "      phrase " << (event == smtk::resource::Event::RESOURCE_ADDED ? "add" : "del")
            << " rsrc " << rsrc << " " << rsrc->location() << "\n";
}

int PhraseModel::handleOperatorEvent(
  Operator::Ptr op, Operator::EventType event, Operator::Result res)
{
  std::cout << "      phrase op " << (event == Operator::EventType::DID_OPERATE ? "ran" : "cre/pre")
            << " " << op << "\n";

  if (!op)
  {
    return 1;
  } // Every event should have an operator
  if (!res)
  {
    return 0;
  } // Ignore operator creation and about-to-operate events.
  if (event != Operator::DID_OPERATE)
  {
    return 0;
  }

  // Find out which resource components were created, modified, or expunged.
  this->handleExpunged(op, res, res->findComponent("expunged components"));
  this->handleModified(op, res, res->findComponent("modified components"));
  this->handleCreated(op, res, res->findComponent("created components"));
  return 0;
}

void PhraseModel::handleExpunged(Operator::Ptr op, Operator::Result res, ComponentItemPtr data)
{
  (void)op;
  (void)res;
  (void)data;
}

void PhraseModel::handleModified(Operator::Ptr op, Operator::Result res, ComponentItemPtr data)
{
  (void)op;
  (void)res;

  if (!data)
  {
    return;
  }

  std::set<smtk::resource::ComponentPtr> modified(data->begin(), data->end());
  auto rootPhrase = this->root();
  if (rootPhrase)
  {
    rootPhrase->visitChildren(
      [this, &modified](DescriptivePhrasePtr phr, const std::vector<int>& idx) -> int {
        if (modified.find(phr->relatedComponent()) != modified.end())
        {
          this->trigger(phr, PhraseModelEvent::PHRASE_MODIFIED, idx, idx, std::vector<int>());
        }
        return 0;
      });
  }
}

void PhraseModel::handleCreated(Operator::Ptr op, Operator::Result res, ComponentItemPtr data)
{
  (void)op;
  (void)res;
  (void)data;

  if (!data)
  {
    return;
  }

  for (auto cre = data->begin(); cre != data->end(); ++cre)
  {
    std::cout << "    add " << (*cre)->id() << " somewhere\n";
  }
}

void PhraseModel::updateChildren(
  smtk::view::PhraseListPtr plist, const DescriptivePhrases& next, const std::vector<int>& idx)
{
  if (!plist)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Null phrase list.");
    return;
  }

  std::map<DescriptivePhrasePtr, int> lkup;
  int ii = 0;
  std::set<int> unused;
  DescriptivePhrases& orig(plist->subphrases());
  for (auto it = orig.begin(); it != orig.end(); ++it, ++ii)
  {
    unused.insert(ii);
    lkup[*it] = ii;
  }

  // Find mapping and subtract out the still-in-use phrases from unused.
  for (auto it = next.begin(); it != next.end(); ++it)
  {
    auto oit = lkup.find(*it);
    if (oit != lkup.end())
    {
      unused.erase(oit->second);
    }
  }

  // Now delete unused from model, starting at the back so we don't invalidate indices.
  std::vector<int> removalRange(2);
  for (auto uu = unused.rbegin(); uu != unused.rend(); ++uu)
  {
    auto u2 = uu;
    auto ug = u2;
    for (++u2; u2 != unused.rend() && (*u2 == (*ug - 1)); ++u2)
    {
      ug = u2;
    }
    removalRange[0] = *ug;
    removalRange[1] = *uu;
    this->trigger(plist, PhraseModelEvent::ABOUT_TO_REMOVE, idx, idx, removalRange);
    orig.erase(orig.begin() + removalRange[0], orig.begin() + removalRange[1] + 1);
    this->trigger(plist, PhraseModelEvent::REMOVE_FINISHED, idx, idx, removalRange);
    uu = ug;
  }

  // Update lkup to account for removed rows.
  int delta = 0;
  auto nxen = lkup.begin();
  for (auto entry = lkup.begin(); entry != lkup.end(); ++entry)
  {
    while (entry != lkup.end() && unused.find(entry->second) != unused.end())
    { // row is unused
      ++delta;
      nxen = entry;
      ++nxen;
      lkup.erase(entry);
      entry = nxen;
    }
    if (entry != lkup.end())
    { // row used... update as needed
      entry->second -= delta;
    }
  }

  std::map<int, DescriptivePhrasePtr> insert;

  // Move rows that previously existed but are now ordered differently.
  // int destRow = static_cast<int>(lkup.size()) - 1;
  int iref = static_cast<int>(next.size()) - 1;
  for (auto it = next.rbegin(); it != next.rend(); ++it, --iref)
  {
    auto oit = lkup.find(*it);
    if (oit == lkup.end())
    { // Skip rows we need to create
      insert[iref] = *it;
      continue;
    }
    // Is this a batch of rows to move in sequence?
    // FIXME: TODO: Move rows!
  }

  // Finally, insert new phrases.
  std::vector<int> insertRange(2);
  DescriptivePhrases batch;
  std::map<int, DescriptivePhrasePtr>::iterator ib;
  for (ib = insert.begin(); ib != insert.end();)
  {
    std::cout << "ib valid\n";
    std::cout << "  " << ib->first << " " << ib->second << "\n";
    batch.clear();
    batch.push_back(ib->second);
    auto ie = ib;
    auto prev = ie;
    for (++ie; ie != insert.end() && (ie->first == prev->first + 1); ++ie, ++prev)
    {
      batch.push_back(ie->second);
    }
    insertRange[0] = ib->first;
    insertRange[1] = ie->first;
    this->trigger(plist, PhraseModelEvent::ABOUT_TO_INSERT, idx, idx, insertRange);
    orig.insert(orig.begin() + insertRange[0], batch.begin(), batch.end());
    this->trigger(plist, PhraseModelEvent::INSERT_FINISHED, idx, idx, insertRange);
    ib = ie;
    if (ib != insert.end())
    {
      ++ib;
    }
  }
}

void PhraseModel::trigger(DescriptivePhrasePtr phr, PhraseModelEvent event,
  const std::vector<int>& src, const std::vector<int>& dst, const std::vector<int>& arg)
{
  for (auto observer : m_observers)
  {
    observer.second(phr, event, src, dst, arg);
  }
}
}
}
