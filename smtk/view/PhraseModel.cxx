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
#include "smtk/view/PhraseListContent.h"
#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/View.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Component.h"

#include "smtk/io/Logger.h"

#undef SMTK_PHRASE_DEBUG

namespace smtk
{
namespace view
{
namespace detail
{

// Sort paths from deepest to shallowest, then rear-most to front-most.
// Doing these things keeps us from invalidating paths when items are removed.
struct PathComp
{
  bool operator()(const std::vector<int>& a, const std::vector<int>& b) const
  {
    if (a.size() < b.size())
    {
      return false;
    }
    else if (a.size() > b.size())
    {
      return true;
    }
    std::size_t ii = 0;
    for (auto ai : a)
    {
      if (ai < b[ii])
      {
        return false;
      }
      else if (ai > b[ii])
      {
        return true;
      }
      ++ii;
    }
    return false; // a == b... neither is less than other.
  }
};
}

class PhraseDeltas : public std::set<std::vector<int>, detail::PathComp>
{
};

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
  m_decorator = [](smtk::view::DescriptivePhrasePtr) {};
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
    ? rsrcMgr->observers().insert(
        [this](Resource::Ptr rsrc, resource::EventType event) {
          this->handleResourceEvent(rsrc, event);
          return 0;
        },
        /*observeImmediately*/ true)
    : -1;
  int operHandle = operMgr
    ? operMgr->observers().insert(
        [this](Operation::Ptr op, operation::EventType event, Operation::Result res) {
          this->handleOperationEvent(op, event, res);
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
        it->m_rsrcMgr->observers().erase(it->m_rsrcHandle);
      }
      if (it->m_operHandle >= 0)
      {
        it->m_operMgr->observers().erase(it->m_operHandle);
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

static void notifyRecursive(
  PhraseModel::Observer obs, DescriptivePhrasePtr parent, std::vector<int>& parentIdx)
{
  if (!parent || !parent->areSubphrasesBuilt())
  {
    return;
  }
  std::vector<int> range(2);
  DescriptivePhrases& children(parent->subphrases());
  range[0] = 0;
  range[1] = static_cast<int>(children.size());
  obs(parent, PhraseModelEvent::ABOUT_TO_INSERT, parentIdx, parentIdx, range);
  obs(parent, PhraseModelEvent::INSERT_FINISHED, parentIdx, parentIdx, range);
  parentIdx.push_back(0);
  for (auto child : children)
  {
    notifyRecursive(obs, child, parentIdx);
    ++parentIdx.back();
  }
  parentIdx.pop_back();
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
    notifyRecursive(obs, this->root(), parents);
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

void PhraseModel::handleResourceEvent(Resource::Ptr rsrc, smtk::resource::EventType event)
{
  std::cout << "      phrase " << (event == smtk::resource::EventType::ADDED ? "add" : "del")
            << " rsrc " << rsrc << " " << rsrc->location() << "\n";
}

int PhraseModel::handleOperationEvent(
  Operation::Ptr op, operation::EventType event, Operation::Result res)
{
  smtkDebugMacro(smtk::io::Logger::instance(), "      Phrase handler: op "
      << (event == operation::EventType::DID_OPERATE ? "ran" : "cre/pre") << " " << op);

  if (!op)
  {
    return 1;
  } // Every event should have an operator
  if (!res)
  {
    return 0;
  } // Ignore operator creation and about-to-operate events.
  if (event != operation::EventType::DID_OPERATE)
  {
    return 0;
  }

  // Find out which resource components were created, modified, or expunged.
  this->handleExpunged(op, res, res->findComponent("expunged"));
  this->handleModified(op, res, res->findComponent("modified"));
  this->handleCreated(op, res, res->findComponent("created"));
  return 0;
}

void PhraseModel::removeChildren(const std::vector<int>& parentIdx, int childRange[2])
{
  auto phr = this->root()->at(parentIdx);
  std::vector<int> removeRange{ childRange[0], childRange[1] };

  this->trigger(phr, PhraseModelEvent::ABOUT_TO_REMOVE, parentIdx, parentIdx, removeRange);
  phr->subphrases().erase(phr->subphrases().begin() + removeRange[0]);
  this->trigger(phr, PhraseModelEvent::REMOVE_FINISHED, parentIdx, parentIdx, removeRange);
}

void PhraseModel::handleExpunged(Operation::Ptr op, Operation::Result res, ComponentItemPtr data)
{
  (void)op;
  (void)res;

  if (!data)
  {
    return;
  }

  // By default, search all existing phrases for matching components and remove them.
  std::set<smtk::common::UUID> uuids;
  for (auto it = data->begin(); it != data->end(); ++it)
  {
    uuids.insert((*it)->id());
  }

  PhraseDeltas pd;

  // TODO: It would be more efficient to aggregate all deletions that share a
  // common path vector and update that path's children at once.
  this->root()->visitChildren([&uuids, &pd](DescriptivePhrasePtr phr, std::vector<int>& path) {
    auto comp = phr->relatedComponent();
    if (comp && uuids.find(comp->id()) != uuids.end())
    {
      pd.insert(path);
    }
    return 0;
  });

  for (auto idx : pd)
  {
    if (idx.empty())
    {
      continue;
    }

    int removeRange[2];
    removeRange[0] = idx.back();
    removeRange[1] = removeRange[0];
    idx.pop_back();
    this->removeChildren(idx, removeRange);
  }
}

void PhraseModel::handleModified(Operation::Ptr op, Operation::Result res, ComponentItemPtr data)
{
  (void)op;
  (void)res;

  if (!data)
  {
    return;
  }

  std::set<smtk::resource::PersistentObjectPtr> modified(data->begin(), data->end());
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

void PhraseModel::handleCreated(Operation::Ptr op, Operation::Result res, ComponentItemPtr data)
{
  (void)op;  // Ignore this in the general case but allow subclasses to special-case it.
  (void)res; // TODO: Different behavior when result is failure vs success?

  if (!data)
  {
    return;
  }

  auto rootPhrase = this->root();
  auto delegate = rootPhrase ? rootPhrase->findDelegate() : nullptr;
  if (!delegate)
  {
    return;
  }

  smtk::resource::PersistentObjectArray objects;
  for (auto cre = data->begin(); cre != data->end(); ++cre)
  {
    // std::cout << "    add " << (*cre)->id() << " somewhere\n";
    objects.push_back(*cre);
  }

  SubphraseGenerator::PhrasesByPath phrasesToInsert;
  delegate->subphrasesForCreatedObjects(objects, rootPhrase, phrasesToInsert);

  std::vector<int> insertRange{ 0, 0 };
  for (auto pathIt = phrasesToInsert.begin(); pathIt != phrasesToInsert.end();)
  {
    SubphraseGenerator::Path path = pathIt->first;
    auto parent = pathIt->second->parent();
    SubphraseGenerator::Path idx(
      path.begin(), path.begin() + path.size() - 1); // Index of parent, not phrase to be inserted.
    insertRange[0] = path.back();
    insertRange[1] = insertRange[0];

    DescriptivePhrases batch;
    batch.push_back(pathIt->second);
    for (++pathIt; pathIt != phrasesToInsert.end() && pathIt->first == path; ++pathIt)
    {
      batch.push_back(pathIt->second);
      ++insertRange[1];
    }
    this->trigger(parent, PhraseModelEvent::ABOUT_TO_INSERT, idx, idx, insertRange);
    auto& children = parent->subphrases();
    children.insert(children.begin() + path.back(), batch.begin(), batch.end());
    this->trigger(parent, PhraseModelEvent::INSERT_FINISHED, idx, idx, insertRange);
  }
}

void PhraseModel::updateChildren(
  smtk::view::DescriptivePhrasePtr src, DescriptivePhrases& next, const std::vector<int>& idx)
{
  if (!src)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Null phrase list.");
    return;
  }

  std::map<DescriptivePhrasePtr, int> lkup;
  int ii = 0;
  std::set<int> unused;
  DescriptivePhrases& orig(src->subphrases());
  for (auto it = orig.begin(); it != orig.end(); ++it, ++ii)
  {
    unused.insert(ii);
    lkup[*it] = ii;
  }

  // Find mapping and subtract out the still-in-use phrases from unused.
  for (DescriptivePhrases::iterator it = next.begin(); it != next.end(); ++it)
  {
    auto oit = lkup.find(*it);
    if (oit != lkup.end())
    {
      unused.erase(oit->second);
    }
    else
    {
      // Run through the entire array to see if there are different instances
      // of the phrase that nonetheless behave identically.
      for (auto it2 = orig.begin(); it2 != orig.end(); ++it2)
      {
        if (it->get() == it2->get())
        {
          *it = *it2;
          unused.erase(lkup[*it2]);
        }
      }
    }
  }
  // But wait, we might have phrases that are different instances
  // but identical in that they refer to the same component/resource/etc.

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
    this->trigger(src, PhraseModelEvent::ABOUT_TO_REMOVE, idx, idx, removalRange);
    orig.erase(orig.begin() + removalRange[0], orig.begin() + removalRange[1] + 1);
    this->trigger(src, PhraseModelEvent::REMOVE_FINISHED, idx, idx, removalRange);
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
    else
    {
      break;
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
    batch.clear();
    batch.push_back(ib->second);
    auto ie = ib;
    auto prev = ie;
    for (++ie; ie != insert.end() && (ie->first == prev->first + 1); ++ie, ++prev)
    {
      batch.push_back(ie->second);
    }
    insertRange[0] = ib->first;
    insertRange[1] = prev->first;
    this->trigger(src, PhraseModelEvent::ABOUT_TO_INSERT, idx, idx, insertRange);
    orig.insert(orig.begin() + insertRange[0], batch.begin(), batch.end());
    this->trigger(src, PhraseModelEvent::INSERT_FINISHED, idx, idx, insertRange);
    ib = ie;
    if (ib != insert.end())
    {
      ++ib;
    }
  }
}

void PhraseModel::decoratePhrase(smtk::view::DescriptivePhrasePtr phr) const
{
  m_decorator(phr);
}

bool PhraseModel::setDecorator(const PhraseDecorator& phraseDecorator)
{
  if (!phraseDecorator)
  {
    return false;
  }

  // TODO: Add warning if we already have phrases? Un-decorate and re-decorate?

  m_decorator = phraseDecorator;
  return true;
}

void PhraseModel::triggerDataChanged()
{
  // It is possible the 2 lines below could work, but the feature is not well-documented in Qt:
  //   auto idx = this->root()->index();
  //   this->trigger(this->root(), PhraseModelEvent::PHRASE_MODIFIED, idx, idx, std::vector<int>());
  // Just to be safe, do it the long way:
  this->root()->visitChildren([this](DescriptivePhrasePtr phr, std::vector<int>& idx) -> int {
    this->trigger(phr, PhraseModelEvent::PHRASE_MODIFIED, idx, idx, std::vector<int>());
    return 0; // continue iterating.
  });
}

void PhraseModel::triggerDataChangedFor(smtk::resource::ComponentPtr comp)
{
  this->root()->visitChildren(
    [this, &comp](DescriptivePhrasePtr phr, std::vector<int>& idx) -> int {
      if (phr->relatedComponent() == comp)
      {
        this->trigger(phr, PhraseModelEvent::PHRASE_MODIFIED, idx, idx, std::vector<int>());
      }
      return 0; // continue iterating.
    });
}

int depth(DescriptivePhrasePtr phr)
{
  int dd = -1;
  while (phr)
  {
    ++dd;
    phr = phr->parent();
  }
  return dd;
}

void PhraseModel::trigger(DescriptivePhrasePtr phr, PhraseModelEvent event,
  const std::vector<int>& src, const std::vector<int>& dst, const std::vector<int>& arg)
{
#ifdef SMTK_PHRASE_DEBUG
  std::cout << "Phrase model update " << phr << " pdepth " << depth(phr) << " event "
            << static_cast<int>(event) << " src";
  for (auto ent : src)
  {
    std::cout << " " << ent;
  }
  std::cout << " dst";
  for (auto ent : dst)
  {
    std::cout << " " << ent;
  }
  std::cout << " arg";
  for (auto ent : arg)
  {
    std::cout << " " << ent;
  }
  std::cout << "\n";
  std::cout.flush();
#endif

  for (auto observer : m_observers)
  {
    observer.second(phr, event, src, dst, arg);
  }
  // Check to see if phrases we just inserted have pre-existing children. If so, trigger them.
  if (event == PhraseModelEvent::INSERT_FINISHED && phr && phr->areSubphrasesBuilt())
  {
    std::vector<int> range(2);
    DescriptivePhrases& children(phr->subphrases());
    range[0] = 0;
    for (int ci = arg[0]; ci <= arg[1]; ++ci)
    {
      if (!children[ci] || !children[ci]->areSubphrasesBuilt())
      {
        continue;
      }
      range[1] = static_cast<int>(children[ci]->subphrases().size());
      // Qt expects range to be closed, not half-open like the STL.
      // But it is possible to have subphrases built but be empty (size() == 0), so:
      if (range[1] == 0)
      {
        continue;
      }
      --range[1];
      std::vector<int> cpath = children[ci]->index();
      this->trigger(children[ci], PhraseModelEvent::ABOUT_TO_INSERT, cpath, cpath, range);
      this->trigger(children[ci], PhraseModelEvent::INSERT_FINISHED, cpath, cpath, range);
    }
  }
}
}
}
