//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pq3DWidget.h"

// ParaView Server Manager includes.
#include "vtkBoundingBox.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkMemberFunctionCommand.h"
#include "vtkPVDataInformation.h"
#include "vtkPVXMLElement.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkSMInputProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMProxySelectionModel.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSession.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMTrace.h"

// Qt includes.
#include <QtDebug>
#include <QPointer>
#include <QShortcut>

// ParaView GUI includes.
#include "pq3DWidgetFactory.h"
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
//#include "pqBoxWidget.h"
#include "pqCoreUtilities.h"
//#include "pqDistanceWidget.h"
//#include "pqImplicitCylinderWidget.h"
//#include "pqImplicitPlaneWidget.h"
#include "pqInterfaceTracker.h"
//#include "pqLineSourceWidget.h"
#include "pqPipelineFilter.h"
#include "pqPipelineSource.h"
//#include "pqPointSourceWidget.h"
//#include "pqPolyLineWidget.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqSMAdaptor.h"
//#include "pqSphereWidget.h"
//#include "pqSplineWidget.h"

namespace
{
  vtkSMProxySelectionModel* getSelectionModel(vtkSMProxy* proxy)
    {
    vtkSMSessionProxyManager* pxm =
      proxy->GetSession()->GetSessionProxyManager();
    return pxm->GetSelectionModel("ActiveSources");
    }
}

//-----------------------------------------------------------------------------
class pq3DWidgetInternal
{
public:
  pq3DWidgetInternal() :
    IgnorePropertyChange(false),
    WidgetVisible(true),
    Selected(false),
    LastWidgetVisibilityGoal(true),
    InDeleteCall(false),
    PickOnMeshPoint(false)
  {
  this->VTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->IsMaster = pqApplicationCore::instance()->getActiveServer()->isMaster();
  }

  vtkSmartPointer<vtkSMProxy> ReferenceProxy;
  vtkSmartPointer<vtkSMNewWidgetRepresentationProxy> WidgetProxy;
  vtkSmartPointer<vtkCommand> ControlledPropertiesObserver;
  vtkSmartPointer<vtkPVXMLElement> Hints;
  vtkSmartPointer<vtkEventQtSlotConnect> VTKConnect;

  QMap<vtkSmartPointer<vtkSMProperty>, vtkSmartPointer<vtkSMProperty> > PropertyMap;

  /// Used to avoid recursion when updating the controlled properties
  bool IgnorePropertyChange;
  /// Stores the visible/hidden state of the 3D widget (controlled by the user)
  bool WidgetVisible;
  /// Stores the selected/not selected state of the 3D widget (controlled by the owning panel)
  bool Selected;

  QKeySequence PickSequence;
  QPointer<QShortcut> PickShortcut;
  bool IsMaster;
  bool LastWidgetVisibilityGoal;
  bool InDeleteCall;
  bool PickOnMeshPoint;
};

//-----------------------------------------------------------------------------
pq3DWidget::pq3DWidget(vtkSMProxy* refProxy, vtkSMProxy* pxy, QWidget* _p) :
  pqProxyPanel(pxy, _p),
  Internal(new pq3DWidgetInternal())
{
  this->UseSelectionDataBounds = false;
  this->Internal->ReferenceProxy = refProxy;

  this->Internal->ControlledPropertiesObserver.TakeReference(
    vtkMakeMemberFunctionCommand(*this,
      &pq3DWidget::onControlledPropertyChanged));
  this->Internal->IgnorePropertyChange = false;

  this->setControlledProxy(pxy);

  QObject::connect( pqApplicationCore::instance(),
                    SIGNAL(updateMasterEnableState(bool)),
                    this, SLOT(updateMasterEnableState(bool)));
  if (refProxy)
    {
    // Listen to UserEvent to allow Python to toggle widget visibility.
    // Leaving the current "core" design intact for this. When we refactor
    // 3DWidgets, we should revisit this design.
    pqCoreUtilities::connect(refProxy, vtkCommand::UserEvent,
      this, SLOT(handleReferenceProxyUserEvent(vtkObject*, unsigned long, void*)));
    }
}

