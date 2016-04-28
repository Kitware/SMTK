//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/polygon/qt/pqPolygonArc.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/extension/vtk/widgets/vtkSMTKArcRepresentation.h"
#include "smtk/model/Operator.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"

#include "vtkCellArray.h"
#include "vtkCommand.h"
#include "vtkIdTypeArray.h"
#include "vtkProcessModule.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMOutputPort.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMSession.h"
#include "vtkNew.h"

#include "vtkClientServerStream.h"

//-----------------------------------------------------------------------------
pqPolygonArc::pqPolygonArc(QObject * prnt)
  : QObject(prnt)
{
  //for an arc the source is actually the arc provider not the arc itself
  this->Source = NULL;
//  this->ArcInfo = NULL;
  this->ArcId = -1;
  this->PlaneProjectionNormal = 2;
  this->PlaneProjectionPosition = 0;
  this->selColor[0]=this->selColor[2]=this->selColor[3]= 1.0;
  this->selColor[1]=0.0;
  this->origColor[0]=this->origColor[1]=this->origColor[2]=this->origColor[3]= 1.0;
}

//-----------------------------------------------------------------------------
pqPolygonArc::~pqPolygonArc()
{
/*  if(this->ArcInfo)
    {
    this->ArcInfo->Delete();
    }

  //delete the arc from the server by calling the delete operator
  if (this->ArcId != -1)
    {
    vtkNew<vtkCMBArcDeleteClientOperator> delOp;
    delOp->DeleteArc(this->ArcId);
    }

  //remove ourselves from each polygon
  std::set<pqCMBPolygon*>::iterator it;
  for (it=this->PolygonsUsingArc.begin();
       it!=this->PolygonsUsingArc.end();
       ++it)
    {
    (*it)->removeArc(this);
    }
*/
}

void pqPolygonArc::setEdgeOperator(smtk::model::OperatorPtr edgeOp)
{
  this->m_edgeOp = edgeOp;
}
smtk::shared_ptr<smtk::model::Operator> pqPolygonArc::edgeOperator()
{ 
  return this->m_edgeOp.lock();
}

//-----------------------------------------------------------------------------
bool pqPolygonArc::createEdge(vtkSMNewWidgetRepresentationProxy *widgetProxy)
{
  if(!widgetProxy || !this->edgeOperator())
    return false;
  smtk::attribute::AttributePtr spec = this->edgeOperator()->specification();
  if(spec->type() != "edit edge")
    return false;
  smtk::attribute::IntItem::Ptr opProxyIdItem =
    this->edgeOperator()->specification()->findInt("HelperGlobalID");
  if(!opProxyIdItem)
    return false;

  vtkSMProxy* smPolyEdgeOp = vtkSMProxyManager::GetProxyManager()->NewProxy(
    "polygon_operators", "PolygonArcOperator");
  if(!smPolyEdgeOp)
    return false;
  smPolyEdgeOp->UpdateVTKObjects();

  // create the vtkPolygonArcOperator proxy, and set its ArcSource with
  // the coutour representation.
  vtkSMProxy* repProxy = widgetProxy->GetRepresentationProxy();
  repProxy->UpdateVTKObjects();
  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
          << VTKOBJECT(repProxy) << "GetContourRepresentationAsPolyData"
          << vtkClientServerStream::End
          << vtkClientServerStream::Invoke
          << VTKOBJECT(smPolyEdgeOp) << "SetArcSource"
          << vtkClientServerStream::LastResult
          << vtkClientServerStream::End;
  smPolyEdgeOp->GetSession()->ExecuteStream(smPolyEdgeOp->GetLocation(), stream);
  // Now set the GlobalId of smPolyEdgeOp proxy to the edge op, and later
  // on the GlobalId will be used to find the proxy
    // for Create and Edit operation, we need arc source
  opProxyIdItem->setValue(smPolyEdgeOp->GetGlobalID());
  emit this->operationRequested(this->edgeOperator());
  return true;
}

