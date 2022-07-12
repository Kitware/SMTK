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
#include "smtk/attribute/ResourceItem.h"

#include "smtk/resource/Component.h"

#include "smtk/io/Logger.h"

#include <chrono>
#include <thread>

#undef SMTK_DBG_PHRASE

namespace smtk
{
namespace view
{
namespace
{

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

PhraseModel::Source::Source(
  smtk::resource::ManagerPtr rm,
  smtk::operation::ManagerPtr om,
  smtk::view::ManagerPtr vm,
  smtk::view::SelectionPtr sn,
  smtk::resource::Observers::Key&& rh,
  smtk::operation::Observers::Key&& oh,
  smtk::view::SelectionObservers::Key&& sh)
  : m_rsrcHandle(std::move(rh))
  , m_operHandle(std::move(oh))
  , m_selnHandle(std::move(sh))
{
  m_managers.insert(rm);
  m_managers.insert(om);
  m_managers.insert(vm);
  m_managers.insert(sn);
}

PhraseModel::Source::Source(
  const smtk::common::TypeContainer& managers,
  smtk::resource::Observers::Key&& rh,
  smtk::operation::Observers::Key&& oh,
  smtk::view::SelectionObservers::Key&& sh)
  : m_managers(managers)
  , m_rsrcHandle(std::move(rh))
  , m_operHandle(std::move(oh))
  , m_selnHandle(std::move(sh))
{
}

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
                                0,     // assign a neutral priority
                                false, // observeImmediately
                                description.str() + "Update phrases when resources change.")
                            : smtk::resource::Observers::Key();
  auto operHandle = operMgr
    ? operMgr->observers().insert(
        [this](const Operation& op, operation::EventType event, const Operation::Result& res) {
          this->handleOperationEvent(op, event, res);
          return 0;
        },
        PhraseModel::operationObserverPriority(),
        /*initialize*/ true,
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

  // Now, we did not immediately invoke the resource-manager observer above – because
  // if we had, m_sources would be empty when the observer was invoked and would leave
  // some phrase models unable to populate their root phrase (e.g., ReferenceItemPhraseModel
  // which visits sources inside populateRoot()). Iterate over resources and invoke the
  // observer now.
  //
  // Also, we are trying to phase out the resource-manager observer (currently it is only
  // used to update when a resource is modified (not added or removed). So, we visit each
  // resource in the newly-available manager and call processResource() on it (as if an
  // operation added the resource in question).
  if (rsrcMgr)
  {
    rsrcMgr->visit([&](smtk::resource::Resource& rsrc) {
      this->processResource(rsrc.shared_from_this(), true);
      return smtk::common::Processing::CONTINUE;
    });
  }
  return true;
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

void PhraseModel::visitSources(SourceVisitorFunction visitor)
{
  if (!visitor)
  {
    return;
  }
  for (const auto& src : m_sources)
  {
    if (
      visitor(
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
           : smtk::view::SelectionPtr())) == smtk::common::Visit::Halt)
    {
      break;
    }
  }
}

smtk::operation::Observers::Priority PhraseModel::operationObserverPriority()
{
  return std::numeric_limits<smtk::operation::Observers::Priority>::lowest() + 1024;
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

  // First, look for entire resources being added/removed.
  if (auto resourcesToExpunge = res->findResource("resourcesToExpunge"))
  {
    for (auto rsrcIt = resourcesToExpunge->begin(); rsrcIt != resourcesToExpunge->end(); ++rsrcIt)
    {
      if (rsrcIt.isSet())
      {
        this->processResource(rsrcIt.as<smtk::resource::Resource>(), false);
      }
    }
  }

  if (auto resourcesToAdd = res->findResource("resource"))
  {
    for (auto rsrcIt = resourcesToAdd->begin(); rsrcIt != resourcesToAdd->end(); ++rsrcIt)
    {
      if (rsrcIt.isSet())
      {
        this->processResource(rsrcIt.as<smtk::resource::Resource>(), true);
      }
    }
  }

  // Find out which components were created, modified, or expunged.
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
      if (createdIt.isSet() && (m_objectMap.find((*createdIt)->id()) == m_objectMap.end()))
      {
        createdObjects.insert(*createdIt);
      }
    }
    this->handleCreated(createdObjects);
  }

