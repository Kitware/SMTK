//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKAttributePanel.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/paraview/server/vtkSMTKSettings.h"

#include "smtk/io/Logger.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/Selection.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqPipelineSource.h"

#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMSessionProxyManager.h"

#include "vtkCommand.h"
#include "vtkVector.h"

#include <QPointer>
#include <QVBoxLayout>

pqSMTKAttributePanel::pqSMTKAttributePanel(QWidget* parent)
  : Superclass(parent)
  , m_attrUIMgr(nullptr)
{
  this->setObjectName("AttributeEditor");
  this->setWindowTitle("Attribute Editor");
  QWidget* w = new QWidget(this);
  this->setWidget(w);
  w->setLayout(new QVBoxLayout);
  QObject::connect(&pqActiveObjects::instance(), SIGNAL(sourceChanged(pqPipelineSource*)), this,
    SLOT(displayPipelineSource(pqPipelineSource*)));
  QObject::connect(
    &pqActiveObjects::instance(), SIGNAL(dataUpdated()), this, SLOT(updatePipeline()));

  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk attribute panel", this);
  }

  auto smtkSettings = vtkSMTKSettings::GetInstance();
  pqCoreUtilities::connect(smtkSettings, vtkCommand::ModifiedEvent, this, SLOT(updateSettings()));
}

pqSMTKAttributePanel::~pqSMTKAttributePanel()
{
  if (auto rsrc = m_rsrc.lock())
  {
    auto rsrcMgr = rsrc->manager();
    if (rsrcMgr && m_observer.assigned())
    {
      rsrcMgr->observers().erase(m_observer);
    }
  }

  m_propertyLinks.clear();
  delete m_attrUIMgr;
}

bool pqSMTKAttributePanel::displayPipelineSource(pqPipelineSource* psrc)
{
  pqSMTKResource* rsrc = dynamic_cast<pqSMTKResource*>(psrc);
  if (rsrc)
  {
    auto attrRsrc = std::dynamic_pointer_cast<smtk::attribute::Resource>(rsrc->getResource());
    if (attrRsrc && attrRsrc != m_rsrc.lock())
    {
      pqSMTKWrapper* wrapper =
        pqSMTKBehavior::instance()->resourceManagerForServer(rsrc->getServer());
      if (wrapper)
      {
        // Keep hold of the selection instance for the active server connection
        // so that this->displayResource() below can make use of it.
        m_seln = wrapper->smtkSelection();
        m_opManager = wrapper->smtkOperationManager();
        m_viewManager = wrapper->smtkViewManager();
      }
      else
      {
        m_seln = nullptr;
        m_opManager = nullptr;
        m_viewManager = nullptr;
      }
      return this->displayResource(attrRsrc);
    }
  }
  return false;
}

