//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/filter/vtkPolyDataStatsFilter.h"
#include "vtkBoundingBox.h"
#include "vtkCellArray.h"
#include "vtkErrorCode.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkTransform.h"
#include "vtkTriangle.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkPolyDataStatsFilter);

vtkPolyDataStatsFilter::vtkPolyDataStatsFilter()
{
  this->Translation[0] = this->Translation[1] = this->Translation[2] = 0.0;
  this->Orientation[0] = this->Orientation[1] = this->Orientation[2] = 0.0;
  this->Scale[0] = this->Scale[1] = this->Scale[2] = 1.0;
  this->Transform = vtkTransform::New();
  this->SetNumberOfInputPorts(1);
  this->AreaStats[0] = this->AreaStats[1] = this->AreaStats[2] = 0.0;
  this->GeometryBounds[0] = this->GeometryBounds[1] = this->GeometryBounds[2] =
    this->GeometryBounds[3] = this->GeometryBounds[4] = this->GeometryBounds[5] = 0.0;
  this->PolygonalSideStats[0] = this->PolygonalSideStats[1] = this->PolygonalSideStats[2] = 0.0;
  this->TotalSurfaceArea = 0.0;
  this->NumberOfPolygons = this->NumberOfLines = this->NumberOfPoints = 0;
}

vtkPolyDataStatsFilter::~vtkPolyDataStatsFilter()
{
  this->Transform->Delete();
}

int vtkPolyDataStatsFilter::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* vtkNotUsed(outputVector))
{
  //Reset Information before recomputing
  this->AreaStats[0] = this->AreaStats[1] = this->AreaStats[2] = 0.0;
  this->GeometryBounds[0] = this->GeometryBounds[1] = this->GeometryBounds[2] =
    this->GeometryBounds[3] = this->GeometryBounds[4] = this->GeometryBounds[5] = 0.0;
  this->PolygonalSideStats[0] = this->PolygonalSideStats[1] = this->PolygonalSideStats[2] = 0.0;
  this->TotalSurfaceArea = 0.0;
  this->NumberOfPolygons = this->NumberOfLines = this->NumberOfPoints = 0;

  // get the info and input data
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkPolyData* input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  this->Transform->Identity();
  this->Transform->PreMultiply();
  this->Transform->Translate(this->Translation);
  this->Transform->RotateZ(this->Orientation[2]);
  this->Transform->RotateX(this->Orientation[0]);
  this->Transform->RotateY(this->Orientation[1]);
  this->Transform->Scale(this->Scale[0], this->Scale[1], this->Scale[2]);

  this->BuildTime.Modified();

  double np[3], area, *pnt;
  vtkIdType i, j, n, *pntIds;
  vtkBoundingBox bbox;
  vtkPoints* tPnts = vtkPoints::New();
  this->NumberOfPoints = input->GetNumberOfPoints();
  tPnts->SetNumberOfPoints(this->NumberOfPoints);
  this->TotalSurfaceArea = 0.0;
  // Transform all of the points
  for (i = 0; i < this->NumberOfPoints; i++)
  {
    this->Transform->TransformPoint(input->GetPoint(i), np);
    tPnts->SetPoint(i, np);
    bbox.AddPoint(np);
  }

  bbox.GetBounds(this->GeometryBounds);

  // Calculate the polygon stats
  this->NumberOfLines = input->GetNumberOfLines();
  vtkCellArray* ca = input->GetPolys();
  this->NumberOfPolygons = ca->GetNumberOfCells();
  double p1[3], p2[3], p3[3];
  ca->InitTraversal();
  for (i = 0; i < this->NumberOfPolygons; i++)
  {
    ca->GetNextCell(n, pntIds);
    if (n == 3)
    {
      tPnts->GetPoint(pntIds[0], p1);
      tPnts->GetPoint(pntIds[1], p2);
      tPnts->GetPoint(pntIds[2], p3);
      area = vtkTriangle::TriangleArea(p1, p2, p3);
    }
    else
    {
      // Assume the polygon is at least point convex w/e to its centroid
      np[0] = np[1] = np[2] = 0.0;
      for (j = 0; j < n; j++)
      {
        pnt = tPnts->GetPoint(pntIds[j]);
        np[0] += pnt[0];
        np[1] += pnt[1];
        np[2] += pnt[2];
      }
      np[0] /= static_cast<double>(n);
      np[1] /= static_cast<double>(n);
      np[2] /= static_cast<double>(n);

      area = vtkTriangle::TriangleArea(np, tPnts->GetPoint(pntIds[0]), tPnts->GetPoint(pntIds[1]));
      for (j = 1; j < n; j++)
      {
        area +=
          vtkTriangle::TriangleArea(np, tPnts->GetPoint(pntIds[j]), tPnts->GetPoint(pntIds[j - 1]));
      }
    }

    this->TotalSurfaceArea += area;

    if (!i)
    {
      this->AreaStats[0] = this->AreaStats[2] = area;
      this->PolygonalSideStats[0] = this->PolygonalSideStats[1] = this->PolygonalSideStats[2] = n;
    }
    else
    {
      if (this->AreaStats[0] > area)
      {
        this->AreaStats[0] = area;
      }
      if (this->AreaStats[2] < area)
      {
        this->AreaStats[2] = area;
      }

      if (this->PolygonalSideStats[0] > n)
      {
        this->PolygonalSideStats[0] = n;
      }
      if (this->PolygonalSideStats[2] < n)
      {
        this->PolygonalSideStats[2] = n;
      }
      this->PolygonalSideStats[1] += static_cast<double>(n);
    }
  }
  if (this->NumberOfPolygons)
  {
    this->AreaStats[1] = this->TotalSurfaceArea / static_cast<double>(this->NumberOfPolygons);
    this->PolygonalSideStats[1] /= static_cast<double>(this->NumberOfPolygons);
  }

  tPnts->Delete();
  return VTK_OK;
}

void vtkPolyDataStatsFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Number of Points: " << this->NumberOfPoints << "\n";
  os << indent << "Number of Lines: " << this->NumberOfLines << "\n";
  os << indent << "Number of Polygons: " << this->NumberOfPolygons << "\n";
  os << indent << "Total Surface Area: " << this->TotalSurfaceArea << "\n";
  os << indent << "Polygonal Side Stats (min, ave, max: (" << this->PolygonalSideStats[0] << ", "
     << this->PolygonalSideStats[1] << ", " << this->PolygonalSideStats[2] << ")\n";
  os << indent << "Area Stats (min, ave, max: (" << this->AreaStats[0] << ", " << this->AreaStats[1]
     << ", " << this->AreaStats[2] << ")\n";
  os << indent << "Geometric Bounds: (" << this->GeometryBounds[0] << ", "
     << this->GeometryBounds[1] << ", " << this->GeometryBounds[2] << this->GeometryBounds[3]
     << ", " << this->GeometryBounds[4] << ", " << this->GeometryBounds[5] << ")\n";
  os << indent << "Translation: (" << this->Translation[0] << ", " << this->Translation[1] << ", "
     << this->Translation[2] << ")\n";
  os << indent << "Orientation: (" << this->Orientation[0] << ", " << this->Orientation[1] << ", "
     << this->Orientation[2] << ")\n";
  os << indent << "Scale: (" << this->Scale[0] << ", " << this->Scale[1] << ", " << this->Scale[2]
     << ")\n";
}
