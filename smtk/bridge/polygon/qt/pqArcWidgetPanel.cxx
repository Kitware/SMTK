//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/bridge/polygon/qt/pqArcWidgetPanel.h"
#include "ui_qtArcWidgetPanel.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqRenderViewSelectionReaction.h"
#include "pqSMAdaptor.h"

#include "vtkDoubleArray.h"
// #include "vtkPVArcInfo.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMVectorProperty.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkNew.h"
#include "vtkContourWidget.h"

#include "smtk/extension/vtk/widgets/vtkSMTKArcRepresentation.h"
#include "smtk/bridge/polygon/qt/pqPolygonArc.h"
#include "smtk/bridge/polygon/qt/pqArcWidgetManager.h"
#include "smtk/extension/paraview/widgets/pqArcWidget.h"

#include <QtDebug>

namespace Ui
{
//-----------------------------------------------------------------------------
ArcPointPicker::ArcPointPicker(QObject * parent):
  QAction(parent),
  Info(NULL),
  Arc(NULL),
  View(NULL),
  Selecter(NULL)
{
  this->setCheckable(true);
}

//-----------------------------------------------------------------------------
ArcPointPicker::~ArcPointPicker()
{
  if(this->Selecter)
    {
    this->Selecter->disconnect();
    delete this->Selecter;
    }
}

//-----------------------------------------------------------------------------
void ArcPointPicker::doPick(pqRenderView *view, pqPolygonArc *arc, PickInfo &info)
{
  if(this->Selecter)
    {
    delete this->Selecter;
    }

  this->View = NULL; //clear the view each time
  if(view)
    {
    this->Selecter = new pqRenderViewSelectionReaction(this,view,
                          pqRenderViewSelectionReaction::SELECT_SURFACE_POINTS);

    // we only want selection on one representation.
    view->setUseMultipleRepresentationSelection(false);
    // things are selected
    QObject::connect(view,SIGNAL(selected(pqOutputPort*)),
                     this,SLOT(selectedInfo(pqOutputPort*)),
                     Qt::UniqueConnection);
    // selection is done
    QObject::connect(view,SIGNAL(selectionModeChanged(bool)),
                     this,SLOT(onPickingFinished()),
                     Qt::UniqueConnection);

    this->Info = &info;
    this->Arc = arc;
    this->View = view;

    emit triggered(true);
    }
}

//-----------------------------------------------------------------------------
void ArcPointPicker::selectedInfo(pqOutputPort* port)
{
  //always update the port
  this->Info->IsValid = false;
  this->Info->port = port;
/*
  if(port && port->getSource() == this->Arc->getSource())
    {
    // get the selected point id
    // This "IDs" only have two components [processId, Index]
    // because the arc source is not a Composite Dataset
    vtkSMSourceProxy* selSource = port->getSelectionInput();
    vtkSMVectorProperty* vp = vtkSMVectorProperty::SafeDownCast(
      selSource->GetProperty("IDs"));
    QList<QVariant> ids = pqSMAdaptor::getMultipleElementProperty(vp);
    int numElemsPerCommand = vp->GetNumberOfElementsPerCommand();
    if(ids.count() > 1 && numElemsPerCommand > 0)
      {

      vtkPVArcInfo* arcInfo = this->Arc->getArcInfo();
      // we just pick the first point
      vtkIdType pointId = ids.value(1).value<vtkIdType>();
      if(arcInfo->GetPointLocation(pointId, this->Info->pointLocation))
        {
        this->Info->IsValid = true;
        this->Info->PointId = pointId;
        }

      }
    }

  // clear selection on this port if it is not the arc we expected
  if(port && port->getSource() != this->Arc->getSource())
    {
    port->setSelectionInput(NULL, 0);
    }
*/
}

//-----------------------------------------------------------------------------
void ArcPointPicker::onPickingFinished()
{
  //we want the connection to happen once the view goes away so
  //remove the connection
  if(this->Selecter)
    {
    this->Selecter->disconnect();
    delete this->Selecter;
    this->Selecter = NULL;
    }
  if(this->View)
    {
    this->View->forceRender();
    // reset multiple selection to true
    this->View->setUseMultipleRepresentationSelection(false);
    }
  emit this->pickFinished();
}
}


class pqArcWidgetPanel::pqInternals : public Ui::qtArcWidgetPanel
  {
  public:
    QPointer<pqDataRepresentation> SubArcRepresentation;
    QPointer<pqPipelineSource> SubArcSource;
  };

