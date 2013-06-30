/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
#include "vtkDiscreteModelFace.h"

#include "ModelVertexClassification.h"
#include "vtkBitArray.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkConnectivityFilter.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelVertex.h"
#include "vtkFeatureEdges.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkSerializer.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelShellUse.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkSmartPointer.h"
#include "vtkSplitEventData.h"

vtkDiscreteModelFace* vtkDiscreteModelFace::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDiscreteModelFace");
  if(ret)
    {
    return static_cast<vtkDiscreteModelFace*>(ret);
    }
  return new vtkDiscreteModelFace;
}

vtkDiscreteModelFace::vtkDiscreteModelFace()
{
}

vtkDiscreteModelFace::~vtkDiscreteModelFace()
{
}

vtkModelEntity* vtkDiscreteModelFace::GetThisModelEntity()
{
  return this;
}

bool vtkDiscreteModelFace::Split(
  double splitAngle, std::map<vtkIdType, FaceEdgeSplitInfo>& FaceSplitInfo)
{
  vtkObject* geometry = this->GetGeometry();
  vtkPolyData* poly = vtkPolyData::SafeDownCast(geometry);

  if(poly == 0)
    {
    if(geometry)
      {
      // we are on the server...
      return 0;
      }
    // BoundaryRep has not been set -> return error
    return 0;
    }

  // on server, go ahead and perform the split
  vtkNew<vtkPolyDataNormals> pdNormals;
  pdNormals->SetFeatureAngle(splitAngle);
  pdNormals->SetInputData(0, poly);

  vtkNew<vtkConnectivityFilter> pdConnectivity;
  pdConnectivity->SetInputConnection(pdNormals->GetOutputPort());
  pdConnectivity->SetColorRegions(1);
  pdConnectivity->Update();

  vtkIdTypeArray* newFaceTags = vtkIdTypeArray::SafeDownCast(
    vtkDataSet::SafeDownCast(pdConnectivity->GetOutputDataObject(0))
    ->GetCellData()->GetArray("RegionId"));

  // filter the output
  // actually, newModelFaces also contains the old/source model face

  // the ids in newModelFaces are with respect to the master poly data
  std::map<vtkIdType, vtkSmartPointer<vtkIdList> > newModelFaces;

  vtkIdTypeArray* masterGeometryCellIndex = this->GetReverseClassificationArray();
  for(vtkIdType i=0;i<newFaceTags->GetNumberOfTuples();i++)
    {
    vtkIdType tag = newFaceTags->GetValue(i);
    if(newModelFaces.find(tag) == newModelFaces.end())
      {
      newModelFaces[tag] = vtkSmartPointer<vtkIdList>::New();
      }
    newModelFaces[tag]->InsertNextId(masterGeometryCellIndex->GetValue(i));
    }

  if(newModelFaces.size() > 1)
    {
    vtkSplitEventData* splitEventData = vtkSplitEventData::New();
    splitEventData->SetSourceEntity(this);
    vtkIdList* newFaceIds = vtkIdList::New();
    newFaceIds->SetNumberOfIds(newModelFaces.size()-1); // the first one is the original
    std::map<vtkIdType, vtkSmartPointer<vtkIdList> >::iterator mit=newModelFaces.begin();
    mit++; // skip the first model face as it is the old/source model face

    bool saveExistingLoop = false;
    for(vtkIdType i=0;mit!=newModelFaces.end();mit++,i++)
      {
      FaceEdgeSplitInfo splitInfo;
      // if this is the last new face, save the loopinfo for existing face
      saveExistingLoop = (i == (newFaceIds->GetNumberOfIds()-1));
      vtkDiscreteModelFace* face =
        this->BuildFromExistingModelFace(mit->second, splitInfo, saveExistingLoop);
      vtkIdType newId = face->GetUniquePersistentId();
      FaceSplitInfo.insert(std::make_pair(newId,splitInfo));
      newFaceIds->SetId(i, newId);
      }
    splitEventData->SetCreatedModelEntityIds(newFaceIds);
    newFaceIds->Delete();
    this->GetModel()->InvokeModelGeometricEntityEvent(ModelGeometricEntitySplit,
                                                      splitEventData);
    splitEventData->Delete();
    }

  return 1;
}

