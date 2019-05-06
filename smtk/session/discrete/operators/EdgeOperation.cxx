//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "EdgeOperation.h"

#include "smtk/session/discrete/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/utility/Reclassify.h"

#include "smtk/model/Edge.h"
#include "smtk/model/Face.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/Volume.h"

#include "smtk/session/discrete/kernel/Model/vtkModel.h"
#include "smtk/session/discrete/kernel/Model/vtkModelFace.h"
#include "smtk/session/discrete/kernel/Model/vtkModelItemIterator.h"
#include "smtk/session/discrete/kernel/Model/vtkModelRegion.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkDiscreteModelWrapper.h"

#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkPolyData.h"
#include "vtkSelectionNode.h"
#include "vtkSelectionSource.h"

#include "EdgeOperation_xml.h"

using namespace smtk::model;
using namespace smtk::attribute;

namespace smtk
{
namespace session
{

namespace discrete
{

inline std::vector<smtk::mesh::ResourcePtr> internal_associatedMeshResources(
  const smtk::attribute::ResourceItem::Ptr& associatedMeshResourcesItem,
  const smtk::model::EntityRef& eref)
{
  std::vector<smtk::mesh::ResourcePtr> meshResources;
  auto resources = eref.resource()->links().linkedFrom(smtk::mesh::Resource::ClassificationRole);
  for (std::size_t i = 0; i < associatedMeshResourcesItem->numberOfValues(); i++)
  {
    resources.insert(associatedMeshResourcesItem->value(i));
  }
  for (auto resource : resources)
  {
    auto meshResource = std::dynamic_pointer_cast<smtk::mesh::Resource>(resource);
    if (meshResource != nullptr)
    {
      bool found = eref.isModel() ? meshResource->associatedModel() == eref.entity()
                                  : !(meshResource->findAssociatedMeshes(eref).is_empty());
      if (found)
      {
        meshResources.push_back(meshResource);
      }
    }
  }

  return meshResources;
}

inline bool internal_mergeAssociatedMeshes(const smtk::model::Vertex& remVert,
  const smtk::model::Edge& toRemove, const smtk::model::Edge& toAddTo,
  const smtk::attribute::ResourceItem::Ptr& associatedMeshResourcesItem,
  smtk::mesh::MeshSets& modifiedMeshes)
{
  bool ok = true;
  std::vector<smtk::mesh::ResourcePtr> meshResources =
    internal_associatedMeshResources(associatedMeshResourcesItem, toAddTo);
  std::vector<smtk::mesh::ResourcePtr>::iterator it;
  for (it = meshResources.begin(); it != meshResources.end(); ++it)
  {
    ok &= smtk::mesh::utility::merge(*it, remVert, toRemove, toAddTo);
    if (ok)
    {
      smtk::mesh::MeshSet toMeshes = (*it)->findAssociatedMeshes(toAddTo);
      if (!toMeshes.is_empty())
        modifiedMeshes.insert(toMeshes);
    }
  }
  return ok;
}

bool internal_splitAssociatedMeshes(const smtk::model::Edge& srcEdge,
  const smtk::model::Edge& newEdge, const smtk::model::Vertex& newVert,
  const smtk::attribute::ResourceItem::Ptr& associatedMeshResourcesItem,
  smtk::mesh::MeshSets& modifiedMeshes)
{
  bool ok = true;
  std::vector<smtk::mesh::ResourcePtr> meshResources =
    internal_associatedMeshResources(associatedMeshResourcesItem, srcEdge);
  std::vector<smtk::mesh::ResourcePtr>::iterator it;
  for (it = meshResources.begin(); it != meshResources.end(); ++it)
  {
    ok &= smtk::mesh::utility::split(*it, srcEdge, newEdge, newVert);
    if (ok)
    {
      smtk::mesh::MeshSet srcMeshes = (*it)->findAssociatedMeshes(srcEdge);
      if (!srcMeshes.is_empty())
        modifiedMeshes.insert(srcMeshes);
    }
  }
  return ok;
}

EdgeOperation::EdgeOperation()
{
}

bool EdgeOperation::ableToOperate()
{
  smtk::model::Model model = this->parameters()->associations()->valueAs<smtk::model::Entity>();

  // The SMTK model must be valid
  if (!model.isValid())
  {
    return false;
  }

  smtk::session::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::discrete::Resource>(model.component()->resource());

  // The CMB model must exist:
  if (!resource->discreteSession()->findModelEntity(model.entity()))
  {
    return false;
  }

  // There must be a MeshEntity for input selection
  if (!this->parameters()->findMeshSelection("selection"))
  {
    return false;
  }

  return true;
}

int EdgeOperation::convertToGlobalPointId(smtk::session::discrete::Resource::Ptr& resource,
  int localPid, vtkDiscreteModelEdge* cmbModelEdge)
{
  int globalPid = -1;
  SessionPtr opsession = resource->discreteSession();
  smtk::common::UUID edgeid = opsession->findOrSetEntityUUID(cmbModelEdge);
  smtk::model::Edge edge(opsession->resource(), edgeid);

  vtkPolyData* edgePoly = vtkPolyData::SafeDownCast(cmbModelEdge->GetGeometry());
  const smtk::model::Tessellation* tess;
  if (!edgePoly || !(tess = edge.hasTessellation()))
  { // Oops.
    return globalPid;
  }

  smtk::model::Tessellation::size_type off;
  int cellIdx = -1, pointIdx = -1;
  std::vector<int>::const_iterator it;
  for (off = tess->begin(); off != tess->end(); off = tess->nextCellOffset(off))
  {
    cellIdx++;
    std::vector<int> cell_conn;
    tess->vertexIdsOfCell(off, cell_conn);
    it = std::find(cell_conn.begin(), cell_conn.end(), localPid);
    if (it != cell_conn.end())
    {
      pointIdx = it - cell_conn.begin();
      break;
    }
  }
  if (pointIdx >= 0 && cellIdx < edgePoly->GetNumberOfCells())
  {
    vtkIdType npts, *cellPnts;
    edgePoly->GetCellPoints(cellIdx, npts, cellPnts);
    if (pointIdx < npts)
      globalPid = cellPnts[pointIdx];
  }
  return globalPid;
}

void EdgeOperation::getSelectedVertsAndEdges(smtk::session::discrete::Resource::Ptr& resource,
  std::map<smtk::common::UUID, vtkDiscreteModelVertex*>& selVTXs,
  std::map<smtk::common::UUID, std::pair<vtkDiscreteModelEdge*, std::set<int> > >& selArcs,
  const smtk::attribute::MeshSelectionItemPtr& inSelectionItem,
  smtk::session::discrete::SessionPtr opsession)
{
  smtk::attribute::MeshSelectionItem::const_sel_map_it mapIt;
  for (mapIt = inSelectionItem->begin(); mapIt != inSelectionItem->end(); ++mapIt)
  {
    vtkModelItem* mitem = opsession->entityForUUID(mapIt->first);
    if (!mitem)
      continue;
    if (mitem->GetType() == vtkModelVertexType)
    {
      selVTXs.insert(std::make_pair(mapIt->first, vtkDiscreteModelVertex::SafeDownCast(mitem)));
    }
    else if (mitem->GetType() == vtkModelEdgeType)
    {
      // Since vtkHardwareSelector's point picking is pixel based, and if we have a coincidental point
      // used by a vertex cell and a line cell, the pick result will depend on the rendering order of
      // the vertex and the line. To make thing more tricky, the vertex and line belong to different
      // block in the multiblock dataset, so the render order of the blocks also comes into play.
      // So we are doing checking ourselves if the picked point on the edge is actually a vertex.
      vtkDiscreteModelEdge* selEdge = vtkDiscreteModelEdge::SafeDownCast(mitem);
      vtkDiscreteModelVertex* cmbModelVertex1 =
        vtkDiscreteModelVertex::SafeDownCast(selEdge->GetAdjacentModelVertex(0));
      vtkDiscreteModelVertex* cmbModelVertex2 =
        vtkDiscreteModelVertex::SafeDownCast(selEdge->GetAdjacentModelVertex(1));

      std::set<int> seledgepts;

      for (std::set<int>::const_iterator sit = mapIt->second.begin(); sit != mapIt->second.end();
           ++sit)
      {
        vtkIdType pId = this->convertToGlobalPointId(resource, *sit, selEdge);
        if (pId < 0)
        {
          continue;
        }
        else if (cmbModelVertex1 && cmbModelVertex1->GetPointId() == pId)
        {
          smtk::common::UUID vid = opsession->findOrSetEntityUUID(cmbModelVertex1);
          selVTXs.insert(std::make_pair(vid, cmbModelVertex1));
        }
        else if (cmbModelVertex2 && cmbModelVertex2->GetPointId() == pId)
        {
          smtk::common::UUID vid = opsession->findOrSetEntityUUID(cmbModelVertex2);
          selVTXs.insert(std::make_pair(vid, cmbModelVertex2));
        }
        else
        {
          seledgepts.insert(pId);
        }
      }

      if (seledgepts.size() > 0)
        selArcs[mapIt->first] = std::make_pair(selEdge, seledgepts);
    }
  }
}

EdgeOperation::Result EdgeOperation::operateInternal()
{
  smtk::model::Model model = this->parameters()->associations()->valueAs<smtk::model::Entity>();

  smtk::session::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::discrete::Resource>(model.component()->resource());

  SessionPtr opsession = resource->discreteSession();

  vtkDiscreteModelWrapper* modelWrapper = opsession->findModelEntity(model.entity());
  bool ok = false;
  smtk::attribute::MeshSelectionItem::Ptr inSelectionItem =
    this->parameters()->findMeshSelection("selection");

  std::map<smtk::common::UUID, vtkDiscreteModelVertex*> selVTXs;
  std::map<smtk::common::UUID, std::pair<vtkDiscreteModelEdge*, std::set<int> > > selArcs;
  smtk::model::EntityRefArray srcsRemoved;
  smtk::model::EntityRefArray srcsModified;
  smtk::model::EntityRefArray srcsCreated;
  smtk::mesh::MeshSets modifiedMeshes;

  MeshModifyMode opType = inSelectionItem->modifyMode();
  switch (opType)
  {
    case ACCEPT:
      this->getSelectedVertsAndEdges(resource, selVTXs, selArcs, inSelectionItem, opsession);

      // We do either merge or split, not both in same operation,
      // and merge is always being processed before split
      // Find if any vertex is selected, if yes, we do merge.
      // Promotion is always being processed before Demotion
      if (selVTXs.size() > 0)
        ok = this->convertSelectedEndNodes(selVTXs, modelWrapper, opsession, srcsRemoved,
          srcsModified, modifiedMeshes, m_mergeOp.GetPointer());
      else if (selArcs.size() > 0)
        ok = this->splitSelectedEdgeNodes(selArcs, modelWrapper, opsession, srcsCreated,
          srcsModified, modifiedMeshes, m_splitOp.GetPointer());
      break;
    case RESET:
    case MERGE:
    case SUBTRACT:
      // don't know what to do, will return smtk::operation::Operation::Outcome::FAILED
      break;
    case NONE:
      ok = true; // stop
      break;
    default:
      std::cerr << "ERROR: Unrecognized MeshModifyMode: " << std::endl;
      break;
  }

  Result result = this->createResult(ok ? smtk::operation::Operation::Outcome::SUCCEEDED
                                        : smtk::operation::Operation::Outcome::FAILED);

  if (ok)
  {
    switch (opType)
    {
      case ACCEPT:
        if (selVTXs.size() > 0)
        {
          if (srcsRemoved.size() > 0)
          {
            smtk::attribute::ComponentItem::Ptr expunged = result->findComponent("expunged");
            for (auto e : srcsRemoved)
            {
              expunged->appendValue(e.component());
            }
          }
        }
        else if (selArcs.size() > 0)
        {
          if (srcsCreated.size() > 0)
          {
            smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
            for (auto c : srcsCreated)
            {
              created->appendValue(c.component());
            }
          }
        }

        if (srcsModified.size() > 0)
        {
          smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
          for (auto m : srcsModified)
          {
            modified->appendValue(m.component());
          }
        }
        if (modifiedMeshes.size() > 0)
        {
          smtk::attribute::MeshItemPtr resultMeshes = result->findMesh("mesh_modified");
          if (resultMeshes)
            resultMeshes->appendValues(modifiedMeshes);
        }
        break;
      case RESET:
      case MERGE:
      case SUBTRACT:
      case NONE:
        break;
      default:
        break;
    }
  }

  return result;
}

bool EdgeOperation::splitSelectedEdgeNodes(
  const std::map<smtk::common::UUID, std::pair<vtkDiscreteModelEdge*, std::set<int> > >& selArcs,
  vtkDiscreteModelWrapper* modelWrapper, smtk::session::discrete::SessionPtr opsession,
  smtk::model::EntityRefArray& srcsCreated, smtk::model::EntityRefArray& srcsModified,
  smtk::mesh::MeshSets& modifiedMeshes, vtkEdgeSplitOperation* splitOp)
{
  if (selArcs.size() == 0)
  {
    return false;
  }
  smtk::model::Model model = this->parameters()->associations()->valueAs<smtk::model::Entity>();

  smtk::session::discrete::Resource::Ptr resource =
    std::static_pointer_cast<smtk::session::discrete::Resource>(model.component()->resource());

  vtkIdType selEdgeId;
  bool success = false;
  std::map<smtk::common::UUID, std::pair<vtkDiscreteModelEdge*, std::set<int> > >::const_iterator
    arcIt;
  for (arcIt = selArcs.begin(); arcIt != selArcs.end(); ++arcIt)
  {
    vtkDiscreteModelEdge* cmbModelEdge = arcIt->second.first;
    if (!cmbModelEdge)
      continue;
    if (arcIt->second.second.size() == 0)
    {
      continue;
    }

    // Find the first point Id that is not a model vertex, and only split at this point,
    // the rest of points will be ignored
    vtkIdType pointId = *(arcIt->second.second.begin());
    if (pointId < 0)
    {
      continue;
    }

    selEdgeId = cmbModelEdge->GetUniquePersistentId();
    splitOp->SetEdgeId(selEdgeId);
    splitOp->SetPointId(pointId);
    splitOp->Operate(modelWrapper);
    success = splitOp->GetOperateSucceeded() != 0;
    if (success)
    {
      smtk::common::UUID modelid = opsession->findOrSetEntityUUID(modelWrapper->GetModel());
      smtk::model::Model inModel(resource, modelid);

      opsession->retranscribeModel(inModel);

      vtkModelVertex* newvtx = vtkModelVertex::SafeDownCast(
        modelWrapper->GetModelEntity(vtkModelVertexType, splitOp->GetCreatedModelVertexId()));
      smtk::common::UUID newVid = opsession->findOrSetEntityUUID(newvtx);
      smtk::model::Vertex newVert(resource, newVid);
      srcsCreated.push_back(newVert);

      vtkModelEdge* newedge = vtkModelEdge::SafeDownCast(
        modelWrapper->GetModelEntity(vtkModelEdgeType, splitOp->GetCreatedModelEdgeId()));
      smtk::common::UUID newEid = opsession->findOrSetEntityUUID(newedge);
      smtk::model::Edge newEdge(resource, newEid);
      srcsCreated.push_back(newEdge);

      // for the modifed edge, add it to "modified" item in op result;
      smtk::common::UUID srcEid = opsession->findOrSetEntityUUID(cmbModelEdge);
      smtk::model::Edge srced(opsession->resource(), srcEid);
      srcsModified.push_back(srced);

      // update associated meshes
      smtk::attribute::ResourceItem::Ptr associatedMeshResourcesItem =
        this->parameters()->findResource("associated mesh resources");

      if (!internal_splitAssociatedMeshes(
            srced, newEdge, newVert, associatedMeshResourcesItem, modifiedMeshes))
      {
        std::cout << "ERROR: Associated edge meshes failed to split properly." << std::endl;
      }

      vtkModelItemIterator* faces = cmbModelEdge->NewAdjacentModelFaceIterator();
      for (faces->Begin(); !faces->IsAtEnd(); faces->Next())
      {
        vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
        smtk::common::UUID faceuuid = opsession->findOrSetEntityUUID(face);
        smtk::model::Face faceErf(opsession->resource(), faceuuid);
        srcsModified.push_back(faceErf);
      }
      faces->Delete();
    }
  }
  return success;
}

bool EdgeOperation::convertSelectedEndNodes(
  const std::map<smtk::common::UUID, vtkDiscreteModelVertex*>& selVTXs,
  vtkDiscreteModelWrapper* modelWrapper, smtk::session::discrete::SessionPtr opsession,
  smtk::model::EntityRefArray& srcsRemoved, smtk::model::EntityRefArray& srcsModified,
  smtk::mesh::MeshSets& modifiedMeshes, vtkMergeOperation* mergOp)
{
  if (selVTXs.size() == 0)
  {
    return false;
  }
  // if we convert one vertex, then return true
  bool success = false;
  std::map<smtk::common::UUID, vtkDiscreteModelVertex*>::const_iterator it;
  for (it = selVTXs.begin(); it != selVTXs.end(); ++it)
  {
    vtkDiscreteModelVertex* vtx = it->second;
    if (!vtx)
    {
      continue;
    }
    int vtxId = vtx->GetUniquePersistentId();

    std::vector<vtkDiscreteModelEdge*> selEdges;
    vtkModelItemIterator* iterEdge = vtx->NewAdjacentModelEdgeIterator();
    for (iterEdge->Begin(); !iterEdge->IsAtEnd(); iterEdge->Next())
    {
      vtkDiscreteModelEdge* edgeEntity =
        vtkDiscreteModelEdge::SafeDownCast(iterEdge->GetCurrentItem());
      if (edgeEntity)
      {
        selEdges.push_back(edgeEntity);
      }
    }
    iterEdge->Delete();
    if (selEdges.size() != 2)
    {
      continue;
    }
    bool targetSwitched = false;
    vtkIdType toEdgeId = selEdges[0]->GetUniquePersistentId();
    vtkIdType fromEdgeId = selEdges[1]->GetUniquePersistentId();
    if (fromEdgeId < toEdgeId)
    {
      toEdgeId = fromEdgeId;
      fromEdgeId = selEdges[0]->GetUniquePersistentId();
      targetSwitched = true;
    }
    // this is before the operation so that we can get a valid uuid,
    // becasue the operation will delete the edge
    smtk::common::UUID fromEid =
      opsession->findOrSetEntityUUID(targetSwitched ? selEdges[0] : selEdges[1]);
    smtk::common::UUID toEid =
      opsession->findOrSetEntityUUID(targetSwitched ? selEdges[1] : selEdges[0]);

    mergOp->SetTargetId(toEdgeId);
    mergOp->SetSourceId(fromEdgeId);
    mergOp->AddLowerDimensionalId(vtxId);
    mergOp->Operate(modelWrapper);
    success = mergOp->GetOperateSucceeded() != 0;
    if (success)
    {
      // add the removed vertex to the list
      smtk::model::Vertex remVert(opsession->resource(), it->first);
      srcsRemoved.push_back(remVert);
      // update the removed and modified edge list
      smtk::model::Edge toRemove(opsession->resource(), fromEid);
      srcsRemoved.push_back(toRemove);

      smtk::attribute::ResourceItem::Ptr associatedMeshResourcesItem =
        this->parameters()->findResource("associated mesh resources");

      // update associated mesh first before removing the edge
      if (!internal_mergeAssociatedMeshes(remVert, toRemove,
            smtk::model::Edge(opsession->resource(), toEid), associatedMeshResourcesItem,
            modifiedMeshes))
      {
        std::cout << "ERROR: Associated edge meshes failed to merge properly." << std::endl;
      }

      opsession->resource()->erase(it->first);
      opsession->resource()->erase(fromEid);

      smtk::common::UUID modelid = opsession->findOrSetEntityUUID(modelWrapper->GetModel());
      smtk::model::Model inModel(opsession->resource(), modelid);

      opsession->retranscribeModel(inModel);

      vtkModelEdge* tgtEdge = targetSwitched ? selEdges[1] : selEdges[0];
      toEid = opsession->findOrSetEntityUUID(tgtEdge);
      srcsModified.push_back(smtk::model::EntityRef(opsession->resource(), toEid));
      vtkModelItemIterator* faces = tgtEdge->NewAdjacentModelFaceIterator();
      for (faces->Begin(); !faces->IsAtEnd(); faces->Next())
      {
        vtkModelFace* face = vtkModelFace::SafeDownCast(faces->GetCurrentItem());
        smtk::common::UUID faceuuid = opsession->findOrSetEntityUUID(face);
        //        opsession->resource()->erase(faceuuid);
        //        faceuuid = opsession->findOrSetEntityUUID(face);
        // Now re-add it (it will have new edges)
        smtk::model::Face faceErf(opsession->resource(), faceuuid);
        //        opsession->transcribe(faceErf, smtk::model::SESSION_EVERYTHING, false);

        srcsModified.push_back(faceErf);
      }
      faces->Delete();
    }
  }

  return success;
}

const char* EdgeOperation::xmlDescription() const
{
  return EdgeOperation_xml;
}

} // namespace discrete
} // namespace session
} // namespace smtk