//-----------------------------------------------------------------------------
pq3DWidget::~pq3DWidget()
{
  this->Internal->InDeleteCall = true;
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  if(widget)
    {
    pqApplicationCore::instance()->get3DWidgetFactory()->free3DWidget(widget);
    }
  this->setWidgetProxy(0);
  this->setView(0);
  this->setControlledProxy(0);
  delete this->Internal;
}

//-----------------------------------------------------------------------------
pqRenderViewBase* pq3DWidget::renderView() const
{
  return qobject_cast<pqRenderViewBase*>(this->view());
}

//-----------------------------------------------------------------------------
void pq3DWidget::pickingSupported(const QKeySequence& key)
{
  this->Internal->PickSequence = key;
}

//-----------------------------------------------------------------------------
void pq3DWidget::setView(pqView* pqview)
{
  pqRenderViewBase* rview = this->renderView();
  if (pqview == rview)
    {
    this->Superclass::setView(pqview);
    return;
    }

  // This test has been added to support proxy that have been created on
  // different servers. We return if we switch from a view from a server
  // to another view from another server.
  vtkSMProxy* widget = this->getWidgetProxy();
  if ((widget && pqview &&
       pqview->getProxy()->GetSession() != widget->GetSession())
      ||
      (rview && pqview &&
       rview->getProxy()->GetSession() != pqview->getProxy()->GetSession()))
    {
    return;
    }

  // get rid of old shortcut.
  delete this->Internal->PickShortcut;

  bool cur_visbility = this->widgetVisible();
  this->hideWidget();

  if (rview && widget)
    {
    // To add/remove the 3D widget display from the view module.
    // we don't use the property. This is so since the 3D widget add/remove
    // should not get saved in state or undo-redo.
    vtkSMPropertyHelper(
      rview->getProxy(), "HiddenRepresentations").Remove(widget);
    rview->getProxy()->UpdateVTKObjects();
    }

  this->Superclass::setView(pqview);

  rview = this->renderView();
  if (rview && !this->Internal->PickSequence.isEmpty())
    {
    this->Internal->PickShortcut = new QShortcut(
      this->Internal->PickSequence, pqview->widget());
    QObject::connect(this->Internal->PickShortcut, SIGNAL(activated()),
      this, SLOT(pickPoint()));
    }

  if (rview && widget)
    {
    // To add/remove the 3D widget display from the view module.
    // we don't use the property. This is so since the 3D widget add/remove
    // should not get saved in state or undo-redo.
    this->updateWidgetVisibility();
    vtkSMPropertyHelper(
      rview->getProxy(), "HiddenRepresentations").Add(widget);
    rview->getProxy()->UpdateVTKObjects();
    }

  if (cur_visbility)
    {
    this->showWidget();
    }
  this->updatePickShortcut();
}

//-----------------------------------------------------------------------------
void pq3DWidget::render()
{
  if (pqRenderViewBase* rview = this->renderView())
    {
    rview->render();
    }
}

//-----------------------------------------------------------------------------
void pq3DWidget::setPickOnMeshPoint(bool enable)
{
  this->Internal->PickOnMeshPoint = enable;
}

//-----------------------------------------------------------------------------
void pq3DWidget::pickPoint()
{
  pqRenderView* rview = qobject_cast<pqRenderView*>(this->renderView());
  if (rview && rview->getRenderViewProxy())
    {
    vtkRenderWindowInteractor* rwi = rview->getRenderViewProxy()->GetInteractor();
    if (!rwi)
      {
      return;
      }

    // Get region
    const int* eventpos = rwi->GetEventPosition();
    double position[3];
    if (rview->getRenderViewProxy()->ConvertDisplayToPointOnSurface(eventpos,
      position, this->Internal->PickOnMeshPoint))
      {
      this->pick(position[0], position[1], position[2]);
      }
    }
}

//-----------------------------------------------------------------------------
void pq3DWidget::onControlledPropertyChanged()
{
  if (this->Internal->IgnorePropertyChange)
    {
    return;
    }

  // Synchronize the 3D and Qt widgets with the controlled properties
  this->reset();
}