vtkDiscreteModelFace* vtkDiscreteModelFace::BuildFromExistingModelFace(
  vtkIdList* masterCellIds, FaceEdgeSplitInfo& splitInfo,
  bool saveLoopForExistingFace)
{
  vtkDiscreteModel* thisModel = vtkDiscreteModel::SafeDownCast(
    this->GetModel());
  bool blockEvent = thisModel->GetBlockModelGeometricEntityEvent();
  thisModel->SetBlockModelGeometricEntityEvent(true);

  vtkDiscreteModelFace* newModelFace = vtkDiscreteModelFace::SafeDownCast(
    thisModel->BuildModelFace(0, 0, 0));
  thisModel->SetBlockModelGeometricEntityEvent(blockEvent);

  newModelFace->AddCellsToGeometry(masterCellIds);

  // Handle splits needed for existing edges
  if(this->GetNumberOfModelEdges())
    {
    this->SplitEdges(newModelFace, splitInfo);
    }

  // put in the adjacencies for the model face use
  for(int i=0;i<2;i++)
    {
    vtkModelFaceUse* sourceFaceUse = this->GetModelFaceUse(i);
    vtkModelShellUse* sourceShellUse = sourceFaceUse->GetModelShellUse();
    vtkModelFaceUse* targetFaceUse = newModelFace->GetModelFaceUse(i);
    if(sourceShellUse)
      {
      sourceShellUse->AddModelFaceUse(targetFaceUse);
      }
    }

  // new model face will belong to same groups as old model face
  newModelFace->CopyModelEntityGroups(this);
  thisModel->InvokeModelGeometricEntityEvent(
    ModelGeometricEntityCreated, newModelFace);
  for(int i=0;i<2;i++)
    {
    if(vtkModelRegion* region = newModelFace->GetModelRegion(i))
      {
      thisModel->InvokeModelGeometricEntityEvent(
        ModelGeometricEntityBoundaryModified, region);
      }
    }

  // update loops and edges info if needed
  if(this->GetNumberOfModelEdges())
    {
    newModelFace->BuildEdges(true, splitInfo);
    this->BuildEdges(true, splitInfo, saveLoopForExistingFace);
    }

  return newModelFace;
}

void vtkDiscreteModelFace::GetAllPointIds(vtkIdList* ptsList)
{
  vtkBitArray* Points = vtkBitArray::New();
  this->GatherAllPointIdsMask(Points);
  for(vtkIdType i=0;i<Points->GetNumberOfTuples();i++)
    {
    if(Points->GetValue(i) != 0)
      {
      ptsList->InsertNextId(i);
      }
    }
  Points->Delete();
}

void vtkDiscreteModelFace::GetInteriorPointIds(vtkIdList* ptsList)
{
  vtkBitArray* allModelFacePoints = vtkBitArray::New();
  this->GatherAllPointIdsMask(allModelFacePoints);
  vtkBitArray* boundaryModelFacePoints = vtkBitArray::New();
  this->GatherBoundaryPointIdsMask(boundaryModelFacePoints);
  for(vtkIdType i=0;i<allModelFacePoints->GetNumberOfTuples();i++)
    {
    if(allModelFacePoints->GetValue(i) != 0 &&
      boundaryModelFacePoints->GetValue(i) == 0)
      {
      ptsList->InsertNextId(i);
      }
    }
  allModelFacePoints->Delete();
  boundaryModelFacePoints->Delete();
}

void vtkDiscreteModelFace::GetBoundaryPointIds(vtkIdList* ptsList)
{
  vtkBitArray* points = vtkBitArray::New();
  this->GatherBoundaryPointIdsMask(points);
  for(vtkIdType i=0;i<points->GetNumberOfTuples();i++)
    {
    if(points->GetValue(i) != 0)
      {
      ptsList->InsertNextId(i);
      }
    }
  points->Delete();
}

void vtkDiscreteModelFace::GatherAllPointIdsMask(vtkBitArray* points)
{
  vtkPointSet* grid = vtkPointSet::SafeDownCast(this->GetGeometry());
  if(!grid)
    {
    vtkWarningMacro("Cannot access model face's grid.");
    return;
    }
  // Since not all points in Grid's points are attached to cells, we
  // iterate over the cells and get their points to set them in Points.
  points->SetNumberOfComponents(1);
  points->SetNumberOfTuples(grid->GetNumberOfPoints());
  for(vtkIdType i=0;i<grid->GetNumberOfPoints();i++)
    {
    points->SetValue(i, 0);
    }
  vtkIdType numberOfCells = grid->GetNumberOfCells();
  vtkIdList* cellPoints = vtkIdList::New();
  for(vtkIdType i=0;i<numberOfCells;i++)
    {
    grid->GetCellPoints(i, cellPoints);
    for(vtkIdType j=0;j<cellPoints->GetNumberOfIds();j++)
      {
      vtkIdType id=cellPoints->GetId(j);
      points->SetValue(id, 1);
      }
    }
  cellPoints->Delete();
}

