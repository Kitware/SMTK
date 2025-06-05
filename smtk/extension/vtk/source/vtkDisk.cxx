//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/vtkDisk.h"

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

vtkStandardNewMacro(vtkDisk);

vtkDisk::vtkDisk(int res)
  : Resolution(res <= 3 ? 3 : res)
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(NumberOfOutputs);
}

vtkDisk::~vtkDisk() = default;

void vtkDisk::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "CenterPoint: (" << this->CenterPoint[0] << ", " << this->CenterPoint[1] << ", "
     << this->CenterPoint[2] << ")\n";
  os << indent << "Radius: " << this->Radius << "\n";
  os << indent << "Normal: (" << this->Normal[0] << ", " << this->Normal[1] << ", "
     << this->Normal[2] << ")\n";
  os << indent << "Resolution: " << this->Resolution << "\n";
  os << indent << "Output Points Precision: " << this->OutputPointsPrecision << "\n";
}

int vtkDisk::RequestData(
  vtkInformation* /*request*/,
  vtkInformationVector** /*inputVector*/,
  vtkInformationVector* outputVector)
{
  // vtkInformation* outInfo = outputVector->GetInformationObject(0);
  // clang-format off
  vtkPolyData* diskFace = vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::DiskFace));
  vtkPolyData* normEdge = vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::DiskNormal));
  vtkPolyData* diskEdge = vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::DiskEdge));
  vtkPolyData* ctrVert = vtkPolyData::GetData(outputVector, static_cast<int>(OutputPorts::CenterVertex));
  if (!diskFace || !normEdge || !diskEdge || !ctrVert)
  {
    vtkErrorMacro("No output provided.");
    return 0;
  }
  diskFace->Initialize();
  normEdge->Initialize();
  diskEdge->Initialize();
  ctrVert->Initialize();

  vtkVector3d p0(this->CenterPoint);
  vtkVector3d nn(this->Normal);

  vtkDebugMacro("DiskSource Executing");

  // Get axes xx and yy in the plane normal to the disk axis
  vtkVector3d axis = nn.Normalized();
  vtkVector3d p1 = p0 + axis * this->Radius;
  vtkVector3d px(1., 0., 0.);
  vtkVector3d py(0., 1., 0.);
  vtkVector3d yy = axis.Cross(px);
  if (yy.Norm() < 1e-10)
  {
    yy = axis.Cross(py);
  }
  yy.Normalize();
  vtkVector3d xx = yy.Cross(axis).Normalized();

  // I. Compute the point coordinates.
  vtkNew<vtkPoints> facePts;
  facePts->SetDataType(
    this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION ? VTK_DOUBLE : VTK_FLOAT);
  facePts->SetNumberOfPoints(1 + this->Resolution);
  facePts->SetPoint(this->Resolution, p0.GetData());

  vtkNew<vtkPoints> normPts;
  normPts->SetDataType(
    this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION ? VTK_DOUBLE : VTK_FLOAT);
  normPts->Allocate(2);
  normPts->InsertNextPoint(p0.GetData());
  normPts->InsertNextPoint(p1.GetData());

  vtkNew<vtkPoints> edgePts;
  edgePts->SetDataType(
    this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION ? VTK_DOUBLE : VTK_FLOAT);
  edgePts->Allocate(this->Resolution);

  vtkNew<vtkPoints> centerPoint;
  centerPoint->SetDataType(
    this->OutputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION ? VTK_DOUBLE : VTK_FLOAT);
  centerPoint->Allocate(1);
  centerPoint->InsertNextPoint(p0.GetData());

  vtkNew<vtkDoubleArray> faceNrm; // point normals
  faceNrm->SetNumberOfComponents(3);
  faceNrm->SetName("normals");
  faceNrm->SetNumberOfTuples(facePts->GetNumberOfPoints());
  faceNrm->FillComponent(0, nn[0]);
  faceNrm->FillComponent(1, nn[1]);
  faceNrm->FillComponent(2, nn[2]);

  double angle = 2.0 * vtkMath::Pi() / this->Resolution;

  // Disk face and edge points
  for (int ii = 0; ii < this->Resolution; ++ii)
  {
    double theta = angle * ii;
    vtkVector3d pt = p0 + this->Radius * (xx * cos(theta) + yy * sin(theta));
    facePts->SetPoint(ii, pt.GetData());
    edgePts->InsertNextPoint(pt.GetData());
  }

  // II. Prepare face connectivity
  vtkIdType numConnEdge = 2 + this->Resolution;
  vtkIdType numConnFace = (3 + 1) * this->Resolution;

  vtkNew<vtkCellArray> facePolys;
  vtkNew<vtkCellArray> edgeLines;
  vtkNew<vtkCellArray> normLines;
  vtkNew<vtkCellArray> centerVertex;
  vtkNew<vtkIdTypeArray> faceConn;
  vtkNew<vtkIdTypeArray> edgeConn;
  vtkNew<vtkIdTypeArray> normConn;
  faceConn->Allocate(numConnFace);
  edgeConn->Allocate(numConnEdge);
  normConn->SetNumberOfTuples(3);
  normLines->Allocate(3);
  vtkIdType zero = 0;
  centerVertex->InsertNextCell(1, &zero);

  // Disk face connectivity (triangles).
  // The last point is the center of the disk.
  edgeConn->InsertNextValue(this->Resolution);
  for (int ii = 0; ii < this->Resolution; ++ii)
  {
    edgeConn->InsertNextValue(ii);
    faceConn->InsertNextValue(3);
    faceConn->InsertNextValue(this->Resolution);
    faceConn->InsertNextValue(ii);
    faceConn->InsertNextValue((ii + 1) % this->Resolution);
  }
  normConn->SetValue(0, 2);
  normConn->SetValue(1, 0);
  normConn->SetValue(2, 1);

  facePolys->SetCells(this->Resolution, faceConn);
  edgeLines->SetCells(1, edgeConn);
  normLines->SetCells(1, normConn);

  diskFace->SetPoints(facePts);
  diskFace->SetPolys(facePolys);
  diskFace->GetPointData()->SetNormals(faceNrm);
  normEdge->SetPoints(normPts);
  normEdge->SetLines(normLines);
  diskEdge->SetPoints(edgePts);
  diskEdge->SetLines(edgeLines);
  ctrVert->SetPoints(centerPoint);
  ctrVert->SetVerts(centerVertex);

  return 1;
}
