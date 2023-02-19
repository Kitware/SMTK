//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationHintsBehavior.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceDock.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourcePanel.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/qt/qtDescriptivePhraseModel.h"
#include "smtk/extension/qt/qtResourceBrowser.h"

#include "smtk/view/PhraseModel.h"
#include "smtk/view/SelectionObserver.h"

#include "smtk/operation/Hints.h"
#include "smtk/operation/Launcher.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/groups/DeleterGroup.h"

#include "smtk/attribute/ReferenceItem.h"

#include <QAbstractProxyModel>
#include <QTreeView>

namespace
{

pqSMTKOperationHintsBehavior* g_instance = nullptr;

std::vector<QAbstractProxyModel*> modelsInnerToOuter(QAbstractItemModel* outerModel)
{
  std::vector<QAbstractProxyModel*> result;
  auto* proxyModel = qobject_cast<QAbstractProxyModel*>(outerModel);
  for (; proxyModel; proxyModel = qobject_cast<QAbstractProxyModel*>(proxyModel->sourceModel()))
  {
    result.push_back(proxyModel);
  }

  // The result now has the innermost model at its tail, but we want
  // the vector to go from inner to outer; reverse it.
  std::reverse(result.begin(), result.end());

  return result;
}

QModelIndex mapFromSource(
  const std::vector<QAbstractProxyModel*>& outerModels,
  smtk::extension::qtDescriptivePhraseModel* innerModel,
  std::vector<int>& path)
{
  auto idx = innerModel->indexFromPath(path);
  for (const auto& model : outerModels)
  {
    idx = model->mapFromSource(idx);
  }
  return idx;
}

smtk::resource::Resource::Ptr relevantResource(const smtk::resource::PersistentObject::Ptr& obj)
{
  if (auto rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(obj))
  {
    return rsrc;
  }
  if (auto comp = std::dynamic_pointer_cast<smtk::resource::Component>(obj))
  {
    return comp->resource();
  }
  return smtk::resource::Resource::Ptr();
}

} // anonymous namespace

class pqSMTKOperationHintsBehavior::pqInternal
{
public:
  std::unordered_map<pqServer*, smtk::operation::Observers::Key> m_opObservers;
  std::unordered_map<pqServer*, smtk::view::SelectionObservers::Key> m_selnObservers;
  std::unordered_set<smtk::resource::PersistentObject::Ptr> m_ephemera;
};

pqSMTKOperationHintsBehavior::pqSMTKOperationHintsBehavior(QObject* parent)
  : QObject(parent)
{
  m_p = new pqInternal;

  auto* behavior = pqSMTKBehavior::instance();
  QObject::connect(
    behavior,
    SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(observeWrapper(pqSMTKWrapper*, pqServer*)));
  QObject::connect(
    behavior,
    SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(unobserveWrapper(pqSMTKWrapper*, pqServer*)));

  // Initialize with current wrapper(s), if any:
  behavior->visitResourceManagersOnServers([this](pqSMTKWrapper* wrapper, pqServer* server) {
    this->observeWrapper(wrapper, server);
    return false; // terminate early
  });
}

pqSMTKOperationHintsBehavior::~pqSMTKOperationHintsBehavior() = default;

pqSMTKOperationHintsBehavior* pqSMTKOperationHintsBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKOperationHintsBehavior(parent);
  }
  return g_instance;
}

