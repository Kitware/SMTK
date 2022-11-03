//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/project/pqSMTKProjectBrowser.h"

#include "smtk/extension/paraview/project/ProjectPanelConfiguration_json.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceRepresentation.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h" // FIXME: Remove the need for me
#include "smtk/extension/paraview/server/vtkSMTKSettings.h"

#include "smtk/extension/qt/qtDescriptivePhraseModel.h"
#include "smtk/extension/qt/qtResourceBrowser.h"
#include "smtk/extension/qt/qtTypeDeclarations.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/project/view/PhraseModel.h"

#include "smtk/view/SubphraseGenerator.h"
#include "smtk/view/TwoLevelSubphraseGenerator.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Resource.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "pqActiveObjects.h"
#include "vtkCompositeRepresentation.h"

#include "smtk/extension/qt/qtResourceBrowserP.h"

namespace
{
const std::string defaultConfigurationJSON = ProjectPanelConfiguration_json;
}

const std::string pqSMTKProjectBrowser::getJSONConfiguration()
{
  return defaultConfigurationJSON;
}

smtk::extension::qtBaseView* pqSMTKProjectBrowser::createViewWidget(
  const smtk::view::Information& info)
{
  pqSMTKProjectBrowser* view = new pqSMTKProjectBrowser(info);
  view->buildUI();
  return view;
}

pqSMTKProjectBrowser::pqSMTKProjectBrowser(const smtk::view::Information& info)
  : Superclass(info)
{
  // The superclass has initialized m_p;
  // now we must add ParaView-specific configuration:
  this->descriptivePhraseModel()->setColumnName("Project");

  if (
    auto phraseModel = std::dynamic_pointer_cast<smtk::view::ResourcePhraseModel>(
      this->descriptivePhraseModel()->phraseModel()))
  {
    phraseModel->setFilter([](const smtk::resource::Resource& resource) {
      return !resource.isOfType(smtk::common::typeName<smtk::project::Project>());
    });
  }

  // Ensure the phrase model is configured to listen to the proper managers.
  // Listen for resources on current connections:
  auto* smtkBehavior = pqSMTKBehavior::instance();
  smtkBehavior->visitResourceManagersOnServers([this](pqSMTKWrapper* r, pqServer* s) {
    this->sourceAdded(r, s);
    return false;
  });

  // Now listen for future connections.
  QObject::connect(
    smtkBehavior,
    SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(sourceAdded(pqSMTKWrapper*, pqServer*)));
  QObject::connect(
    smtkBehavior,
    SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(sourceRemoved(pqSMTKWrapper*, pqServer*)));

  this->updateSettings();
}

pqSMTKProjectBrowser::~pqSMTKProjectBrowser()
{
  QObject::disconnect(this);
}

void pqSMTKProjectBrowser::searchTextChanged(const QString& searchText)
{
  // For now, just rebuild.
  (void)searchText;
}

void pqSMTKProjectBrowser::sourceAdded(pqSMTKWrapper* wrapper, pqServer* server)
{
  if (!wrapper || !server)
  {
    return;
  }

  smtk::resource::ManagerPtr resourceManager = wrapper->smtkResourceManager();
  if (!resourceManager)
  {
    return;
  }
  this->addSource(wrapper->smtkManagers());
}

void pqSMTKProjectBrowser::sourceRemoved(pqSMTKWrapper* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }
  this->removeSource(mgr->smtkManagers());
}

void pqSMTKProjectBrowser::updateSettings()
{
  auto* smtkSettings = vtkSMTKSettings::GetInstance();
  this->setHighlightOnHover(smtkSettings->GetHighlightOnHover());
}
