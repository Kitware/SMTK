//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkEnclosingModelEntityOperator.h"

#include "vtkBoundingBox.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkDoubleArray.h"
#include "vtkGenericCell.h"
#include "vtkIdTypeArray.h"
#include "vtkMath.h"
#include "vtkModelUserName.h"

#include "vtkModelRegion.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"


vtkStandardNewMacro(vtkEnclosingModelEntityOperator);

//----------------------------------------------------------------------------
vtkEnclosingModelEntityOperator::vtkEnclosingModelEntityOperator()
{
  this->OperateSucceeded = 0;
  this->CellLocator = 0;
  this->EnclosingEntity = 0;
}

//----------------------------------------------------------------------------
vtkEnclosingModelEntityOperator::~vtkEnclosingModelEntityOperator()
{
  if (this->CellLocator)
    {
    this->CellLocator->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkEnclosingModelEntityOperator::BuildLinks(vtkDiscreteModelWrapper* modelWrapper)
{
  modelWrapper->GetModel()->GetMesh().BuildLinks();
}

//----------------------------------------------------------------------------
void vtkEnclosingModelEntityOperator::Operate(vtkDiscreteModelWrapper* modelWrapper,
                                              double *pt)
{
  vtkDebugMacro("Operating on a model.");
  this->OperateSucceeded = 0;

  const DiscreteMesh& mesh = modelWrapper->GetModel()->GetMesh();

  if (!this->CellLocator)
    {
    vtkSmartPointer<vtkPolyData> mainPD = mesh.ShallowCopyFaceData();
    this->CellLocator = vtkCellLocator::New();
    this->CellLocator->SetDataSet( mainPD );
    this->CellLocator->BuildLocator();
    }

  // reset, per chance we don't handle condition specific to the point and closest cell
  this->EnclosingEntity = 0;

  vtkIdType closestCellId;
  int subId;
  double dist2, closestPt[3];
  vtkSmartPointer<vtkGenericCell> cell = vtkSmartPointer<vtkGenericCell>::New();
  this->CellLocator->FindClosestPoint(pt, closestPt, cell, closestCellId, subId, dist2);

  // if closest isn't a poly, then start asking for cells within a bounds, and check polys
  // that are returned for which is closest?
//    virtual void FindCellsWithinBounds(double *bbox, vtkIdList *cells);

  if (cell->GetCellType() != VTK_TRIANGLE && cell->GetCellType() != VTK_POLYGON)
    {
    vtkErrorMacro("Check point location not supported if closest cell isn't a polygon!");
    return;
    }

  // determine whether we are on the positive side or negative side of the
  // polygon... note, need to handle cases where closest to edge of the polygon

  double normal[3];
  mesh.ComputeCellNormal(closestCellId,normal);


  vtkDiscreteModel::ClassificationType& classified =
      modelWrapper->GetModel()->GetMeshClassification();

  vtkModelEntity* entity = classified.GetEntity(closestCellId)->GetThisModelEntity();

  vtkDiscreteModelFace *faceEntity = vtkDiscreteModelFace::SafeDownCast(entity);

  // don't want failure due to bounds test
  vtkBoundingBox bbox(cell->GetBounds());
  bbox.AddPoint(pt);
  double bounds[6];
  bbox.GetBounds(bounds);

  double *cellPts =
    vtkDoubleArray::SafeDownCast(cell->GetPoints()->GetData())->GetPointer(0);
  int inside = vtkPolygon::PointInPolygon(pt, cell->GetNumberOfPoints(),
    cellPts, bounds, normal);

  // if "inside", can do simple normal test... more interesting if "outside", which
  // will happen plenty often
  if (inside != 1 )
    {
    // check the edges... which one are we closest to;  then check polys
    // that share that edge;  if dot of vector from centroid of poly1 ("cell")
    // to centroid of poly2 ("neighborCell") with normal vector of poly1 > 0,
    // then the point is "inside" region 0 (of face); else if < 0, then point
    // is outside region 0 (inside region 1) of the face
    int closestEdge = -1;
    double pCoords[3], weights[2], bestDist2 = -1,  edgeDist2;
    for (int i = 0; i < cell->GetNumberOfEdges(); i++)
      {
      vtkCell *edge = cell->GetEdge(i);
      if (edge->EvaluatePosition(pt, closestPt, subId, pCoords, edgeDist2, weights) == -1)
        {
        vtkErrorMacro("numerical error while anlysinz edge");
        continue;
        }
      if (closestEdge == -1 || edgeDist2 < bestDist2)
        {
        closestEdge = i;
        bestDist2 = edgeDist2;
        }
      }

    vtkIdType ptIds[2] = {cell->GetPointId(closestEdge),
      cell->GetPointId((closestEdge+1) % cell->GetNumberOfPoints())};

    vtkSmartPointer<vtkIdList> neighborIds  = vtkSmartPointer<vtkIdList>::New();
    mesh.GetCellEdgeNeighbors(closestCellId, ptIds[0], ptIds[1], neighborIds);

    if (neighborIds->GetNumberOfIds() == 1)
      {
      // centroid of neighber cell
      double neighborCentroid[3];
      mesh.ComputeCellCentroid(neighborIds->GetId(0), neighborCentroid);

      double centroid[3];
      mesh.ComputeCellCentroid(closestCellId, centroid);

      // vector from centroid of cell to centroid of neighborCell
      double centroidVector[3] = {neighborCentroid[0] - centroid[0],
        neighborCentroid[1] - centroid[1], neighborCentroid[2] - centroid[2]};

      if (vtkMath::Dot(centroidVector, normal) > 0)
        {
        this->EnclosingEntity = faceEntity->GetModelRegion(0);
        }
      else
        {
        this->EnclosingEntity = faceEntity->GetModelRegion(1);
        }
      this->OperateSucceeded = 1;
      return;
      }
    else
      {
      vtkErrorMacro("Condition not handled!");
      return;
      }
    }

  // if inside, check to see if "on" the polygon

  // check to see if outside or inside the region specified
  double testVector[3] = {pt[0] - cellPts[0], pt[1] - cellPts[1],
    pt[2] - cellPts[2]};

  if (vtkMath::Dot(testVector, normal) > 0)
    {
    this->EnclosingEntity = faceEntity->GetModelRegion(1);
    }
  else
    {
    this->EnclosingEntity = faceEntity->GetModelRegion(0);
    }

  vtkDebugMacro("Finished operating on a model.");
  this->OperateSucceeded = 1;
  return;
}

//----------------------------------------------------------------------------
void vtkEnclosingModelEntityOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