int pqSMTKOperationHintsBehavior::processHints(
  const smtk::operation::Operation& operation,
  smtk::operation::EventType event,
  smtk::operation::Operation::Result result)
{
  using namespace smtk::operation;

  if (event != smtk::operation::EventType::DID_OPERATE)
  {
    return 0;
  }
  (void)operation;

  // I. Process selection hints.
  visitSelectionHints(
    result,
    [&](
      const smtk::attribute::ReferenceItem::Ptr& assoc,
      smtk::view::SelectionAction action,
      int value,
      bool bitwise,
      bool ephemeral) {
      auto objects = assoc->as<std::set<smtk::resource::PersistentObject::Ptr>>();
      auto selection = pqSMTKBehavior::instance()->builtinOrActiveWrapper()->smtkSelection();
      if (!selection)
      {
        return;
      }
      if (selection->modifySelection(objects, "operation observer", value, action, bitwise))
      {
        if (ephemeral)
        {
          // Save references to any ephemeral objects added to the selection.
          // These will be deleted upon their removal from the selection (as
          // observed by our selection observer which calls removeEphemera()).
          m_p->m_ephemera.insert(objects.begin(), objects.end());
        }
      }
    });

  // II. Process resource panel hints (scroll-to, expand, rename).
  if (auto* resourceDock = this->parent()->findChild<pqSMTKResourceDock*>())
  {
    auto* panel = qobject_cast<pqSMTKResourcePanel*>(resourceDock->widget());
    if (auto* browser = panel->resourceBrowser())
    {
      auto* view = browser->view();
      auto* innerModel = browser->descriptivePhraseModel();
      auto* outerModel = view->model();
      auto outerModels = modelsInnerToOuter(outerModel);
      std::vector<int> path;

      visitFocusHintsOfType(
        result, "browser expand hint", [&](const smtk::attribute::ReferenceItem::Ptr& assoc) {
          const auto& idsToPhrases(browser->phraseModel()->uuidPhraseMap());
          for (std::size_t ii = 0; ii < assoc->numberOfValues(); ++ii)
          {
            if (auto obj = assoc->value(ii))
            {
              auto it = idsToPhrases.find(obj->id());
              if (it != idsToPhrases.end())
              {
                for (const auto& weakPhrase : it->second)
                {
                  if (auto phrase = weakPhrase.lock())
                  {
                    phrase->index(path);
                    auto modelIndex = mapFromSource(outerModels, innerModel, path);
                    view->setExpanded(modelIndex, true); // TODO: allow collapse, too.
                  }
                }
              }
            }
          }
        });

      std::vector<int> bestPath;
      visitFocusHintsOfType(
        result, "browser scroll hint", [&](const smtk::attribute::ReferenceItem::Ptr& assoc) {
          const auto& idsToPhrases(browser->phraseModel()->uuidPhraseMap());
          for (std::size_t ii = 0; ii < assoc->numberOfValues(); ++ii)
          {
            if (auto obj = assoc->value(ii))
            {
              auto it = idsToPhrases.find(obj->id());
              if (it != idsToPhrases.end())
              {
                for (const auto& weakPhrase : it->second)
                {
                  if (auto phrase = weakPhrase.lock())
                  {
                    phrase->index(path);
                    if (bestPath.empty() || path < bestPath)
                    {
                      bestPath = path;
                    }
                  }
                }
              }
            }
          }
        });
      if (!bestPath.empty())
      {
        auto modelIndex = mapFromSource(outerModels, innerModel, bestPath);
        view->scrollTo(modelIndex); // TODO: pass a QAbstractItemView::ScrollHint?
      }
    }
  }

  // III. Process render view hints.
  // TODO.

  // IV. Process task hints.
  smtk::operation::visitTaskHintsOfType(
    result,
    "activate task hint",
    [&](
      const std::set<smtk::project::Project::Ptr>& projects,
      const std::set<smtk::string::Token>& taskIds) {
      if (projects.empty() || !*projects.begin())
      {
        return;
      }
      if (taskIds.empty())
      {
        return;
      }
      auto project = *projects.begin();
      auto task = project->taskManager().taskInstances().findById(*taskIds.begin());
      if (task)
      {
        project->taskManager().active().switchTo(task.get());
      }
    });

  return 0;
}