//-----------------------------------------------------------------------------
void pq3DWidget::setWidgetProxy(vtkSMNewWidgetRepresentationProxy* pxy)
{
  this->Internal->VTKConnect->Disconnect();

  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  pqRenderViewBase* rview = this->renderView();
  vtkSMProxy* viewProxy = rview? rview->getProxy() : NULL;
  if (rview && widget)
    {
    // To add/remove the 3D widget display from the view module.
    // we don't use the property. This is so since the 3D widget add/remove
    // should not get saved in state or undo-redo.
    vtkSMPropertyHelper(viewProxy,"HiddenRepresentations").Remove(widget);
    viewProxy->UpdateVTKObjects();
    rview->render();
    }

  this->Internal->WidgetProxy = pxy;

  if (pxy)
    {
    this->Internal->VTKConnect->Connect(pxy, vtkCommand::StartInteractionEvent,
      this, SIGNAL(widgetStartInteraction()));
    this->Internal->VTKConnect->Connect(pxy, vtkCommand::InteractionEvent,
      this, SLOT(setModified()));
    this->Internal->VTKConnect->Connect(pxy, vtkCommand::InteractionEvent,
      this, SIGNAL(widgetInteraction()));
    this->Internal->VTKConnect->Connect(pxy, vtkCommand::EndInteractionEvent,
      this, SIGNAL(widgetEndInteraction()));
    }

  if (rview && pxy)
    {
    // To add/remove the 3D widget display from the view module.
    // we don't use the property. This is so since the 3D widget add/remove
    // should not get saved in state or undo-redo.
    vtkSMPropertyHelper(viewProxy,"HiddenRepresentations").Add(widget);
    viewProxy->UpdateVTKObjects();
    rview->render();
    }
}

//-----------------------------------------------------------------------------
vtkSMNewWidgetRepresentationProxy* pq3DWidget::getWidgetProxy() const
{
  return this->Internal->WidgetProxy;
}

//-----------------------------------------------------------------------------
vtkSMProxy* pq3DWidget::getControlledProxy() const
{
  return this->proxy();
}

//-----------------------------------------------------------------------------
vtkSMProxy* pq3DWidget::getReferenceProxy() const
{
  return this->Internal->ReferenceProxy;
}

//-----------------------------------------------------------------------------
void pq3DWidget::setControlledProxy(vtkSMProxy* /*pxy*/)
{
  foreach(vtkSMProperty* controlledProperty, this->Internal->PropertyMap)
    {
    controlledProperty->RemoveObserver(
      this->Internal->ControlledPropertiesObserver);
    }
  this->Internal->PropertyMap.clear();
}

//-----------------------------------------------------------------------------
vtkPVXMLElement* pq3DWidget::getHints() const
{
  return this->Internal->Hints;
}

//-----------------------------------------------------------------------------
void pq3DWidget::setHints(vtkPVXMLElement* hints)
{
  this->Internal->Hints = hints;
  if (!hints)
    {
    return;
    }

  if (!this->proxy())
    {
    qDebug() << "pq3DWidget::setHints must be called only after the controlled "
      << "proxy has been set.";
    return;
    }
  if (QString("PropertyGroup") != hints->GetName())
    {
    qDebug() << "Argument to setHints must be a <PropertyGroup /> element.";
    return;
    }

  vtkSMProxy* pxy = this->proxy();
  unsigned int max_props = hints->GetNumberOfNestedElements();
  for (unsigned int i=0; i < max_props; i++)
    {
    vtkPVXMLElement* propElem = hints->GetNestedElement(i);
    this->setControlledProperty(propElem->GetAttribute("function"),
      pxy->GetProperty(propElem->GetAttribute("name")));
    }
}

//-----------------------------------------------------------------------------
bool pq3DWidget::pickOnMeshPoint() const
{
  return this->Internal->PickOnMeshPoint;
}

//-----------------------------------------------------------------------------
void pq3DWidget::setControlledProperty(const char* function,
  vtkSMProperty* controlled_property)
{
  this->Internal->PropertyMap.insert(
    this->Internal->WidgetProxy->GetProperty(function),
    controlled_property);

  controlled_property->AddObserver(vtkCommand::ModifiedEvent,
    this->Internal->ControlledPropertiesObserver);
}