bool pqSMTKAttributePanel::displayResource(const smtk::attribute::ResourcePtr& rsrc)
{
  bool didDisplay = false;
  auto previousResource = m_rsrc.lock();
  if (!rsrc || rsrc == previousResource)
  {
    return didDisplay;
  }

  if (previousResource)
  {
    auto rsrcMgr = previousResource->manager();
    if (rsrcMgr && m_observer.assigned())
    {
      rsrcMgr->observers().erase(m_observer);
    }
  }
  m_rsrc = rsrc;
  if (m_attrUIMgr)
  {
    m_propertyLinks.clear();
    delete m_attrUIMgr;
    while (QWidget* w = this->widget()->findChild<QWidget*>())
    {
      delete w;
    }
  }

  m_attrUIMgr = new smtk::extension::qtUIManager(rsrc);
  m_attrUIMgr->setOperationManager(m_opManager); // Assign the operation manager
  m_attrUIMgr->setViewManager(m_viewManager);
  m_attrUIMgr->setSelection(m_seln); // NB: m_seln may be null.
  m_attrUIMgr->setSelectionBit(1);   // ToDo: should be set by application

  // Find or Create a value for highlight on hover
  if (m_seln)
  {
    int hoverBit = m_seln->findOrCreateLabeledValue("hovered");
    m_attrUIMgr->setHoverBit(hoverBit);
  }

  // Start watching the resource's associate PV server for user preference changes.
  pqServer* server = pqActiveObjects::instance().activeServer();
  vtkSMSessionProxyManager* pxm = server ? server->proxyManager() : nullptr;
  auto paletteProxy = pxm ? pxm->GetProxy("settings", "ColorPalette") : nullptr;
  auto defaultValueColorProp =
    paletteProxy ? paletteProxy->GetProperty("SMTKDefaultValueBackground") : nullptr;
  auto invalidValueColorProp =
    paletteProxy ? paletteProxy->GetProperty("SMTKInvalidValueBackground") : nullptr;
  if (defaultValueColorProp && invalidValueColorProp)
  {
    vtkVector3d dc;
    vtkSMPropertyHelper(defaultValueColorProp).Get(dc.GetData(), 3);
    QVariantList vdc;
    vdc << dc[0] << dc[1] << dc[2];
    m_attrUIMgr->setDefaultValueColorRgbF(vdc);

    vtkVector3d ic;
    vtkSMPropertyHelper(invalidValueColorProp).Get(ic.GetData(), 3);
    QVariantList vic;
    vic << ic[0] << ic[1] << ic[2];
    m_attrUIMgr->setInvalidValueColorRgbF(vic);

    m_propertyLinks.addPropertyLink(m_attrUIMgr, "defaultValueColorRgbF",
      SIGNAL(defaultValueColorChanged()), paletteProxy, defaultValueColorProp);
    m_propertyLinks.addPropertyLink(m_attrUIMgr, "invalidValueColorRgbF",
      SIGNAL(invalidValueColorChanged()), paletteProxy, invalidValueColorProp);
  }

  // Fetch the current user preferences and update the UI manager with them.
  this->updateSettings();

  smtk::view::ConfigurationPtr view = rsrc ? rsrc->findTopLevelView() : nullptr;
  if (view)
  {
    didDisplay = this->displayView(view);
  }
  auto rsrcMgr = rsrc->manager();
  if (rsrcMgr)
  {
    std::weak_ptr<smtk::resource::Manager> weakResourceManager = rsrcMgr;
    m_observer = rsrcMgr->observers().insert(
      [this, weakResourceManager](
        const smtk::resource::Resource& attrRsrc, smtk::resource::EventType evnt) {
        auto rsrc = m_rsrc.lock();
        if (rsrc == nullptr ||
          (evnt == smtk::resource::EventType::REMOVED && &attrRsrc == rsrc.get()))
        {
          // The application is removing the attribute resource we are viewing.
          // Clear out the panel and unobserve the manager.
          delete m_attrUIMgr;
          m_attrUIMgr = nullptr;
          m_seln = nullptr;
          while (QWidget* w = this->widget()->findChild<QWidget*>())
          {
            delete w;
          }
          if (auto rsrcMgr = weakResourceManager.lock())
          {
            rsrcMgr->observers().erase(m_observer);
          }
        }
      },
      "pqSMTKAttributePanel: Clear panel if a removed resource is being displayed.");
  }
  return didDisplay;
}

bool pqSMTKAttributePanel::displayResourceOnServer(const smtk::attribute::ResourcePtr& rsrc)
{
  smtk::resource::ManagerPtr rsrcMgr;
  if (rsrc && (rsrcMgr = rsrc->manager()))
  {
    auto behavior = pqSMTKBehavior::instance();
    pqSMTKWrapper* wrapper = behavior->getPVResourceManager(rsrcMgr);
    if (wrapper)
    {
      // Keep hold of the selection instance for the active server connection
      // so that this->displayResource() below can make use of it.
      m_seln = wrapper->smtkSelection();
      m_opManager = wrapper->smtkOperationManager();
      m_viewManager = wrapper->smtkViewManager();
    }
    else
    {
      m_seln = nullptr;
      m_opManager = nullptr;
      m_viewManager = nullptr;
    }
    return this->displayResource(rsrc);
  }
  return false;
}

bool pqSMTKAttributePanel::displayView(smtk::view::ConfigurationPtr view)
{
  if (!view)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Null view passed to attribute panel.");
    return false;
  }

  if (!m_attrUIMgr)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "View passed but no resource indicated.");
    return false;
  }
  auto qview = m_attrUIMgr->setSMTKView(view, this->widget());
  return qview != nullptr;
}

bool pqSMTKAttributePanel::updatePipeline()
{
  auto dataSource = pqActiveObjects::instance().activeSource();
  return this->displayPipelineSource(dataSource);
}

void pqSMTKAttributePanel::updateSettings()
{
  if (!m_attrUIMgr)
  {
    return;
  }

  auto smtkSettings = vtkSMTKSettings::GetInstance();
  m_attrUIMgr->setHighlightOnHover(smtkSettings->GetHighlightOnHover());
}
