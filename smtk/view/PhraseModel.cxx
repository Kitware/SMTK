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

#include "smtk/view/BadgeSet.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/DescriptivePhrase.h"
#include "smtk/view/EmptySubphraseGenerator.h"
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
  PhraseModel::Observer obs,
  DescriptivePhrasePtr parent,
  std::vector<int>& parentIdx)
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
} // namespace

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
    return m_sources.front().m_managers.get<smtk::operation::ManagerPtr>();
  }
  return nullptr;
}

PhraseModel::PhraseModel()
  : m_observers(std::bind(notify, std::placeholders::_1, this->root()))
{
}

PhraseModel::PhraseModel(const Configuration* config, Manager* manager)
  : m_observers(std::bind(notify, std::placeholders::_1, this->root()))
  , m_badges(config, manager->shared_from_this(), this)
  , m_manager(manager->shared_from_this())
{
}

PhraseModel::~PhraseModel()
{
  this->resetSources();
}

SubphraseGeneratorPtr PhraseModel::configureSubphraseGenerator(
  const Configuration* config,
  Manager* manager)
{
  SubphraseGeneratorPtr result;
  int modelIndex = -1;
  int subphraseIndex = -1;
  if (config && (modelIndex = config->details().findChild("PhraseModel")) >= 0)
  {
    const auto& modelConfig = config->details().child(modelIndex);
    if ((subphraseIndex = modelConfig.findChild("SubphraseGenerator")) >= 0)
    {
      const auto& subphraseConfig = modelConfig.child(subphraseIndex);
      std::string spType;
      subphraseConfig.attribute("Type", spType);
      if (spType.empty() || spType == "default")
      {
        spType = "smtk::view::SubphraseGenerator";
      }
      result = manager->subphraseGeneratorFactory().createFromConfiguration(&subphraseConfig);
    }
  }
  if (!result)
  {
    result = smtk::view::EmptySubphraseGenerator::create();
  }
  return result;
}

std::multimap<std::string, std::string> PhraseModel::configureFilterStrings(
  const Configuration* config,
  Manager*)
{
  std::multimap<std::string, std::string> result;
  int modelIndex = -1;
  int filterIndex = -1;
  if (config && (modelIndex = config->details().findChild("PhraseModel")) >= 0)
  {
    const auto& modelConfig = config->details().child(modelIndex);
    if ((filterIndex = modelConfig.findChild("Accepts")) >= 0)
    {
      const auto& filterConfig = modelConfig.child(filterIndex);
      for (std::size_t ii = 0; ii < filterConfig.numberOfChildren(); ++ii)
      {
        const auto& acceptConfig = filterConfig.child(ii);
        if (acceptConfig.name() == "Resource")
        {
          std::pair<std::string, std::string> filterPair;
          bool haveName = acceptConfig.attribute("Name", filterPair.first);
          acceptConfig.attribute("Filter", filterPair.second);
          if (haveName)
          {
            result.insert(filterPair);
          }
          else
          {
            smtkWarningMacro(
              smtk::io::Logger::instance(),
              "A \"Resource\" entry did not have a \"Name\" attribute.");
          }
        }
      }
    }
  }
  return result;
}