void vtkDiscreteModelFace::GatherBoundaryPointIdsMask(vtkBitArray* points)
{
  vtkPointSet* grid = vtkPointSet::SafeDownCast(this->GetGeometry());
  // add in an array of the original point ids to grid
  vtkIdTypeArray* originalPointIds = vtkIdTypeArray::New();
  originalPointIds->SetNumberOfComponents(1);
  originalPointIds->SetNumberOfTuples(grid->GetNumberOfPoints());
  for(vtkIdType i=0;i<grid->GetNumberOfPoints();i++)
    {
    originalPointIds->SetValue(i, i);
    }
  const char arrayName[] = "vtkModelNodalGroupPointIdArray";
  originalPointIds->SetName(arrayName);
  grid->GetPointData()->AddArray(originalPointIds);
  originalPointIds->Delete();

  vtkNew<vtkFeatureEdges> featureEdges;
  featureEdges->BoundaryEdgesOn();
  featureEdges->NonManifoldEdgesOff();
  featureEdges->ManifoldEdgesOff();
  featureEdges->FeatureEdgesOff();
  featureEdges->SetInputData(grid);
  featureEdges->Update();
  vtkPointSet* gridEdges = vtkPointSet::SafeDownCast(featureEdges->GetOutput(0));

  // Since not all points in GridEdges's points are attached to cells, we
  // iterate over the cells and get their points to set them in Points.
  originalPointIds = vtkIdTypeArray::SafeDownCast(
    gridEdges->GetPointData()->GetArray(arrayName));

  points->SetNumberOfComponents(1);
  points->SetNumberOfTuples(grid->GetNumberOfPoints());
  for(vtkIdType i=0;i<grid->GetNumberOfPoints();i++)
    {
    points->SetValue(i, 0);
    }
  vtkIdType numberOfCells = gridEdges->GetNumberOfCells();
  vtkNew<vtkIdList> cellPoints;
  for(vtkIdType i=0;i<numberOfCells;i++)
    {
    gridEdges->GetCellPoints(i, cellPoints.GetPointer());
    for(vtkIdType j=0;j<cellPoints->GetNumberOfIds();j++)
      {
      vtkIdType id=cellPoints->GetId(j);
      vtkIdType masterGridId = originalPointIds->GetValue(id);
      points->SetValue(masterGridId, 1);
      }
    }
  grid->GetPointData()->RemoveArray(arrayName);
}
void vtkDiscreteModelFace::ExtractEdges(vtkPolyData* result)
{
  vtkPolyData* grid = vtkPolyData::SafeDownCast(this->GetGeometry());
  result->Reset();
  // add in an array for storing the cell ID that "owns" the edge
  vtkNew<vtkIdTypeArray> cellIds;
  cellIds->SetNumberOfComponents(1);
  cellIds->Allocate(grid->GetNumberOfCells() * 0.1);
  result->SetPoints(grid->GetPoints());
  const char arrayName[] = "OriginalFacetCellID";
  cellIds->SetName(arrayName);
  result->GetCellData()->AddArray(cellIds.GetPointer());

  // Setup the resulting edges
  vtkNew<vtkCellArray> newLines;
  newLines->Allocate(grid->GetNumberOfCells() * 0.1);
  result->SetLines(newLines.GetPointer());

  // Get the original cell IDs w/r to the mesh
  vtkIdTypeArray* masterGeometryCellIndex = this->GetReverseClassificationArray();

  // Iterate over all of the cells of the face and find any boundary edge
  vtkIdType cellId, npts, *pts, i, linePts[2];
  vtkNew<vtkIdList> neighbors;
  vtkCellArray *facePolys = grid->GetPolys();
  neighbors->Allocate(VTK_CELL_SIZE);
  grid->BuildLinks();
  for(cellId = 0, facePolys->InitTraversal();
      facePolys->GetNextCell(npts, pts); cellId++)
    {
    if (npts < 3)
      {
      continue; // Degenerate Cell!
      }
    // Set the "first line point to be the last one in the poly
    linePts[0] = pts[npts-1];
    for (i = 0; i < npts; i++)
      {
      grid->GetCellEdgeNeighbors(cellId, linePts[0], pts[i], neighbors.GetPointer());
      if (!(neighbors->GetNumberOfIds()))
        {
        linePts[1] = pts[i];
        newLines->InsertNextCell(2, linePts);
        cellIds->InsertNextValue(masterGeometryCellIndex->GetValue(cellId));
        }
      linePts[0] = pts[i]; // Set this to be the new start point
      }
    }
}

