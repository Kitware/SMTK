//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceBrowser.h"

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

#include "smtk/Regex.h"

const std::string pqSMTKResourceBrowser::getJSONConfiguration()
{
  // we want to use the base config, but use our type instead of qtResourceBrowser.
  std::string baseConfig = smtk::extension::qtResourceBrowser::getJSONConfiguration();
  return smtk::regex_replace(baseConfig, smtk::regex("qtResourceBrowser"), "pqSMTKResourceBrowser");
}

smtk::extension::qtBaseView* pqSMTKResourceBrowser::createViewWidget(
  const smtk::view::Information& info)
{
  pqSMTKResourceBrowser* view = new pqSMTKResourceBrowser(info);
  view->buildUI();
  return view;
}

pqSMTKResourceBrowser::pqSMTKResourceBrowser(const smtk::view::Information& info)
  : Superclass(info)
{
  // The superclass has initialized m_p;
  // now we must add ParaView-specific configuration:

  // I. Decorators have been removed.

  // II. Prepare the subphrase generator.
  //     This is important since otherwise m_p->m_phraseModel will
  //     not be set properly.
  this->initSubphraseGenerator();

  // III. Ensure the phrase model is configured to listen to the proper managers.
  // Listen for resources on current connections:
  auto* smtkBehavior = pqSMTKBehavior::instance();
  smtkBehavior->visitResourceManagersOnServers([this](pqSMTKWrapper* r, pqServer* s) {
    this->resourceManagerAdded(r, s);
    return false;
  });
  // Now listen for future connections.
  QObject::connect(
    smtkBehavior,
    SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(resourceManagerAdded(pqSMTKWrapper*, pqServer*)));
  QObject::connect(
    smtkBehavior,
    SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(resourceManagerRemoved(pqSMTKWrapper*, pqServer*)));

  this->updateSettings();
}

pqSMTKResourceBrowser::~pqSMTKResourceBrowser()
{
  QObject::disconnect(this);
}

void pqSMTKResourceBrowser::searchTextChanged(const QString& searchText)
{ // For now, just rebuild.
  (void)searchText;
  // m_p->m_model->rebuildSubphrases(QModelIndex());
}

void pqSMTKResourceBrowser::resourceManagerAdded(pqSMTKWrapper* wrapper, pqServer* server)
{
  if (!wrapper || !server)
  {
    return;
  }

  // wrapper->smtkProxy()->UpdateVTKObjects();
  smtk::resource::ManagerPtr rsrcMgr = wrapper->smtkResourceManager();
  if (!rsrcMgr)
  {
    return;
  }
  this->addSource(wrapper->smtkManagers());
}

void pqSMTKResourceBrowser::resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }
  this->removeSource(mgr->smtkManagers());
}

void pqSMTKResourceBrowser::initSubphraseGenerator()
{
  std::string subphraseViewType = smtk::view::SubphraseGenerator::getType(this->configuration());
  auto* smtkSettings = vtkSMTKSettings::GetInstance();

  int resourceTreeStyle = smtkSettings->GetResourceTreeStyle();
  std::string defaultSubphraseType;
  if (resourceTreeStyle != m_p->m_resourceTreeStyle)
  {
    switch (resourceTreeStyle)
    {
      case vtkSMTKSettings::HierarchicalStyle:
        defaultSubphraseType = "smtk::view::SubphraseGenerator";
        break;
      case vtkSMTKSettings::TwoLevelStyle:
        defaultSubphraseType = "smtk::view::TwoLevelSubphraseGenerator";
        break;
      default:
        smtkWarningMacro(
          smtk::io::Logger::instance(), "Unsupported resource tree style. Resetting to default.");
        defaultSubphraseType = "smtk::view::SubphraseGenerator";
        break;
    }
  }
  if (
    m_p->m_resourceTreeType.empty() || m_p->m_resourceTreeType != subphraseViewType ||
    (subphraseViewType == "default" && resourceTreeStyle != m_p->m_resourceTreeStyle))
  {
    smtk::view::ManagerPtr manager = this->uiManager()->viewManager();
    smtk::view::SubphraseGenerator::Ptr spg = smtk::view::SubphraseGenerator::create(
      subphraseViewType == "default" ? defaultSubphraseType : subphraseViewType, manager);
    if (spg)
    {
      m_p->m_resourceTreeType = subphraseViewType;
      if (subphraseViewType == "default")
      {
        m_p->m_resourceTreeStyle = resourceTreeStyle;
      }
      this->setPhraseGenerator(spg);
    }
  }
}

void pqSMTKResourceBrowser::updateSettings()
{
  auto* smtkSettings = vtkSMTKSettings::GetInstance();
  this->setHighlightOnHover(smtkSettings->GetHighlightOnHover());

  int resourceTreeStyle = smtkSettings->GetResourceTreeStyle();
  if (resourceTreeStyle != m_p->m_resourceTreeStyle && m_p->m_resourceTreeType == "default")
  {
    this->initSubphraseGenerator();
  }
}
