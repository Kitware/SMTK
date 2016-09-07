//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqArcWidgetManager
// .SECTION Description
//  Create and controls the arc editing singelton widget
// .SECTION Caveats

#include "smtk/bridge/polygon/qt/pqArcWidgetManager.h"

#include "smtk/bridge/polygon/qt/pqArcWidgetPanel.h"
#include "smtk/bridge/polygon/qt/pqPolygonArc.h"
#include "smtk/extension/paraview/widgets/pqArcWidget.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"

#include "vtkCommand.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkProcessModule.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMInputProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkAbstractWidget.h"
#include "vtkNew.h"
#include <QDebug>

//-----------------------------------------------------------------------------
pqArcWidgetManager::pqArcWidgetManager(pqServer *server, pqRenderView *view)
{
  //server and view need to be set before we call createContourWidget
  this->Server = server;
  this->View = view;
  this->ArcWidget = NULL;

  this->EditWidget = NULL;
  this->ActiveWidget = NULL;
}

//-----------------------------------------------------------------------------
pqArcWidgetManager::~pqArcWidgetManager()
{
  this->reset();
}

//-----------------------------------------------------------------------------
void pqArcWidgetManager::reset()
{
  this->Server = NULL;
  this->setActiveArc(NULL);
  if ( this->ArcWidget )
    {
    //if a widget is deleted without having an active view it throws errors
    if ( this->View && !this->ArcWidget->view() )
      {
      this->ArcWidget->setView(this->View);
      }

    delete this->ArcWidget;
    }
  this->ArcWidget = NULL;
  if ( this->EditWidget )
    {
    delete this->EditWidget;
    }
  this->EditWidget = NULL;
  this->View = NULL;
}

//-----------------------------------------------------------------------------
pqPolygonArc* pqArcWidgetManager::activeArc()
{
  return this->Arc;
}
void pqArcWidgetManager::setActiveArc(pqPolygonArc* arc)
{
  if(this->Arc != arc)
    {
    if(this->Arc)
      delete this->Arc;
    this->Arc = arc;
    }
}

//-----------------------------------------------------------------------------
int pqArcWidgetManager::create()
{
  if(this->EditWidget && this->EditWidget->isVisible())
    {
    this->EditWidget->hideArcWidget();
    this->EditWidget->hide();
    this->ActiveWidget = NULL;
    }

  emit this->Busy();
  if ( !this->Arc )
    {
    emit this->Ready();
    return 0;
    }
  bool created = false;
  int normal;
  double planepos;
  if ( !this->ArcWidget )
    {
    this->ArcWidget = this->createDefaultContourWidget(normal, planepos);
    QObject::connect(this->ArcWidget,SIGNAL(contourDone()),
      this,SLOT(createEdge()));
    created = true;
    }

  if ( !created )
    {
    this->ArcWidget->setView(this->View);
    this->getDefaultArcPlane(normal, planepos);
    this->resetArcPlane(normal, planepos);
    this->ArcWidget->setView(this->View);
    this->ArcWidget->setWidgetVisible(true);

    vtkSMPropertyHelper(this->ArcWidget->getWidgetProxy(), "Enabled").Set(1);
    this->ArcWidget->getWidgetProxy()->UpdateVTKObjects();
    this->ArcWidget->showWidget();
    }

  this->ArcWidget->select();
  this->Arc->setPlaneProjectionNormal(normal);
  this->Arc->setPlaneProjectionPosition(planepos);

  this->ActiveWidget = this->ArcWidget;
  return 1;
}

//-----------------------------------------------------------------------------
int pqArcWidgetManager::edit()
{
  if(this->ArcWidget && this->ArcWidget->isVisible())
    {
    this->disableArcWidget();
    this->ActiveWidget = NULL;
    }

  emit this->Busy();
  if ( !this->Arc )
    {
    emit this->Ready();
    return 0;
    }
  if(!this->EditWidget)
    {
    this->EditWidget = new pqArcWidgetPanel();
    QObject::connect(this->EditWidget,SIGNAL(
      arcModified(pqArcWidget*, const smtk::common::UUID&)),
      this,SLOT(updateEdge(pqArcWidget*, const smtk::common::UUID&)));
    QObject::connect(this->EditWidget,SIGNAL(arcModificationfinished()),
      this,SLOT(editingFinished()));
    QObject::connect(this->EditWidget,SIGNAL(startArcEditing()),
      this,SIGNAL(editingStarted()));
    QObject::connect(this->EditWidget,SIGNAL(startArcPicking()),
      this,SIGNAL(startPicking()));
    }

  pqPolygonArc* arcObj = this->Arc;
  this->EditWidget->setView(this->View);

  this->EditWidget->setArc(arcObj);
  this->EditWidget->setArcManager(this);
  this->EditWidget->resetWidget();
  this->EditWidget->show();
  this->ActiveWidget = this->EditWidget;

  return 1;
}