//-----------------------------------------------------------------------------
void pq3DWidget::setControlledProperty(vtkSMProperty* widget_property,
  vtkSMProperty* controlled_property)
{
  this->Internal->PropertyMap.insert(
    widget_property,
    controlled_property);

  controlled_property->AddObserver(vtkCommand::ModifiedEvent,
    this->Internal->ControlledPropertiesObserver);
}

//-----------------------------------------------------------------------------
void pq3DWidget::accept()
{
  this->Internal->IgnorePropertyChange = true;
  QMap<vtkSmartPointer<vtkSMProperty>,
    vtkSmartPointer<vtkSMProperty> >::const_iterator iter;
  for (iter = this->Internal->PropertyMap.constBegin() ;
    iter != this->Internal->PropertyMap.constEnd();
    ++iter)
    {
    iter.value()->Copy(iter.key());
    }
  if (this->proxy())
    {
    this->proxy()->UpdateVTKObjects();
    }
  this->Internal->IgnorePropertyChange = false;
}

//-----------------------------------------------------------------------------
void pq3DWidget::reset()
{
  // We don't want to fire any widget modified events while resetting the
  // 3D widget, hence we block all signals. Otherwise, on reset, we fire a
  // widget modified event, which makes the accept button enabled again.
  this->blockSignals(true);
  QMap<vtkSmartPointer<vtkSMProperty>,
    vtkSmartPointer<vtkSMProperty> >::const_iterator iter;
  for (iter = this->Internal->PropertyMap.constBegin() ;
    iter != this->Internal->PropertyMap.constEnd();
    ++iter)
    {
    iter.key()->Copy(iter.value());
    }

  if (this->Internal->WidgetProxy)
    {
    this->Internal->WidgetProxy->UpdateVTKObjects();
    this->Internal->WidgetProxy->UpdatePropertyInformation();
    this->render();
    }
  this->blockSignals(false);
}

//-----------------------------------------------------------------------------
bool pq3DWidget::widgetVisible() const
{
  return this->Internal->WidgetVisible;
}

//-----------------------------------------------------------------------------
bool pq3DWidget::widgetSelected() const
{
  return this->Internal->Selected;
}

//-----------------------------------------------------------------------------
void pq3DWidget::select()
{
  if(true != this->Internal->Selected)
    {
    this->Internal->Selected = true;
    this->updateWidgetVisibility();
    }
}

//-----------------------------------------------------------------------------
void pq3DWidget::deselect()
{
  if(false != this->Internal->Selected)
    {
    this->Internal->Selected = false;
    this->updateWidgetVisibility();
    }
}

//-----------------------------------------------------------------------------
void pq3DWidget::setWidgetVisible(bool visible)
{
  if(this->Internal->IsMaster)
    {
    this->Internal->LastWidgetVisibilityGoal = visible;
    }

  if( visible != this->Internal->WidgetVisible &&
      ((!this->Internal->IsMaster && !visible) || this->Internal->IsMaster))
    {
    this->Internal->WidgetVisible = visible;
    this->updateWidgetVisibility();

    if (vtkSMProxy* refProxy = this->getReferenceProxy())
      {
      SM_SCOPED_TRACE(CallFunction)
        .arg(visible? "Show3DWidgets" : "Hide3DWidgets")
        .arg("proxy", refProxy)
        .arg("comment", "toggle 3D widget visibility (only when running from the GUI)");
      }
    emit this->widgetVisibilityChanged(visible);
    }
}

//-----------------------------------------------------------------------------
void pq3DWidget::showWidget()
{
  this->setWidgetVisible(true);
}

//-----------------------------------------------------------------------------
void pq3DWidget::hideWidget()
{
  this->setWidgetVisible(false);
}