//-----------------------------------------------------------------------------
bool pqPolygonArc::editArc(vtkSMNewWidgetRepresentationProxy *widget)
{
  if (this->ArcId == -1)
    {
    return false;
    }
/*
  //for now only works in built in mode
  vtkNew<vtkCMBArcEditClientOperator> editOp;
  editOp->SetArcIsClosed(this->isClosedLoop());
  return editOp->Operate(this->Source->getProxy(),widget);
*/
  return false;
}

//-----------------------------------------------------------------------------
bool pqPolygonArc::findPickPoint(vtkSMOutputPort* port)
{
/*
  if (this->ArcId == -1)
    {
    return false;
    }

  //for now only works in built in mode
  vtkNew<vtkCMBArcFindPickPointOperator> pickOp;
  return pickOp->Operate(this->ArcId,port);
*/
  return false;
}
//-----------------------------------------------------------------------------
bool pqPolygonArc::updateArc(vtkSMNewWidgetRepresentationProxy *widget,
                            vtkIdTypeArray *newlyCreatedArcIds)
{
/*
  if (this->ArcId == -1)
    {
    bool valid = this->createArc(widget);
    if (!valid)
      {
      return false;
      }
    }

  if ( newlyCreatedArcIds == NULL)
    {
    //this needs to be created before being passed in
    return false;
    }

  //call the update arc operator
  vtkNew<vtkCMBArcUpdateAndSplitClientOperator> updateAndSplitOp;
  bool valid = updateAndSplitOp->Operate(this->ArcId,widget);
  if (!valid)
    {
    //we didn't update ourselves, most likely bad widget representation
    //ie. a representation that has 1 or 0 points
    return false;
    }

  //copy the arc ids to create new arcs for
  newlyCreatedArcIds->DeepCopy(updateAndSplitOp->GetCreatedArcs());

  //update the rep
  this->updateRepresentation();

  //update the plane normal and position
  this->updatePlaneProjectionInfo(widget);
*/

  return true;
}

//-----------------------------------------------------------------------------
vtkIdType pqPolygonArc::autoConnect(const vtkIdType& secondArcId)
{
  if(this->ArcId == -1 || secondArcId == -1 || this->ArcId == secondArcId)
    {
    return -1;
    }
/*
  vtkNew<vtkCMBArcAutoConnectClientOperator> autoConnectOp;
  bool valid = autoConnectOp->Operate(this->ArcId,secondArcId);
  if(valid)
    {
    return autoConnectOp->GetArcId();
    }
*/
  return -1;
}
/*
//-----------------------------------------------------------------------------
vtkPVArcInfo* pqPolygonArc::getArcInfo()
{
  if (!this->Source)
    {
    return NULL;
    }
  if ( !this->ArcInfo )
    {
    this->ArcInfo = vtkPVArcInfo::New();
    }
  this->ArcInfo->SetGatherAllInfo();

  //collect the information from the server poly source into
  //the representation info.
  vtkSMProxy *proxy = this->Source->getProxy();

  proxy->GatherInformation(this->ArcInfo);
  return this->ArcInfo;
}
*/

//-----------------------------------------------------------------------------
bool pqPolygonArc::isClosedLoop()
{
/*
  //we need to query the server for this information. The reason is
  //that another arc could have moved an end node causing this to go from
  //unclosed to being closed.
  if ( !this->ArcInfo )
    {
    this->ArcInfo = vtkPVArcInfo::New();
    }

  if (!this->Source)
    {
    //we have no arc this is an invalid call
    return false;
    }

  this->ArcInfo->SetGatherLoopInfoOnly();

   //collect the information from the server poly source into
  //the representation info.
  this->Source->getProxy()->GatherInformation(this->ArcInfo);

  return this->ArcInfo->IsClosedLoop();
*/
  return false;
}
//-----------------------------------------------------------------------------
int pqPolygonArc::getClosedLoop()
{
  //we need to query the server for this information. The reason is
  //that another arc could have moved an end node causing this to go from
  //unclosed to being closed.
  return this->isClosedLoop()? 1 : 0;
}


