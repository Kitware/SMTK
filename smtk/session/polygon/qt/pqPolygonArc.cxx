//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/polygon/qt/pqPolygonArc.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/extension/vtk/widgets/vtkSMTKArcRepresentation.h"
#include "smtk/model/Edge.h"
#include "smtk/model/IntegerData.h"
#include "smtk/model/Model.h"
#include "smtk/operation/Operation.h"
#include "smtk/session/polygon/qt/vtkPolygonArcInfo.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"

#include "vtkCellArray.h"
#include "vtkCommand.h"
#include "vtkIdTypeArray.h"
#include "vtkNew.h"
#include "vtkProcessModule.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMOutputPort.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSession.h"
#include "vtkSMSourceProxy.h"

#include "vtkClientServerStream.h"
#include <QDebug>

pqPolygonArc::pqPolygonArc(QObject* prnt)
  : QObject(prnt)
{
  //for an arc the source is actually the arc provider not the arc itself
  this->Source = nullptr;
  this->ArcInfo = nullptr;
  m_currentModelId = smtk::common::UUID::null();
  this->PlaneProjectionNormal = 2;
  this->PlaneProjectionPosition = 0;
  this->selColor[0] = this->selColor[2] = this->selColor[3] = 1.0;
  this->selColor[1] = 0.0;
  this->origColor[0] = this->origColor[1] = this->origColor[2] = this->origColor[3] = 1.0;
}

pqPolygonArc::~pqPolygonArc()
{
  if (this->ArcInfo)
  {
    this->ArcInfo->Delete();
  }
}

void pqPolygonArc::setEdgeOperation(smtk::operation::OperationPtr edgeOp)
{
  m_edgeOp = edgeOp;
}
smtk::shared_ptr<smtk::operation::Operation> pqPolygonArc::edgeOperation()
{
  return m_edgeOp.lock();
}

inline vtkSMProxy* internal_createVTKEdgeOperation(vtkSMNewWidgetRepresentationProxy* widgetProxy)
{
  vtkSMProxy* smPolyEdgeOp =
    vtkSMProxyManager::GetProxyManager()->NewProxy("polygon_operators", "PolygonArcOperation");
  if (!smPolyEdgeOp)
    return nullptr;
  smPolyEdgeOp->UpdateVTKObjects();

  // create the vtkPolygonArcOperation proxy, and set its ArcRepresentation with
  // the coutour representation.
  vtkSMProxy* repProxy = widgetProxy->GetRepresentationProxy();
  repProxy->UpdateVTKObjects();
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(smPolyEdgeOp) << "SetArcRepresentation"
         << VTKOBJECT(repProxy) << vtkClientServerStream::End;
  smPolyEdgeOp->GetSession()->ExecuteStream(smPolyEdgeOp->GetLocation(), stream);
  return smPolyEdgeOp;
}

vtkSMProxy* pqPolygonArc::prepareOperation(vtkSMNewWidgetRepresentationProxy* widgetProxy)
{
  if (!widgetProxy || !this->edgeOperation())
    return nullptr;
  smtk::attribute::AttributePtr spec = this->edgeOperation()->parameters();
  if (spec->type() != "tweak edge" && spec->type() != "create edge")
    return nullptr;
  smtk::attribute::IntItem::Ptr opProxyIdItem = spec->findInt("HelperGlobalID");
  if (!opProxyIdItem)
    return nullptr;
  vtkSMProxy* smPolyEdgeOp = internal_createVTKEdgeOperation(widgetProxy);
  if (!smPolyEdgeOp)
    return nullptr;
  // Now set the GlobalId of smPolyEdgeOp proxy to the edge op, and later
  // on the GlobalId will be used to find the proxy
  // for Create and Edit operation, we need arc source
  opProxyIdItem->setValue(smPolyEdgeOp->GetGlobalID());
  return smPolyEdgeOp;
}

bool pqPolygonArc::createEdge(vtkSMNewWidgetRepresentationProxy* widgetProxy)
{
  vtkSMProxy* smPolyEdgeOp = this->prepareOperation(widgetProxy);
  if (!smPolyEdgeOp)
    return false;
  Q_EMIT this->operationRequested(this->edgeOperation());
  smPolyEdgeOp->Delete();
  return true;
}

