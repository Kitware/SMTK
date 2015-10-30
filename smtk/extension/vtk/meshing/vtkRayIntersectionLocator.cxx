//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkRayIntersectionLocator.h"

#include <cstdlib>
#include <cassert>
#include <cmath>
#include <stack>
#include <vector>
#include <limits>
#include <algorithm>
#include "vtkObjectFactory.h"
#include "vtkGenericCell.h"
#include "vtkIdListCollection.h"
#include "vtkSmartPointer.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkBoundingBox.h"
#include "vtkPointData.h"
#include "vtkVectorOperators.h"

namespace smtk {
  namespace vtk {

typedef std::stack<vtkRayIntersectionLocator::vtkCellTreeNode*, std::vector<vtkRayIntersectionLocator::vtkCellTreeNode*> > nodeStack;

vtkStandardNewMacro(vtkRayIntersectionLocator);

vtkRayIntersectionLocator::vtkRayIntersectionLocator( )
{
}

vtkRayIntersectionLocator::~vtkRayIntersectionLocator()
{
}

void vtkRayIntersectionLocator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//int vtkRayIntersectionLocator::IntersectWithLine(double p1[3], double p2[3], double tol,
//  double& t, double x[3], double pcoords[3],
//  int &subId, vtkIdType &cellIds)
int vtkRayIntersectionLocator::AllIntersectionsAlongSegment(
  const vtkVector3d& p1,
  const vtkVector3d& p2,
  std::vector<vtkVector3d>& points,
  std::vector<double>& params,
  std::vector<vtkVector3d>& pcoords,
  std::vector<vtkIdType>& cellIds,
  std::vector<int>& subIds)
{
  vtkCellTreeNode* node;
  vtkCellTreeNode* near;
  vtkCellTreeNode* far;
  double    ctmin, ctmax, tmin, tmax, _tmin, _tmax, tDist;
  vtkVector3d ray_vec = p2 - p1;

  double cellBounds[6];

  this->BuildLocatorIfNeeded();

  // Does ray pass through root BBox
  tmin = 0; tmax = 1;

  if (!this->RayMinMaxT(p1.GetData(), ray_vec.GetData(), tmin, tmax)) // 0 for root node
    {
    return false;
    }
  // Ok, setup a stack and various params
  nodeStack  ns;

  //
  // OK, lets walk the tree and find intersections
  //
  vtkCellTreeNode* n = &this->Tree->Nodes.front();
  ns.push(n);
  while (!ns.empty())
    {
    node = ns.top();
    ns.pop();

    int mustCheck = 0;  // to check if both left and right sub trees need to be checked

    while (!node->IsLeaf())
      { // this must be a parent node
      // Which child node is closest to ray origin - given direction
      this->Classify(p1.GetData(), ray_vec.GetData(), tDist, near, node, far, mustCheck);
      // if the distance to the far edge of the near box is > tmax, no need to test far box
      // (we still need to test Mid because it may overlap slightly)
      if(mustCheck)
        {
        ns.push(far);
        node = near;
        }
      else if ((tDist > tmax) || (tDist <= 0) )
        { // <=0 for ray on edge
        node = near;
        }
      // if the distance to the far edge of the near box is < tmin, no need to test near box
      else if (tDist < tmin)
        {
        ns.push(near);
        node = far;
        }
      // All the child nodes may be candidates, keep near, push far then mid
      else
        {
        ns.push(far);
        node = near;
        }
      }
    // Ok, so we're a leaf node, first check the BBox against the ray
    // then test the candidates in our sorted ray direction order
    _tmin = tmin; _tmax = tmax;
    for (int i=0; i< static_cast<int>(node->Size()); i++)
      {
      vtkIdType cell_ID = this->Tree->Leaves[node->Start()+i];
      //

      double* boundsPtr = cellBounds;
      if (this->CellBounds)
        {
        boundsPtr = this->CellBounds[cell_ID];
        }
      else
        {
        this->DataSet->GetCellBounds(cell_ID, cellBounds);
        }
      //
      ctmin = _tmin; ctmax = _tmax;
      if (this->RayMinMaxT(boundsPtr, p1.GetData(), ray_vec.GetData(), ctmin, ctmax))
        {
        double tol = 0.;
        double t_hit;
        vtkVector3d ipt;
        vtkVector3d pcoord;
        int subId;
        if (
          this->IntersectCellInternal(
            cell_ID, p1.GetData(), p2.GetData(), tol,
            t_hit, ipt.GetData(), pcoord.GetData(), subId))
          {
          params.push_back(t_hit);
          pcoords.push_back(pcoord);
          cellIds.push_back(cell_ID);
          subIds.push_back(subId);
          points.push_back(ipt);
          }
        }
      }
    }

  return cellIds.size();
}

int vtkRayIntersectionLocator::IntersectWithLine(
  double p1[3], double p2[3],
  double tol, double &t,
  double x[3], double pcoords[3],
  int &subId, vtkIdType &cellId, vtkGenericCell *cell)
{
  int hit = this->IntersectWithLine(p1, p2, tol, t, x, pcoords, subId, cellId);
  if (hit)
    {
    this->DataSet->GetCell(cellId, cell);
    }
  return hit;
}
  } // namespace vtk
} // namespace smtk