void vtkDiscreteModelFace::BuildEdges(bool showEdge,
  FaceEdgeSplitInfo& splitInfo, bool saveLoopInfo)
{
  vtkModelEdge *gedge;
  vtkNew<vtkPolyData> edges;
  std::vector<LoopInfo> loops;
  std::vector<bool> visited;
  NewModelEdgeInfo newEdgesInfo;
  int i, nlines, currentLoop, dummyInt, nLoops, j, nEdges;
  vtkIdType gid;
  vtkIdType idLoops, idEdges, idOrient, fid = this->GetUniquePersistentId();

  vtkCellArray *lines;

  this->ExtractEdges(edges.GetPointer());
  vtkIdTypeArray *facetIds =
    dynamic_cast<vtkIdTypeArray*>(edges->GetCellData()->
                                  GetArray("OriginalFacetCellID", dummyInt));
  if (!facetIds)
    {
    std::cout << "Could not find Facet Mapping!!\n";
    return;
    }

  edges->BuildLinks(); // We need to "walk" the loops

  lines = edges->GetLines();
  nlines = lines->GetNumberOfCells();
  if (!nlines)
    {
    std::cout << "\tFace " << this->GetUniquePersistentId() << "has no edges!\n";
    return;
    }
  visited.assign(nlines, false);
  loops.clear();
  newEdgesInfo.Reset();
  for (i = 0; i < nlines; i++)
    {
    if (visited[i])
      {
      continue;
      }
    //Create a new loop
    currentLoop = loops.size();
    loops.resize(currentLoop+1);

    this->WalkLoop(i, edges.GetPointer(), visited, facetIds,
                   newEdgesInfo, loops[currentLoop]);
    }

  // OK so we have now processed all of the loops we now need to create
  // any new model edges
  std::map<int, vtkDiscreteModelEdge*> newModelEdges;
  this->CreateModelEdges(newEdgesInfo, newModelEdges, showEdge, splitInfo);

  // Now we are ready to add the model edges to the model face
  nLoops = loops.size();
  if (!nLoops)
    {
    // This face has no loops - like a sphere
    return;
    }

  // destroy the existing loop uses
  this->DestroyLoopUses();

  if(saveLoopInfo)
    {
    // The map of <faceId, nloops, nEdges, (gedges[n]...), (orientations[n]...)>
    splitInfo.FaceEdgeLoopIDs->InsertNextTupleValue(&fid);
    idLoops = nLoops;
    splitInfo.FaceEdgeLoopIDs->InsertNextTupleValue(&idLoops);
    }
  std::vector<int> orientations;
  std::vector<vtkModelEdge *> gedges;
  for (i = 0; i < nLoops; i++)
    {
    // std::cout << "\tLoop " << i << ":";
    nEdges = loops[i].loop.size();
    orientations.resize(nEdges);
    gedges.resize(nEdges);
    if(saveLoopInfo)
      {
      idEdges = nEdges;
      splitInfo.FaceEdgeLoopIDs->InsertNextTupleValue(&idEdges);
      }

    for (j = 0; j < nEdges; j++)
      {
      // Is this a new model edge or a prexisting one
      gid = loops[i].loop[j].first;
      if (gid < 0)
        {
        gedge = newModelEdges[gid];
        }
      else
        {
        gedge = dynamic_cast<vtkModelEdge *>
          (this->GetModel()->GetModelEntity(vtkModelEdgeType, gid));
        }
      if (!gedge)
        {
        std::cout << "Could not find model edge: " << gid << "\n";
        continue;
        }
      if (loops[i].loop[j].second)
        {
        orientations[j] = 1;
        // std::cout << gedge->GetUniquePersistentId() << " ";
        }
      else
        {
        orientations[j] = 0;
        // std::cout << "~" << gedge->GetUniquePersistentId() << " ";
        }
      gedges[j] = gedge;

      if(saveLoopInfo)
        {
        gid = gedge->GetUniquePersistentId();
        splitInfo.FaceEdgeLoopIDs->InsertNextTupleValue(&gid);
        idOrient = orientations[j];
        splitInfo.FaceEdgeLoopIDs->InsertNextTupleValue(&idOrient);
        }
      }
    // std::cout << "\n";
    this->AddLoop(nEdges, &gedges.front(), &orientations.front());
    }
}