  return 0;
}

void PhraseModel::processResource(
  const std::shared_ptr<smtk::resource::Resource>& resource,
  bool adding)
{
  (void)resource;
  (void)adding;
}

void PhraseModel::removeChildren(const std::vector<int>& parentIdx, int childRange[2])
{
  auto phr = this->root()->at(parentIdx);
  std::vector<int> removeRange{ childRange[0], childRange[1] };

  this->trigger(phr, PhraseModelEvent::ABOUT_TO_REMOVE, parentIdx, parentIdx, removeRange);
  if (removeRange[0] == removeRange[1])
  {
    phr->subphrases().erase(phr->subphrases().begin() + removeRange[0]);
  }
  else
  {
    phr->subphrases().erase(
      phr->subphrases().begin() + removeRange[0], phr->subphrases().begin() + removeRange[1] + 1);
  }
  this->trigger(phr, PhraseModelEvent::REMOVE_FINISHED, parentIdx, parentIdx, removeRange);
}

void PhraseModel::handleExpunged(const smtk::resource::PersistentObjectSet& expungedObjects)
{
  if (this->root() == nullptr)
  {
    return;
  }
  // Remove phrases that correspond to the set of expunged objects
  // For each object get all of the phrased that corresponds to it, calculate their indices,
  //  and add them to the Phrase Delta
  PhraseDeltas phrasePaths;
  for (const auto& object : expungedObjects)
  {
    auto it = m_objectMap.find(object->id());
    if (it == m_objectMap.end())
    {
      continue; // the object is not in the tree
    }
    for (const auto& wdp : it->second)
    {
      auto dp = wdp.lock(); // get the shared pointer from the phrase weak pointer
      if (dp == nullptr)
      {
        continue; // the phrase was previously released
      }
      std::vector<int> path;
      dp->index(path);
      phrasePaths.insert(path);
    }
  }

  std::vector<int> lastPath;
  int removeRange[2] = { -1, -1 };
  bool remaining = false;
  for (auto idx : phrasePaths)
  {
    if (idx.empty())
    {
      continue;
    }

    auto backIdx = idx.back();
    idx.pop_back();
    if (removeRange[1] == -1)
    {
      removeRange[0] = backIdx;
      removeRange[1] = removeRange[0];
      remaining = true;
      lastPath = idx;
    }
    else if (removeRange[0] == backIdx + 1 && lastPath == idx)
    {
      removeRange[0] = backIdx;
    }
    else
    {
#ifdef SMTK_DBG_PHRASE
      std::cout << "Remove (";
      for (const auto& rr : lastPath)
      {
        std::cout << " " << rr;
      }
      std::cout << " ) " << removeRange[0] << " " << removeRange[1] << "\n";
#endif
      this->removeChildren(lastPath, removeRange);
      removeRange[0] = backIdx;
      removeRange[1] = removeRange[0];
      remaining = true;
      lastPath = idx;
    }
  }
  if (remaining && removeRange[1] != -1)
  {
#ifdef SMTK_DBG_PHRASE
    std::cout << "Remove (";
    for (const auto& rr : lastPath)
    {
      std::cout << " " << rr;
    }
    std::cout << " ) " << removeRange[0] << " " << removeRange[1] << "\n";
#endif
    this->removeChildren(lastPath, removeRange);
  }
}

void PhraseModel::setPhraseParent(
  const DescriptivePhrasePtr& phrase,
  const DescriptivePhrasePtr& parent) const
{
  phrase->m_parent = parent;
}

