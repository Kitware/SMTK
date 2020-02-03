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

#include "smtk/view/Configuration.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/Manager.h"
#include "smtk/view/PhraseListContent.h"
#include "smtk/view/SubphraseGenerator.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"

#include "smtk/resource/Component.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace view
{
namespace
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

void notifyRecursive(
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
  for (const auto& child : children)
  {
    notifyRecursive(obs, child, parentIdx);
    ++parentIdx.back();
  }
  parentIdx.pop_back();
}

void notify(PhraseModel::Observer obs, DescriptivePhrasePtr parent)
{
  std::vector<int> parentIdx;
  return notifyRecursive(obs, parent, parentIdx);
}
}

class PhraseDeltas : public std::set<std::vector<int>, PathComp>
{
};

// Returns the operation manager - right now it assumes the first source
// TODO: figure out the proper behavior when there is more
// than one source
smtk::operation::ManagerPtr PhraseModel::operationManager() const
{
  if (!m_sources.empty())
  {
    return m_sources.front().m_operMgr;
  }
  return nullptr;
}

PhraseModelPtr PhraseModel::create(
  const smtk::view::ConfigurationPtr& viewSpec, const smtk::view::ManagerPtr& manager)
{
  if (!manager || !viewSpec || viewSpec->name().empty() || viewSpec->name() != "ResourceBrowser")
  {
    return nullptr;
  }
  // Look at viewSpec child, should be "PhraseModel", look for its Type attribute
  std::string typeName;
  if ((viewSpec->details().numberOfChildren() != 1) ||
    (viewSpec->details().child(0).name() != "PhraseModel"))
  {
    return nullptr;
  }
  viewSpec->details().child(0).attribute("Type", typeName);

  // find things that match the Type, and create one.
  return manager->create(typeName);
}

PhraseModel::PhraseModel()
  : m_observers(std::bind(notify, std::placeholders::_1, this->root()))
  , m_mutableAspects(PhraseContent::EVERYTHING)
{
  m_decorator = [](smtk::view::DescriptivePhrasePtr /*unused*/) {};
}

PhraseModel::~PhraseModel()
{
  this->resetSources();
}

bool PhraseModel::addSource(smtk::resource::ManagerPtr rsrcMgr, smtk::operation::ManagerPtr operMgr,
  smtk::view::ManagerPtr viewMgr, smtk::view::SelectionPtr seln)
{
  for (const auto& source : m_sources)
  {
    if (source.m_rsrcMgr == rsrcMgr && source.m_operMgr == operMgr && source.m_viewMgr == viewMgr &&
      source.m_seln == seln)
    {
      return false; // Do not add what we already have
    }
  }
  std::ostringstream description;
  description << "PhraseModel " << this << ": ";
  auto rsrcHandle = rsrcMgr
    ? rsrcMgr->observers().insert(
        [this](const Resource& rsrc, const resource::EventType& event) {
          this->handleResourceEvent(rsrc, event);
          return 0;
        },
        0,    // assign a neutral priority
        true, // observeImmediately
        description.str() + "Update phrases when resources change.")
    : smtk::resource::Observers::Key();
  auto operHandle = operMgr
    ? operMgr->observers().insert(
        [this](const Operation& op, operation::EventType event, const Operation::Result& res) {
          this->handleOperationEvent(op, event, res);
          return 0;
        },
        description.str() + "Update phrases based on operation results.")
    : smtk::operation::Observers::Key();
  auto selnHandle = seln
    ? seln->observers().insert(
        [this](const std::string& src, smtk::view::SelectionPtr seln) {
          this->handleSelectionEvent(src, seln);
        },
        0,    // assign a neutral priority
        true, // observeImmediately
        description.str() + "Update phrases when selection changes.")
    : smtk::view::SelectionObservers::Key();
  m_sources.emplace_back(rsrcMgr, operMgr, viewMgr, seln, rsrcHandle, operHandle, selnHandle);
  return true;
}