//----------------------------------------------------------------------------
void vtkDiscreteModelFace::WalkLoop(vtkIdType startingEdge,
                                           vtkPolyData *edges,
                                           std::vector<bool> &visited,
                                           vtkIdTypeArray *facetIds,
                                           NewModelEdgeInfo &newEdgesInfo,
                                           LoopInfo &loopInfo)
{
  vtkIdType currentEdge = startingEdge;
  vtkNew<vtkIdList>cellIds, pointIds;
  vtkIdType currentPoint, firstPoint, nextPoint;
  vtkIdType gedge;
  std::string currentFaceInfo;
  vtkDiscreteModel* thisModel = vtkDiscreteModel::SafeDownCast(
    this->GetModel());
  const DiscreteMesh &mesh = thisModel->GetMesh();
  vtkDiscreteModel::ClassificationType &classificationInfo =
    thisModel->GetMeshClassification();
  vtkDiscreteModelEdge *dedge;
  // Setup the newEdgesInfo to tag the first mesh that needs to be
  // created and to force a new model edge to be "defined" when that
  // happens
  newEdgesInfo.ClearCurrentFaceInfo();
  newEdgesInfo.ClearTaggedFaceInfo();
  // Walk the loop
  currentPoint = -1; // Indicates the point has not been set

  while(!visited[currentEdge])
    {
    visited[currentEdge] = true;
    // Get the points of the edge
    edges->GetCellPoints(currentEdge, pointIds.GetPointer());
    if (pointIds->GetNumberOfIds() != 2)
      {
      std::cout << "Found incorrect number of points: "
                << pointIds->GetNumberOfIds() << "\n";
      break;
      }
    if (currentPoint == -1)
      {
      firstPoint = currentPoint = pointIds->GetId(0);
      nextPoint = pointIds->GetId(1);
      }
    else if (currentPoint != pointIds->GetId(0))
      {
      if (currentPoint == pointIds->GetId(1))
        {
        std::cout << "Found flipped Edge!\n" ;
        nextPoint = pointIds->GetId(0);
        }
      else
        {
        std::cout << "Found Disconnected Edge!\n" ;
        }
      }
    else
      {
      nextPoint = pointIds->GetId(1);
      }
    // Does the edge exist?
    bool createdEdge, orientation;
    vtkIdType medge = mesh.AddEdgeIfNotExisting(currentPoint,
                                                nextPoint,
                                                orientation,
                                                createdEdge);
    // std::cout << "Edge: " << currentEdge << " Verts: "
    //           << currentPoint << ", " << nextPoint << " MEdge: "
    //           << medge << " Created: " << createdEdge << "\n";
    if (createdEdge)
      {
      //Need to get the face information bounding the edge
      currentFaceInfo =
        this->EncodeModelFaces(facetIds->GetValue(currentEdge),
                               currentPoint, nextPoint);
      gedge = newEdgesInfo.InsertMeshEdge(medge, currentFaceInfo);
      loopInfo.InsertModelEdge(gedge, true);
      }
    else
      {
      // The model edge exists so the next time we find an edge
      // that doesn't exist it must be on a new model edge
      newEdgesInfo.ClearCurrentFaceInfo();
      // Need to get the model edge classified on the mesh edge
      dedge = dynamic_cast<vtkDiscreteModelEdge*>
        (classificationInfo.GetEntity(medge));

      gedge = dedge->GetUniquePersistentId();
      loopInfo.InsertModelEdge(gedge, orientation);
      }
    // Lets get the next edge in the loop
    currentPoint = nextPoint;
    edges->GetPointCells(currentPoint, cellIds.GetPointer());
    // Should be 2 lines comming into the point
    if (cellIds->GetNumberOfIds() < 2)
      {
      std::cout << "Found unconnected edge!\n";
      }
    else if (cellIds->GetNumberOfIds() > 2)
      {
      std::cout << "Found non-manifold edge!\n";
      // Is there only one edge left to visit?
      vtkIdType  numEdgesToVisit = 0, lastEdgeToBeVisited, j;
      vtkIdType testMEdge, testEdge;
      for (j = 0;  j < cellIds->GetNumberOfIds(); j++)
        {
        if (!visited[cellIds->GetId(j)])
          {
          ++numEdgesToVisit;
          lastEdgeToBeVisited = cellIds->GetId(j);
          }
        }
      if (!numEdgesToVisit)
        {
        continue; // We have reached the end of the loop
        }
      if (numEdgesToVisit == 1)
        {
        // There is only one way to go - take it!
        currentEdge = lastEdgeToBeVisited;
        continue;
        }
      // OK we need to determine how to go - if we created a new model
      // edge then we should look for an edge with the same face classification
      // else we should see if we have an edge on the same model edge
      if (createdEdge)
        {
        // For each edge see if we can find the same bounding faces
        std::string nextFaceInfo;
        for (j = 0;  j < cellIds->GetNumberOfIds(); j++)
          {
          testEdge = cellIds->GetId(j);
          // Skip if we have already visited the edge or if the edge is on
          // an existing model edge
          if (visited[testEdge])
            {
            continue;
            }
          edges->GetCellPoints(testEdge, pointIds.GetPointer());
          if (mesh.EdgeExists(pointIds->GetId(0), pointIds->GetId(1), testMEdge))
            {
            continue;
            }
          nextFaceInfo = this->EncodeModelFaces(facetIds->GetValue(testEdge),
                                                pointIds->GetId(0),
                                                pointIds->GetId(1));
          if (currentFaceInfo == nextFaceInfo)
            {
            // Found next edge!
            currentEdge = testEdge;
            break;
            }
          }
        }
      else
        {
        // This is the reverse of the previous method - here we want to follow
        // only existing model edges
        for (j = 0;  j < cellIds->GetNumberOfIds(); j++)
          {
          testEdge = cellIds->GetId(j);
          // Skip if we have already visited the edge or if the edge is not on
          // an existing model edge
          if (visited[testEdge])
            {
            continue;
            }
          edges->GetCellPoints(testEdge, pointIds.GetPointer());
          if (!mesh.EdgeExists(pointIds->GetId(0), pointIds->GetId(1), testMEdge))
            {
            continue;
            }
          // Need to get the model edge classified on the mesh edge
          dedge = dynamic_cast<vtkDiscreteModelEdge*>
            (classificationInfo.GetEntity(testMEdge));
          if (dedge->GetUniquePersistentId() == gedge)
            {
            // Found next edge!
            currentEdge = testEdge;
            break;
            }
          }
        }
      }
    else
      {
      if (cellIds->GetId(0) == currentEdge)
        {
        currentEdge = cellIds->GetId(1);
        }
      else if (cellIds->GetId(1) != currentEdge)
        {
        std::cout << "Invalid Cell Structure!!\n";
        }
      else
        {
        currentEdge = cellIds->GetId(0);
        }
      }
    }
  // Do we need to fix the first edge in the loop?  This would occur if
  // we didn't  not start at a "model vertex".  The condition we need to
  // check for is if the first and last edges are either both "new" or both
  // existing
  int numLoops = loopInfo.loop.size();
  if (numLoops == 1)
    {
    // There is only a single model edge so there is no problem
    return;
    }
  // Does the loop start and end with the same model edge?  If so remove one
  if (loopInfo.loop[0].first == loopInfo.loop[numLoops - 1].first)
    {
    loopInfo.loop.erase(loopInfo.loop.begin());
    }
  else if ((loopInfo.loop[0].first < 0) && (loopInfo.loop[numLoops - 1].first < 0))
    {
    // OK so we know that the first and last edges of the loop need to be
    // created.  If they are bounding the same model faces they need to be
    // combined.
    if (newEdgesInfo.CheckLastModelEdge(gedge))
      {
      loopInfo.RemoveModelEdge(gedge);
      }
    }
}