//-----------------------------------------------------------------------------
int pq3DWidget::getReferenceInputBounds(double bounds[6]) const
{
  vtkSMProxy* refProxy = this->getReferenceProxy();
  if (!refProxy)
    {
    return 0;
    }

  vtkSMSourceProxy* input = NULL;
  vtkSMInputProperty* ivp = vtkSMInputProperty::SafeDownCast(
    refProxy->GetProperty("Input"));
  int output_port = 0;
  if (ivp && ivp->GetNumberOfProxies())
    {
    vtkSMProxy* pxy = ivp->GetProxy(0);
    input = vtkSMSourceProxy::SafeDownCast(pxy);
    output_port =ivp->GetOutputPortForConnection(0);
    }
  else
    {
    // reference proxy has no input. This generally happens when the widget is
    // controlling properties of a source. In that case, if the source has been
    // "created", simply use the source's bounds.
    input = vtkSMSourceProxy::SafeDownCast(refProxy);
    }

  if(input)
    {
    input->GetDataInformation(output_port)->GetBounds(bounds);
    return (bounds[1] >= bounds[0] && bounds[3] >= bounds[2] && bounds[5] >=
      bounds[4]) ? 1 : 0;
    }
  return 0;
}

//-----------------------------------------------------------------------------
void pq3DWidget::updateWidgetVisibility()
{
  const bool widget_visible = this->Internal->Selected
    && this->Internal->WidgetVisible;

  const bool widget_enabled = widget_visible;

  this->updateWidgetState(widget_visible, widget_enabled);
}

//-----------------------------------------------------------------------------
void pq3DWidget::updateWidgetState(bool widget_visible, bool widget_enabled)
{
  if (this->Internal->WidgetProxy && this->renderView())
    {
    if(vtkSMIntVectorProperty* const visibility =
      vtkSMIntVectorProperty::SafeDownCast(
        this->Internal->WidgetProxy->GetProperty("Visibility")))
      {
      visibility->SetElement(0, widget_visible);
      }

    if(vtkSMIntVectorProperty* const enabled =
      vtkSMIntVectorProperty::SafeDownCast(
        this->Internal->WidgetProxy->GetProperty("Enabled")))
      {
      enabled->SetElement(0, widget_enabled);
      }

    this->Internal->WidgetProxy->UpdateVTKObjects();

    // need to render since the widget visibility state changed.
    this->render();
    }
  this->updatePickShortcut();
}

//-----------------------------------------------------------------------------
void pq3DWidget::updatePickShortcut()
{
  bool pickable = (this->Internal->Selected
    && this->Internal->WidgetVisible &&
    this->Internal->WidgetProxy && this->renderView());

  this->updatePickShortcut(pickable);
}

//-----------------------------------------------------------------------------
void pq3DWidget::updatePickShortcut(bool pickable)
{
  if (this->Internal->PickShortcut)
    {
    this->Internal->PickShortcut->setEnabled(pickable);
    }
}

//-----------------------------------------------------------------------------
void pq3DWidget::resetBounds()
{
  vtkSMNewWidgetRepresentationProxy* widget = this->getWidgetProxy();
  if (!widget)
    {
    return;
    }

  double input_bounds[6];
  if (this->UseSelectionDataBounds)
    {
    vtkSMProxySelectionModel* selModel = getSelectionModel(widget);

    if (!selModel->GetSelectionDataBounds(input_bounds))
      {
      return;
      }
    }
  else if (!this->getReferenceInputBounds(input_bounds))
    {
    return;
    }
  this->resetBounds(input_bounds);
  this->setModified();
  this->render();
}
//-----------------------------------------------------------------------------
void pq3DWidget::updateMasterEnableState(bool I_am_the_Master)
{
  this->Internal->IsMaster = I_am_the_Master;
  if(I_am_the_Master)
    {
    this->setWidgetVisible(this->Internal->LastWidgetVisibilityGoal);
    }
  else
    {
    this->hideWidget();
    }
}

//-----------------------------------------------------------------------------
void pq3DWidget::handleReferenceProxyUserEvent(
  vtkObject* caller, unsigned long eventid, void* calldata)
{
  (void)caller;
  (void)eventid;

  Q_ASSERT(caller == this->getReferenceProxy());
  Q_ASSERT(eventid == vtkCommand::UserEvent);

  const char* message = reinterpret_cast<const char*>(calldata);
  if (message != NULL && strcmp("HideWidget", message) == 0)
      {
      this->hideWidget();
      }
  else if (message != NULL && strcmp("ShowWidget", message) == 0)
    {
    this->showWidget();
    }
}