//-----------------------------------------------------------------------------
pqArcWidgetPanel::pqArcWidgetPanel(QWidget *parent) :
  QWidget(parent),
  Internals(new pqArcWidgetPanel::pqInternals),
  Picker(parent),
  View(NULL),
  Arc(NULL),
  SubWidget(NULL),
  ArcManager(NULL),
  StartPoint(),
  EndPoint()
{
  Internals->setupUi(this);
  this->setObjectName("pqArcWidgetPanel");

  //connect up the pick buttons
  QObject::connect(this->Internals->PickButtonBox, SIGNAL(rejected()),
    this, SLOT(finishedArcModification()));

  QObject::connect(this->Internals->WholeArcButton, SIGNAL(released()),
    this, SLOT(pickWholeArc()));
  QObject::connect(this->Internals->StartPointButton, SIGNAL(released()),
    this, SLOT(PickStartPoint()));
  QObject::connect(this->Internals->EndPointButton, SIGNAL(released()),
    this, SLOT(PickEndPoint()));

  // operations buttons
  QObject::connect(this->Internals->ArcCollapseButton, SIGNAL(released()),
    this, SLOT(onCollapseSubArc()));
  QObject::connect(this->Internals->ArcStraightenButton, SIGNAL(released()),
    this, SLOT(onStraightenArc()));
  QObject::connect(this->Internals->EditButton, SIGNAL(released()),
    this, SLOT(showEditWidget()));
  QObject::connect(this->Internals->MakeArcButton, SIGNAL(released()),
    this, SLOT(onMakeArc()));
  //QObject::connect(this->Internals->RectArcButton, SIGNAL(released()),
  //  this, SLOT(generateRectangleArc()));

  //connect up the edit buttons
  //QObject::connect(this->Internals->EditButtonBox, SIGNAL(accepted()),
  //  this, SLOT(saveEdit()));
  QObject::connect(this->Internals->EditButtonBox, SIGNAL(rejected()),
    this, SLOT(cancelEdit()));

  //connect the picker up so we know when it is done.
  QObject::connect(&this->Picker, SIGNAL(pickFinished()),
    this, SLOT(showPickWidget()), Qt::QueuedConnection);

  //setup the widget to look correctly
  //this->Internals->ArcEditWidgtet->hide();
  //this->updateGeometry();
  //this->resetWidget();
}

//-----------------------------------------------------------------------------