//-----------------------------------------------------------------------------
void pqPolygonArc::inheritPolygonRelationships(pqPolygonArc *parent)
{
/*
  //make us have the same polygons
  this->PolygonsUsingArc = parent->PolygonsUsingArc;
  std::set<pqCMBPolygon*>::iterator it;
  for (it=this->PolygonsUsingArc.begin();
       it!=this->PolygonsUsingArc.end();
       ++it)
    {
    (*it)->addArc(this);
    }
*/
}

//-----------------------------------------------------------------------------
void pqPolygonArc::setSelectionInput(vtkSMSourceProxy *selectionInput)
{
  if(!selectionInput)
    {
    this->deselect();
    }
  else
    {
    this->select();
    }
  //this->Superclass::setSelectionInput(selectionInput);
}

//-----------------------------------------------------------------------------
void pqPolygonArc::select()
{
  //this->Superclass::setColor(this->selColor);
}

//-----------------------------------------------------------------------------

void pqPolygonArc::deselect()
{
  //this->Superclass::setColor(this->origColor);
}

//-----------------------------------------------------------------------------
void pqPolygonArc::getColor(double color[4]) const
{
  for(int i=0; i<4; i++)
    {
    color[i] = this->origColor[i];
    }
}

//-----------------------------------------------------------------------------
void pqPolygonArc::setColor(double color[4], bool updateRep)
{
  for(int i=0; i<4; i++)
    {
    this->origColor[i] = color[i];
    }
  //this->Superclass::setColor(color, updateRep);
}

//-----------------------------------------------------------------------------
void pqPolygonArc::setMarkedForDeletion()
{
  if (!this->Source)
    {
    return;
    }
/*
  vtkNew<vtkCMBArcDeleteClientOperator> undoOp;
  undoOp->SetMarkedForDeletion(this->ArcId);

  this->Superclass::setMarkedForDeletion();
*/
}

//-----------------------------------------------------------------------------
void pqPolygonArc::unsetMarkedForDeletion()
{
  if (!this->Source)
    {
    return;
    }
/*
  vtkNew<vtkCMBArcDeleteClientOperator> undoOp;
  undoOp->SetUnMarkedForDeletion(this->ArcId);

  this->Superclass::unsetMarkedForDeletion();
*/
}

//-----------------------------------------------------------------------------
void pqPolygonArc::arcIsModified()
{
/*
  std::set<pqCMBPolygon*>::iterator it;
  for (it=this->PolygonsUsingArc.begin();
       it!=this->PolygonsUsingArc.end();
       ++it)
    {
    (*it)->arcIsDirty(this);
    }
*/
}

//-----------------------------------------------------------------------------
void pqPolygonArc::setRepresentation(pqDataRepresentation *rep)
{
/*
  pqCMBSceneObjectBase::setRepresentation(rep);

  // For now lets turn off LOD
  this->setLODMode(1);
*/
  //set the original color
  vtkSMProxy* reprProxy = rep->getProxy();
  vtkSMPropertyHelper(reprProxy,"DiffuseColor").Get(this->origColor, 3);
  this->origColor[3] = vtkSMPropertyHelper(reprProxy, "Opacity").GetAsDouble();
}


//-----------------------------------------------------------------------------
void pqPolygonArc::updatePlaneProjectionInfo(vtkSMNewWidgetRepresentationProxy *widget)
{
  vtkSMProxy* repProxy = widget->GetRepresentationProxy();
  vtkSMProxyProperty* proxyProp =
    vtkSMProxyProperty::SafeDownCast(
    repProxy->GetProperty("PointPlacer"));
  if (proxyProp && proxyProp->GetNumberOfProxies())
    {
    vtkSMProxy* pointplacer = proxyProp->GetProxy(0);
    this->PlaneProjectionNormal = pqSMAdaptor::getElementProperty(
      pointplacer->GetProperty("ProjectionNormal")).toInt();
    this->PlaneProjectionPosition = pqSMAdaptor::getElementProperty(
      pointplacer->GetProperty("ProjectionPosition")).toDouble();
    }
}