bool pqPolygonArc::editEdge(
  vtkSMNewWidgetRepresentationProxy* widgetProxy,
  const smtk::common::UUID& edgeId)
{
  (void)edgeId;
  vtkSMProxy* smPolyEdgeOp = this->prepareOperation(widgetProxy);
  if (!smPolyEdgeOp)
  {
    return false;
  }

  smtk::attribute::AttributePtr opSpec = this->edgeOperation()->parameters();
  // TODO: cannot access the manager through the operator (unless its a parameter...)
  // smtk::model::Edge edge(this->edgeOperation()->manager(), edgeId);
  // if (!edge.isValid())
  // {
  //   return false;
  // }
  // if (!opSpec->isEntityAssociated(edge))
  // {
  //   opSpec->removeAllAssociations();
  //   opSpec->associateEntity(edge);
  // }

  // Q_EMIT this->operationRequested(this->edgeOperation());
  smPolyEdgeOp->Delete();
  return true;
}

bool pqPolygonArc::updateArc(
  vtkSMNewWidgetRepresentationProxy* widget,
  vtkIdTypeArray* newlyCreatedArcIds)
{
  (void)widget;
  (void)newlyCreatedArcIds;
  /*
  if (this->ArcId == -1)
    {
    bool valid = this->createArc(widget);
    if (!valid)
      {
      return false;
      }
    }

  if ( newlyCreatedArcIds == nullptr)
    {
    //this needs to be created before being passed in
    return false;
    }

  //call the update arc operator
  vtkNew<vtkCMBArcUpdateAndSplitClientOperation> updateAndSplitOp;
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

vtkIdType pqPolygonArc::autoConnect(const vtkIdType& secondArcId)
{
  (void)secondArcId;
  /*
  vtkNew<vtkCMBArcAutoConnectClientOperation> autoConnectOp;
  bool valid = autoConnectOp->Operate(this->ArcId,secondArcId);
  if(valid)
    {
    return autoConnectOp->GetArcId();
    }
*/
  return -1;
}

vtkPolygonArcInfo* pqPolygonArc::getArcInfo(int blockIndex)
{
  if (blockIndex < 0)
  {
    qCritical() << "The render view is in use with another selection. Stop that selection first.\n";
  }
  bool newInfo = false;
  if (!this->ArcInfo)
  {
    this->ArcInfo = vtkPolygonArcInfo::New();
    newInfo = true;
  }

  if (this->Source && blockIndex >= 0 && (newInfo || blockIndex != this->ArcInfo->GetBlockIndex()))
  {
    //collect the information from the server poly source into
    //the representation info.
    vtkSMProxy* proxy = this->Source->getProxy();
    this->ArcInfo->SetBlockIndex(blockIndex);
    proxy->GatherInformation(this->ArcInfo);
  }

  return this->ArcInfo;
}

void pqPolygonArc::resetOperationSource()
{
  // need to reset
  this->Source = nullptr;
  m_currentModelId = smtk::common::UUID::null();
  // the Source should always reference to the source for the referenced model,
  // which should be activated by emitting activateModel()
  if (m_edgeOp.lock() && m_edgeOp.lock()->parameters())
  {
    smtk::model::EntityRef entref =
      m_edgeOp.lock()->parameters()->associations()->valueAs<smtk::model::Entity>();
    if (!entref.isValid())
    {
      return;
    }
    smtk::model::Model model;
    if (entref.isModel()) // "create edge"
    {
      model = entref.as<smtk::model::Model>();
    }
    else if (entref.isEdge()) // "tweak edge"
    {
      model = entref.as<smtk::model::Edge>().owningModel();
    }
    if (
      model.isValid() && m_currentModelId == model.entity() && this->Source &&
      this->Source == pqActiveObjects::instance().activeSource())
    {
      // nothing to do
      return;
    }
    if (model.isValid())
    {
      m_currentModelId = model.entity();
      Q_EMIT this->activateModel(model.entity());
      this->Source = pqActiveObjects::instance().activeSource();
      int blockIndex = this->getAssignedEdgeBlock();
      if (blockIndex < 0)
      {
        qCritical() << "Invalid block index for the edge.\n";
      }
      else
      {
        this->getArcInfo(blockIndex);
      }
    }
  }
}

pqPipelineSource* pqPolygonArc::getSource()
{
  return this->Source;
}

void pqPolygonArc::setSource(pqPipelineSource* modelSource)
{
  this->Source = modelSource;
  pqActiveObjects::instance().setActiveSource(modelSource);
}