bool PhraseModel::removeSource(smtk::resource::ManagerPtr rsrcMgr,
  smtk::operation::ManagerPtr operMgr, smtk::view::ManagerPtr viewMgr,
  smtk::view::SelectionPtr seln)
{
  for (auto it = m_sources.begin(); it != m_sources.end(); ++it)
  {
    if (it->m_rsrcMgr == rsrcMgr && it->m_operMgr == operMgr && it->m_viewMgr == viewMgr &&
      it->m_seln == seln)
    {
      if (it->m_rsrcHandle != smtk::resource::Observers::Key())
      {
        it->m_rsrcMgr->observers().erase(it->m_rsrcHandle);
      }
      if (it->m_operHandle != smtk::operation::Observers::Key())
      {
        it->m_operMgr->observers().erase(it->m_operHandle);
      }
      if (it->m_selnHandle != smtk::view::SelectionObservers::Key())
      {
        it->m_seln->observers().erase(it->m_selnHandle);
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
    if (this->removeSource(src->m_rsrcMgr, src->m_operMgr, src->m_viewMgr, src->m_seln))
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

void PhraseModel::visitSources(SourceVisitor visitor)
{
  for (const auto& src : m_sources)
  {
    if (!visitor(src.m_rsrcMgr, src.m_operMgr, src.m_viewMgr, src.m_seln))
    {
      break;
    }
  }
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

void PhraseModel::handleResourceEvent(const Resource& rsrc, smtk::resource::EventType event)
{
  std::cout << "      phrase " << (event == smtk::resource::EventType::ADDED ? "add" : "del")
            << " rsrc " << &rsrc << " " << rsrc.location() << "\n";
}

int PhraseModel::handleOperationEvent(
  const Operation& op, operation::EventType event, const Operation::Result& res)
{
  smtkDebugMacro(smtk::io::Logger::instance(), "      Phrase handler: op "
      << (event == operation::EventType::DID_OPERATE ? "ran" : "cre/pre") << " " << &op);

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

void PhraseModel::handleExpunged(
  const Operation& op, const Operation::Result& res, const ComponentItemPtr& data)
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
    if (!it.isSet())
    {
      continue;
    }
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

void PhraseModel::handleModified(
  const Operation& op, const Operation::Result& res, const ComponentItemPtr& data)
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
          // Now check whether the modification requires a reorder
          auto pp = phr->parent();
          smtk::view::DescriptivePhrases sorted(pp->subphrases().begin(), pp->subphrases().end());
          std::sort(sorted.begin(), sorted.end(), DescriptivePhrase::compareByTypeThenTitle);
          std::vector<int> pidx(idx.begin(), idx.begin() + idx.size() - 1);
          this->updateChildren(pp, sorted, pidx);
        }
        return 0;
      });
  }
}

void PhraseModel::handleCreated(
  const Operation& op, const Operation::Result& res, const ComponentItemPtr& data)
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

void PhraseModel::redecorate()
{
  this->root()->visitChildren(
    [this](DescriptivePhrasePtr phr, std::vector<int> & /*unused*/) -> int {
      PhraseContentPtr topContent = phr->content();
      PhraseContentPtr content = topContent->undecoratedContent();
      if (content != topContent)
      {
        phr->setContent(content);
      }
      this->decoratePhrase(phr);
      return 0; // Continue iterating, incl. children.
    });
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
  std::map<int, DescriptivePhrasePtr> insert;
  int ii = 0;
  std::set<int> unused;
  DescriptivePhrases& orig(src->subphrases());
  for (auto it = orig.begin(); it != orig.end(); ++it, ++ii)
  {
    unused.insert(ii);
    lkup[*it] = ii;
  }

  // Find mapping and subtract out the still-in-use phrases from unused.
  ii = 0;
  for (DescriptivePhrases::iterator it = next.begin(); it != next.end(); ++it, ++ii)
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
      // Note that we might have phrases that are different instances
      // but identical in that they refer to the same component/resource/etc.,
      // hence the second test in the conditional below:
      bool preexist = false;
      DescriptivePhrase* nval = it->get();
      for (auto it2 = orig.begin(); it2 != orig.end(); ++it2)
      {
        if ((*nval == **it2) ||
          (nval && nval->relatedObject() && nval->relatedObject() == it2->get()->relatedObject()))
        {
          *it = *it2;
          unused.erase(lkup[*it2]);
          preexist = true;
          break; // only accept the first match.
        }
      }
      // If there is no matching entry in orig for *it, then mark its
      // location for insertion.
      if (!preexist)
      {
        insert[ii] = *it;
      }
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
    this->trigger(src, PhraseModelEvent::ABOUT_TO_REMOVE, idx, idx, removalRange);
    orig.erase(orig.begin() + removalRange[0], orig.begin() + removalRange[1] + 1);
    this->trigger(src, PhraseModelEvent::REMOVE_FINISHED, idx, idx, removalRange);
    uu = ug;
  }

  if (insert.size() + orig.size() != next.size())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Update to descriptive phrases has correspondence problems"
        << " (" << orig.size() << " + " << insert.size() << " != " << next.size() << ")\n");
  }

  // Insert new entries, starting at the front so as not to attempt
  // insertion beyond the end of the list.
  DescriptivePhrases batch;
  std::vector<int> insertRange(2);
  for (auto entry = insert.begin(); entry != insert.end(); /* handled inside */)
  {
    // Batch consecutive insertions into a single event
    batch.push_back(entry->second);
    int osz = static_cast<int>(orig.size());
    insertRange[0] = entry->first > osz ? osz : entry->first;
    insertRange[1] = entry->first;
    ii = entry->first;
    auto ep1 = entry;
    for (++ep1, ++ii; ep1 != insert.end() && ep1->first == ii; ++ep1, ++ii)
    {
      batch.push_back(ep1->second);
      insertRange[1] = ii;
    }

    this->trigger(src, PhraseModelEvent::ABOUT_TO_INSERT, idx, idx, insertRange);
    orig.insert(orig.begin() + insertRange[0], batch.begin(), batch.end());
    this->trigger(src, PhraseModelEvent::INSERT_FINISHED, idx, idx, insertRange);
    batch.clear();

    entry = ep1;
  }

  // Now reconcile entries that have been reordered in orig.
  auto ni = next.begin();
  auto oi = orig.begin();
  for (ii = 0; ni != next.end() && oi != orig.end(); ++ii)
  {
    if (*ni == *oi)
    { // Entries match.
      ++ni;
      ++oi;
    }
    else
    { // Reorder required.
      auto mv = ni;
      for (; mv != next.end() && *mv != *oi; ++mv)
      {
        // do nothing
      }
      if (mv == next.end())
      {
        smtkErrorMacro(smtk::io::Logger::instance(), "Mismatched entries in descriptive phrase.");
        break;
      }
      else
      {
        // We have a start and destination location; see if we
        // should batch move.
        auto bi = oi;
        auto ci = mv;
        for (++bi, ++ci; bi != orig.end() && ci != next.end() && *bi == *ci; ++bi, ++ci)
        {
          // Do nothing (advancing to find batch size)
        }
        std::vector<int> moveRange(3);
        moveRange[0] = static_cast<int>(oi - orig.begin());
        moveRange[1] = static_cast<int>(bi == orig.end() ? orig.size() : bi - orig.begin()) - 1;
        moveRange[2] = static_cast<int>((mv - next.begin()) + (moveRange[1] - moveRange[0]) + 1);
        this->trigger(src, PhraseModelEvent::ABOUT_TO_MOVE, idx, idx, moveRange);
        // Copy batch to destination (which must be *after* source)
        DescriptivePhrases moved(orig.begin() + moveRange[0], orig.begin() + moveRange[1] + 1);
        orig.insert(orig.begin() + moveRange[2], moved.begin(), moved.end());
        // Erase batch in its original location (we cannot use iterators in orig
        // as they may have been invalidated by insertion).
        orig.erase(orig.begin() + moveRange[0], orig.begin() + moveRange[1] + 1);
        this->trigger(src, PhraseModelEvent::MOVE_FINISHED, idx, idx, moveRange);
        // Update iterator to continue search for relocated entries.
        oi = orig.begin() + moveRange[0];
      }
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

  m_decorator = phraseDecorator;

  // We should un-decorate and re-decorate.
  this->redecorate();

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

#ifdef SMTK_PHRASE_DEBUG
namespace
{
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
}
#endif

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

  this->observers()(phr, event, src, dst, arg);
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