//----------------------------------------------------------------------------
void pqArcWidgetManager::cancelOperation(const smtk::model::OperatorPtr& op)
{
  if( !this->Arc || this->Arc->edgeOperator() != op)
    return;

  if(this->EditWidget && this->EditWidget->isVisible())
    {
    this->EditWidget->cancelEdit();
    this->EditWidget->hide();
    }

  if(this->ArcWidget && this->ArcWidget->isVisible())
    {
    this->disableArcWidget();
    }

  this->ActiveWidget = NULL;
  emit this->operationCancelled();
}

//-----------------------------------------------------------------------------
void pqArcWidgetManager::createEdge()
{
  if ( !this->Arc )
    {
    return;
    }

  //push the polydata from the widget representation to the poly source

  pqPolygonArc* obj = this->Arc;
  if ( obj )
    {
    vtkSMNewWidgetRepresentationProxy *widget = this->ArcWidget->getWidgetProxy();
    if(!obj->createEdge(widget))
      {
      qDebug() << "Can't create an edge with given widget!" << widget;
      return;
      }
    }

  //update the object
  this->disableArcWidget();
  this->ActiveWidget = NULL;

  emit this->Ready();
  emit this->operationDone();
}

//-----------------------------------------------------------------------------
void pqArcWidgetManager::editingFinished()
{
  this->ActiveWidget = NULL;
  emit this->Ready();
  emit this->operationDone();
}

//-----------------------------------------------------------------------------
void pqArcWidgetManager::updateEdge(
  pqArcWidget* subArcWidget, const smtk::common::UUID& edgeId)
{
  if ( (!this->Arc) || this->ActiveWidget != this->EditWidget)
    {
    return;
    }

  //push the polydata from the widget representation to the poly source
  pqPolygonArc* obj = this->Arc;
  if ( obj )
    {
    vtkSMNewWidgetRepresentationProxy *widget = subArcWidget->getWidgetProxy();
    obj->editEdge(widget, edgeId);
/*
    //if the object hasn't been created yet update will call createArc
    //this way we don't have to check here
    QList<vtkIdType> newArcIds;
    vtkNew<vtkIdTypeArray> arcIdsFromSplit;

    //call the update arc operator
    vtkNew<vtkCMBSubArcModifyClientOperator> updateAndSplitOp;
    updateAndSplitOp->SetStartPointId(startPID);
    updateAndSplitOp->SetEndPointId(endPID);
    bool valid = updateAndSplitOp->Operate(obj->getArcId(),widget,
      vtkSMSourceProxy::SafeDownCast(obj->getSource()->getProxy()));
    if (!valid)
      {
      //we didn't update ourselves, most likely bad widget representation
      //ie. a representation that has 1 or 0 points
      return;
      }

    //copy the arc ids to create new arcs for
    arcIdsFromSplit->DeepCopy(updateAndSplitOp->GetCreatedArcs());

    vtkIdType arcIdsSize = arcIdsFromSplit->GetNumberOfTuples();
    if(arcIdsSize > 0)
      {
      //convert this into a QList of vtkIdTypes so we can emit to the tree
      for(vtkIdType idx=0; idx < arcIdsSize; ++idx)
        {
        newArcIds.push_back(arcIdsFromSplit->GetValue(idx));
        }
      }

    //make sure the model rep is visible, it would be hidden if we can from edit mode
    pqDataRepresentation *modelRep = obj->getRepresentation();
    if(modelRep)
      {
      modelRep->setVisible(true);
      }

    //pass onto the scene tree that this scene polyline is finished being editing
    //it needs the signal so that the tree can split the arcset into arcs.
    //Also this is need to make all the arc representation rerender to fix
    //any old end nodes hanging around
    emit this->ArcSplit2(this->Arc,newArcIds);
*/
    }
}

//-----------------------------------------------------------------------------
pqArcWidget* pqArcWidgetManager::createDefaultContourWidget(
  int& normal, double& planePos)
{
  this->getDefaultArcPlane(normal, planePos);
  return this->createContourWidget(normal, planePos);
}

//-----------------------------------------------------------------------------
pqArcWidget* pqArcWidgetManager::createContourWidget(
   int normal, double position)
{
  vtkSMProxy* pointplacer = vtkSMProxyManager::GetProxyManager()->NewProxy(
    "point_placers", "BoundedPlanePointPlacer");

  pqArcWidget *widget= new pqArcWidget(
    pointplacer, pointplacer, NULL);

  widget->setObjectName("smtkArcWidget");

  vtkSMPropertyHelper(pointplacer, "ProjectionNormal").Set(normal);
  vtkSMPropertyHelper(pointplacer, "ProjectionPosition").Set(position);
  widget->setLineInterpolator(0);
  widget->setPointPlacer(pointplacer);
  pointplacer->UpdateVTKObjects();
  pointplacer->Delete();


  //this block is needed to create the widget in the right order
  //we need to set on the proxy enabled, not the widget
  //than we need to call Initialize
  widget->setView( this->View );
  widget->setWidgetVisible( this->View != NULL );

  vtkSMPropertyHelper(widget->getWidgetProxy(), "AlwaysOnTop").Set(1);
  vtkSMPropertyHelper(widget->getWidgetProxy(), "Enabled").Set(1);
  widget->getWidgetProxy()->UpdateVTKObjects();
  widget->showWidget();

  return widget;
}