void pqSMTKOperationHintsBehavior::removeEphemera(
  const std::string& /*changeSource*/,
  smtk::view::Selection::Ptr selection)
{
  auto* bb = pqSMTKBehavior::instance();
  std::map<
    std::pair<smtk::operation::Operation::Index, smtk::operation::Manager::Ptr>,
    std::set<smtk::resource::PersistentObject::Ptr>>
    deleters;
  std::set<smtk::resource::PersistentObject::Ptr> undeletable;

  // Find Delete operations for each candidate or mark as "undeletable"
  // and which is not a member of the selection any longer (for the
  // selection value specified). We group ephemera together when the
  // same deletion operation can be used for them.
  for (const auto& candidate : m_p->m_ephemera)
  {
    if (!candidate)
    {
      continue;
    }

    bool haveDeleter = false;
    auto selnIt = selection->currentSelection().find(candidate);
    if (selnIt == selection->currentSelection().end() || (selnIt->second & 1) == 0)
    {
      auto rsrc = relevantResource(candidate);
      if (rsrc)
      {
        auto* wrapper = bb->getPVResourceManager(rsrc->manager());
        auto operationManager = wrapper ? wrapper->smtkOperationManager() : nullptr;
        if (operationManager)
        {
          auto opIndex =
            smtk::operation::DeleterGroup(operationManager).matchingOperation(*candidate);
          if (opIndex)
          {
            auto key = std::make_pair(opIndex, operationManager);
            deleters[key].insert(candidate);
            haveDeleter = true;
          }
        }
      }
    }
    else
    {
      // We do not need to delete this yet
      continue;
    }
    if (!haveDeleter)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "No deleter for " << candidate << " (" << candidate->typeName() << ", " << candidate->name()
                          << "). "
                             "It will remain undeleted.");
      undeletable.insert(candidate);
    }
  }

  // Create and launch deleters.
  for (const auto& entry : deleters)
  {
    auto operation = entry.first.second->create(entry.first.first);
    if (operation)
    {
      if (auto option = operation->parameters()->findVoid("delete dependents"))
      {
        // We really want to delete ephemera.
        // Users should not be doing things to them that make them undeletable
        // but instead should run operations that copy their data into
        // a non-ephemeral object.
        option->setIsEnabled(true);
      }
      auto assoc = operation->parameters()->associations();
      for (const auto& operand : entry.second)
      {
        if (!assoc->appendValue(operand))
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Could not associate ephemeral " << operand << " (" << operand->typeName() << ", "
                                             << operand->name()
                                             << ") "
                                                "to its deleter.");
        }
        undeletable.insert(operand);
      }
      entry.first.second->launchers()(operation);
    }
    else
    {
      undeletable.insert(entry.second.begin(), entry.second.end());
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Could not create a deleter for " << entry.second.size() << " ephemerals.");
    }
  }
  // Remove all of the ephemera scheduled for deletion.
  for (const auto& member : undeletable)
  {
    m_p->m_ephemera.erase(member);
  }
}

void pqSMTKOperationHintsBehavior::observeWrapper(pqSMTKWrapper* wrapper, pqServer* server)
{
  if (!wrapper || !server)
  {
    return;
  }
  auto operationManager = wrapper->smtkOperationManager();
  if (operationManager)
  {
    bool didInsert = m_p->m_opObservers
                       .emplace(std::make_pair(
                         server,
                         operationManager->observers().insert(
                           [this](
                             const smtk::operation::Operation& op,
                             smtk::operation::EventType eventType,
                             smtk::operation::Operation::Result result) -> int {
                             return this->processHints(op, eventType, result);
                           },
                           smtk::view::PhraseModel::operationObserverPriority() - 128,
                           /*initialize*/ true,
                           "Process operation hints.")))
                       .second;
    if (!didInsert)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Could not add observer to operation manager on server " << server << ".");
    }
  }

  auto selection = wrapper->smtkSelection();
  if (selection)
  {
    bool didInsert =
      m_p->m_selnObservers
        .emplace(std::make_pair(
          server,
          selection->observers().insert(
            [this](
              const std::string& selectionSource, const smtk::view::Selection::Ptr& selection) {
              return this->removeEphemera(selectionSource, selection);
            })))
        .second;
    if (!didInsert)
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Could not add observer to selection on server " << server << ".");
    }
  }
}

void pqSMTKOperationHintsBehavior::unobserveWrapper(pqSMTKWrapper* wrapper, pqServer* server)
{
  (void)wrapper;
  auto oit = m_p->m_opObservers.find(server);
  if (oit != m_p->m_opObservers.end())
  {
    m_p->m_opObservers.erase(oit);
  }
  else
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "No operation observer existed for server " << server << " being disconnected.");
  }
  auto sit = m_p->m_selnObservers.find(server);
  if (sit != m_p->m_selnObservers.end())
  {
    m_p->m_selnObservers.erase(sit);
  }
  else
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "No selection observer existed for server " << server << " being disconnected.");
  }
}
