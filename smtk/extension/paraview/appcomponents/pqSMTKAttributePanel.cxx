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
#include "smtk/resource/Observer.h"
#include "smtk/resource/Properties.h"
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
{
  this->setObjectName("attributeEditor");
  this->updateTitle();
  QWidget* w = new QWidget(this);
  w->setObjectName("attributePanel");
  this->setWidget(w);
  w->setLayout(new QVBoxLayout);
  auto* behavior = pqSMTKBehavior::instance();
  QObject::connect(
    behavior,
    SIGNAL(postProcessingModeChanged(bool)),
    this,
    SLOT(displayActivePipelineSource(bool)));
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk attribute panel", this);
  }

  auto* smtkSettings = vtkSMTKSettings::GetInstance();
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

void pqSMTKAttributePanel::resetPanel(smtk::resource::ManagerPtr rsrcMgr)
{
  (void)rsrcMgr;
  if (m_attrUIMgr)
  {
    m_propertyLinks.clear();
    delete m_attrUIMgr;
    m_attrUIMgr = nullptr;
    while (QWidget* w = this->widget()->findChild<QWidget*>())
    {
      delete w;
    }
  }

  m_observer.release();
  m_rsrc = std::weak_ptr<smtk::resource::Resource>();
}

bool pqSMTKAttributePanel::displayResource(const smtk::attribute::ResourcePtr& rsrc)
{
  bool didDisplay = false;

  if (rsrc)
  {
    auto previousResource = m_rsrc.lock();

    if (rsrc->isPrivate() && rsrc != previousResource)
    {
      if (previousResource)
      {
        previousResource->properties().erase<bool>("smtk.attribute_panel.display_hint");
      }
      resetPanel(rsrc->manager());
      didDisplay = displayResourceInternal(rsrc);
    }
    else if (!rsrc->isPrivate() && rsrc == previousResource)
    {
      // the panel is displaying a resource that is now private
      // stop displaying it
      resetPanel(rsrc->manager());
    }
  }
  else
  {
    this->updateTitle();
  }

  return didDisplay;
}

bool pqSMTKAttributePanel::displayResourceInternal(const smtk::attribute::ResourcePtr& rsrc)
{
  bool didDisplay = false;

  m_rsrc = rsrc;

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
  auto* paletteProxy = pxm ? pxm->GetProxy("settings", "ColorPalette") : nullptr;
  auto* defaultValueColorProp =
    paletteProxy ? paletteProxy->GetProperty("SMTKDefaultValueBackground") : nullptr;
  auto* invalidValueColorProp =
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

    m_propertyLinks.addPropertyLink(
      m_attrUIMgr,
      "defaultValueColorRgbF",
      SIGNAL(defaultValueColorChanged()),
      paletteProxy,
      defaultValueColorProp);
    m_propertyLinks.addPropertyLink(
      m_attrUIMgr,
      "invalidValueColorRgbF",
      SIGNAL(invalidValueColorChanged()),
      paletteProxy,
      invalidValueColorProp);
  }

  // Fetch the current user preferences and update the UI manager with them.
  this->updateSettings();

  smtk::view::ConfigurationPtr view = rsrc ? rsrc->findTopLevelView() : nullptr;
  if (view)
  {
    didDisplay = this->displayView(view);
    if (didDisplay)
    {
      rsrc->properties().get<bool>()["smtk.attribute_panel.display_hint"] = true;
    }
  }
  this->updateTitle(view);
  auto rsrcMgr = rsrc->manager();
  if (rsrcMgr)
  {
    std::weak_ptr<smtk::resource::Manager> weakResourceManager = rsrcMgr;
    m_observer = rsrcMgr->observers().insert(
      [this, weakResourceManager](
        const smtk::resource::Resource& attrRsrc, smtk::resource::EventType evnt) {
        auto rsrc = m_rsrc.lock();
        if (
          rsrc == nullptr ||
          (evnt == smtk::resource::EventType::REMOVED && &attrRsrc == rsrc.get()))
        {
          // The application is removing the attribute resource we are viewing.
          // Clear out the panel and unobserve the manager.
          this->resetPanel(weakResourceManager.lock());
          this->updateTitle();
          m_seln = nullptr;
        }
      },
      std::numeric_limits<smtk::resource::Observers::Priority>::lowest(),
      /* initialize */ false,
      "pqSMTKAttributePanel: Clear panel if a removed resource is being displayed.");
  }
  m_rsrc = rsrc;
  return didDisplay;
}

bool pqSMTKAttributePanel::displayResourceOnServer(const smtk::attribute::ResourcePtr& rsrc)
{
  smtk::resource::ManagerPtr rsrcMgr;
  if (rsrc && (rsrcMgr = rsrc->manager()))
  {
    auto* behavior = pqSMTKBehavior::instance();
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
  auto* qview = m_attrUIMgr->setSMTKView(view, this->widget());
  return qview != nullptr;
}

bool pqSMTKAttributePanel::updatePipeline()
{
  auto* dataSource = pqActiveObjects::instance().activeSource();
  return this->displayPipelineSource(dataSource);
}

void pqSMTKAttributePanel::updateSettings()
{
  if (!m_attrUIMgr)
  {
    return;
  }

  auto* smtkSettings = vtkSMTKSettings::GetInstance();
  m_attrUIMgr->setHighlightOnHover(smtkSettings->GetHighlightOnHover());
}

void pqSMTKAttributePanel::displayActivePipelineSource(bool doDisplay)
{
  if (doDisplay)
  {
    QObject::connect(
      &pqActiveObjects::instance(),
      SIGNAL(sourceChanged(pqPipelineSource*)),
      this,
      SLOT(displayPipelineSource(pqPipelineSource*)),
      Qt::QueuedConnection);
    QObject::connect(
      &pqActiveObjects::instance(),
      SIGNAL(dataUpdated()),
      this,
      SLOT(updatePipeline()),
      Qt::QueuedConnection);
  }
  else
  {
    QObject::disconnect(
      &pqActiveObjects::instance(),
      SIGNAL(sourceChanged(pqPipelineSource*)),
      this,
      SLOT(displayPipelineSource(pqPipelineSource*)));
    QObject::disconnect(
      &pqActiveObjects::instance(), SIGNAL(dataUpdated()), this, SLOT(updatePipeline()));
  }
}

void pqSMTKAttributePanel::updateTitle(const smtk::view::ConfigurationPtr& view)
{
  // By default the Panel's name is Attribute Editor
  std::string panelName = "Attribute Editor";
  if (view)
  {
    // Lets see if the view wants a different base name
    view->details().attribute("AttributePanelTitle", panelName);
    // Lets see if we are suppose to add the resource name to it
    if (view->details().attributeAsBool("IncludeResourceNameInPanel"))
    {
      auto rsrc = m_rsrc.lock();
      panelName = rsrc ? (panelName + '(' + rsrc->name() + ')') : panelName;
    }
  }
  this->setWindowTitle(panelName.c_str());
}
