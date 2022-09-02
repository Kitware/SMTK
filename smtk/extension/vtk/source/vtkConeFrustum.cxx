//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/vtkConeFrustum.h"

#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTransform.h"
#include "vtkVector.h"
#include "vtkVectorOperators.h"

#include <cmath>

vtkStandardNewMacro(vtkConeFrustum);

vtkConeFrustum::vtkConeFrustum(int res)
  : BottomPoint{ 0, 0, 0 }
  , TopPoint{ 0, 0, 1 }
  , Resolution(res <= 3 ? 3 : res)
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(NumberOfOutputs);
}

vtkConeFrustum::~vtkConeFrustum() = default;

void vtkConeFrustum::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "BottomPoint: (" << this->BottomPoint[0] << ", " << this->BottomPoint[1] << ", "
     << this->BottomPoint[2] << ")\n";
  os << indent << "BottomRadius: " << this->BottomRadius << "\n";
  os << indent << "TopPoint: (" << this->TopPoint[0] << ", " << this->TopPoint[1] << ", "
     << this->TopPoint[2] << ")\n";
  os << indent << "TopRadius: " << this->TopRadius << "\n";
  os << indent << "Resolution: " << this->Resolution << "\n";
  os << indent << "Output Points Precision: " << this->OutputPointsPrecision << "\n";
}

double vtkConeFrustum::GetAngle() const
{
  vtkVector3d p0(this->BottomPoint);
  vtkVector3d p1(this->TopPoint);
  double height = (p1 - p0).Norm();
  return vtkMath::DegreesFromRadians(atan2(fabs(this->TopRadius - this->BottomRadius), height));
}

