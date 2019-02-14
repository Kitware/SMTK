//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKPipelineSelectionBehavior.h"

// SMTK
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"

#include "smtk/view/Selection.h"

#include "smtk/resource/Resource.h"

#include "smtk/io/Logger.h"

// Client side
#include "pqActiveObjects.h"
#include "pqLiveInsituManager.h"
#include "pqPipelineSource.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"

// Qt
#include <QWidget>

using namespace smtk;

static pqSMTKPipelineSelectionBehavior* g_pipelineSelection = nullptr;

pqSMTKPipelineSelectionBehavior::pqSMTKPipelineSelectionBehavior(QObject* parent)
  : Superclass(parent)
  , m_changingSource(false)
  , m_selectionValue("selected")
{
  if (!g_pipelineSelection)
  {
    g_pipelineSelection = this;
  }

  auto& activeObjects = pqActiveObjects::instance();

  QObject::connect(&activeObjects, SIGNAL(sourceChanged(pqPipelineSource*)), this,
    SLOT(onActiveSourceChanged(pqPipelineSource*)));

  // Track server connects/disconnects
  auto rsrcBehavior = pqSMTKBehavior::instance();
  QObject::connect(rsrcBehavior, SIGNAL(addedManagerOnServer(vtkSMSMTKWrapperProxy*, pqServer*)),
    this, SLOT(observeSelectionOnServer(vtkSMSMTKWrapperProxy*, pqServer*)));
  QObject::connect(rsrcBehavior,
    SIGNAL(removingManagerFromServer(vtkSMSMTKWrapperProxy*, pqServer*)), this,
    SLOT(unobserveSelectionOnServer(vtkSMSMTKWrapperProxy*, pqServer*)));
}

pqSMTKPipelineSelectionBehavior::~pqSMTKPipelineSelectionBehavior()
{
  if (g_pipelineSelection == this)
  {
    g_pipelineSelection = nullptr;
  }
}

pqSMTKPipelineSelectionBehavior* pqSMTKPipelineSelectionBehavior::instance(QObject* parent)
{
  if (!g_pipelineSelection)
  {
    g_pipelineSelection = new pqSMTKPipelineSelectionBehavior(parent);
  }

  return g_pipelineSelection;
}

void pqSMTKPipelineSelectionBehavior::setSelectionValue(const std::string& selectionValue)
{
  if (m_selectionValue != selectionValue)
  {
    m_selectionValue = selectionValue;
  }
}

void pqSMTKPipelineSelectionBehavior::observeSelectionOnServer(
  vtkSMSMTKWrapperProxy* mgr, pqServer* server)
{
  (void)server;
  if (!mgr)
  {
    std::cerr << "pqSMTKPipelineSelectionBehavior::observeSelectionOnServer: no wrapper\n";
    return;
  }
  auto seln = mgr->GetSelection();
  if (!seln)
  {
    std::cerr << "pqSMTKPipelineSelectionBehavior::observeSelectionOnServer: no selection\n";
    return;
  }

  int observerId =
    seln->observers().insert([&](const std::string& source, smtk::view::SelectionPtr selection) {
      int selnValue = selection->selectionValueFromLabel(m_selectionValue);
      if (source != "pqSMTKPipelineSelectionBehavior")
      {
        smtk::resource::ResourcePtr selectedResource;
        selection->visitSelection([&](smtk::resource::PersistentObject::Ptr obj, int val) {
          if ((val & selnValue) == selnValue)
          {
            auto rsrc = std::dynamic_pointer_cast<smtk::resource::Resource>(obj);
            if (rsrc)
            {
              selectedResource = rsrc;
            }
          }
        });
        if (selectedResource)
        {
          // Make the reader owning the first selected resource the active PV pipeline source:
          auto behavior = pqSMTKBehavior::instance();
          auto rsrcSrc = behavior->getPVResource(selectedResource);
          if (rsrcSrc)
          {
            pqActiveObjects::instance().setActiveSource(rsrcSrc);
          }
        }
      }
    });
  m_selectionObservers[seln] = observerId;
}

void pqSMTKPipelineSelectionBehavior::unobserveSelectionOnServer(
  vtkSMSMTKWrapperProxy* mgr, pqServer* server)
{
  (void)server;
  if (!mgr)
  {
    return;
  }
  auto seln = mgr->GetSelection();
  if (!seln)
  {
    return;
  }

  auto entry = m_selectionObservers.find(seln);
  if (entry != m_selectionObservers.end())
  {
    seln->observers().erase(entry->second);
    m_selectionObservers.erase(entry);
  }
}

void pqSMTKPipelineSelectionBehavior::onActiveSourceChanged(pqPipelineSource* source)
{
  auto rsrc = dynamic_cast<pqSMTKResource*>(source);
  if (rsrc && !m_changingSource)
  {
    // We need to update the SMTK selection to be the active pipeline source.
    smtk::resource::PersistentObjectSet activeResources;
    activeResources.insert(rsrc->getResource());

    m_changingSource = true;
    auto behavior = pqSMTKBehavior::instance();
    behavior->visitResourceManagersOnServers(
      [this, &activeResources](pqSMTKWrapper* wrapper, pqServer* server) -> bool {
        // skip bad servers
        if (!wrapper || pqLiveInsituManager::isInsituServer(server))
        {
          return false;
        }

        auto seln = wrapper->smtkSelection();
        if (!seln)
        {
          return false;
        }

        int value = seln->selectionValueFromLabel(m_selectionValue);
        seln->modifySelection(activeResources, "pqSMTKPipelineSelectionBehavior", value,
          smtk::view::SelectionAction::UNFILTERED_REPLACE, true);

        return false;
      });
    m_changingSource = false;
  }
}