void PhraseModel::handleModified(const smtk::resource::PersistentObjectSet& modifiedObjects)
{
  if (this->root() == nullptr)
  {
    return;
  }

  for (const auto& object : modifiedObjects)
  {
    auto it = m_objectMap.find(object->id());
    if (it == m_objectMap.end())
    {
      continue; // the object is not in the tree
    }
    for (const auto& wdp : it->second)
    {
      auto dp = wdp.lock(); // get the shared pointer from the phrase weak pointer
      if (dp == nullptr)
      {
        continue; // the phrase was previously released
      }
      std::vector<int> path;
      dp->index(path);

      this->trigger(dp, PhraseModelEvent::PHRASE_MODIFIED, path, path, std::vector<int>());
      // Now check whether the modification requires a reorder
      auto pp = dp->parent();
      smtk::view::DescriptivePhrases sorted(pp->subphrases().begin(), pp->subphrases().end());
      std::sort(sorted.begin(), sorted.end(), DescriptivePhrase::compareByTypeThenTitle);
      std::vector<int> pidx(path.begin(), path.begin() + path.size() - 1);
      this->updateChildren(pp, sorted, pidx);
    }
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
    std::sort(
      children.begin(), children.end(), smtk::view::DescriptivePhrase::compareByTypeThenTitle);
    this->trigger(parent, PhraseModelEvent::INSERT_FINISHED, idx, idx, insertRange);
    for (const auto& childPhrase : batch)
    {
      childPhrase->subphrases(); // make sure the children subphrases are built
    }
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
  // Are we in a recursive call to updateChildren?
  if (m_updatingChildren)
  {
    src->subphrases() = next;
    return;
  }

  m_updatingChildren = true;
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
    // Let's create the phrases for all new descendants
    std::vector<std::future<DescriptivePhrases>> results;
    for (const auto& newPhrase : batch)
    {
      results.push_back(m_pool([=] { return newPhrase->subphrases(); }));
    }

    while (!results.empty())
    {
      auto newChildPhrases = results.back().get();
      results.pop_back();
      for (const auto& newChildPhrase : newChildPhrases)
      {
        results.push_back(m_pool([=] { return newChildPhrase->subphrases(); }));
      }
    }
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
  m_updatingChildren = false;
}

void PhraseModel::triggerDataChanged()
{
  // If m_pending is true, then there is already a timer set to expire in the
  // future and we should ignore this call.
  // If m_pending is false, then we should set it true and queue a timer to call
  // the invokeContentObserversInternal() function some small time in the future.
  bool expected = false;
  if (std::atomic_compare_exchange_strong(&m_pending, &expected, true))
  {
    std::function<DescriptivePhrases()> invokeContentObserversInternal = [this]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      // Clear the "pending-timer" bit before we start signaling observers
      // Otherwise, it is possible to miss an incoming signal that more
      // content has changed while we are busy invoking observers.
      m_pending.store(false);

      // m_contentObservers(); // TODO: Alternative to the below?
      auto rootPhrase = this->root();
      if (!rootPhrase || rootPhrase->subphrases().empty())
      {
        return DescriptivePhrases{};
      }

      std::vector<int> i0;
      std::vector<int> i1;
      // It is doubtful trees with 64 levels will be useful; reserve
      // that much space so recursion does not cause reallocation.
      i0.reserve(64);
      i1.reserve(64);

      recursiveTrigger(rootPhrase, i0, i1);

      return DescriptivePhrases{};
    };
    std::thread runme(invokeContentObserversInternal);
    runme.detach();
  }
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

void PhraseModel::removeFromMap(const DescriptivePhrasePtr& phr)
{
  if (!phr)
  {
    return;
  }
  // Does the phrase contain a Persistent Object?
  smtk::resource::PersistentObjectPtr obj = phr->relatedObject();
  if (obj)
  {
    auto it = m_objectMap.find(obj->id());
    // If the object is in the map then remove the phrase from the set
    if (it != m_objectMap.end())
    {
      std::size_t numPhrases = it->second.size();
      if (numPhrases == 0)
      {
        smtkWarningMacro(
          smtk::io::Logger::instance(),
          "Found an empty set of descriptive phrases for persistent object: "
            << obj->name() << " with UUID: " << obj->id().toString());
      }
      else
      {
        it->second.erase(phr);
        if (it->second.size() == numPhrases)
        {
          smtkWarningMacro(
            smtk::io::Logger::instance(),
            "Found a set of descriptive phrases for persistent object: "
              << obj->name() << " with UUID: " << obj->id().toString()
              << " that did not include the phrase being removed");
        }
        else if (it->second.empty())
        {
          m_objectMap.erase(it);
        }
      }
    }
  }
  // see if the phrase has children that need to be removed
  if (!phr->areSubphrasesBuilt())
  {
    return;
  }

  DescriptivePhrases& children(phr->subphrases());
  for (const auto& child : children)
  {
    this->removeFromMap(child);
  }
}

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

  if ((event == PhraseModelEvent::ABOUT_TO_REMOVE) && phr && phr->areSubphrasesBuilt())
  {
    DescriptivePhrases& children(phr->subphrases());
    for (int ci = arg[0]; ci <= arg[1]; ++ci)
    {
      this->removeFromMap(children[ci]);
    }
  }

  this->observers()(phr, event, src, dst, arg);
  // Check to see if phrases we just inserted have pre-existing children. If so, trigger them.
  if (event == PhraseModelEvent::INSERT_FINISHED && phr && phr->areSubphrasesBuilt())
  {
    DescriptivePhrases& children(phr->subphrases());
    for (int ci = arg[0]; ci <= arg[1]; ++ci)
    {
      // If the child is null then skip it
      if (!children[ci])
      {
        continue;
      }
      this->insertIntoMap(children[ci]);
    }
  }
}