bool PhraseModel::addSource(
  smtk::resource::ManagerPtr rsrcMgr,
  smtk::operation::ManagerPtr operMgr,
  smtk::view::ManagerPtr viewMgr,
  smtk::view::SelectionPtr seln)
{
  for (const auto& source : m_sources)
  {
    if (
      ((!rsrcMgr && !source.m_managers.contains<smtk::resource::ManagerPtr>()) ||
       (source.m_managers.contains<smtk::resource::ManagerPtr>() &&
        source.m_managers.get<smtk::resource::ManagerPtr>() == rsrcMgr)) &&
      ((!operMgr && !source.m_managers.contains<smtk::operation::ManagerPtr>()) ||
       (source.m_managers.contains<smtk::operation::ManagerPtr>() &&
        source.m_managers.get<smtk::operation::ManagerPtr>() == operMgr)) &&
      ((!viewMgr && !source.m_managers.contains<smtk::view::ManagerPtr>()) ||
       (source.m_managers.contains<smtk::view::ManagerPtr>() &&
        source.m_managers.get<smtk::view::ManagerPtr>() == viewMgr)) &&
      ((!seln && !source.m_managers.contains<smtk::view::SelectionPtr>()) ||
       (source.m_managers.contains<smtk::view::SelectionPtr>() &&
        source.m_managers.get<smtk::view::SelectionPtr>() == seln)))
    {
      return false; // Do not add what we already have
    }
  }
  std::ostringstream description;
  description << "PhraseModel " << this << ": ";
  auto rsrcHandle = rsrcMgr ? rsrcMgr->observers().insert(
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
  auto selnHandle = seln ? seln->observers().insert(
                             [this](const std::string& src, smtk::view::SelectionPtr seln) {
                               this->handleSelectionEvent(src, seln);
                             },
                             0,    // assign a neutral priority
                             true, // observeImmediately
                             description.str() + "Update phrases when selection changes.")
                         : smtk::view::SelectionObservers::Key();
  m_sources.emplace_back(
    rsrcMgr,
    operMgr,
    viewMgr,
    seln,
    std::move(rsrcHandle),
    std::move(operHandle),
    std::move(selnHandle));
  return true;
}

bool PhraseModel::addSource(const smtk::common::TypeContainer& managers)
{
  const auto& rsrcMgr =
    (managers.contains<smtk::resource::ManagerPtr>() ? managers.get<smtk::resource::ManagerPtr>()
                                                     : smtk::resource::ManagerPtr());
  const auto& operMgr =
    (managers.contains<smtk::operation::ManagerPtr>() ? managers.get<smtk::operation::ManagerPtr>()
                                                      : smtk::operation::ManagerPtr());
  const auto& viewMgr =
    (managers.contains<smtk::view::ManagerPtr>() ? managers.get<smtk::view::ManagerPtr>()
                                                 : smtk::view::ManagerPtr());
  const auto& seln =
    (managers.contains<smtk::view::SelectionPtr>() ? managers.get<smtk::view::SelectionPtr>()
                                                   : smtk::view::SelectionPtr());

  for (const auto& source : m_sources)
  {
    if (
      ((!rsrcMgr && !source.m_managers.contains<smtk::resource::ManagerPtr>()) ||
       (source.m_managers.contains<smtk::resource::ManagerPtr>() &&
        source.m_managers.get<smtk::resource::ManagerPtr>() == rsrcMgr)) &&
      ((!operMgr && !source.m_managers.contains<smtk::operation::ManagerPtr>()) ||
       (source.m_managers.contains<smtk::operation::ManagerPtr>() &&
        source.m_managers.get<smtk::operation::ManagerPtr>() == operMgr)) &&
      ((!viewMgr && !source.m_managers.contains<smtk::view::ManagerPtr>()) ||
       (source.m_managers.contains<smtk::view::ManagerPtr>() &&
        source.m_managers.get<smtk::view::ManagerPtr>() == viewMgr)) &&
      ((!seln && !source.m_managers.contains<smtk::view::SelectionPtr>()) ||
       (source.m_managers.contains<smtk::view::SelectionPtr>() &&
        source.m_managers.get<smtk::view::SelectionPtr>() == seln)))
    {
      return false; // Do not add what we already have
    }
  }
  std::ostringstream description;
  description << "PhraseModel " << this << ": ";
  auto rsrcHandle = rsrcMgr ? rsrcMgr->observers().insert(
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
  auto selnHandle = seln ? seln->observers().insert(
                             [this](const std::string& src, smtk::view::SelectionPtr seln) {
                               this->handleSelectionEvent(src, seln);
                             },
                             0,    // assign a neutral priority
                             true, // observeImmediately
                             description.str() + "Update phrases when selection changes.")
                         : smtk::view::SelectionObservers::Key();
  m_sources.emplace_back(
    managers, std::move(rsrcHandle), std::move(operHandle), std::move(selnHandle));
  return true;
}

bool PhraseModel::removeSource(
  smtk::resource::ManagerPtr rsrcMgr,
  smtk::operation::ManagerPtr operMgr,
  smtk::view::ManagerPtr viewMgr,
  smtk::view::SelectionPtr seln)
{
  for (auto it = m_sources.begin(); it != m_sources.end(); ++it)
  {
    if (
      it->m_managers.get<smtk::resource::ManagerPtr>() == rsrcMgr &&
      it->m_managers.get<smtk::operation::ManagerPtr>() == operMgr &&
      it->m_managers.get<smtk::view::ManagerPtr>() == viewMgr &&
      it->m_managers.get<smtk::view::SelectionPtr>() == seln)
    {
      m_sources.erase(it);
      return true;
    }
  }
  return false;
}

bool PhraseModel::removeSource(const smtk::common::TypeContainer& managers)
{
  const auto& rsrcMgr =
    (managers.contains<smtk::resource::ManagerPtr>() ? managers.get<smtk::resource::ManagerPtr>()
                                                     : smtk::resource::ManagerPtr());
  const auto& operMgr =
    (managers.contains<smtk::operation::ManagerPtr>() ? managers.get<smtk::operation::ManagerPtr>()
                                                      : smtk::operation::ManagerPtr());
  const auto& viewMgr =
    (managers.contains<smtk::view::ManagerPtr>() ? managers.get<smtk::view::ManagerPtr>()
                                                 : smtk::view::ManagerPtr());
  const auto& seln =
    (managers.contains<smtk::view::SelectionPtr>() ? managers.get<smtk::view::SelectionPtr>()
                                                   : smtk::view::SelectionPtr());

  for (auto it = m_sources.begin(); it != m_sources.end(); ++it)
  {
    if (
      it->m_managers.get<smtk::resource::ManagerPtr>() == rsrcMgr &&
      it->m_managers.get<smtk::operation::ManagerPtr>() == operMgr &&
      it->m_managers.get<smtk::view::ManagerPtr>() == viewMgr &&
      it->m_managers.get<smtk::view::SelectionPtr>() == seln)
    {
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
    if (this->removeSource(src->m_managers))
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
    if (!visitor(
          (src.m_managers.contains<smtk::resource::ManagerPtr>()
             ? src.m_managers.get<smtk::resource::ManagerPtr>()
             : smtk::resource::ManagerPtr()),
          (src.m_managers.contains<smtk::operation::ManagerPtr>()
             ? src.m_managers.get<smtk::operation::ManagerPtr>()
             : smtk::operation::ManagerPtr()),
          (src.m_managers.contains<smtk::view::ManagerPtr>()
             ? src.m_managers.get<smtk::view::ManagerPtr>()
             : smtk::view::ManagerPtr()),
          (src.m_managers.contains<smtk::view::SelectionPtr>()
             ? src.m_managers.get<smtk::view::SelectionPtr>()
             : smtk::view::SelectionPtr())))
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
  const Operation& op,
  operation::EventType event,
  const Operation::Result& res)
{
  smtkDebugMacro(
    smtk::io::Logger::instance(),
    "      Phrase handler: op " << (event == operation::EventType::DID_OPERATE ? "ran" : "cre/pre")
                                << " " << &op);

  if (!res)
  {
    return 0;
  } // Ignore operator creation and about-to-operate events.
  if (event != operation::EventType::DID_OPERATE)
  {
    return 0;
  }

  // Find out which resource components were created, modified, or expunged.
  // Only inserting elements that are "set" so the callee, handle*(), is not
  // passed any nulllptrs.

  ComponentItemPtr expungedItem = res->findComponent("expunged");
  if (expungedItem)
  {
    smtk::resource::PersistentObjectSet expungedObjects;
    for (auto expungedIt = expungedItem->begin(); expungedIt != expungedItem->end(); expungedIt++)
    {
      if (expungedIt.isSet())
      {
        expungedObjects.insert(*expungedIt);
      }
    }
    this->handleExpunged(expungedObjects);
  }

  ComponentItemPtr modifiedItem = res->findComponent("modified");
  if (modifiedItem)
  {
    smtk::resource::PersistentObjectSet modifiedObjects;
    for (auto modifiedIt = modifiedItem->begin(); modifiedIt != modifiedItem->end(); modifiedIt++)
    {
      if (modifiedIt.isSet())
      {
        modifiedObjects.insert(*modifiedIt);
      }
    }
    this->handleModified(modifiedObjects);
  }

  ComponentItemPtr createdItem = res->findComponent("created");
  if (createdItem)
  {
    smtk::resource::PersistentObjectSet createdObjects;
    for (auto createdIt = createdItem->begin(); createdIt != createdItem->end(); createdIt++)
    {
      if (createdIt.isSet())
      {
        createdObjects.insert(*createdIt);
      }
    }
    this->handleCreated(createdObjects);
  }

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

void PhraseModel::handleExpunged(const smtk::resource::PersistentObjectSet& expungedObjects)
{
  // By default, search all existing phrases for matching components and remove them.
  std::set<smtk::common::UUID> uuids;
  for (auto it = expungedObjects.begin(); it != expungedObjects.end(); ++it)
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

void PhraseModel::handleModified(const smtk::resource::PersistentObjectSet& modifiedObjects)
{
  auto rootPhrase = this->root();
  if (rootPhrase)
  {
    rootPhrase->visitChildren(
      [this, &modifiedObjects](DescriptivePhrasePtr phr, const std::vector<int>& idx) -> int {
        if (modifiedObjects.find(phr->relatedComponent()) != modifiedObjects.end())
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

void PhraseModel::handleCreated(const smtk::resource::PersistentObjectSet& createdObjects)
{
  auto rootPhrase = this->root();
  auto delegate = rootPhrase ? rootPhrase->findDelegate() : nullptr;
  if (!delegate)
  {
    return;
  }

  smtk::resource::PersistentObjectArray objects(createdObjects.begin(), createdObjects.end());

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

void PhraseModel::redecorate() {}

void PhraseModel::updateChildren(
  smtk::view::DescriptivePhrasePtr src,
  DescriptivePhrases& next,
  const std::vector<int>& idx)
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
        if (
          (*nval == **it2) ||
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
      smtk::io::Logger::instance(),
      "Update to descriptive phrases has correspondence problems"
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
      else if (mv == ni)
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(), "Comparing same phrases yielded different results.");
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

        // Roll back mv to find the phrase in "next" just _before_ our destination.
        --mv;
        auto di = bi;
        for (; di != orig.end() && *di != *mv; ++di)
        {
          // Do nothing (advancing to find insertion point in orig that matches location in next)
        }
#if SMTK_PHRASE_DEBUG
        std::cout << "        oi " << (oi - orig.begin()) << " " << (*oi)->title() << "\n"
                  << "        ni " << (ni - next.begin()) << " " << (*ni)->title() << "\n"
                  << "        mv " << (mv - next.begin()) << " " << (*mv)->title() << "\n"
                  << "        bi " << (bi - orig.begin()) << " " << (*bi)->title() << "\n"
                  << "        ci " << (ci - next.begin()) << " " << (*ci)->title() << "\n"
                  << "        di " << (di - orig.begin()) << " " << (*di)->title() << "\n";
#endif
        ++mv;
        if (di != orig.end())
        {
          ++di;
        }
        std::vector<int> moveRange(3);
        moveRange[0] = static_cast<int>(oi - orig.begin());
        moveRange[1] = static_cast<int>(bi == orig.end() ? orig.size() : bi - orig.begin()) - 1;
        moveRange[2] = static_cast<int>((di - orig.begin()));
        // Now if there are things we failed to insert, we don't want to get stuck in
        // an infinite loop trying to reorder things to accommodate them. This will
        // manifest itself as trying to move all of the remaining items in orig
        // to the end of the orig.
        if (di == orig.end() && (moveRange[1] == static_cast<int>(orig.size() - 1)))
        {
          break;
        }
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
} // namespace
#endif

void PhraseModel::trigger(
  DescriptivePhrasePtr phr,
  PhraseModelEvent event,
  const std::vector<int>& src,
  const std::vector<int>& dst,
  const std::vector<int>& arg)
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
} // namespace view
} // namespace smtk
