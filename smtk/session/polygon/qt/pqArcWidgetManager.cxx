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

#include "smtk/session/polygon/qt/pqArcWidgetManager.h"

#include "smtk/session/polygon/qt/pqArcWidgetPanel.h"
#include "smtk/session/polygon/qt/pqPolygonArc.h"
#include "smtk/session/polygon/qt/qtArcWidget.h"

#include "pqRenderView.h"
#include "pqServer.h"
#include "vtkAbstractWidget.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkNew.h"
#include "vtkProcessModule.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"

#include <QDebug>

pqArcWidgetManager::pqArcWidgetManager(pqServer* server, pqRenderView* view)
{
  //server and view need to be set before we call createContourWidget
  this->Server = server;
  this->View = view;
  this->ArcWidget = nullptr;

  this->EditWidget = nullptr;
  this->ActiveWidget = nullptr;
  this->EnableWidgetApplyButton = true;
}

pqArcWidgetManager::~pqArcWidgetManager()
{
  this->reset();
}

void pqArcWidgetManager::reset()
{
  this->Server = nullptr;
  this->setActiveArc(nullptr);
  if (this->ArcWidget)
  {
    //if a widget is deleted without having an active view it throws errors
    if (this->View && !this->ArcWidget->view())
    {
      this->ArcWidget->setView(this->View);
    }

    delete this->ArcWidget;
  }
  this->ArcWidget = nullptr;
  if (this->EditWidget)
  {
    delete this->EditWidget;
  }
  this->EditWidget = nullptr;
  this->View = nullptr;
}

pqPolygonArc* pqArcWidgetManager::activeArc()
{
  return this->Arc;
}
void pqArcWidgetManager::setActiveArc(pqPolygonArc* arc)
{
  if (this->Arc != arc)
  {
    if (this->Arc)
      delete this->Arc;
    this->Arc = arc;
  }
}

int pqArcWidgetManager::create()
{
  if (this->EditWidget && this->EditWidget->isVisible())
  {
    this->EditWidget->hideArcWidget();
    this->EditWidget->hide();
    this->ActiveWidget = nullptr;
  }

  Q_EMIT this->Busy();
  if (!this->Arc)
  {
    Q_EMIT this->Ready();
    return 0;
  }
  bool created = false;
  int normal;
  double planepos;
  if (!this->ArcWidget)
  {
    this->ArcWidget = this->createDefaultContourWidget(normal, planepos);
    QObject::connect(this->ArcWidget, SIGNAL(contourDone()), this, SLOT(createEdge()));
    created = true;
  }

  if (!created)
  {
    this->getDefaultArcPlane(normal, planepos);
    this->resetArcPlane(normal, planepos);
    this->ArcWidget->setView(this->View);
    this->ArcWidget->setEnableInteractivity(true);
  }

  // this->ArcWidget->select();
  this->Arc->setPlaneProjectionNormal(normal);
  this->Arc->setPlaneProjectionPosition(planepos);
  this->ArcWidget->enableApplyButton(this->EnableWidgetApplyButton);
  this->ActiveWidget = this->ArcWidget;
  return 1;
}

int pqArcWidgetManager::edit()
{
  delete this->ArcWidget;

  Q_EMIT this->Busy();
  if (!this->Arc)
  {
    Q_EMIT this->Ready();
    return 0;
  }
  if (!this->EditWidget)
  {
    this->EditWidget = new pqArcWidgetPanel();
    QObject::connect(
      this->EditWidget,
      SIGNAL(arcModified(qtArcWidget*, const smtk::common::UUID&)),
      this,
      SLOT(updateEdge(qtArcWidget*, const smtk::common::UUID&)));
    QObject::connect(
      this->EditWidget, SIGNAL(arcModificationfinished()), this, SLOT(editingFinished()));
    QObject::connect(this->EditWidget, SIGNAL(startArcEditing()), this, SIGNAL(editingStarted()));
    QObject::connect(this->EditWidget, SIGNAL(startArcPicking()), this, SIGNAL(startPicking()));
  }

  pqPolygonArc* arcObj = this->Arc;
  this->EditWidget->setView(this->View);
  this->EditWidget->setArc(arcObj);
  this->EditWidget->setArcManager(this);
  this->EditWidget->resetWidget();
  //this->EditWidget->show();
  this->ActiveWidget = this->EditWidget;
  return 1;
}