int vtkConeFrustum::RequestData(
  vtkInformation* /*request*/,
  vtkInformationVector** /*inputVector*/,
  vtkInformationVector* outputVector)
{
  // vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* conical =
    vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::SideFace));
  vtkPolyData* botFace =
    vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::BottomFace));
  vtkPolyData* topFace = vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::TopFace));
  vtkPolyData* coneAxs = vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::Axis));
  vtkPolyData* botEdge =
    vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::BottomEdge));
  vtkPolyData* topEdge = vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::TopEdge));
  vtkPolyData* botVert =
    vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::BottomVertex));
  vtkPolyData* topVert =
    vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::TopVertex));
  if (!conical || !botFace || !topFace || !coneAxs || !botEdge || !topEdge || !botVert || !topVert)
  {
    vtkErrorMacro("No output provided.");
    return 0;
  }
  conical->Initialize();
  botFace->Initialize();
  topFace->Initialize();
  coneAxs->Initialize();
  botEdge->Initialize();
  topEdge->Initialize();
  botVert->Initialize();
  topVert->Initialize();

  vtkVector3d p0(this->BottomPoint);
  vtkVector3d p1(this->TopPoint);
  double length = (p1 - p0).Norm();
  bool p0Apex = this->BottomRadius <= 0.0;
  bool p1Apex = this->TopRadius <= 0.0;
  bool degenerate = (p0Apex && p1Apex) || (length <= 0.0);
  if (degenerate)
  {
    vtkErrorMacro(
      "Cone frustum requested is degenerate. Either the length ("
      << length
      << ") "
         "is not positive or both "
         "radii ("
      << this->BottomRadius << ", " << this->TopRadius << ") are zero.");
    return 0;
  }

  vtkDebugMacro("ConeSource Executing");

  // Get axes xx and yy in the plane normal to the cylinder axis
  vtkVector3d axis = (p1 - p0).Normalized();
  vtkVector3d px(1., 0., 0.);
  vtkVector3d py(0., 1., 0.);
  vtkVector3d yy = axis.Cross(px);
  if (yy.Norm() < 1e-10)
  {
    yy = axis.Cross(py);
  }
  yy.Normalize();
  vtkVector3d xx = yy.Cross(axis).Normalized();
  vtkVector3d ss(0., 0., (this->TopRadius - this->BottomRadius) / length);

  // I. Compute the point coordinates.
  //    Always put the bottom face's point(s) first, followed by the top.
  //    Note that each face may have either 1 or Resolution points, depending
  //    on whether it is an apex.
  vtkNew<vtkPoints> pts;
  pts->SetDataType(
    this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION ? VTK_DOUBLE : VTK_FLOAT);
  pts->Allocate(2 * this->Resolution);

  vtkNew<vtkPoints> botPts;
  botPts->SetDataType(
    this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION ? VTK_DOUBLE : VTK_FLOAT);
  botPts->Allocate(this->Resolution);

  vtkNew<vtkPoints> topPts;
  topPts->SetDataType(
    this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION ? VTK_DOUBLE : VTK_FLOAT);
  topPts->Allocate(this->Resolution);

  vtkNew<vtkPoints> axisPoints;
  axisPoints->SetDataType(
    this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION ? VTK_DOUBLE : VTK_FLOAT);
  axisPoints->Allocate(2);
  axisPoints->InsertNextPoint(p0.GetData());
  axisPoints->InsertNextPoint(p1.GetData());

  vtkNew<vtkPoints> botPoint;
  botPoint->SetDataType(
    this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION ? VTK_DOUBLE : VTK_FLOAT);
  botPoint->Allocate(1);
  botPoint->InsertNextPoint(p0.GetData());

  vtkNew<vtkPoints> topPoint;
  topPoint->SetDataType(
    this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION ? VTK_DOUBLE : VTK_FLOAT);
  topPoint->Allocate(1);
  topPoint->InsertNextPoint(p1.GetData());

  vtkNew<vtkDoubleArray> nrm; // point normals
  nrm->SetNumberOfComponents(3);
  nrm->SetName("normals");
  nrm->Allocate(2 * this->Resolution);

  vtkNew<vtkDoubleArray> botNrm; // point normals
  botNrm->SetName("normals");
  botNrm->SetNumberOfComponents(3);
  botNrm->Allocate(this->Resolution + 1);

  vtkNew<vtkDoubleArray> topNrm; // point normals
  topNrm->SetName("normals");
  topNrm->SetNumberOfComponents(3);
  topNrm->Allocate(this->Resolution + 1);

  double angle = 2.0 * vtkMath::Pi() / this->Resolution;

  // Bottom point
  vtkIdType bottomCenter = botPts->InsertNextPoint(p0.GetData());
  // Bottom-facing points
  vtkVector3d bottomNormal = -axis;
  botNrm->InsertNextTuple(bottomNormal.GetData());
  for (int ii = 0; ii < this->Resolution; ++ii)
  {
    double theta = angle * ii;
    vtkVector3d pt = p0 + this->BottomRadius * (xx * cos(theta) + yy * sin(theta));
    botPts->InsertNextPoint(pt.GetData());
    botNrm->InsertNextTuple(bottomNormal.GetData());
  }

  // Bottom side points
  for (int ii = 0; ii < this->Resolution; ++ii)
  {
    double theta = angle * ii;
    vtkVector3d nn = (xx * cos(theta) + yy * sin(theta));
    vtkVector3d pt = p0 + this->BottomRadius * nn;
    pts->InsertNextPoint(pt.GetData());
    nrm->InsertNextTuple((nn - ss).Normalized().GetData());
  }

  // Top side points
  for (int ii = 0; ii < this->Resolution; ++ii)
  {
    double theta = angle * ii;
    vtkVector3d nn = (xx * cos(theta) + yy * sin(theta));
    vtkVector3d pt = p1 + this->TopRadius * nn;
    pts->InsertNextPoint(pt.GetData());
    nrm->InsertNextTuple((nn - ss).Normalized().GetData());
  }

  // Top-facing points
  vtkVector3d topNormal = axis;
  topNrm->InsertNextTuple(topNormal.GetData());
  for (int ii = 0; ii < this->Resolution; ++ii)
  {
    double theta = angle * ii;
    vtkVector3d pt = p1 + this->TopRadius * (xx * cos(theta) + yy * sin(theta));
    topPts->InsertNextPoint(pt.GetData());
    topNrm->InsertNextTuple(topNormal.GetData());
  }

  // Top point
  vtkIdType topCenter = topPts->InsertNextPoint(p1.GetData());

  // II. Prepare face connectivity
  vtkIdType numConnSide = (4 + 1) * this->Resolution;
  vtkIdType numConnCap = (3 + 1) * this->Resolution;

  vtkNew<vtkCellArray> polys;
  vtkNew<vtkCellArray> botPolys;
  vtkNew<vtkCellArray> topPolys;
  vtkNew<vtkCellArray> axsLines;
  vtkNew<vtkCellArray> botLines;
  vtkNew<vtkCellArray> topLines;
  vtkNew<vtkCellArray> botVertices;
  vtkNew<vtkCellArray> topVertices;
  vtkNew<vtkIdTypeArray> sideConn;
  vtkNew<vtkIdTypeArray> botConn;
  vtkNew<vtkIdTypeArray> topConn;
  sideConn->Allocate(numConnSide);
  botConn->Allocate(numConnCap);
  topConn->Allocate(numConnCap);
  axsLines->Allocate(3);
  botLines->Allocate(this->Resolution + 2);
  topLines->Allocate(this->Resolution + 2);

  // Side-face connectivity (quads).
  vtkIdType start0 = 0;
  vtkIdType start1 = this->Resolution;
  for (int ii = 0; ii < this->Resolution; ++ii)
  {
    sideConn->InsertNextValue(4);
    sideConn->InsertNextValue(start0 + ii);
    sideConn->InsertNextValue(start0 + (ii + 1) % this->Resolution);
    sideConn->InsertNextValue(start1 + (ii + 1) % this->Resolution);
    sideConn->InsertNextValue(start1 + ii);
  }

  // Generate endcap connectivity.
  // Insert bottom face triangles.
  vtkIdType circleStart = bottomCenter + 1;
  for (int ii = 0; ii < this->Resolution; ++ii)
  {
    botConn->InsertNextValue(3);
    botConn->InsertNextValue(circleStart + ii);
    botConn->InsertNextValue(bottomCenter);
    botConn->InsertNextValue(circleStart + (ii + 1) % this->Resolution);
  }

  // Insert top face triangles.
  circleStart = topCenter - this->Resolution;
  for (int ii = 0; ii < this->Resolution; ++ii)
  {
    topConn->InsertNextValue(3);
    topConn->InsertNextValue(circleStart + (ii + 1) % this->Resolution);
    topConn->InsertNextValue(topCenter);
    topConn->InsertNextValue(circleStart + ii);
  }

  // Connectivity for bottom and top edges
  vtkNew<vtkIdTypeArray> botEdgeConn;
  vtkNew<vtkIdTypeArray> topEdgeConn;
  vtkNew<vtkIdTypeArray> axsEdgeConn;
  botEdgeConn->SetNumberOfValues(this->Resolution + 2);
  topEdgeConn->SetNumberOfValues(this->Resolution + 2);
  axsEdgeConn->SetNumberOfValues(3);
  botEdgeConn->SetValue(0, this->Resolution + 1);
  topEdgeConn->SetValue(0, this->Resolution + 1);
  axsEdgeConn->SetValue(0, 2);
  axsEdgeConn->SetValue(1, 0);
  axsEdgeConn->SetValue(2, 1);
  for (int ii = 0; ii <= this->Resolution; ++ii)
  {
    botEdgeConn->SetValue(ii + 1, 1 + (ii % this->Resolution));
    topEdgeConn->SetValue(ii + 1, ii % this->Resolution);
  }
  botLines->SetCells(1, botEdgeConn);
  topLines->SetCells(1, topEdgeConn);
  axsLines->SetCells(1, axsEdgeConn);

  // Update the output
  polys->SetCells(this->Resolution, sideConn);
  conical->SetPoints(pts);
  conical->SetPolys(polys);
  conical->GetPointData()->SetNormals(nrm);

  botPolys->SetCells(this->Resolution, botConn);
  botFace->SetPoints(botPts);
  botFace->SetPolys(botPolys);
  botFace->GetPointData()->SetNormals(botNrm);

  topPolys->SetCells(this->Resolution, topConn);
  topFace->SetPoints(topPts);
  topFace->SetPolys(topPolys);
  topFace->GetPointData()->SetNormals(topNrm);

  botEdge->SetPoints(botPts);
  botEdge->SetLines(botLines);

  topEdge->SetPoints(topPts);
  topEdge->SetLines(topLines);

  coneAxs->SetPoints(axisPoints);
  coneAxs->SetLines(axsLines);

  vtkIdType vertId = 0;
  botVert->SetPoints(botPoint);
  botVert->SetVerts(botVertices);
  botVert->InsertNextCell(VTK_VERTEX, 1, &vertId);

  topVert->SetPoints(topPoint);
  topVert->SetVerts(topVertices);
  topVert->InsertNextCell(VTK_VERTEX, 1, &vertId);

  return 1;
}