int pqPolygonArc::getAssignedEdgeBlock() const
{
  if (m_edgeOp.lock() && m_edgeOp.lock()->parameters())
  {
    // for Destroy and Modify operation, we need edge is set
    smtk::model::EntityRef entref =
      m_edgeOp.lock()->parameters()->associations()->valueAs<smtk::model::Entity>();
    smtk::model::Edge edge;
    if (entref.isModel()) // "create edge"
    {
      edge = entref.as<smtk::model::Edge>();
    }
    else if (entref.isEdge()) // "tweak edge"
    {
      edge = entref.as<smtk::model::Edge>();
    }
    if (edge.isValid())
    {
      const smtk::model::IntegerList& prop(edge.integerProperty("block_index"));
      if (!prop.empty())
      {
        return prop[0];
      }
    }
  }
  return -1;
}

bool pqPolygonArc::isClosedLoop()
{
  /*
  //we need to query the server for this information. The reason is
  //that another arc could have moved an end node causing this to go from
  //unclosed to being closed.
  if ( !this->ArcInfo )
    {
    this->ArcInfo = vtkPolygonArcInfo::New();
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

int pqPolygonArc::getClosedLoop()
{
  //we need to query the server for this information. The reason is
  //that another arc could have moved an end node causing this to go from
  //unclosed to being closed.
  return this->isClosedLoop() ? 1 : 0;
}

void pqPolygonArc::inheritPolygonRelationships(pqPolygonArc* parent)
{
  (void)parent;
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

void pqPolygonArc::setSelectionInput(vtkSMSourceProxy* selectionInput)
{
  if (!selectionInput)
  {
    this->deselect();
  }
  else
  {
    this->select();
  }
  //this->Superclass::setSelectionInput(selectionInput);
}

void pqPolygonArc::select()
{
  //this->Superclass::setColor(this->selColor);
}

void pqPolygonArc::deselect()
{
  //this->Superclass::setColor(this->origColor);
}

void pqPolygonArc::getColor(double color[4]) const
{
  for (int i = 0; i < 4; i++)
  {
    color[i] = this->origColor[i];
  }
}

void pqPolygonArc::setColor(double color[4], bool updateRep)
{
  (void)updateRep;
  for (int i = 0; i < 4; i++)
  {
    this->origColor[i] = color[i];
  }
  //this->Superclass::setColor(color, updateRep);
}

void pqPolygonArc::setMarkedForDeletion()
{
  if (!this->Source)
  {
    return;
  }
  /*
  vtkNew<vtkCMBArcDeleteClientOperation> undoOp;
  undoOp->SetMarkedForDeletion(this->ArcId);

  this->Superclass::setMarkedForDeletion();
*/
}

void pqPolygonArc::unsetMarkedForDeletion()
{
  if (!this->Source)
  {
    return;
  }
  /*
  vtkNew<vtkCMBArcDeleteClientOperation> undoOp;
  undoOp->SetUnMarkedForDeletion(this->ArcId);

  this->Superclass::unsetMarkedForDeletion();
*/
}

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

void pqPolygonArc::setRepresentation(pqDataRepresentation* rep)
{
  /*
  pqCMBSceneObjectBase::setRepresentation(rep);

  // For now lets turn off LOD
  this->setLODMode(1);
*/
  //set the original color
  vtkSMProxy* reprProxy = rep->getProxy();
  vtkSMPropertyHelper(reprProxy, "DiffuseColor").Get(this->origColor, 3);
  this->origColor[3] = vtkSMPropertyHelper(reprProxy, "Opacity").GetAsDouble();
}

void pqPolygonArc::updatePlaneProjectionInfo(vtkSMNewWidgetRepresentationProxy* widget)
{
  vtkSMProxy* repProxy = widget->GetRepresentationProxy();
  vtkSMProxyProperty* proxyProp =
    vtkSMProxyProperty::SafeDownCast(repProxy->GetProperty("PointPlacer"));
  if (proxyProp && proxyProp->GetNumberOfProxies())
  {
    vtkSMProxy* pointplacer = proxyProp->GetProxy(0);
    this->PlaneProjectionNormal =
      pqSMAdaptor::getElementProperty(pointplacer->GetProperty("ProjectionNormal")).toInt();
    this->PlaneProjectionPosition =
      pqSMAdaptor::getElementProperty(pointplacer->GetProperty("ProjectionPosition")).toDouble();
  }
}