void pqArcWidgetManager::cancelOperation(const smtk::operation::OperationPtr& op)
{
  if (!this->Arc || this->Arc->edgeOperation() != op)
    return;

  if (this->EditWidget && this->EditWidget->isVisible())
  {
    this->EditWidget->cancelEdit();
    this->EditWidget->hide();
  }

  delete this->ArcWidget;
  this->ActiveWidget = nullptr;
  delete this->EditWidget;
  this->EditWidget = nullptr;
  Q_EMIT this->operationCancelled();
}

void pqArcWidgetManager::enableApplyButton(bool state)
{
  this->EnableWidgetApplyButton = state;
  if (this->ArcWidget)
  {
    this->ArcWidget->enableApplyButton(state);
  }
}

void pqArcWidgetManager::createEdge()
{
  if (!this->Arc)
  {
    return;
  }

  Q_ASSERT(this->ArcWidget);

  // push the polydata from the widget representation to the poly source

  pqPolygonArc* obj = this->Arc;
  if (obj)
  {
    vtkSMNewWidgetRepresentationProxy* widget = this->ArcWidget->widgetProxy();
    if (!obj->createEdge(widget))
    {
      qDebug() << "Can't create an edge with given widget!" << widget;
      return;
    }
  }

  //update the object
  this->disableArcWidget();
  this->ActiveWidget = nullptr;

  Q_EMIT this->Ready();
  Q_EMIT this->operationDone();
}

void pqArcWidgetManager::editingFinished()
{
  this->ActiveWidget = nullptr;
  Q_EMIT this->Ready();
  Q_EMIT this->operationDone();
}

void pqArcWidgetManager::updateEdge(qtArcWidget* subArcWidget, const smtk::common::UUID& edgeId)
{
  if ((!this->Arc) || this->ActiveWidget != this->EditWidget)
  {
    return;
  }

  //push the polydata from the widget representation to the poly source
  pqPolygonArc* obj = this->Arc;
  if (obj)
  {
    vtkSMNewWidgetRepresentationProxy* widget = subArcWidget->widgetProxy();
    obj->editEdge(widget, edgeId);
    /*
    //if the object hasn't been created yet update will call createArc
    //this way we don't have to check here
    QList<vtkIdType> newArcIds;
    vtkNew<vtkIdTypeArray> arcIdsFromSplit;

    //call the update arc operator
    vtkNew<vtkCMBSubArcModifyClientOperation> updateAndSplitOp;
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
      //convert this into a QList of vtkIdTypes so we can Q_EMIT to the tree
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
    Q_EMIT this->ArcSplit2(this->Arc,newArcIds);
*/
  }
}

qtArcWidget* pqArcWidgetManager::createDefaultContourWidget(int& normal, double& planePos)
{
  this->getDefaultArcPlane(normal, planePos);
  return this->createContourWidget(normal, planePos);
}

qtArcWidget* pqArcWidgetManager::createContourWidget(int normal, double position)
{
  qtArcWidget* widget = new qtArcWidget(nullptr);
  widget->setObjectName("smtkArcWidget");

  vtkSMProxy* pointplacer = widget->pointPlacer();
  vtkSMPropertyHelper(pointplacer, "ProjectionNormal").Set(normal);
  vtkSMPropertyHelper(pointplacer, "ProjectionPosition").Set(position);
  pointplacer->UpdateVTKObjects();

  //this block is needed to create the widget in the right order
  //we need to set on the proxy enabled, not the widget
  //than we need to call Initialize
  widget->setView(this->View);
  vtkSMPropertyHelper(widget->widgetProxy(), "AlwaysOnTop").Set(1);
  widget->widgetProxy()->UpdateVTKObjects();
  widget->setEnableInteractivity(true);
  widget->emphasize();
  return widget;
}