pqArcWidgetPanel::~pqArcWidgetPanel()
{
  if(this->Internals->SubArcSource)
    {
    pqApplicationCore::instance()->getObjectBuilder()->destroy(
      this->Internals->SubArcSource);
    this->Internals->SubArcSource = NULL;
    }

  if ( this->SubWidget )
    {
    //if a widget is deleted without having an active view it throws errors
    if ( this->View && !this->SubWidget->view() )
      {
      this->SubWidget->setView(this->View);
      }
    delete this->SubWidget;
    }
  this->SubWidget = NULL;
  delete Internals;
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::setArc(pqPolygonArc* arc)
{
  if(this->Arc != arc)
    {
    this->Arc=arc;
    if(this->Arc)
      {
      this->resetWidget();
      }
    }
}

//-----------------------------------------------------------------------------
bool pqArcWidgetPanel::isSubArcValid()
{
  // for a closed loop arc, we don't allow a sub-arc to include the end node
  // in the middle of the sub-arc, but it can be at either end of the sub-arc
  bool isEndNodeInMiddle = (this->StartPoint.IsValid && this->EndPoint.IsValid &&
   this->Arc && this->Arc->isClosedLoop() &&
   this->StartPoint.PointId > this->EndPoint.PointId &&
   this->EndPoint.PointId > 0);
  if(isEndNodeInMiddle)
    {
    qCritical() << "Currently for a closed loop arc, we don't allow a sub-arc \n"
     <<"to include the end node in the middle of the sub-arc, \n"
     << "but the end node can be at either end of the sub-arc. \n";
    return false;
    }
  return (this->StartPoint.IsValid && this->EndPoint.IsValid &&
    this->StartPoint.PointId != this->EndPoint.PointId);
}

//-----------------------------------------------------------------------------
bool pqArcWidgetPanel::isWholeArcSelected()
{
  if(!this->StartPoint.IsValid || !this->EndPoint.IsValid )
    {
    return false;
    }

  vtkIdType numRequestedArcPoints =
  (this->EndPoint.PointId >= this->StartPoint.PointId) ?
  (this->EndPoint.PointId - this->StartPoint.PointId + 1) :
  (this->StartPoint.PointId - this->EndPoint.PointId + 1);
/*
  //see if this is a closed arc
  vtkPVArcInfo* arcInfo = this->Arc->getArcInfo();
  bool closedArc = arcInfo->IsClosedLoop();
  return ((numRequestedArcPoints == arcInfo->GetNumberOfPoints() &&
   !closedArc) || (closedArc && this->StartPoint.PointId == this->EndPoint.PointId));
*/
  return false;
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::showEditWidget()
{
  //hide the pointWidget from the layout to reclaim the space
  this->Internals->ArcEditWidgtet->show();
  this->Internals->SelectPointsWidget->hide();

  //update the layout
  this->updateGeometry();

  //this->Internals->EditButtonBox->button(
  //  QDialogButtonBox::Save)->setEnabled(false);
  this->modifySubArc();
  emit this->startArcEditing();
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::showPickWidget()
{
  //shows the pick widget and hides the edit widget
  this->Internals->SelectPointsWidget->show();
  this->Internals->ArcEditWidgtet->hide();

  //update the layout
  this->updateGeometry();

  //setup the state correctly
  //update the labels
  QString labelText = this->StartPoint.IsValid ?
    this->StartPoint.text() : "Start Point Location";
  this->Internals->StartPointLabel->setText(labelText);
  labelText = this->EndPoint.IsValid ?
    this->EndPoint.text() : "End Point Location";
  this->Internals->EndPointLabel->setText(labelText);
  const bool enabled(this->isSubArcValid() || this->isWholeArcSelected());
  this->Internals->OperationsWidget->setEnabled(enabled);

  if(this->isWholeArcSelected())
    {
    this->Internals->WholeArcOpWidget->setVisible(true);
    this->Internals->SubArcOpWidget->setVisible(false);
    this->Internals->ArcStraightenButton->setVisible(this->Arc->isClosedLoop() ? false : true);
    }
  else
    {
    this->Internals->WholeArcOpWidget->setVisible(false);
    this->Internals->SubArcOpWidget->setVisible(true);
    this->Internals->ArcStraightenButton->setVisible(true);
    }
  // if a valid arc is picked, clear point selection on the arc
  if(enabled)
    {
//    this->Arc->pqCMBSceneObjectBase::setSelectionInput(NULL);
    }
  this->updateSubArcRepresentation(enabled);
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::PickStartPoint()
{
  //need to convey what we are pick (start,end)
  this->Picker.doPick(this->View,this->Arc,this->StartPoint);

  //do we need to lock the UI?
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::pickWholeArc()
{
/*
  if ( !this->Arc || !this->Arc->getSource())
    {
    return;
    }
  // set up start point as first point
  this->StartPoint.PointId = 0;
  this->StartPoint.port = this->Arc->getSource()->getOutputPort(0);

  vtkPVArcInfo* arcInfo = this->Arc->getArcInfo();
  // we just pick the first point
  if(arcInfo->GetPointLocation(
    this->StartPoint.PointId, this->StartPoint.pointLocation))
    {
    this->StartPoint.IsValid = true;
    }

  // set up end point as last point
  this->EndPoint.PointId = arcInfo->IsClosedLoop() ?
    0 : arcInfo->GetNumberOfPoints() - 1;
  this->EndPoint.port = this->Arc->getSource()->getOutputPort(0);
  if(arcInfo->GetPointLocation(
    this->EndPoint.PointId, this->EndPoint.pointLocation))
    {
    this->EndPoint.IsValid = true;
    }

  // update the UI for whole arc
  this->showPickWidget();
*/
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::PickEndPoint()
{
  this->Picker.doPick(this->View,this->Arc,this->EndPoint);
}
//-----------------------------------------------------------------------------
void pqArcWidgetPanel::hideArcWidget()
{
  if(this->SubWidget)
    {
    this->SubWidget->setWidgetVisible(false);
    this->SubWidget->setVisible(false);
    this->SubWidget->deselect();
    this->SubWidget->hideWidget();
    this->SubWidget->getWidgetProxy()->UpdatePropertyInformation();
    this->SubWidget->setView(NULL);
    this->SubWidget->hide();
    }
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::modifySubArc()
{
  if (!this->Internals->SubArcSource || !this->ArcManager)
    {
    qCritical() << "There was not valid sub-arc to modify yet.\n";
    return;
    }
  pqPolygonArc* arcObj = this->ArcManager->getActiveArc();
  if(!arcObj)
    {
    return;
    }
  bool created = false;
  int normal;
  double planePos;
  if ( !this->SubWidget )
    {
    this->SubWidget = this->ArcManager->createDefaultContourWidget(normal, planePos);
    this->SubWidget->setParent(this->Internals->ArcEditWidgtet);
    this->Internals->ContourLayout->addWidget(this->SubWidget);

    QObject::connect(this->SubWidget,SIGNAL(contourDone()),
      this,SLOT(arcEditingFinished()));
    created = true;
    }
  if ( !created )
    {
    this->SubWidget->setView(this->View);
    if(arcObj)
      {
      vtkSMProxyProperty* proxyProp =
        vtkSMProxyProperty::SafeDownCast(
        this->SubWidget->getWidgetProxy()->GetProperty("PointPlacer"));
      if (proxyProp && proxyProp->GetNumberOfProxies())
        {
        vtkSMProxy* pointplacer = proxyProp->GetProxy(0);
        vtkSMPropertyHelper(pointplacer, "ProjectionNormal").Set(
          arcObj->getPlaneProjectionNormal());
        vtkSMPropertyHelper(pointplacer, "ProjectionPosition").Set(
          arcObj->getPlaneProjectionPosition());
        pointplacer->MarkModified(pointplacer);
        pointplacer->UpdateVTKObjects();
        }
      }
    vtkSMPropertyHelper(this->SubWidget->getWidgetProxy(), "Enabled").Set(1);
    this->SubWidget->getWidgetProxy()->UpdateVTKObjects();
    }

  this->SubWidget->useArcEditingUI(this->isWholeArcSelected());
  this->SubWidget->show();
  this->SubWidget->setEnabled(true);
  this->SubWidget->setWidgetVisible(true);

  if ( arcObj /*&& arcObj->getType() == pqCMBSceneObjectBase::Arc*/)
    {
    //pass the info from the arc into the widget proxy
    //for now only works in built in mode
/*
    vtkNew<vtkCMBArcEditClientOperator> editOp;
    editOp->SetArcIsClosed(this->Arc->isClosedLoop() && this->isWholeArcSelected());
    editOp->Operate(this->Internals->SubArcSource->getProxy(),
      this->SubWidget->getWidgetProxy());
*/
    if (!created)
      {
      this->SubWidget->reset();
      }

    this->SubWidget->checkContourLoopClosed();
    this->SubWidget->ModifyMode();
    this->SubWidget->checkCanBeEdited();

/* TO DO: We may allow this in the future.
    // if the end node of a closed-loop arc is in the middle of the
    // the specified sub-arc, we need to set that node in the SubWidget
    // to be selected.
    if(this->Arc->isClosedLoop() &&
      this->StartPoint.PointId > this->EndPoint.PointId &&
      this->EndPoint.PointId > 0)
      {
      int nodeSubArcIndex =
        this->Arc->getNumberOfPoints() - this->EndPoint.PointId;
      vtkSMPropertyHelper(this->SubWidget->getWidgetProxy(),
        "NthNodeSelected").Set(nodeSubArcIndex);
      this->SubWidget->getWidgetProxy()->UpdateVTKObjects();
      }
*/

    this->SubWidget->setModified();
    }
  //this->setSubArcVisible(false);
  this->View->forceRender();
  this->SubWidget->select();
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::resetWidget()
{
  //resets the widget to what it would be like if it was just created
  this->StartPoint.IsValid = false;
  this->EndPoint.IsValid = false;
  if(this->Internals->SubArcRepresentation)
    {
    this->setSubArcVisible(false);
    pqApplicationCore::instance()->getObjectBuilder()->destroy(
      this->Internals->SubArcRepresentation);
    this->Internals->SubArcRepresentation = NULL;
    }
  this->showPickWidget();
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::updateWholeArcRepresentation(bool visible)
{
  if ( !this->Arc )//|| !this->Arc->getRepresentation())
    {
    return;
    }
  // use a green color for representation if we are showing whole arc,
  // like sub-arc; else, set the representation back to the original color
  double rgba[4] ={0, 0.8, 0, 1.0};
  if(!visible)
    {
    this->Arc->getColor(rgba);
    }
  //this->Arc->getRepresentation()->setVisible(visible);
  // only change the representation color
//  this->Arc->pqCMBSceneObjectBase::setColor(rgba, visible);
  this->View->forceRender();
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::updateSubArcRepresentation(bool visible)
{
  if ( !this->Arc )//|| !this->Arc->getSource())
    {
    return;
    }
  if(!visible)
    {
    this->setSubArcVisible(visible);
    return;
    }
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  if (!this->Internals->SubArcSource)
    {
    //create an arc provider for this arc
    this->Internals->SubArcSource = builder->createSource(
      "CmbArcGroup", "ArcProvider", core->getActiveServer());
    }

  //tell the provider the arc id it needs to be connected too
  vtkSMProxy *sourceProxy = this->Internals->SubArcSource->getProxy();
  vtkSMPropertyHelper(sourceProxy,"ArcId").Set(this->Arc->getArcId());
  vtkSMPropertyHelper(sourceProxy,"StartPointId").Set(this->StartPoint.PointId);
  vtkSMPropertyHelper(sourceProxy,"EndPointId").Set(this->EndPoint.PointId);
  sourceProxy->UpdateVTKObjects();

  //key line to tell the client to re-render the arc
  sourceProxy->MarkModified(NULL);

  if (!this->Internals->SubArcRepresentation)
    {
    this->Internals->SubArcRepresentation = builder->createDataRepresentation(
      this->Internals->SubArcSource->getOutputPort(0),
      this->View, "GeometryRepresentation");
    vtkSMProxy* repProxy = this->Internals->SubArcRepresentation->getProxy();
    vtkSMPropertyHelper(repProxy, "LineWidth").Set(2);
    vtkSMPropertyHelper(repProxy, "PointSize").Set(6.0);

    double rgb[3] ={0, 0.8, 0};
    vtkSMPropertyHelper(repProxy, "DiffuseColor").Set(rgb, 3);
    vtkSMPropertyHelper(repProxy, "AmbientColor").Set(rgb, 3);
    vtkSMPropertyHelper(repProxy, "Pickable").Set(0);
    }
  this->setSubArcVisible(visible);
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::setSubArcVisible(int visible)
{
  if(this->Internals->SubArcRepresentation)
    {
    this->Internals->SubArcRepresentation->setVisible(visible);
    this->Internals->SubArcRepresentation->getProxy()->UpdateVTKObjects();
    vtkSMRepresentationProxy::SafeDownCast(
      this->Internals->SubArcRepresentation->getProxy())->UpdatePipeline();
    this->View->forceRender();
    }
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::arcEditingFinished()
{
/* TO DO: We may allow this in the future.
  vtkContourWidget *widget = vtkContourWidget::SafeDownCast(
    this->SubWidget->getWidgetProxy()->GetWidget());
  vtkCMBArcWidgetRepresentation *widgetRep =
    vtkCMBArcWidgetRepresentation::SafeDownCast(
    widget->GetRepresentation());

  // if the end node of a closed-loop arc is in the middle of the
  // the specified sub-arc, the finished editing SubWidget has to
  // have at least one End node between Start and End points.

  if(this->Arc->isClosedLoop() &&
    this->StartPoint.PointId > this->EndPoint.PointId &&
    this->EndPoint.PointId > 0 &&
    widgetRep->GetNumberOfSelectedNodes()<=2)
    {
    qCritical() << "To replace the end node in a closed arc loop,\n"
      << "an explicit end node has to be set in the widget.\n"
      << "Use Ctrl+Click on a node to set it.";
    return;
    }
*/
  this->saveEdit();
  emit this->arcModificationfinished();
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::finishedArcModification()
{
  //marks that we are finished editing this arc
  //update the server and close the widget
  this->resetWidget();

  emit this->arcModificationfinished();
  this->close();
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::cancelEdit()
{
  //marks that that we don't want to save the modifications
  //to the arc
  this->hideArcWidget();

  //now show the pick widget
  this->resetWidget();
  emit this->arcModificationfinished();
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::saveEdit()
{
  // modify/replace the sub-arc with the arc from the arc widget
  emit this->arcModified(
    this->SubWidget, this->StartPoint.PointId, this->EndPoint.PointId);

  //hide the arc widget
  this->hideArcWidget();

  //now show the pick widget
  this->resetWidget();
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::onStraightenArc()
{
  if (!this->Internals->SubArcSource || !this->ArcManager)
    {
    qCritical() << "There was not valid sub-arc to do operations yet.\n";
    return;
    }
  this->ArcManager->straightenArc(this->StartPoint.PointId, this->EndPoint.PointId);
  this->resetWidget();
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::onCollapseSubArc()
{
  if (!this->Internals->SubArcSource || !this->ArcManager)
    {
    qCritical() << "There was not valid sub-arc to do operations yet.\n";
    return;
    }
  this->ArcManager->collapseSubArc(this->StartPoint.PointId, this->EndPoint.PointId);
  this->resetWidget();
}

//-----------------------------------------------------------------------------
void pqArcWidgetPanel::onMakeArc()
{
  if (!this->Internals->SubArcSource || !this->ArcManager)
    {
    qCritical() << "There was not valid sub-arc to do operations yet.\n";
    return;
    }
  this->ArcManager->makeArc(this->StartPoint.PointId, this->EndPoint.PointId);
  this->resetWidget();
}