void PhraseModel::recursiveTrigger(
  DescriptivePhrasePtr phr,
  std::vector<int>& tl,
  std::vector<int>& br)
{
  if (!phr)
  {
    return;
  }
  const auto& sp = phr->subphrases();
  if (sp.empty())
  {
    return;
  }

  std::size_t ii = tl.size();
  tl.push_back(0);
  br.push_back(sp.size() - 1);
  this->trigger(this->root(), PhraseModelEvent::PHRASE_MODIFIED, tl, br, std::vector<int>());
  for (std::size_t jj = 0; jj < sp.size(); ++jj)
  {
    tl[ii] = jj;
    br[ii] = jj;
    recursiveTrigger(sp[jj], tl, br);
  }
  tl.pop_back();
  br.pop_back();
}

void PhraseModel::insertIntoMap(const DescriptivePhrasePtr& phrase)
{
  bool needToCheckChildren = true;
  // Does this phrase have a persistent object we need to insert into the map?
  smtk::resource::PersistentObjectPtr obj = phrase->relatedObject();
  if (obj)
  {
    auto it = m_objectMap.find(obj->id());
    if (it == m_objectMap.end())
    {
      // OK this is the first descriptive phrase that uses this object
      std::set<
        std::weak_ptr<smtk::view::DescriptivePhrase>,
        std::owner_less<std::weak_ptr<smtk::view::DescriptivePhrase>>>
        phrases;
      phrases.insert(phrase);
      m_objectMap.insert({ { obj->id(), phrases } });
    }
    else
    {
      // In this case we know the object already existed in the map so
      // try to insert it and see if the insertion was successful (else
      // it was already in the list of phrases for the object)
      auto result = it->second.insert(phrase);
      needToCheckChildren = result.second;
    }
  }

  // Do we need to deal with its children?
  if (!(needToCheckChildren && phrase->areSubphrasesBuilt()))
  {
    return;
  }

  DescriptivePhrases& children(phrase->subphrases());
  for (DescriptivePhrasePtr& child : children)
  {
    this->insertIntoMap(child);
  }
}
} // namespace view
} // namespace smtk