//-----------------------------------------------------------------------------
pqPolygonArc* pqArcWidgetManager::createLegacyV1Contour(
  const int &normal,const double &position,const int &closedLoop,
  vtkDoubleArray* nodePositions, vtkIdTypeArray* SelIndices)
{

  pqArcWidget* contourWidget =
    this->createContourWidget(normal,position);

  vtkSMNewWidgetRepresentationProxy *widgetProxy =
    contourWidget->getWidgetProxy();

  if(nodePositions && nodePositions->GetNumberOfTuples() > 0)
    {
    QList<QVariant> values;
    double pointPos[3];
    for(vtkIdType i=0; i<nodePositions->GetNumberOfTuples(); i++)
      {
      nodePositions->GetTuple(i,pointPos);
      values << pointPos[0] << pointPos[1] << pointPos[2];
      }
    pqSMAdaptor::setMultipleElementProperty(
      widgetProxy->GetRepresentationProxy()->GetProperty("NodePositions"),
      values);
    }

  if ( SelIndices && SelIndices->GetNumberOfTuples() > 0 )
    {
    QList<QVariant> values;
    for(vtkIdType i=0; i<SelIndices->GetNumberOfTuples(); i++)
      {
      values << SelIndices->GetValue(i);
      }
    pqSMAdaptor::setMultipleElementProperty(
      widgetProxy->GetRepresentationProxy()->GetProperty("SelectNodes"),
      values);
    }

  //push all the node positions down to the server before
  //we call on close loop, or else close loop will fail
  widgetProxy->UpdateVTKObjects();

  if ( closedLoop )
    {
    widgetProxy->InvokeCommand("CloseLoop");
    }

  pqPolygonArc *obj = new pqPolygonArc();
  obj->createEdge(widgetProxy);

  //obj->SetPlaneProjectionNormal(normal);
  //obj->SetPlaneProjectionPosition(position);

  //now we need to delete the widget
  delete contourWidget;

  return obj;
}

//-----------------------------------------------------------------------------
void pqArcWidgetManager::getDefaultArcPlane(
  int& orthoPlane, double& projpos)
{
  double focalPt[3], position[3], viewUp[3], viewDirection[3];
  double cameraDistance, parallelScale;
/*
  pqCMBCommonMainWindowCore::getViewCameraInfo(
    this->View, focalPt, position, viewDirection, cameraDistance,
                      viewUp, parallelScale);
*/
  projpos = 0;
  QList<QVariant> values =
    pqSMAdaptor::getMultipleElementProperty(
    this->View->getProxy()->GetProperty("CameraFocalPointInfo"));
  projpos = values[2].toDouble();
  orthoPlane = 2; // z axis
  if (viewDirection[0] < -.99 || viewDirection[0] > .99)
    {
    projpos = values[0].toDouble();
    orthoPlane = 0; // x axis
    }
  else if (viewDirection[1] < -.99 || viewDirection[1] > .99)
    {
    orthoPlane = 1; // y axis;
    projpos = values[1].toDouble();
    }
}

//-----------------------------------------------------------------------------
void pqArcWidgetManager::resetArcPlane(
  int normal, double planePos)
{
  vtkSMProxyProperty* proxyProp =
    vtkSMProxyProperty::SafeDownCast(
    this->ArcWidget->getWidgetProxy()->GetProperty("PointPlacer"));
  if (proxyProp && proxyProp->GetNumberOfProxies())
    {
    vtkSMProxy* pointplacer = proxyProp->GetProxy(0);
    vtkSMPropertyHelper(pointplacer, "ProjectionNormal").Set(normal);
    vtkSMPropertyHelper(pointplacer, "ProjectionPosition").Set(planePos);
    pointplacer->MarkModified(pointplacer);
    pointplacer->UpdateVTKObjects();
    }
}

//-----------------------------------------------------------------------------
void pqArcWidgetManager::disableArcWidget()
{
  if(this->ArcWidget)
    {
    this->ArcWidget->setVisible(false);
    this->ArcWidget->reset();
    this->ArcWidget->removeAllNodes();
    this->ArcWidget->setWidgetVisible(false);
    this->ArcWidget->getWidgetProxy()->UpdatePropertyInformation();
    this->ArcWidget->setView(NULL);
    }
}