pqPolygonArc* pqArcWidgetManager::createLegacyV1Contour(
  const int& normal,
  const double& position,
  const int& closedLoop,
  vtkDoubleArray* nodePositions,
  vtkIdTypeArray* SelIndices)
{
  qtArcWidget* contourWidget = this->createContourWidget(normal, position);
  vtkSMNewWidgetRepresentationProxy* widgetProxy = contourWidget->widgetProxy();
  if (nodePositions && nodePositions->GetNumberOfTuples() > 0)
  {
    std::vector<double> points;
    for (vtkIdType i = 0; i < nodePositions->GetNumberOfTuples(); i++)
    {
      double pointPos[3];
      nodePositions->GetTuple(i, pointPos);
      points.push_back(pointPos[0]);
      points.push_back(pointPos[1]);
      points.push_back(pointPos[2]);
    }
    vtkSMPropertyHelper(widgetProxy, "NodePositions")
      .Set(points.data(), static_cast<unsigned int>(points.size()));
  }
  else
  {
    vtkSMPropertyHelper(widgetProxy, "NodePositions").SetNumberOfElements(0);
  }
  if (SelIndices && SelIndices->GetNumberOfTuples() > 0)
  {
    vtkSMPropertyHelper(widgetProxy, "SelectNodes")
      .Set(SelIndices->GetPointer(0), SelIndices->GetNumberOfTuples());
  }
  else
  {
    vtkSMPropertyHelper(widgetProxy, "SelectNodes").SetNumberOfElements(0);
  }

  //push all the node positions down to the server before
  //we call on close loop, or else close loop will fail
  widgetProxy->UpdateVTKObjects();

  if (closedLoop)
  {
    widgetProxy->InvokeCommand("CloseLoop");
  }

  pqPolygonArc* obj = new pqPolygonArc();
  obj->createEdge(widgetProxy);

  //obj->SetPlaneProjectionNormal(normal);
  //obj->SetPlaneProjectionPosition(position);

  //now we need to delete the widget
  delete contourWidget;

  return obj;
}

void pqArcWidgetManager::getDefaultArcPlane(int& orthoPlane, double& projpos)
{
  double focalPt[3], position[3], viewUp[3], viewDirection[3];
  double cameraDistance, parallelScale;
  (void)parallelScale;
  (void)viewUp;
  (void)position;
  (void)cameraDistance;
  (void)focalPt;

  vtkCamera* camera = this->View->getRenderViewProxy()->GetActiveCamera();
  Q_ASSERT(camera);
  camera->GetFocalPoint(focalPt);
  camera->GetDirectionOfProjection(viewDirection);

  /*
    pqCMBCommonMainWindowCore::getViewCameraInfo(
      this->View, focalPt, position, viewDirection, cameraDistance,
                        viewUp, parallelScale);
  */
  /// FIXME: why does this code assume we'll never get an non-axis aligned
  /// viewing direction?
  projpos = focalPt[2];
  orthoPlane = 2; // z axis
  if (viewDirection[0] < -.99 || viewDirection[0] > .99)
  {
    projpos = focalPt[0];
    orthoPlane = 0; // x axis
  }
  else if (viewDirection[1] < -.99 || viewDirection[1] > .99)
  {
    orthoPlane = 1; // y axis;
    projpos = focalPt[1];
  }
}

void pqArcWidgetManager::resetArcPlane(int normal, double planePos)
{
  if (vtkSMProxy* pointplacer = this->ArcWidget->pointPlacer())
  {
    vtkSMPropertyHelper(pointplacer, "ProjectionNormal").Set(normal);
    vtkSMPropertyHelper(pointplacer, "ProjectionPosition").Set(planePos);
    // pointplacer->MarkModified(pointplacer); why was this needed?
    pointplacer->UpdateVTKObjects();
  }
}

void pqArcWidgetManager::disableArcWidget()
{
  delete this->ArcWidget;
  // if(this->ArcWidget)
  //  {
  //  this->ArcWidget->setVisible(false);
  //  this->ArcWidget->reset();
  //  this->ArcWidget->removeAllNodes();
  //  this->ArcWidget->setWidgetVisible(false);
  //  this->ArcWidget->getWidgetProxy()->UpdatePropertyInformation();
  //  this->ArcWidget->setView(nullptr);
  //  }
}