//----------------------------------------------------------------------------
std::string vtkDiscreteModelFace::
EncodeModelFaces(vtkIdType facetId, vtkIdType v0, vtkIdType v1)
{
  vtkDiscreteModel* thisModel = vtkDiscreteModel::SafeDownCast(
    this->GetModel());

  vtkNew<vtkIdList> cellIds;
  vtkDiscreteModel::ClassificationType &classificationInfo =
    thisModel->GetMeshClassification();
  int j, nids;
  vtkDiscreteModelFace *dface;
  vtkIdType gface;
  thisModel->GetMesh().GetCellEdgeNeighbors(facetId, v0, v1,
                                              cellIds.GetPointer());
  // Convert the facet Ids to model Ids
  nids = cellIds->GetNumberOfIds();
  for (j = 0; j < nids; j++)
    {
    dface = dynamic_cast<vtkDiscreteModelFace*>
      (classificationInfo.GetEntity(cellIds->GetId(j)));
    gface = dface->GetUniquePersistentId();
    cellIds->SetId(j, gface);
    }
  return NewModelEdgeInfo::to_key(cellIds.GetPointer());
}
//----------------------------------------------------------------------------
void vtkDiscreteModelFace::
CreateModelEdges(NewModelEdgeInfo &newEdgesInfo,
                 std::map<int, vtkDiscreteModelEdge*> &newEdges,
                 bool bShow, FaceEdgeSplitInfo& splitInfo)
{
  // Lets walk the new edge information and create the appropriate model
  // topology
  vtkDiscreteModel* thisModel = vtkDiscreteModel::SafeDownCast(
    this->GetModel());

  newEdges.clear();
  vtkIdType currentModelEdgeId = 0, numEdges;
  vtkNew<vtkIdList> edgeCells, edgePnts;
  const DiscreteMesh &mesh = thisModel->GetMesh();
  vtkDiscreteModel::ClassificationType &classificationInfo =
    thisModel->GetMeshClassification();
  ModelVertexClassification vertexInfo(thisModel);
  vtkDiscreteModelVertex *v0 = NULL, *v1;
  vtkDiscreteModelEdge *gedge;
  std::vector<std::pair<vtkIdType, vtkIdType> > &info = newEdgesInfo.info;
  std::size_t i, numMeshEdges = info.size();

  //First we need to extend the classification information to include the
  // mesh edges we created
  numEdges = classificationInfo.size(vtkDiscreteModel::ClassificationType::EDGE_DATA);
  classificationInfo.resize(numEdges+numMeshEdges,
                            vtkDiscreteModel::ClassificationType::EDGE_DATA);

  for (i = 0; i < numMeshEdges; i++)
    {
    //Are we on the same model edge?
    if (info[i].second == currentModelEdgeId)
      {
      // Just add the cell to the list
      edgeCells->InsertNextId(info[i].first);
      continue;
      }

    // Have we found any mesh edges to create a model edge yet?
    // Note that the first time into the loop this will not be true
    numEdges = edgeCells->GetNumberOfIds();
    if (numEdges)
      {
      // We need to create v1 for this model edge by looking at the
      // last mesh edge added
      mesh.GetCellPointIds(edgeCells->GetId(numEdges-1), edgePnts.GetPointer());

      v1 = vertexInfo.AddModelVertex(edgePnts->GetId(1), true).second;
      if (v0 && v1)
        {
        gedge =
          dynamic_cast<vtkDiscreteModelEdge*>(thisModel->BuildModelEdge(v0, v1));
        // Don't show edges by default
        gedge->SetVisibility(bShow);
        v0->SetVisibility(bShow);
        v1->SetVisibility(bShow);

        // The map of <NewEdgeId, VertId1, VertId2>
        vtkIdType newEdgeVV[3] =
          {gedge->GetUniquePersistentId(),
           v0->GetUniquePersistentId(),
           v1->GetUniquePersistentId()};
        splitInfo.CreatedModelEdgeVertIDs->InsertNextTupleValue(newEdgeVV);

        gedge->AddCellsToGeometry(edgeCells.GetPointer());
        newEdges[currentModelEdgeId] = gedge;
        // std::cout << "Create Model Edge: " << gedge->GetUniquePersistentId()
        //           << " (ID = " << currentModelEdgeId
        //           << ") From Vertices ("
        //           << v0->GetUniquePersistentId() << ","
        //           << v1->GetUniquePersistentId() << ")\n";
        }
      else
        {
        std::cout << "Could not build model edge!!\n";
        }
      edgeCells->Reset();
      }

    currentModelEdgeId = info[i].second;
    // Get the first vertex of the edge
    mesh.GetCellPointIds(info[i].first, edgePnts.GetPointer());

    v0 = vertexInfo.AddModelVertex(edgePnts->GetId(0), true).second;
    // Add the mesh edge to the list
    edgeCells->InsertNextId(info[i].first);
    }

  // Finally we need to process the last model edge
  // Have we found any mesh edges to create a model edge yet?
  // Note that the first time into the loop this will not be true
  numEdges = edgeCells->GetNumberOfIds();
  if (numEdges)
    {
    // We need to create v1 for this model edge by looking at the
    // last mesh edge added
    mesh.GetCellPointIds(edgeCells->GetId(numEdges-1), edgePnts.GetPointer());

    v1 = vertexInfo.AddModelVertex(edgePnts->GetId(1), true).second;
    if (v0 && v1)
      {
      gedge =
        dynamic_cast<vtkDiscreteModelEdge*>(thisModel->BuildModelEdge(v0, v1));
      // Don't show edges and vertex by default
      gedge->SetVisibility(bShow);
      v0->SetVisibility(bShow);
      v1->SetVisibility(bShow);

      // The map of <NewEdgeId, VertId1, VertId2>
      vtkIdType newEdgeVV[3] =
        {gedge->GetUniquePersistentId(),
         v0->GetUniquePersistentId(),
         v1->GetUniquePersistentId()};
      splitInfo.CreatedModelEdgeVertIDs->InsertNextTupleValue(newEdgeVV);

      gedge->AddCellsToGeometry(edgeCells.GetPointer());
      newEdges[currentModelEdgeId] = gedge;
      // std::cout << "Create Model Edge: " << gedge->GetUniquePersistentId()
      //           << " (ID = " << currentModelEdgeId
      //           << ") From Vertices ("
      //           << v0->GetUniquePersistentId() << ","
      //           << v1->GetUniquePersistentId() << ")\n";
      }
    else
      {
      std::cout << "Could not build model edge!!\n";
      }
    }
}

