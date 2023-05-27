//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtInteractionWidget.h"

#include "pqActiveObjects.h"
#include "pqServer.h"
#include "pqView.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkNew.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMParaViewPipelineController.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMSessionProxyManager.h"

qtInteractionWidget::qtInteractionWidget(
  const vtkSmartPointer<vtkSMNewWidgetRepresentationProxy>& smproxy,
  QWidget* parentWdg)
  : Superclass(parentWdg)
  , WidgetProxy(smproxy.Get())
{
  Q_ASSERT(smproxy != nullptr);

  this->VTKConnect->Connect(
    smproxy, vtkCommand::StartInteractionEvent, this, SIGNAL(widgetStartInteraction()));
  this->VTKConnect->Connect(
    smproxy, vtkCommand::InteractionEvent, this, SIGNAL(widgetInteraction()));
  this->VTKConnect->Connect(
    smproxy, vtkCommand::EndInteractionEvent, this, SIGNAL(widgetEndInteraction()));
}

qtInteractionWidget::~qtInteractionWidget()
{
  this->setView(nullptr);
}

vtkSmartPointer<vtkSMNewWidgetRepresentationProxy> qtInteractionWidget::createWidget(
  const char* smgroup,
  const char* smname)
{
  pqServer* server = pqActiveObjects::instance().activeServer();
  Q_ASSERT(server);

  vtkSMSessionProxyManager* pxm = server->proxyManager();
  Q_ASSERT(pxm);
  vtkSmartPointer<vtkSMProxy> proxy;
  proxy.TakeReference(pxm->NewProxy(smgroup, smname));
  vtkSmartPointer<vtkSMNewWidgetRepresentationProxy> reprProxy;
  reprProxy = vtkSMNewWidgetRepresentationProxy::SafeDownCast(proxy.Get());

  if (reprProxy)
  {
    vtkNew<vtkSMParaViewPipelineController> controller;
    controller->InitializeProxy(reprProxy);
  }
  return reprProxy;
}

vtkSMNewWidgetRepresentationProxy* qtInteractionWidget::widgetProxy() const
{
  return this->WidgetProxy;
}

void qtInteractionWidget::setView(pqView* aview)
{
  if (this->View != aview)
  {
    if (vtkSMProxy* viewProxy = this->View ? this->View->getProxy() : nullptr)
    {
      vtkSMPropertyHelper(viewProxy, "HiddenRepresentations").Remove(this->widgetProxy());
      viewProxy->UpdateVTKObjects();
      this->render();
    }
    this->View = aview;
    if (vtkSMProxy* viewProxy = this->View ? this->View->getProxy() : nullptr)
    {
      vtkSMPropertyHelper(viewProxy, "HiddenRepresentations").Add(this->widgetProxy());
      viewProxy->UpdateVTKObjects();
      this->render();
    }

    // this will update the 3d widget's enabled state & visibility as needed.
    this->setEnableInteractivity(this->isInteractivityEnabled());
  }
}

pqView* qtInteractionWidget::view() const
{
  return this->View;
}

void qtInteractionWidget::setEnableInteractivity(bool val)
{
  bool trueInteractivity = (val && this->view());

  vtkSMProxy* wdgProxy = this->widgetProxy();
  Q_ASSERT(wdgProxy != nullptr);

  vtkSMPropertyHelper(wdgProxy, "Visibility", true).Set(trueInteractivity);
  vtkSMPropertyHelper(wdgProxy, "Enabled", true).Set(trueInteractivity);
  wdgProxy->UpdateVTKObjects();
  this->render();

  if (this->Interactivity != val)
  {
    this->Interactivity = val;
    Q_EMIT this->enableInteractivityChanged(this->Interactivity);
  }
}

void qtInteractionWidget::render()
{
  if (this->View)
  {
    this->View->render();
  }
}