//----------------------------------------------------------------------------
void vtkDiscreteModelFace::SplitEdges(vtkDiscreteModelFace* newModelFace,
  FaceEdgeSplitInfo& splitInfo)
{
  vtkDiscreteModel* thisModel = vtkDiscreteModel::SafeDownCast(
    this->GetModel());

  // If we have a new face breaking existing edges,
  // that edge needs to be splitted
  const DiscreteMesh &mesh = thisModel->GetMesh();
  vtkDiscreteModel::ClassificationType &classificationInfo =
    thisModel->GetMeshClassification();
  vtkDiscreteModelEdge *dedge;
  vtkDiscreteModelFace *dface;

  std::vector<vtkModelEdge*> adjEdges;
  this->GetModelEdges(adjEdges);
  for(std::vector<vtkModelEdge*>::iterator eit=adjEdges.begin();
    eit != adjEdges.end(); ++eit)
    {
    vtkDiscreteModelEdge* splitEdge = vtkDiscreteModelEdge::SafeDownCast(*eit);
    vtkPolyData* grid = vtkPolyData::SafeDownCast(splitEdge->GetGeometry());
    vtkCellArray* lines = grid->GetLines();
    vtkIdType npts, *pts;
    int nids;
    vtkNew<vtkIdList>cellIds;
    bool newFace1, newFace2;
    std::vector<vtkIdType> NewVertexList;
    for(lines->InitTraversal();lines->GetNextCell(npts,pts); )
      {
      // process first point
      mesh.GetPointFaceCells(pts[0], cellIds.GetPointer());
      // Convert the facet Ids to model Ids
      nids = cellIds->GetNumberOfIds();
      newFace1 = false;
      for (int j = 0; j < nids; j++)
        {
        dface = dynamic_cast<vtkDiscreteModelFace*>
          (classificationInfo.GetEntity(cellIds->GetId(j)));
        newFace1 = (dface == newModelFace);
        if(newFace1)
          {
          break;
          }
        }
       // process second point
      mesh.GetPointFaceCells(pts[1], cellIds.GetPointer());
      // Convert the facet Ids to model Ids
      nids = cellIds->GetNumberOfIds();
      newFace2 = false;
      for (int j = 0; j < nids; j++)
        {
        dface = dynamic_cast<vtkDiscreteModelFace*>
          (classificationInfo.GetEntity(cellIds->GetId(j)));
        newFace2 = (dface == newModelFace);
        if(newFace2)
          {
          break;
          }
        }

      // if only one point in the cell (xor) is part of the new face,
      // we should split at the point that's part of the new face.
      if(newFace1 ^ newFace2)
        {
        NewVertexList.push_back(newFace1 ? pts[0] : pts[1]);
        }
      }
    for(std::vector<vtkIdType>::iterator pidIt=NewVertexList.begin();
      splitEdge && pidIt != NewVertexList.end(); ++pidIt)
      {
      vtkIdType newEdgeId=-1, newVertexId=-1;
      if(splitEdge->Split(*pidIt, newVertexId, newEdgeId))
        {
        vtkIdType splitEdgeVEdge[3] =
          {splitEdge->GetUniquePersistentId(), newVertexId, newEdgeId};
        splitInfo.SplitEdgeVertIds->InsertNextTupleValue(splitEdgeVEdge);
        splitEdge = vtkDiscreteModelEdge::SafeDownCast(
          thisModel->GetModelEntity(newEdgeId, vtkModelEdgeType));
        // The map of <OldEdgeId, NewVertId, NewEdgId>
        }
      else
        {
        splitEdge = NULL;
        }
      }
    }
}

void vtkDiscreteModelFace::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

bool vtkDiscreteModelFace::Destroy()
{
  if(this->Superclass::Destroy())
    {
    this->RemoveAllAssociations(vtkDiscreteModelEntityGroupType);
    this->Modified();
    return true;
    }
  return false;
}

void vtkDiscreteModelFace::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
